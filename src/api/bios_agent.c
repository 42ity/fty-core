#include <zmq.h>

#include "bios_agent.h"
#include "log.h"

#define TIMEOUT 1000
#define KEY_REPEAT "repeat"
#define KEY_STATUS "status"
#define KEY_CONTENT_TYPE "content-type"
#define PREFIX_X "x-"
#define OK      "ok"
#define ERROR   "error"
#define YES     "yes"
#define NO      "no"

struct _bios_agent_t {
    mlm_client_t *client;   // malamute client instance
    void* seq;              // message sequence number
};

BIOS_EXPORT bios_agent_t*
bios_agent_new (const char* endpoint, const char* address) {
    if (!endpoint || !address) {
        return NULL;
    }

    bios_agent_t *self = (bios_agent_t *) zmalloc (sizeof (bios_agent_t));
    if (self) {
        self->client = mlm_client_new (endpoint, TIMEOUT, address);
        if (!self->client) {
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

BIOS_EXPORT void
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

BIOS_EXPORT int
bios_agent_send (bios_agent_t *self, const char *subject, ymsg_t **msg_p) {
    if (!self || !subject || !msg_p || !(*msg_p)) {
        return -2;
    }
    // Note: zmq_atomic_counter_inc() returns current value and increments by one
    ymsg_set_seq (*msg_p, zmq_atomic_counter_inc (self->seq));
    zmsg_t *zmsg = ymsg_encode (msg_p);
    if (!zmsg) {
        return -1;
    }
    int rc = mlm_client_send (self->client, subject, &zmsg);
    return rc;
}

MLM_EXPORT extern volatile int
    mlm_client_verbose;
BIOS_EXPORT int
bios_agent_sendto (bios_agent_t *self, const char *address, const char *subject, ymsg_t **send_p) {
    if (!self || !address || !subject || !send_p || !(*send_p) || ymsg_id (*send_p) != YMSG_SEND) {
        return -2;
    }
    ymsg_set_seq (*send_p, zmq_atomic_counter_inc (self->seq)); // return value && increment by one
    zmsg_t *zmsg = ymsg_encode (send_p);
    if (!zmsg) {
        return -1;
    }
    int rc = mlm_client_sendto (self->client, address, subject, NULL, TIMEOUT, &zmsg);
    return rc;
}

BIOS_EXPORT int
bios_agent_replyto (bios_agent_t *self, const char *address, const char *subject, ymsg_t **reply_p, ymsg_t *send) {
    if (!self || !address || !subject || !reply_p || !(*reply_p) || !send || ymsg_id (*reply_p) != YMSG_REPLY || ymsg_id (send) != YMSG_SEND ) {
        return -2;
    }
    ymsg_set_seq (*reply_p, zmq_atomic_counter_inc (self->seq)); // return value && increment by one
    ymsg_set_rep (*reply_p, ymsg_seq (send));
    zhash_t *zhash = ymsg_aux (send); // without ownership transfer
    const char *value = (char *) zhash_lookup (zhash, KEY_REPEAT);
    if (value && streq (value, YES)) { // default is not to repeat
        zchunk_t *chunk = ymsg_get_request (send); // ownership transfer
        ymsg_set_request (*reply_p, &chunk);
    }
    zmsg_t *zmsg = ymsg_encode (reply_p);
    if (!zmsg) {
        return -1;
    }
    int rc = mlm_client_sendto (self->client, address, subject, NULL, TIMEOUT, &zmsg);
    return rc;
}

BIOS_EXPORT int
bios_agent_sendfor (bios_agent_t *self, const char *address, const char *subject, ymsg_t **send_p) {
    if (!self || !address || !subject || !send_p || !(*send_p) || ymsg_id (*send_p) != YMSG_SEND) {
        return -2;
    }
    ymsg_set_seq (*send_p, zmq_atomic_counter_inc (self->seq)); // return value && increment by one
    zmsg_t *zmsg = ymsg_encode (send_p);
    if (!zmsg) {
        return -1;
    }
    int rc = mlm_client_sendfor (self->client, address, subject, NULL, TIMEOUT, &zmsg);
    return rc;
}

BIOS_EXPORT int
bios_agent_set_producer (bios_agent_t *self, const char *stream)
{
    if (!self || !stream) {
        return -2;
    }
    return mlm_client_set_producer (self->client, stream);
}

BIOS_EXPORT int
bios_agent_set_consumer (bios_agent_t *self, const char *stream, const char *pattern)
{
    if (!self || !stream || !pattern) {
        return -2;
    }
    return mlm_client_set_consumer (self->client, stream, pattern);
}

BIOS_EXPORT ymsg_t *
bios_agent_recv (bios_agent_t *self) {
    if (!self) {
        return NULL;
    }
    zmsg_t *zmsg = mlm_client_recv (self->client);
    if (!zmsg) {
        return NULL;
    }
    ymsg_t *ymsg = ymsg_decode (&zmsg);
    return ymsg;
}

BIOS_EXPORT int
ymsg_status (ymsg_t *self) {
    if (!self || ymsg_id (self) != YMSG_REPLY) {
        return -1;
    }
    const char *value = ymsg_aux_string (self, KEY_STATUS, NULL);
    int rc = -1;
    if (!value) {
        rc = -1;
    } else if (streq (value, OK)) {
        rc = 1;
    } else {
        rc = 0;
    }
    return rc;
}

BIOS_EXPORT int
ymsg_set_status (ymsg_t *self, bool status) {
    if (!self || ymsg_id (self) != YMSG_REPLY) {
        return -1;
    }
    ymsg_aux_insert (self, KEY_STATUS, "%s", status ? OK : ERROR);
    return 0;
}

BIOS_EXPORT int
ymsg_repeat (ymsg_t *self) {
    if (!self) {
        return -1;
    }
    const char *value = ymsg_aux_string (self, KEY_REPEAT, NULL);
    int rc = -1;
    if (!value) {
        rc = -1;
    } else if (streq (value, YES)) {
        rc = 1;
    } else {
        rc = 0;
    }
    return rc;
}

BIOS_EXPORT int
ymsg_set_repeat (ymsg_t *self, bool repeat) {
    if (!self) {
        return -1;
    }
    ymsg_aux_insert (self, KEY_REPEAT, "%s", repeat ? YES : NO);
    return 0;
}

BIOS_EXPORT const char *
ymsg_content_type (ymsg_t *self) {
    if (!self) {
        return NULL;
    }
    return ymsg_aux_string (self, KEY_CONTENT_TYPE, NULL);
}

BIOS_EXPORT int
ymsg_set_content_type (ymsg_t *self, const char *content_type) {
    if (!self || !content_type) {
        return -1;
    }
    ymsg_aux_insert (self, KEY_CONTENT_TYPE, "%s", content_type);
    return 0;
}

BIOS_EXPORT const char *
bios_agent_command (bios_agent_t *self) {
    if (!self) {
        return NULL;
    }
    return mlm_client_command (self->client);
}

BIOS_EXPORT int
bios_agent_status (bios_agent_t *self) {
    if (!self) {
        return -2;
    }
    return mlm_client_status (self->client);
}

BIOS_EXPORT const char *
bios_agent_reason (bios_agent_t *self) {
    if (!self) {
        return NULL;
    }
    return mlm_client_reason (self->client);
}

BIOS_EXPORT const char *
bios_agent_address (bios_agent_t *self) {
    if (!self) {
        return NULL;
    }
    return mlm_client_address (self->client);
}

BIOS_EXPORT const char *
bios_agent_sender (bios_agent_t *self) {
    if (!self) {
        return NULL;
    }
    return mlm_client_sender (self->client);
}

BIOS_EXPORT const char *
bios_agent_subject (bios_agent_t *self) {
    if (!self) {
        return NULL;
    }
    return mlm_client_subject (self->client);
}

BIOS_EXPORT ymsg_t *
bios_agent_content (bios_agent_t *self) {
    if (!self) {
        return NULL;
    }
    zmsg_t *zmsg = mlm_client_content (self->client);
    if (!zmsg) {
        return NULL;
    }
    return ymsg_decode (&zmsg);
}
