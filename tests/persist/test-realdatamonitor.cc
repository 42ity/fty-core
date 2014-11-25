#include <catch.hpp>

#include <iostream>
#include <czmq.h>

#include "common_msg.h"
#include "assetmsg.h"
#include "monitor.h"

#include "dbpath.h"
/* tests for real monitoring assumed, that DB has the following state
 * In case, the initial data should be changed, the following tests should be changed too

current table:
+----+-----------+----------------------+---------------------+--------+-----------+-------+
| id | id_client | id_discovered_device | timestamp           | id_key | id_subkey | value |
+----+-----------+----------------------+---------------------+--------+-----------+-------+
|  1 |         1 |                    1 | 2014-11-12 09:45:59 |      1 |         1 |    32 |
|  2 |         1 |                    1 | 2014-11-12 09:46:59 |      1 |         1 |     3 |
|  3 |         1 |                    1 | 2014-11-12 09:47:59 |      2 |         1 |    31 |
|  4 |         1 |                    1 | 2014-11-12 09:48:59 |      2 |         2 |    12 |
|  5 |         1 |                    1 | 2014-11-12 09:49:59 |      1 |         2 |     1 |
|  6 |         1 |                    1 | 2014-11-12 09:59:59 |      3 |         1 |     1 |
+----+-----------+----------------------+---------------------+--------+-----------+-------+

last:
+----+----------------------+--------+-----------+-------+---------------------+-------+
| id | id_discovered_device | id_key | id_subkey | value | timestamp           | scale |
+----+----------------------+--------+-----------+-------+---------------------+-------+
|  2 |                    1 |      1 |         1 |     3 | 2014-11-12 09:46:59 |    -2 |
|  3 |                    1 |      2 |         1 |    31 | 2014-11-12 09:47:59 |     0 |
|  4 |                    1 |      2 |         2 |    12 | 2014-11-12 09:48:59 |     1 |
|  5 |                    1 |      1 |         2 |     1 | 2014-11-12 09:49:59 |     1 |
|  6 |                    1 |      3 |         1 |     1 | 2014-11-12 09:59:59 |     0 |
+----+----------------------+--------+-----------+-------+---------------------+-------+

"keytag_id:subkeytag_id:value:scale"

1:1:3:-2
2:1:31:0
2:2:12:1
1:2:1:1
3:1:1:0
*/
TEST_CASE("real_measurements: select_last_measurements", "[db][select][lastmeasurements]")
{
    //SUCCESS
    uint32_t id = 1;
    zlist_t* measurements = select_last_measurements (url.c_str(), id);
    REQUIRE ( measurements );
    char forth1[10]  = "1:1:3:-2";
    char third1[10]  = "2:1:31:0";
    char second1[10] = "2:2:12:1";
    char first1[10]  = "1:2:1:1";
    char zero1[10]   = "3:1:1:0";
    char* zero  = (char*) zlist_first  (measurements);
    REQUIRE ( zero  != NULL );
    char* first  = (char*) zlist_next  (measurements);
    REQUIRE ( first  != NULL );
    char* second = (char*) zlist_next  (measurements);
    REQUIRE ( second != NULL );
    char* third  = (char*) zlist_next  (measurements);
    REQUIRE ( third  != NULL );
    char* forth  = (char*) zlist_next  (measurements);
    REQUIRE ( forth  != NULL );
    REQUIRE ( zlist_next (measurements) == NULL );
    REQUIRE ( strstr(zero, zero1)       == zero );
    REQUIRE ( strstr(first, first1)     == first );
    REQUIRE ( strstr(second, second1)   == second );
    REQUIRE ( strstr(third, third1)     == third );
    REQUIRE ( strstr(forth, forth1)     == forth );

    zlist_destroy (&measurements);
    
    //FAIL
    id = 2;
    measurements = select_last_measurements (url.c_str(), id);
    REQUIRE ( measurements );
    REQUIRE (zlist_size(measurements) == 0 );
    zlist_destroy (&measurements);
}

TEST_CASE("helper functions: convert_asset_to_monitor", "[db][convert_to_monitor]")
{
    uint32_t id = 1;
    REQUIRE ( convert_asset_to_monitor (url.c_str(), id) == -DB_ERROR_NOTFOUND );
    id = 4;
    REQUIRE ( convert_asset_to_monitor (url.c_str(), id) == 1 );
}

TEST_CASE("helper functions: convert_monitor_to_asset", "[db][convert_to_asset]")
{
    uint32_t id = 1;
    REQUIRE ( convert_monitor_to_asset (url.c_str(), id) == 4 );
    id = 4;
    REQUIRE ( convert_monitor_to_asset (url.c_str(), id) == -DB_ERROR_NOTFOUND );
}


TEST_CASE("get_last_measurements", "[db][get][lastmeasurements]")
{
    //SUCCESS
    uint32_t id = 4;
    zmsg_t* getlastmeasurements = common_msg_encode_get_last_measurements (id);
    common_msg_t* glm = common_msg_decode (&getlastmeasurements);
    zmsg_t* returnmeasurements = get_last_measurements (url.c_str(), glm);
    REQUIRE ( returnmeasurements );
    //common_msg_print(common_msg_decode(&returnmeasurements));

    REQUIRE ( is_common_msg (returnmeasurements) );
    common_msg_destroy (&glm);

    glm = common_msg_decode (&returnmeasurements);
    REQUIRE ( common_msg_id (glm) == COMMON_MSG_RETURN_LAST_MEASUREMENTS );
    REQUIRE ( common_msg_device_id (glm) == id );
    zlist_t* info = common_msg_get_measurements (glm);

    char forth1[10]  = "1:1:3:-2";
    char third1[10]  = "2:1:31:0";
    char second1[10] = "2:2:12:1";
    char first1[10]  = "1:2:1:1";
    char zero1[10]   = "3:1:1:0";
    char* zero   = (char*) zlist_first (info);
    REQUIRE ( zero  != NULL );
    char* first  = (char*) zlist_next  (info);
    REQUIRE ( first  != NULL );
    char* second = (char*) zlist_next  (info);
    REQUIRE ( second != NULL );
    char* third  = (char*) zlist_next  (info);
    REQUIRE ( third  != NULL );
    char* forth  = (char*) zlist_next  (info);
    REQUIRE ( forth  != NULL );
    REQUIRE ( zlist_next (info)      == NULL );
    REQUIRE ( strstr(zero,zero1)     == zero );
    REQUIRE ( strstr(first,first1)   == first );
    REQUIRE ( strstr(second,second1) == second );
    REQUIRE ( strstr(third,third1)   == third );
    REQUIRE ( strstr(forth,forth1)   == forth );  
    
    zlist_destroy (&info);
    common_msg_destroy (&glm);

    //FAIL
    id = 3;
    getlastmeasurements = common_msg_encode_get_last_measurements (id);
    glm = common_msg_decode (&getlastmeasurements);
    returnmeasurements = get_last_measurements (url.c_str(), glm);
    REQUIRE ( returnmeasurements );
    REQUIRE ( is_common_msg (returnmeasurements) );
    common_msg_destroy (&glm);

    glm = common_msg_decode (&returnmeasurements);
    REQUIRE ( common_msg_id (glm) == COMMON_MSG_FAIL );
    REQUIRE ( common_msg_errorno (glm) == DB_ERROR_NOTFOUND );
    common_msg_destroy (&glm);

}

TEST_CASE("generate_return_measurements", "[db][generate][return_measurements]")
{
    char fifth1[10]  = "3:1:1:0";
    char forth1[10]  = "1:1:3:-2";
    char third1[10]  = "2:1:31:0";
    char second1[10] = "2:2:12:1";
    char first1[10]  = "1:2:1:1";
    
    zlist_t* measurements = zlist_new();
    zlist_push (measurements, fifth1);
    zlist_push (measurements, forth1);
    zlist_push (measurements, third1);
    zlist_push (measurements, second1);
    zlist_push (measurements, first1);

    uint32_t id = 4;
    common_msg_t* gm = generate_return_measurements (id, &measurements);
    REQUIRE ( gm );
    REQUIRE ( measurements == NULL );
    REQUIRE ( common_msg_device_id (gm) == id );
    measurements = common_msg_measurements (gm);
    REQUIRE ( measurements );
    char* first  = (char*) zlist_first (measurements);
    REQUIRE ( first  != NULL );
    char* second = (char*) zlist_next  (measurements);
    REQUIRE ( second != NULL );
    char* third  = (char*) zlist_next  (measurements);
    REQUIRE ( third  != NULL );
    char* forth  = (char*) zlist_next  (measurements);
    REQUIRE ( forth  != NULL );
    char* fifth  = (char*) zlist_next  (measurements);
    REQUIRE ( fifth  != NULL );
    REQUIRE ( zlist_next (measurements)  == NULL );
    REQUIRE ( strstr(first,first1)       == first );
    REQUIRE ( strstr(second,second1)     == second );
    REQUIRE ( strstr(third,third1)       == third );
    REQUIRE ( strstr(forth,forth1)       == forth );  
    REQUIRE ( strstr(fifth,fifth1)       == fifth );

    common_msg_destroy (&gm);  
}
