/*
 *
 * Copyright (C) 2015 Eaton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
#include <catch.hpp>
#include <string>
#include <limits.h>

#include "tntmlm.h"

TEST_CASE ("tntmlm1", "[tntmlm]")
{
    std::string &ref = const_cast <std::string&> (MlmClient::ENDPOINT);
    ref = "inproc://tntmlm-catch-1";
    CHECK (MlmClient::ENDPOINT == "inproc://tntmlm-catch-1");

    zactor_t *server = zactor_new (mlm_server, (void*) "Malamute");    
    zstr_sendx (server, "BIND", MlmClient::ENDPOINT.c_str () , NULL);
 
    { // Invariant: malamute MUST be destroyed last

        // tntmlm.h already has one pool but we can't use that one 
        // for this test because valgrind will detect memory leak
        // which occurs because malamute server is destroyed only
        // after clients allocated in the pool of the header file
        MlmClientPool mlm_pool_test {3};   

        mlm_client_t *agent1 = mlm_client_new ();
        mlm_client_connect (agent1, MlmClient::ENDPOINT.c_str (), 1000, "AGENT1");

        mlm_client_t *agent2 = mlm_client_new ();    
        mlm_client_connect (agent2, MlmClient::ENDPOINT.c_str (), 1000, "AGENT2");


        printf ("\n ---- max pool ----\n");
        {   
            CHECK (mlm_pool_test.getMaximumSize () == 3);
            MlmClientPool::Ptr ui_client = mlm_pool_test.get ();
            CHECK (ui_client.getPointer () != NULL); 

            MlmClientPool::Ptr ui_client2 = mlm_pool_test.get ();
            CHECK (ui_client2.getPointer ()!= NULL);

            MlmClientPool::Ptr ui_client3 = mlm_pool_test.get ();
            CHECK (ui_client3.getPointer () != NULL);

            MlmClientPool::Ptr ui_client4 = mlm_pool_test.get ();
            CHECK (ui_client4.getPointer () != NULL);
 
            MlmClientPool::Ptr ui_client5 = mlm_pool_test.get ();
            CHECK (ui_client5.getPointer () != NULL);
        }
        CHECK (mlm_pool_test.getMaximumSize () == 3); // this tests the policy that excess clients
                                                      // that were created are dropped
        printf ("OK");

        printf ("\n ---- no reply ----\n");
        {   
            MlmClientPool::Ptr ui_client = mlm_pool_test.get ();
            CHECK (ui_client.getPointer () != NULL);

            zmsg_t *reply = ui_client->recv ("uuid1", 1);
            CHECK (reply == NULL);

            reply = ui_client->recv ("uuid1", 0);
            CHECK (reply == NULL);
        }
        printf ("OK\n");

        printf ("\n ---- send - correct reply ----\n");
        {   
            zmsg_t *msg = zmsg_new ();
            zmsg_addstr (msg, "uuid1");

            MlmClientPool::Ptr ui_client = mlm_pool_test.get ();
            CHECK (ui_client.getPointer () != NULL);
            MlmClientPool::Ptr ui_client2 = mlm_pool_test.get ();
            CHECK (ui_client2.getPointer () != NULL);
            int rv = ui_client2->sendto ("AGENT1", "TEST", 1000, &msg);
            CHECK (rv == 0);

            zmsg_t *agent1_msg = mlm_client_recv (agent1);
            CHECK (agent1_msg != NULL);
            char *agent1_msg_uuid = zmsg_popstr (agent1_msg);
            zmsg_destroy (&agent1_msg);
            agent1_msg = zmsg_new ();
            zmsg_addstr (agent1_msg, agent1_msg_uuid);
            zstr_free (&agent1_msg_uuid);
            zmsg_addstr (agent1_msg, "abc");
            zmsg_addstr (agent1_msg, "def");
            rv = mlm_client_sendto (agent1, mlm_client_sender (agent1), mlm_client_subject (agent1), NULL, 1000, &agent1_msg);
            CHECK (rv == 0);

            zmsg_t *reply = ui_client2->recv ("uuid1", 1);
            CHECK (reply != NULL);
            char *tmp = zmsg_popstr (reply);
            CHECK (streq (tmp, "abc"));
            zstr_free (&tmp);
            tmp = zmsg_popstr (reply);
            CHECK (streq (tmp, "def"));
            zstr_free (&tmp);
            zmsg_destroy (&reply);
        }
        printf ("OK\n");

        printf ("\n ---- send - reply with bad uuid ----\n");
        {   
            zmsg_t *msg = zmsg_new ();
            zmsg_addstr (msg, "uuid1");

            MlmClientPool::Ptr ui_client = mlm_pool_test.get ();
            CHECK (ui_client.getPointer ()  != NULL);
            ui_client->sendto ("AGENT1", "TEST", 1000, &msg);

            zmsg_t *agent1_msg = mlm_client_recv (agent1);
            CHECK (agent1_msg != NULL);
            zmsg_destroy (&agent1_msg);
            agent1_msg = zmsg_new ();
            zmsg_addstr (agent1_msg, "BAD-UUID");
            zmsg_addstr (agent1_msg, "abc");
            zmsg_addstr (agent1_msg, "def");
            mlm_client_sendto (agent1, mlm_client_sender (agent1), mlm_client_subject (agent1), NULL, 1000, &agent1_msg);

            zmsg_t *reply = ui_client->recv ("uuid1", 1);
            CHECK (reply == NULL);
        }
        printf ("OK");

        printf ("\n ---- send - correct reply - expect bad uuid ----\n");
        {   
            zmsg_t *msg = zmsg_new ();
            zmsg_addstr (msg, "uuid1");

            MlmClientPool::Ptr ui_client = mlm_pool_test.get ();
            CHECK (ui_client.getPointer ()  != NULL);
            ui_client->sendto ("AGENT1", "TEST", 1000, &msg);

            zmsg_t *agent1_msg = mlm_client_recv (agent1);
            CHECK (agent1_msg != NULL);
            zmsg_destroy (&agent1_msg);
            agent1_msg = zmsg_new ();
            zmsg_addstr (agent1_msg, "uuid1");
            zmsg_addstr (agent1_msg, "abc");
            zmsg_addstr (agent1_msg, "def");
            mlm_client_sendto (agent1, mlm_client_sender (agent1), mlm_client_subject (agent1), NULL, 1000, &agent1_msg);

            zmsg_t *reply = ui_client->recv ("BAD-UUID", 1);
            CHECK (reply == NULL);
        }
        printf ("OK");

        printf ("\n ---- send - reply 3x with bad uuid first - then correct reply ----\n");
        {   
            zmsg_t *msg = zmsg_new ();
            zmsg_addstr (msg, "uuid34");

            MlmClientPool::Ptr ui_client = mlm_pool_test.get ();
            CHECK (ui_client.getPointer () != NULL);
            ui_client->sendto ("AGENT1", "TEST", 1000, &msg);

            zmsg_t *agent1_msg = mlm_client_recv (agent1);
            CHECK (agent1_msg != NULL);
            zmsg_destroy (&agent1_msg);

            agent1_msg = zmsg_new ();
            zmsg_addstr (agent1_msg, "BAD-UUID1");
            zmsg_addstr (agent1_msg, "abc");
            zmsg_addstr (agent1_msg, "def");
            int rv = mlm_client_sendto (agent1, mlm_client_sender (agent1), mlm_client_subject (agent1), NULL, 1000, &agent1_msg);
            CHECK (rv == 0);

            agent1_msg = zmsg_new ();
            zmsg_addstr (agent1_msg, "BAD-UUID2");
            zmsg_addstr (agent1_msg, "abc");
            rv = mlm_client_sendto (agent1, mlm_client_sender (agent1), mlm_client_subject (agent1), NULL, 1000, &agent1_msg);
            CHECK (rv == 0);

            agent1_msg = zmsg_new ();
            zmsg_addstr (agent1_msg, "BAD-UUID3");
            rv = mlm_client_sendto (agent1, mlm_client_sender (agent1), mlm_client_subject (agent1), NULL, 1000, &agent1_msg);
            CHECK (rv == 0);

            agent1_msg = zmsg_new ();
            zmsg_addstr (agent1_msg, "uuid34");
            zmsg_addstr (agent1_msg, "OK");
            zmsg_addstr (agent1_msg, "element");
            rv = mlm_client_sendto (agent1, mlm_client_sender (agent1), mlm_client_subject (agent1), NULL, 1000, &agent1_msg);
            CHECK (rv == 0);

            zmsg_t *reply = ui_client->recv ("uuid34", 2);
            CHECK (reply != NULL);
            char *tmp = zmsg_popstr (reply);
            CHECK (streq (tmp, "OK"));
            zstr_free (&tmp);
            tmp = zmsg_popstr (reply);
            CHECK (streq (tmp, "element"));
            zstr_free (&tmp);
            zmsg_destroy (&reply);
        }
        printf ("OK");
        printf ("\n ---- Simulate thread switch ----\n");
        {   
            MlmClientPool::Ptr ui_client = mlm_pool_test.get ();

            zmsg_t *msg = zmsg_new ();
            zmsg_addstr (msg, "uuid25");
            ui_client->sendto ("AGENT1", "TEST", 1000, &msg);

            msg = zmsg_new ();
            zmsg_addstr (msg, "uuid33");
            ui_client->sendto ("AGENT2", "TEST", 1000, &msg);

            zmsg_t *agent1_msg = mlm_client_recv (agent1);
            CHECK (agent1_msg != NULL);
            zmsg_destroy (&agent1_msg);

            agent1_msg = zmsg_new ();
            zmsg_addstr (agent1_msg, "uuid25");
            zmsg_addstr (agent1_msg, "ABRAKADABRA");
            int rv = mlm_client_sendto (agent1, mlm_client_sender (agent1), mlm_client_subject (agent1), NULL, 1000, &agent1_msg);
            CHECK (rv == 0);

            zmsg_t *agent2_msg = mlm_client_recv (agent2);
            CHECK (agent2_msg != NULL);
            zmsg_destroy (&agent2_msg);

            agent2_msg = zmsg_new ();
            zmsg_addstr (agent2_msg, "uuid33");
            zmsg_addstr (agent2_msg, "HABAKUKA");
            rv = mlm_client_sendto (agent2, mlm_client_sender (agent2), mlm_client_subject (agent2), NULL, 1000, &agent2_msg);
            CHECK (rv == 0);

            zmsg_t *reply = ui_client->recv ("uuid33", 1);
            CHECK (reply != NULL);
            char *tmp = zmsg_popstr (reply);
            CHECK (streq (tmp, "HABAKUKA"));
            zstr_free (&tmp);
            zmsg_destroy (&reply);
        }
        mlm_client_destroy (&agent1);
        mlm_client_destroy (&agent2);
    }

    zactor_destroy (&server);
    printf ("OK");
}

