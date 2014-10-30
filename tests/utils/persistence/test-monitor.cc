#include <catch.hpp>

#include <iostream>
#include <czmq.h>

#include "common_msg.h"
#include "testmonitor.h"
#include "assetmsgpersistence.h"

#include "dbpath.h"

TEST_CASE("Common messages: _generate_db_fail","[common][generate][db_fail]")
{
    uint32_t errnonew = 1;
    common_msg_t* fail = _generate_db_fail (errnonew, NULL,  NULL);
    REQUIRE ( fail );
    REQUIRE ( common_msg_id(fail) == COMMON_MSG_FAIL );
//    common_msg_print (fail);

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
//    common_msg_print (fail);

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
//    common_msg_print (fail);
    
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
    uint32_t rowid = 11111111;
    common_msg_t* okmsg = _generate_ok (rowid);
    REQUIRE ( okmsg );
    
    REQUIRE ( common_msg_id (okmsg) == COMMON_MSG_DB_OK );
//    common_msg_print (okmsg);
    uint32_t rowid_ = common_msg_rowid (okmsg);
    REQUIRE ( rowid == rowid_ );

    common_msg_destroy (&okmsg);
    REQUIRE ( okmsg == NULL );
}

TEST_CASE("Common messages: _generate_client","[common][generate][client]")
{
    char name[]= "TestGenerateClient";
    common_msg_t* msgclient = _generate_client (name);
    REQUIRE ( msgclient );
    REQUIRE ( common_msg_id (msgclient) == COMMON_MSG_CLIENT );
//    common_msg_print (msgclient);
    const char* name_client = common_msg_name (msgclient);
    REQUIRE ( streq(name, name_client) );

    common_msg_destroy (&msgclient);
    REQUIRE ( msgclient == NULL );
}

TEST_CASE("Common messages: _generate_return_client","[common][generate][return_client]")
{
    char name[]= "TestGenerateReturnClient";
    common_msg_t* msgclient = _generate_client (name);
    REQUIRE ( msgclient );

//    common_msg_print (msgclient);
    uint32_t client_id = 4;
    
    common_msg_t* msgreturnclient = _generate_return_client (client_id, &msgclient);
    REQUIRE ( msgreturnclient != NULL );
    REQUIRE ( msgclient == NULL );
    
//    common_msg_print (msgreturnclient);
    REQUIRE ( common_msg_id (msgreturnclient) == COMMON_MSG_RETURN_CLIENT );
    REQUIRE ( common_msg_rowid (msgreturnclient) == client_id );

    zmsg_t* newmsg = common_msg_get_msg (msgreturnclient);
    REQUIRE ( newmsg != NULL );
    REQUIRE ( zmsg_is (newmsg) == true );

    common_msg_t* newclient = common_msg_decode (&newmsg);
    REQUIRE ( newmsg == NULL );
    REQUIRE ( newclient != NULL );
    REQUIRE ( common_msg_id (newclient) == COMMON_MSG_CLIENT );
    REQUIRE ( streq(common_msg_name (newclient), name) );

    common_msg_destroy (&msgreturnclient);
    REQUIRE ( msgreturnclient == NULL );   
    common_msg_destroy (&newclient); 
}

TEST_CASE("Common messages: select_client","[common][select][client][byName]")
{
    char name[] = "NUT";
    common_msg_t* newreturn = select_client (url.c_str(), name);
    // this row shold be there
    REQUIRE ( common_msg_id (newreturn) == COMMON_MSG_RETURN_CLIENT );
    REQUIRE ( common_msg_rowid (newreturn) == 4 ); 
    // it is inserted during the db creation  and must have 4
    
//    common_msg_print (newreturn);
    zmsg_t* newmsg = common_msg_get_msg (newreturn);
    common_msg_t* newclient = common_msg_decode (&newmsg);
    REQUIRE ( newclient != NULL );
    REQUIRE ( streq(common_msg_name (newclient), name) );

    common_msg_destroy (&newclient);
    common_msg_destroy (&newreturn);
    
    char name1[] = "ITISNOTTHERE";
    newreturn = select_client (url.c_str(), name1);
    // this row shold not be there
    REQUIRE ( common_msg_id (newreturn) == COMMON_MSG_FAIL );
    
    REQUIRE ( common_msg_errtype(newreturn) == ERROR_DB );
    REQUIRE ( common_msg_errorno(newreturn) == DB_ERROR_NOTFOUND );

    common_msg_destroy (&newreturn);
}

TEST_CASE("Common messages: select_client2","[common][select][client][byId]")
{
    uint32_t id = 4;
    char name[] = "NUT";
    common_msg_t* newreturn = select_client (url.c_str(), id);
    // this row shold be there
    REQUIRE ( common_msg_id (newreturn) == COMMON_MSG_RETURN_CLIENT );
    REQUIRE ( common_msg_rowid (newreturn) == 4 ); 
    
//    common_msg_print (newreturn);
    zmsg_t* newmsg = common_msg_get_msg (newreturn);
    common_msg_t* newclient = common_msg_decode (&newmsg);
    REQUIRE ( newclient != NULL );
    REQUIRE ( streq(common_msg_name (newclient), name) );

    common_msg_destroy (&newclient);
    common_msg_destroy (&newreturn);
    
    id = 11111;
    newreturn = select_client (url.c_str(), id);
    // this row shold not be there
    REQUIRE ( common_msg_id (newreturn) == COMMON_MSG_FAIL );
    
    REQUIRE ( common_msg_errtype(newreturn) == ERROR_DB );
    REQUIRE ( common_msg_errorno(newreturn) == DB_ERROR_NOTFOUND );

    common_msg_destroy (&newreturn);
}

TEST_CASE("Common messages: insert_client/delete_client","[common][insert][delete][client]")
{
    char name[] = "insert/delete";
    common_msg_t* response = insert_client (url.c_str(), name);
    REQUIRE ( response != NULL );
//    common_msg_print (response);

    // this row shold be there
    REQUIRE ( common_msg_id (response) == COMMON_MSG_DB_OK );
    uint32_t newid = common_msg_rowid (response);
    REQUIRE ( newid > 0 );

    common_msg_t* response2 = delete_client (url.c_str(), newid);
    REQUIRE ( response2 != NULL );
//    common_msg_print (response);

    REQUIRE ( common_msg_id (response) == COMMON_MSG_DB_OK );
    uint32_t newid2 = common_msg_rowid (response2);
    REQUIRE ( newid2 == newid );
    
    common_msg_destroy (&response);
    common_msg_destroy (&response2);
}

TEST_CASE("Common messages: insert_client/delete_client fail","[common][insert][delete][client]")
{
    char name[] = "insert/delete/tooooooooooooooooooooooolongname";
    common_msg_t* response = insert_client (url.c_str(), name);
    REQUIRE ( response != NULL );
//    common_msg_print (response);

    // this row shold not be inserted
    REQUIRE ( common_msg_id (response) == COMMON_MSG_FAIL );
    REQUIRE ( common_msg_errorno (response) == DB_ERROR_BADINPUT);

    common_msg_destroy (&response);

    char name2[] = "";
    response = insert_client (url.c_str(), name2);
    REQUIRE ( response != NULL );
//    common_msg_print (response);

    // this row shold not be inserted, empty name is not allowed
    REQUIRE ( common_msg_id (response) == COMMON_MSG_FAIL );
    REQUIRE ( common_msg_errorno (response) == DB_ERROR_BADINPUT);

    common_msg_destroy (&response);

    uint32_t newid = 777; // this row doesn't exists in db
    response = delete_client (url.c_str(), newid);
    REQUIRE ( response != NULL );
//    common_msg_print (response);

    // this row shold not be deleted, empty name is not allowed
    REQUIRE ( common_msg_id (response) == COMMON_MSG_FAIL );
    REQUIRE ( common_msg_errorno (response) == DB_ERROR_BADINPUT);
    
    common_msg_destroy (&response);
}

TEST_CASE("Common messages: update_client1","[common][update][client]")
{
    char name[] = "insert_for_update1";
    common_msg_t* response = insert_client (url.c_str(), name);
    REQUIRE ( response != NULL );
//    common_msg_print (response);

    // this row should be inserted
    REQUIRE ( common_msg_id (response) == COMMON_MSG_DB_OK );
    uint32_t newid = common_msg_rowid (response);

    common_msg_destroy (&response);

    common_msg_t* client = _generate_client ("insert_updated");
    response = update_client (url.c_str(), newid, &client);
    REQUIRE ( response != NULL );
    REQUIRE ( client == NULL );
//    common_msg_print (response);

    // this row should be updated
    REQUIRE ( common_msg_id (response) == COMMON_MSG_DB_OK );
    REQUIRE ( common_msg_rowid (response) == newid );
    
    common_msg_destroy (&response);

    response = delete_client (url.c_str(), newid);
    REQUIRE ( response != NULL );
//    common_msg_print (response);

    REQUIRE ( common_msg_id (response) == COMMON_MSG_DB_OK );
    common_msg_destroy (&response);
}

TEST_CASE("Common messages: update_client2 fail","[common][update][client]")
{
    char name[] = "insert_for_update8";
    common_msg_t* response = insert_client (url.c_str(), name);
    REQUIRE ( response != NULL );
//    common_msg_print (response);

    // this row should be inserted
    REQUIRE ( common_msg_id (response) == COMMON_MSG_DB_OK );
    uint32_t newid = common_msg_rowid (response);

    common_msg_destroy (&response);

    common_msg_t* client = _generate_client ("toooooooooooooooooooooooooooooolongname");
    response = update_client (url.c_str(), newid, &client);
    REQUIRE ( response != NULL );
//    common_msg_print (response);

    // this row should not be updated
    REQUIRE ( common_msg_id (response) == COMMON_MSG_FAIL );
    REQUIRE ( common_msg_errorno (response) == DB_ERROR_BADINPUT );
    
    common_msg_destroy (&response);
    common_msg_destroy (&client);

    client = _generate_client ("");
    response = update_client (url.c_str(), newid, &client);
    REQUIRE ( response != NULL );
//    common_msg_print (response);

    // this row should not be updated
    REQUIRE ( common_msg_id (response) == COMMON_MSG_FAIL );
    REQUIRE ( common_msg_errorno (response) == DB_ERROR_BADINPUT );
    
    common_msg_destroy (&response);
    common_msg_destroy (&client);

    response = delete_client (url.c_str(), newid);
    REQUIRE ( response != NULL );
//    common_msg_print (response);

    REQUIRE ( common_msg_id (response) == COMMON_MSG_DB_OK );
    common_msg_destroy (&response);
}

