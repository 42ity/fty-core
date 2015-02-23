#include <zmq.h>

#include "bios_agent.h"

/*
struct _bios_agent_t {
    mlm_client_t *client;   // isn't this obvious?
    void* seq;              //  
};
*/

bios_agent_t*
bios_agent_new (const char* endpoint, uint32_t timeout, const char* address) {
    if (!endpoint || !address) {
        return NULL;
    }

    bios_agent_t *bios_agent = (bios_agent_t *) zmalloc (sizeof (bios_agent_t));  
    if (bios_agent) {
        bios_agent->client = mlm_client_new (endpoint, timeout, address);
        if (!bios_agent->client) {
            free (bios_agent); 
            return NULL;
        }
        bios_agent->seq = zmq_atomic_counter_new (); // create new && set value to zero
        if (!bios_agent->seq) {            
            mlm_client_destroy (&bios_agent->client);
            assert (bios_agent->client == NULL);
            free (bios_agent);
            return NULL;
        }                
        zmq_atomic_counter_set (bios_agent->seq, 0);
    }
    return bios_agent;
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
        assert (&self->client == NULL);
        zmq_atomic_counter_destroy (&self->seq);
        assert (&self->seq == NULL);

        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}

int
bios_agent_send (bios_agent_t *bios_agent, const char *subject, ymsg_t **ymsg_p) {
    if (!bios_agent || !subject || !ymsg_p || !(*ymsg_p)) {
        return -2;  // bad input args
    }
    ymsg_set_seq (*ymsg_p, zmq_atomic_counter_inc (bios_agent->seq)); // return value && increment by one
    zmsg_t *zmsg = ymsg_encode (ymsg_p);
    assert (*ymsg_p == NULL);
    if (!zmsg) {
        return -1;
    }
    return mlm_client_send (bios_agent->client, subject, &zmsg);
}

int
bios_agent_sendto (bios_agent_t *bios_agent, const char *address, const char *subject, const char *tracker, uint32_t timeout, ymsg_t **send_p) {
    if (!bios_agent || !address || !subject || !tracker || !send_p || !(*send_p)) {
        return -2;
    }    
    if (ymsg_id (*send_p) != YMSG_SEND) {
        return -2;
    }
    ymsg_set_seq (*send_p, zmq_atomic_counter_inc (bios_agent->seq)); // return value && increment by one
    
    zmsg_t *zmsg = ymsg_encode (send_p);
    assert (*send_p == NULL);
    if (!zmsg) {
        return -1;
    }
    return mlm_client_sendto (bios_agent->client, address, subject, tracker, timeout, &zmsg);      
}

int
bios_agent_replyto (bios_agent_t *bios_agent, const char *address, const char *subject, const char *tracker, uint32_t timeout, ymsg_t **reply_p, ymsg_t *send) {
    if (!bios_agent || !address || !subject || !tracker || !reply_p || !(*reply_p) || !send) {
        return -2;
    }
    if (ymsg_id (*reply_p) != YMSG_REPLY || ymsg_id (send) != YMSG_SEND) {
        return -2;
    }

    ymsg_set_seq (*reply_p, zmq_atomic_counter_inc (bios_agent->seq)); // return value && increment by one
    ymsg_set_rep (*reply_p, ymsg_seq (send));
    zhash_t *zhash = ymsg_aux (send);
    const char *value = (char *) zhash_lookup (zhash, YMSG_KEY_REPEAT);
    if (value && streq (value, YMSG_YES)) {
        zchunk_t *chunk = ymsg_get_request (send);
        ymsg_set_request (*reply_p, &chunk);
    }
        
    zmsg_t *zmsg = ymsg_encode (reply_p);
    assert (*reply_p == NULL);
    if (!zmsg) {
        return -1;
    }
    return mlm_client_sendto (bios_agent->client, address, subject, tracker, timeout, &zmsg);      
}

int
bios_agent_sendfor (bios_agent_t *bios_agent, const char *address, const char *subject, const char *tracker, uint32_t timeout, ymsg_t **send_p) {
    if (!bios_agent || !address || !subject || !tracker || !send_p || !(*send_p)) {
        return -2;
    }    
    if (ymsg_id (*send_p) != YMSG_SEND) {
        return -2;
    }
    ymsg_set_seq (*send_p, zmq_atomic_counter_inc (bios_agent->seq)); // return value && increment by one
    
    zmsg_t *zmsg = ymsg_encode (send_p);
    assert (*send_p == NULL);
    if (!zmsg) {
        return -1;
    }
    return mlm_client_sendfor (bios_agent->client, address, subject, tracker, timeout, &zmsg);      
}

int
bios_agent_replyfor (bios_agent_t *bios_agent, const char *address, const char *subject, const char *tracker, uint32_t timeout, ymsg_t **reply_p, ymsg_t *send) {
    if (!bios_agent || !address || !subject || !tracker || !reply_p || !(*reply_p) || !send) {
        return -2;
    }
    if (ymsg_id (*reply_p) != YMSG_REPLY || ymsg_id (send) != YMSG_SEND) {
        return -2;
    }

    ymsg_set_seq (*reply_p, zmq_atomic_counter_inc (bios_agent->seq)); // return value && increment by one
    ymsg_set_rep (*reply_p, ymsg_seq (send));
    zhash_t *zhash = ymsg_aux (send);
    const char *value = (char *) zhash_lookup (zhash, YMSG_KEY_REPEAT);
    if (value && streq (value, YMSG_YES)) {
        zchunk_t *chunk = ymsg_get_request (send);
        ymsg_set_request (*reply_p, &chunk);
    }

    zmsg_t *zmsg = ymsg_encode (reply_p);
    assert (*reply_p == NULL);
    if (!zmsg) {
        return -1;
    }
    return mlm_client_sendfor (bios_agent->client, address, subject, tracker, timeout, &zmsg);
}

ymsg_t *
bios_agent_recv (bios_agent_t *bios_agent) {
    if (!bios_agent) {
        return NULL;  // bad input args
    }
    zmsg_t *zmsg = mlm_client_recv (bios_agent->client);
    if (!zmsg) {
        return NULL;
    }
    ymsg_t *ymsg = ymsg_decode (&zmsg);
    assert (zmsg == NULL);
    return ymsg;
}

const char *
bios_agent_command (bios_agent_t *bios_agent) {
    if (!bios_agent) {
        return NULL;
    }
    return mlm_client_command (bios_agent->client);
}

int
bios_agent_status (bios_agent_t *bios_agent) {
    if (!bios_agent) {
        return -2;
    }
    return mlm_client_status (bios_agent->client);
}

const char *
bios_agent_reason (bios_agent_t *bios_agent) {
    if (!bios_agent) {
        return NULL;
    }
    return mlm_client_reason (bios_agent->client);
}

const char *
bios_agent_address (bios_agent_t *bios_agent) {
    if (!bios_agent) {
        return NULL;
    }
    return mlm_client_address (bios_agent->client);
}

const char *
bios_agent_sender (bios_agent_t *bios_agent) {
    if (!bios_agent) {
        return NULL;
    }
    return mlm_client_sender (bios_agent->client);
}

const char *
bios_agent_subject (bios_agent_t *bios_agent) {
    if (!bios_agent) {
        return NULL;
    }
    return mlm_client_subject (bios_agent->client);
}

ymsg_t *
bios_agent_content (bios_agent_t *bios_agent) {
    if (!bios_agent) {
        return NULL;
    }
    zmsg_t *zmsg = mlm_client_content (bios_agent->client);
    if (!zmsg) {
        return NULL;
    }
    return ymsg_decode (&zmsg);
}

const char *
bios_agent_tracker (bios_agent_t *bios_agent) {
    if (!bios_agent) {
        return NULL;
    }
    return mlm_client_tracker (bios_agent->client);
}

