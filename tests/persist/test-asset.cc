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
|  1 |         1 |                    1 | 2014-11-12 09:45:59 |      1 |         1 |     3 |
|  2 |         1 |                    1 | 2014-11-12 09:46:59 |      1 |         1 |    32 |
|  3 |         1 |                    1 | 2014-11-12 09:47:59 |      2 |         1 |    31 |
|  4 |         1 |                    1 | 2014-11-12 09:48:59 |      2 |         2 |    12 |
|  5 |         1 |                    1 | 2014-11-12 09:49:59 |      1 |         2 |   142 |
+----+-----------+----------------------+---------------------+--------+-----------+-------+

last:
+----+----------------------+--------+-----------+-------+---------------------+-------+
| id | id_discovered_device | id_key | id_subkey | value | timestamp           | scale |
+----+----------------------+--------+-----------+-------+---------------------+-------+
|  2 |                    1 |      1 |         1 |    32 | 2014-11-12 09:46:59 |    -2 |
|  3 |                    1 |      2 |         1 |    31 | 2014-11-12 09:47:59 |     0 |
|  4 |                    1 |      2 |         2 |    12 | 2014-11-12 09:48:59 |     1 |
|  5 |                    1 |      1 |         2 |   142 | 2014-11-12 09:49:59 |     1 |
+----+----------------------+--------+-----------+-------+---------------------+-------+

"keytag_id:subkeytag_id:value:scale"

1:1:32:-2
2:1:31:0
2:2:12:1
1:2:142:1
*/
TEST_CASE("asset_msg: select_last_measurements","[asset][select][55][lastmeasurements]")
{
    zlist_t* measurements = select_last_measurements (url.c_str(), 1);
    char forth1[10]  = "1:1:32:-2";
    char third1[10] = "2:1:31:0";
    char second1[10]  = "2:2:12:1";
    char first1[10]  = "1:2:142:1";
    char* first = (char*) zlist_first (measurements);
    char* second = (char*) zlist_next (measurements);
    char* third = (char*) zlist_next (measurements);
    char* forth = (char*) zlist_next (measurements);
    REQUIRE ( strstr(first,first1) == first );
    REQUIRE ( strstr(second,second1) == second );
    REQUIRE ( strstr(third,third1) == third );
    REQUIRE ( strstr(forth,forth1) == forth );



}
