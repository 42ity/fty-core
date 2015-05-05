#include <catch.hpp>

#include <iostream>
#include <czmq.h>

#include "common_msg.h"
#include "assetcrud.h"

#include "monitor.h"
#include "dbpath.h"

#include "cleanup.h"

TEST_CASE("Common messages: generate_client_info","[common][generate][client_info][db]")
{
    uint32_t client_id = 1;
    uint32_t device_id = 2;
    uint32_t mytime = 4294967295;
    byte data[] = "1234567890";
    uint32_t datasize = 10;
    _scoped_common_msg_t* msgclient_info = generate_client_info (client_id, device_id, mytime, data, datasize);
    REQUIRE ( msgclient_info );
    REQUIRE ( common_msg_id (msgclient_info) == COMMON_MSG_CLIENT_INFO );
//    common_msg_print (msgclient_info);
    REQUIRE ( client_id == common_msg_client_id (msgclient_info) );
    REQUIRE ( device_id == common_msg_device_id (msgclient_info) );
    _scoped_zchunk_t* info = common_msg_info (msgclient_info);
    REQUIRE ( memcmp (zchunk_data (info), data, datasize) == 0);
    REQUIRE ( zchunk_size(info) == datasize );
    REQUIRE ( mytime == common_msg_date (msgclient_info) );

    common_msg_destroy (&msgclient_info);
    REQUIRE ( msgclient_info == NULL );
}

TEST_CASE("Common messages: generate_return_client_info","[common][generate][return_client_info][db]")
{
    uint32_t client_id = 1;
    uint32_t device_id = 2;
    uint32_t mytime = 4294967295;
    byte data[] = "1234567890";
    uint32_t datasize = 10;
    _scoped_common_msg_t* msgclient_info = generate_client_info (client_id, device_id, mytime, data, datasize);
    REQUIRE ( msgclient_info );
    uint32_t client_info_id = 77;
    _scoped_common_msg_t* msgreturnclient_info = generate_return_client_info (client_info_id, &msgclient_info);
    REQUIRE ( msgreturnclient_info != NULL );
    REQUIRE ( msgclient_info == NULL );
    
//    common_msg_print (msgreturnclient_info);
    REQUIRE ( common_msg_id (msgreturnclient_info) == COMMON_MSG_RETURN_CINFO );
    REQUIRE ( common_msg_rowid (msgreturnclient_info) == client_info_id );

    _scoped_zmsg_t* newmsg = common_msg_get_msg (msgreturnclient_info);
    REQUIRE ( newmsg != NULL );
    REQUIRE ( zmsg_is (newmsg) == true );

    _scoped_common_msg_t* newclient_info = common_msg_decode (&newmsg);
    REQUIRE ( newmsg == NULL );
    REQUIRE ( newclient_info != NULL );
    REQUIRE ( common_msg_id (newclient_info) == COMMON_MSG_CLIENT_INFO );

    REQUIRE ( client_id == common_msg_client_id (newclient_info) );
    REQUIRE ( device_id == common_msg_device_id (newclient_info) );
    _scoped_zchunk_t* info = common_msg_info (newclient_info);
    REQUIRE ( memcmp (zchunk_data (info), data, datasize) == 0);
    REQUIRE ( zchunk_size(info) == datasize );
    REQUIRE ( mytime == common_msg_date (newclient_info) );
    
    common_msg_destroy (&msgreturnclient_info);
    REQUIRE ( msgreturnclient_info == NULL );   
    common_msg_destroy (&newclient_info); 
    REQUIRE ( newclient_info == NULL );
}

TEST_CASE("Common messages: insert_client_info/delete_client_info","[common][insert][delete][client_info][db]")
{
    uint32_t device_id = 1;
    uint32_t client_id = 1;
    zchunk_t* blob = zchunk_new("jjj",4);

    _scoped_common_msg_t* response = insert_client_info (url.c_str(), device_id, client_id, &blob);
    REQUIRE ( response != NULL );
//    common_msg_print (response);

    // this row shold be there
    REQUIRE ( common_msg_id (response) == COMMON_MSG_DB_OK );
    uint32_t newid = common_msg_rowid (response);
    REQUIRE ( newid > 0 );


    _scoped_common_msg_t* newreturn = select_client_info (url.c_str(), newid);
    // this row shold be there
    REQUIRE ( common_msg_id (newreturn) == COMMON_MSG_RETURN_CINFO );
    REQUIRE ( common_msg_rowid (newreturn) == newid ); 
    
//    common_msg_print (newreturn);
    _scoped_zmsg_t* newmsg = common_msg_get_msg (newreturn);
    _scoped_common_msg_t* newclient_info = common_msg_decode (&newmsg);
    REQUIRE ( newclient_info != NULL );
    REQUIRE ( common_msg_id (newclient_info) == COMMON_MSG_CLIENT_INFO );
    REQUIRE ( common_msg_client_id (newclient_info) == client_id );
    REQUIRE ( common_msg_device_id (newclient_info) == device_id );

    common_msg_destroy (&newclient_info);
    common_msg_destroy (&newreturn);
    
    uint32_t id = 11111;
    newreturn = select_client_info (url.c_str(), id);
    // this row shold not be there
    REQUIRE ( common_msg_id (newreturn) == COMMON_MSG_FAIL );
    
    REQUIRE ( common_msg_errtype(newreturn) == BIOS_ERROR_DB );
    REQUIRE ( common_msg_errorno(newreturn) == DB_ERROR_NOTFOUND );

    common_msg_destroy (&newreturn);




    _scoped_common_msg_t* response2 = delete_client_info (url.c_str(), newid);
    REQUIRE ( response2 != NULL );
//    common_msg_print (response);

    REQUIRE ( common_msg_id (response) == COMMON_MSG_DB_OK );
    uint32_t newid2 = common_msg_rowid (response2);
    REQUIRE ( newid2 == newid );
    
    common_msg_destroy (&response);
    common_msg_destroy (&response2);
}
/*
TEST_CASE("Common messages: update_client_info1","[common][update][client_info]")
{
    char name[] = "insert_for_update1";
    uint32_t device_id = 1;
    uint32_t client_id = 1;
    _scoped_zchunk_t* blob = zchunk_new("jjj",4);

    _scoped_common_msg_t* response = insert_client_info (url.c_str(), device_id, client_id, &blob);
    REQUIRE ( response != NULL );
//    common_msg_print (response);

    // this row should be inserted
    REQUIRE ( common_msg_id (response) == COMMON_MSG_DB_OK );
    uint32_t newid = common_msg_rowid (response);

    common_msg_destroy (&response);

    _scoped_common_msg_t* client_info = generate_client_info ("insert_updated");
    response = update_client_info (url.c_str(), newid, &client_info);
    REQUIRE ( response != NULL );
    REQUIRE ( client_info == NULL );
//    common_msg_print (response);

    // this row should be updated
    REQUIRE ( common_msg_id (response) == COMMON_MSG_DB_OK );
    REQUIRE ( common_msg_rowid (response) == newid );
    
    common_msg_destroy (&response);

    response = delete_client_info (url.c_str(), newid);
    REQUIRE ( response != NULL );
//    common_msg_print (response);

    REQUIRE ( common_msg_id (response) == COMMON_MSG_DB_OK );
    common_msg_destroy (&response);
}

*/
