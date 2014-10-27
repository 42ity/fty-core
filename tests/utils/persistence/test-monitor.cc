#include <catch.hpp>

#include <iostream>
#include <czmq.h>

#include "common_msg.h"
#include "testmonitor.h"

#include "dbpath.h"

TEST_CASE("Common messages: _generate_db_fail","[common][generate][db_fail]")
{
    int errnonew = 1;
    common_msg_t* fail = _generate_db_fail (errnonew, NULL,  NULL);
    REQUIRE ( fail );
    REQUIRE ( common_msg_id(fail) == COMMON_MSG_FAIL );
    common_msg_print (fail);

    byte        errno1 = common_msg_errorno (fail);
    const char* errmsg = common_msg_errmsg  (fail);
    zhash_t *   erraux = common_msg_erraux  (fail);
    REQUIRE ( errnonew == errno1 );
    REQUIRE ( erraux   == NULL );
    REQUIRE ( streq(errmsg, "")  );

    common_msg_destroy(&fail);
    REQUIRE ( fail == NULL );

    errnonew = 2;
    fail = _generate_db_fail (errnonew, "",  NULL);
    REQUIRE( fail );
    REQUIRE ( common_msg_id(fail) == COMMON_MSG_FAIL );
    common_msg_print (fail);

    errno1 = common_msg_errorno (fail);
    errmsg = common_msg_errmsg  (fail);
    erraux = common_msg_erraux  (fail);
    REQUIRE ( errnonew == errno1 );
    REQUIRE ( erraux   == NULL );
    REQUIRE ( streq(errmsg, "")  );

    common_msg_destroy(&fail);
    REQUIRE ( fail == NULL );

    errnonew             = 3;
    char     errmsgnew[] = "abracadabra";
    zhash_t* errauxnew   = zhash_new();
    char     keytag[]    = "mykey";
    char     value[]     = "myvalue";
    zhash_insert (errauxnew, &keytag, &value);

    fail = _generate_db_fail (errnonew, errmsgnew, errauxnew);
    REQUIRE ( fail );
    REQUIRE ( common_msg_id(fail) == COMMON_MSG_FAIL );
    // REQUIRE ( errauxnew == NULL );  // this doesn't work
    common_msg_print (fail);
    
    errno1 = common_msg_errorno (fail);
    errmsg = common_msg_errmsg  (fail);
    erraux = common_msg_erraux  (fail);
    REQUIRE ( errnonew == errno1 );
    REQUIRE ( streq( errmsg, errmsgnew) );
    // TODO how to compare zhash_t ??
    common_msg_destroy (&fail);
    REQUIRE ( fail == NULL );
}

TEST_CASE("Common messages: _generate_ok","[common][generate][db_ok]")
{
    unsigned int rowid = 11111111;
    common_msg_t* okmsg = _generate_ok (rowid);
    REQUIRE ( okmsg );
    
    REQUIRE ( common_msg_id (okmsg) == COMMON_MSG_DB_OK );
    common_msg_print (okmsg);
    unsigned int rowid_ = common_msg_rowid (okmsg);
    REQUIRE ( rowid == rowid_ );

    common_msg_destroy (&okmsg);
    REQUIRE ( okmsg == NULL );
}

TEST_CASE("Common messages: _generate_client","[common][generate][client]")
{
    char name[]= "TestSetClientName";
    common_msg_t* msgclient = _generate_client (name);
    REQUIRE ( msgclient );
    REQUIRE ( common_msg_id (msgclient) == COMMON_MSG_CLIENT );
    common_msg_print (msgclient);
    const char* name_client = common_msg_name (msgclient);
    REQUIRE ( streq(name, name_client) );

    common_msg_destroy (&msgclient);
    REQUIRE ( msgclient == NULL );
}

TEST_CASE("Common messages: _generate_return_client","[common][generate][return_client]")
{
    char name[]= "TestReturnClient";
    // common_msg_t* msgclient = _generate_client (name);
    // REQUIRE ( msgclient );
    common_msg_t* msgclient = common_msg_new(COMMON_MSG_CLIENT);
    REQUIRE (msgclient != NULL);
    REQUIRE (common_msg_id (msgclient) == COMMON_MSG_CLIENT);
    common_msg_set_name (msgclient, "%s", "karol");
    REQUIRE (streq(common_msg_name(msgclient), "karol"));

    puts ("DEBUG: OUR PPRINT\n");
    common_msg_print (msgclient);
    puts ("DEBUG: OUR PPRINT END\n");
    unsigned int client_id = 4;
    
    common_msg_t* msgreturnclient = _generate_return_client (client_id, &msgclient);
    REQUIRE (msgreturnclient != NULL);
    REQUIRE ( common_msg_id (msgreturnclient) == COMMON_MSG_RETURN_CLIENT );
    REQUIRE ( common_msg_client_id(msgreturnclient) == client_id);

    zmsg_t* newmsg = common_msg_get_msg(msgreturnclient);
    REQUIRE (newmsg != NULL);
    REQUIRE (zmsg_is (newmsg) == true);
    puts ("DEBUG: OUR PPRINT 2\n");
    zmsg_print (newmsg);
    puts ("DEBUG: OUR PPRINT 2 END\n");


    common_msg_t* newclient = common_msg_decode(&newmsg);
    REQUIRE (newclient != NULL);
    printf("ok\n");
//    common_msg_destroy (&msgreturnclient);
//    REQUIRE ( msgreturnclient == NULL );
    
//    common_msg_destroy (&msgclient);      // zframe.c:87: zframe_destroy: Assertion `zframe_is (self)' failed.
//    REQUIRE ( msgclient == NULL );
}


TEST_CASE("Common messages: select_client","[common][select][client]")
{
    char name[] = "NUT";
    common_msg_t* newreturn = select_client(url.c_str(), name);
    printf("return_client_here\n");

    REQUIRE ( common_msg_id(newreturn) == COMMON_MSG_RETURN_CLIENT );
   
    //zmsg_t* newmsg = common_msg_msg(newreturn);
    //common_msg_t* newclient = common_msg_decode(&newmsg);
    //REQUIRE ( common_msg_name(newclient) == name);
}


