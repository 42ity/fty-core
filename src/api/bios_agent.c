#include <zmq.h>
#include <czmq.h>
#include <zhash.h>
#include <errno.h>

#include "bios_agent.h"
#include "utils_ymsg.h"
#include "defs.h"
#include "cleanup.h"

#define TIMEOUT 1000
#define BIOS_MLM_STREAM "bios"
#define BIOS_MLM_MEASUREMENTS_STREAM "measurements"
#define BIOS_MLM_ASSETS_STREAM "assets"
#define BIOS_MLM_ALERTS_STREAM "alerts"
#define BIOS_MLM_STREAM_COUNT 4

static const char *bios_streams[] = 
{
    BIOS_MLM_STREAM,
    BIOS_MLM_MEASUREMENTS_STREAM,
    BIOS_MLM_ASSETS_STREAM,
    BIOS_MLM_ALERTS_STREAM,
    NULL
};

struct _bios_agent_t {
    mlm_client_t *client;   // malamute client instance
    void* seq;              // message sequence number
};

bios_agent_t*
bios_agent_new (const char* endpoint, const char* address) {
    if (!endpoint || !address) {
        return NULL;
    }

    bios_agent_t *self = (bios_agent_t *) zmalloc (sizeof (bios_agent_t));
    if (self) {
        self->client = mlm_client_new();
        if (!self->client) {
            free (self);
            return NULL;
        }
        if(mlm_client_connect(self->client, endpoint, TIMEOUT, address) != 0) {
            mlm_client_destroy(&self->client);
            free (self);
            return NULL;
        }
        self->seq = zmq_atomic_counter_new (); // create new && set value to zero
        if (!self->seq) {
            mlm_client_destroy (&self->client);
            free (self);
            return NULL;
        }
        zmq_atomic_counter_set (self->seq, 0);
    }
    return self;
}

void
bios_agent_destroy (bios_agent_t **self_p) {
    if (self_p == NULL) {
        return;
    }
    if (*self_p) {
        bios_agent_t *self = *self_p;

        //  Free class properties
        mlm_client_destroy (&self->client);
        zmq_atomic_counter_destroy (&self->seq);

        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}

int
bios_agent_send (bios_agent_t *self, const char *subject, ymsg_t **msg_p) {
    if (!self || !subject || !msg_p || !(*msg_p)) {
        return -2;
    }
    // Note: zmq_atomic_counter_inc() returns current value and increments by one
    ymsg_set_seq (*msg_p, zmq_atomic_counter_inc (self->seq));
    _scoped_zmsg_t *zmsg = ymsg_encode (msg_p);
    if (!zmsg) {
        return -1;
    }
    int rc = mlm_client_send (self->client, subject, &zmsg);
    return rc;
}

int
bios_agent_sendto (bios_agent_t *self, const char *address, const char *subject, ymsg_t **send_p) {
    if (!self || !address || !subject || !send_p || !(*send_p) || ymsg_id (*send_p) != YMSG_SEND) {
        return -2;
    }
    ymsg_set_seq (*send_p, zmq_atomic_counter_inc (self->seq)); // return value && increment by one
    _scoped_zmsg_t *zmsg = ymsg_encode (send_p);
    if (!zmsg) {
        return -1;
    }
    int rc = mlm_client_sendto (self->client, address, subject, NULL, TIMEOUT, &zmsg);
    return rc;
}

int
bios_agent_replyto (bios_agent_t *self, const char *address, const char *subject, ymsg_t **reply_p, ymsg_t *send) {
    if (!self || !address || !subject || !reply_p || !(*reply_p) || !send || ymsg_id (*reply_p) != YMSG_REPLY || ymsg_id (send) != YMSG_SEND ) {
        return -2;
    }
    ymsg_set_seq (*reply_p, zmq_atomic_counter_inc (self->seq)); // return value && increment by one
    ymsg_set_rep (*reply_p, ymsg_seq (send));

    if (ymsg_is_repeat (send)) { // default is not to repeat
        _scoped_zchunk_t *chunk = ymsg_get_request (send);
        ymsg_set_request (*reply_p, &chunk);
    }
    _scoped_zmsg_t *zmsg = ymsg_encode (reply_p);
    if (!zmsg) {
        return -1;
    }
    int rc = mlm_client_sendto (self->client, address, subject, NULL, TIMEOUT, &zmsg);
    return rc;
}

int
bios_agent_sendfor (bios_agent_t *self, const char *address, const char *subject, ymsg_t **send_p) {
    if (!self || !address || !subject || !send_p || !(*send_p) || ymsg_id (*send_p) != YMSG_SEND) {
        return -2;
    }
    ymsg_set_seq (*send_p, zmq_atomic_counter_inc (self->seq)); // return value && increment by one
    _scoped_zmsg_t *zmsg = ymsg_encode (send_p);
    if (!zmsg) {
        return -1;
    }
    int rc = mlm_client_sendfor (self->client, address, subject, NULL, TIMEOUT, &zmsg);
    return rc;
}

int
bios_agent_set_producer (bios_agent_t *self, const char *stream)
{
    if (!self || !stream) {
        return -2;
    }
    return mlm_client_set_producer (self->client, stream);
}

int
bios_agent_set_consumer (bios_agent_t *self, const char *stream, const char *pattern)
{
    if (!self || !stream || !pattern) {
        return -2;
    }
    return mlm_client_set_consumer (self->client, stream, pattern);
}

ymsg_t *
bios_agent_recv (bios_agent_t *self) {
    if (!self) {
        return NULL;
    }
    _scoped_zmsg_t *zmsg = mlm_client_recv (self->client);
    if (!zmsg) {
        return NULL;
    }
    ymsg_t *ymsg = ymsg_decode (&zmsg);
    return ymsg;
}

ymsg_t *
bios_agent_recv_wait(bios_agent_t *self, int timeout) {

    if(!self) {
        return NULL;
    }

    zsock_t *pipe = bios_agent_msgpipe(self);
    if (!pipe) {
        return NULL;
    }

    _scoped_zmsg_t *zmsg = NULL;
    zsock_t *which = NULL;
    _scoped_zpoller_t *poller = zpoller_new(pipe, NULL);
    if (!poller) {
        return NULL;
    }

    which = (zsock_t *) zpoller_wait (poller, timeout);
    if(which) {
        zmsg = mlm_client_recv (self->client);
    }
    zpoller_destroy (&poller);

    if (!zmsg) {
        return NULL;
    }
    ymsg_t *ymsg = ymsg_decode(&zmsg);
    return ymsg;
}


const char *
bios_agent_command (bios_agent_t *self) {
    if (!self) {
        return NULL;
    }
    return mlm_client_command (self->client);
}

int
bios_agent_status (bios_agent_t *self) {
    if (!self) {
        return -2;
    }
    return mlm_client_status (self->client);
}

const char *
bios_agent_reason (bios_agent_t *self) {
    if (!self) {
        return NULL;
    }
    return mlm_client_reason (self->client);
}

const char *
bios_agent_address (bios_agent_t *self) {
    if (!self) {
        return NULL;
    }
    return mlm_client_address (self->client);
}

const char *
bios_agent_sender (bios_agent_t *self) {
    if (!self) {
        return NULL;
    }
    return mlm_client_sender (self->client);
}

const char *
bios_agent_subject (bios_agent_t *self) {
    if (!self) {
        return NULL;
    }
    return mlm_client_subject (self->client);
}

ymsg_t *
bios_agent_content (bios_agent_t *self) {
    if (!self) {
        return NULL;
    }
    _scoped_zmsg_t *zmsg = mlm_client_content (self->client);
    if (!zmsg) {
        return NULL;
    }
    return ymsg_decode (&zmsg);
}

zactor_t *
bios_agent_actor (bios_agent_t *self) {
    if (!self)
        return NULL;
    return mlm_client_actor (self->client);
}

zsock_t *
bios_agent_msgpipe (bios_agent_t *self) {
    if (!self)
        return NULL;
    return mlm_client_msgpipe (self->client);
}

const char *
bios_get_stream_main () {
    return BIOS_MLM_STREAM;
}

const char *
bios_get_stream_measurements () {
    return BIOS_MLM_MEASUREMENTS_STREAM;
}

const char *
bios_get_stream_assets () {
    return BIOS_MLM_ASSETS_STREAM;
}

const char *
bios_get_stream_alerts () {
    return BIOS_MLM_ALERTS_STREAM;
}

const char **
bios_get_streams (uint8_t *count) {
    if (!count) {
        return NULL;
    } 
    *count = BIOS_MLM_STREAM_COUNT;
    return bios_streams;
}

uint16_t
bios_agent_seq (bios_agent_t *self) {
    return (uint16_t) zmq_atomic_counter_value (self->seq);
}

bool
ymsg_is_ok (ymsg_t *self) {
    if (!self) {
        return false;
    }
    return streq (ymsg_aux_string (self, KEY_STATUS, ""), OK);
}

void
ymsg_set_status (ymsg_t *self, bool status) {
    if (!self) {
        return;
    }
    ymsg_aux_insert (self, KEY_STATUS, "%s", status ? OK : ERROR);
}

bool
ymsg_is_repeat (ymsg_t *self) {
    if (!self) {
        return false;
    }
    return streq (ymsg_aux_string (self, KEY_REPEAT, ""), YES);
}

void
ymsg_set_repeat (ymsg_t *self, bool repeat) {
    if (!self) {
        return;
    }
    ymsg_aux_insert (self, KEY_REPEAT, "%s", repeat ? YES : NO);
}

const char *
ymsg_content_type (ymsg_t *self) {
    if (!self) {
        return NULL;
    }
    return ymsg_aux_string (self, KEY_CONTENT_TYPE, NULL);
}

void
ymsg_set_content_type (ymsg_t *self, const char *content_type) {
    if (!self || !content_type) {
        return;
    }
    ymsg_aux_insert (self, KEY_CONTENT_TYPE, "%s", content_type);
}

void
set_hash(ymsg_t *msg, const void *key, void *value) {
    _scoped_zhash_t *hash = ymsg_get_aux(msg);
    if(hash == NULL) {
        hash = zhash_new();
        zhash_autofree(hash);
    }
    zhash_update(hash, key, value);
    ymsg_set_aux(msg, &hash);
}    

const char *
ymsg_get_string(ymsg_t* msg, const char *key) {
    static char nullchar[1] = { '\0' };
    char *val = zhash_lookup(ymsg_aux(msg), key);
    if(val == NULL) {
        errno = EKEYREJECTED;
        return nullchar;
    }
    return val;
}

int32_t
ymsg_get_int32(ymsg_t* msg, const char *key) {
    int32_t ret;
    char *val = zhash_lookup(ymsg_aux(msg), key);
    if(val == NULL) {
        errno = EKEYREJECTED;
        return 0;
    }
    if(sscanf(val, "%" SCNi32, &ret) != 1) {
        errno = EBADMSG;
        return 0;
    }
    return ret;
}

int64_t
ymsg_get_int64(ymsg_t* msg, const char *key) {
    int64_t ret;
    char *val = zhash_lookup(ymsg_aux(msg), key);
    if(val == NULL) {
        errno = EKEYREJECTED;
        return 0;
    }
    if(sscanf(val, "%" SCNi64, &ret) != 1) {
        errno = EBADMSG;
        return 0;
    }
    return ret;
}

void ymsg_set_string(ymsg_t* msg, const char *key, const char *value) {
    set_hash(msg, key, (void*)value);
}

void
ymsg_set_int32(ymsg_t* msg, const char *key, int32_t value) {
    char buff[16];
    sprintf(buff, "%" PRIi32, value);
    set_hash(msg, key, buff);
}

void
ymsg_set_int64(ymsg_t* msg, const char *key, int64_t value) {
    char buff[24];
    sprintf(buff, "%" PRIi64, value);
    set_hash(msg, key, buff);
}
