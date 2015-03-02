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

// This field should be used, if there is no more suitable space for the information
// keytag      | value
// --------------------
// add_info    | x_a;x_b;x_c;
// x_a         | "something1"
// x_b         | "something2"
// x_c         | "something3"
#define KEY_ADD_INFO      "add_info"
// "persistence" always return this field for: insert/update/delete requests
#define KEY_AFFECTED_ROWS "affected_rows"
// these fields are valid only if KEY_STATUS = ERROR
#define KEY_ERROR_TYPE     "error_type"
#define KEY_ERROR_SUBTYPE  "error_subtype"
#define KEY_ERROR_MSG      "error_msg"
// this field is valid only if KEY_STATUS = OK
#define KEY_ROWID          "rowid"

#define TEST_NULLITY(PTR) \
    if (PTR != NULL) { \
        log_open (); \
        log_critical ("%s was not properly destroyed or nullified. Possible memory leak.", #PTR); \
        log_close (); \
    }

bios_agent_t*
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
            TEST_NULLITY (self->client)
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
        TEST_NULLITY (self->client)
        zmq_atomic_counter_destroy (&self->seq);
        TEST_NULLITY (self->seq)

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
    zmsg_t *zmsg = ymsg_encode (msg_p);
    TEST_NULLITY (*msg_p)
    if (!zmsg) {
        return -1;
    }
    int rc = mlm_client_send (self->client, subject, &zmsg);
    TEST_NULLITY (zmsg)
    return rc;
}

int
bios_agent_sendto (bios_agent_t *self, const char *address, const char *subject, ymsg_t **send_p) {
    if (!self || !address || !subject || !send_p || !(*send_p) || ymsg_id (*send_p) != YMSG_SEND) {
        return -2;
    }
    ymsg_set_seq (*send_p, zmq_atomic_counter_inc (self->seq)); // return value && increment by one
    zmsg_t *zmsg = ymsg_encode (send_p);
    TEST_NULLITY (*send_p)
    if (!zmsg) {
        return -1;
    }
    int rc = mlm_client_sendto (self->client, address, subject, NULL, TIMEOUT, &zmsg);
    TEST_NULLITY (zmsg)
    return rc;
}

int
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
    TEST_NULLITY (*reply_p)
    if (!zmsg) {
        return -1;
    }
    int rc = mlm_client_sendto (self->client, address, subject, NULL, TIMEOUT, &zmsg);
    TEST_NULLITY (zmsg)
    return rc;
}

int
bios_agent_sendfor (bios_agent_t *self, const char *address, const char *subject, ymsg_t **send_p) {
    if (!self || !address || !subject || !send_p || !(*send_p) || ymsg_id (*send_p) != YMSG_SEND) {
        return -2;
    }
    ymsg_set_seq (*send_p, zmq_atomic_counter_inc (self->seq)); // return value && increment by one
    zmsg_t *zmsg = ymsg_encode (send_p);
    TEST_NULLITY (*send_p)
    if (!zmsg) {
        return -1;
    }
    int rc = mlm_client_sendfor (self->client, address, subject, NULL, TIMEOUT, &zmsg);
    TEST_NULLITY (zmsg)
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
    zmsg_t *zmsg = mlm_client_recv (self->client);
    if (!zmsg) {
        return NULL;
    }
    ymsg_t *ymsg = ymsg_decode (&zmsg);
    TEST_NULLITY (zmsg)
    return ymsg;
}

int
ymsg_rowid (ymsg_t *self) {
    if (!self || ymsg_id (self) != YMSG_REPLY) {
        return -1;
    }
    //TODO wait Karol
    //rowid = ymsg_aux_string (self, KEY_ROWID, NULL);
    return 12345;
}

int
ymsg_set_rowid (ymsg_t *self, uint64_t rowid) {
    if (!self || ymsg_id (self) != YMSG_REPLY) {
        return -1;
    }
    //TODO
    ymsg_aux_insert (self, KEY_ROWID, "%d", rowid);
    return rowid;
}

int
ymsg_errtype (ymsg_t *self) {
    if (!self || ymsg_id (self) != YMSG_REPLY) {
        return -1;
    }
    //TODO wait Karol
    //rowid = ymsg_aux_string (self, KEY_ERROR_TYPE, NULL);
    return 12345;
}

int
ymsg_set_errtype (ymsg_t *self, int error_type) {
    if (!self || ymsg_id (self) != YMSG_REPLY) {
        return -1;
    }
    //TODO
    //ymsg_aux_insert (self,KEY_ERROR_TYPE , "%d", error_type);
    return error_type;
}

int
ymsg_errsubtype (ymsg_t *self) {
    if (!self || ymsg_id (self) != YMSG_REPLY) {
        return -1;
    }
    //TODO wait Karol
    //rowid = ymsg_aux_string (self, KEY_ERROR_SUBTYPE, NULL);
    return 12345;
}

int
ymsg_set_errsubtype (ymsg_t *self, int error_subtype) {
    if (!self || ymsg_id (self) != YMSG_REPLY) {
        return -1;
    }
    //TODO
    //ymsg_aux_insert (self,KEY_ERROR_SUBTYPE, "%d", error_subtype);
    return error_subtype;
}

char*
ymsg_errmsg (ymsg_t *self) {
    if (!self || ymsg_id (self) != YMSG_REPLY) {
        return NULL;
    }
    //TODO wait Karol
    //rowid = ymsg_aux_string (self, KEY_ERROR_MSG, NULL);;
    return NULL;
}

int
ymsg_set_errmsg (ymsg_t *self, const char *error_msg) {
    if (!self || ymsg_id (self) != YMSG_REPLY) {
        return -1;
    }
    //TODO
    ymsg_aux_insert (self, KEY_ERROR_MSG, "%s", error_msg);
    return 0;
}

zhash_t*
ymsg_addinfo (ymsg_t *self) {
    if (!self || ymsg_id (self) != YMSG_REPLY) {
        return NULL;
    }
    //TODO wait Karol
    //rowid = ymsg_aux_string (self, KEY_ERROR_ADDINFO, NULL);
    return NULL;
}

int
ymsg_set_addinfo (ymsg_t *self, zhash_t *addinfo) {
    if (!self || ymsg_id (self) != YMSG_REPLY) {
        return -1;
    }
    //TODO
    //ymsg_aux_insert (self, KEY_ERROR_ADDINFO, "%d", addinfo);
    return 0;
}



ymsg_t*
ymsg_generate_ok(uint64_t rowid, zhash_t *addinfo)
{
    ymsg_t *resultmsg = ymsg_new (YMSG_REPLY);
    ymsg_set_rowid (resultmsg, rowid);
    if ( addinfo != NULL )
        ymsg_set_addinfo (resultmsg, addinfo);
    return resultmsg;
}

ymsg_t*
ymsg_generate_fail (int errtype, int errsubtype, const char *errmsg, zhash_t *addinfo)
{
    ymsg_t* resultmsg = ymsg_new (YMSG_REPLY);
    ymsg_set_errtype    (resultmsg, errtype);
    ymsg_set_errsubtype (resultmsg, errsubtype);
    ymsg_set_errmsg     (resultmsg, errmsg );
    if ( addinfo != NULL )
        ymsg_set_addinfo  (resultmsg, addinfo);
    return resultmsg;
}

int
ymsg_affected_rows (ymsg_t *self)
{
    if (!self || ymsg_id (self) != YMSG_REPLY)
        return -1;
    // TODO wait KAROL
    //int value = ymsg_aux_number (self, KEY_AFFECTED_ROWS, NULL);
    return 3;
}

int
ymsg_set_affected_rows (ymsg_t *self, int n)
{
    if (!self || ymsg_id (self) != YMSG_REPLY)
        return -1;
    // TODO wait KAROL
    //ymsg_aux_insert (self, KEY_AFFECTED_ROWS, "%d", n);
    return 1;
}

int
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

int
ymsg_set_status (ymsg_t *self, bool status) {
    if (!self || ymsg_id (self) != YMSG_REPLY) {
        return -1;
    }
    ymsg_aux_insert (self, KEY_STATUS, "%s", status ? OK : ERROR);
    return 0;
}

int
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

int
ymsg_set_repeat (ymsg_t *self, bool repeat) {
    if (!self) {
        return -1;
    }
    ymsg_aux_insert (self, KEY_REPEAT, "%s", repeat ? YES : NO);
    return 0;
}

const char *
ymsg_content_type (ymsg_t *self) {
    if (!self) {
        return NULL;
    }
    return ymsg_aux_string (self, KEY_CONTENT_TYPE, NULL);
}

int
ymsg_set_content_type (ymsg_t *self, const char *content_type) {
    if (!self || !content_type) {
        return -1;
    }
    ymsg_aux_insert (self, KEY_CONTENT_TYPE, "%s", content_type);
    return 0;
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
    zmsg_t *zmsg = mlm_client_content (self->client);
    if (!zmsg) {
        return NULL;
    }
    return ymsg_decode (&zmsg);
}
