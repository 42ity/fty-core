#include <catch.hpp>
#include "asset_types.h"
#include "location_helpers.h"

TEST_CASE (
"location_helpers",
"[web][rest][api]") {

    SECTION ("element_id () valid input") {
        std::string uri = "api/v1/asset/room/5";
        int ret = -1;
        REQUIRE (element_id (uri, ret) == 0);
        REQUIRE (ret == 5);

        uri = "api/v1/asset/datacenter/1234";
        ret = -1;
        REQUIRE (element_id (uri, ret) == 0);
        REQUIRE (ret == 1234);

        uri = "api/v1/asset/x/0";
        ret = -1;
        REQUIRE (element_id (uri, ret) == 0);
        REQUIRE (ret == 0);

        uri = "api/v1/asset/asset sub/4";
        ret = -1;
        REQUIRE (element_id (uri, ret) == 0);
        REQUIRE (ret == 4);

        uri = "api/v1/asset/asset\tsub/50 500 5000";
        ret = -1;
        REQUIRE (element_id (uri, ret) == 0);
        REQUIRE (ret == 50);

        uri = "api/v1/asset/asset \t sub/50\t500 5000";
        ret = -1;
        REQUIRE (element_id (uri, ret) == 0);
        REQUIRE (ret == 50);

    } // SECTION ("element_id () valid input")

    SECTION ("element_id () invalid input") {
        // uri should be non-empty
        std::string uri = "";
        int ret = -1;
        REQUIRE (element_id (uri, ret) == -1);
        REQUIRE (ret == -1);

        // uri has to start with 'api/v1/asset/'
        uri = "api/v1/topology/datacenter/1234";
        ret = -1;
        REQUIRE (element_id (uri, ret) == -1);
        REQUIRE (ret == -1);

        uri = "api/v1/asset";
        ret = -1;
        REQUIRE (element_id (uri, ret) == -1);
        REQUIRE (ret == -1);

        uri = "api/v2/asset/datacenter/1234";
        ret = -1;
        REQUIRE (element_id (uri, ret) == -1);
        REQUIRE (ret == -1);

        uri = "v1/asset/datacenter/1234";
        ret = -1;
        REQUIRE (element_id (uri, ret) == -1);
        REQUIRE (ret == -1);

        uri = "x/v1/asset/datacenter/1234";
        ret = -1;
        REQUIRE (element_id (uri, ret) == -1);
        REQUIRE (ret == -1);

        uri = "api/v1/asset/";
        ret = -1;
        REQUIRE (element_id (uri, ret) == -1);
        REQUIRE (ret == -1);

        uri = "api/v 1/asset/room/5";
        ret = -1;
        REQUIRE (element_id (uri, ret) == -1);
        REQUIRE (ret == -1);

        // other
        uri = "api/v1/asset/ ";
        ret = -1;
        REQUIRE (element_id (uri, ret) == -1);
        REQUIRE (ret == -1);

        uri = "api/v1/asset/x";
        ret = -1;
        REQUIRE (element_id (uri, ret) == -1);
        REQUIRE (ret == -1);

        uri = "api/v1/asset/x5";
        ret = -1;
        REQUIRE (element_id (uri, ret) == -1);
        REQUIRE (ret == -1);

        uri = "api/v1/asset/datacenter/x5";
        ret = -1;
        REQUIRE (element_id (uri, ret) == -1);
        REQUIRE (ret == -1);

        uri = "api/v1/asset/room/x 5";
        ret = -1;
        REQUIRE (element_id (uri, ret) == -1);
        REQUIRE (ret == -1);

        uri = "api/v1/asset/room/x\t5";
        ret = -1;
        REQUIRE (element_id (uri, ret) == -1);
        REQUIRE (ret == -1);

    } // SECTION ("element_id () invalid input") {

    SECTION ("asset() valid input") {
        std::string uri = "api/v1/asset/room/5";
        REQUIRE (asset (uri) == asset_type::ROOM);

        uri = "api/v1/asset/datacenter/1234";
        REQUIRE (asset (uri) == asset_type::DATACENTER);

        uri = "api/v1/asset/row/1234";
        REQUIRE (asset (uri) == asset_type::ROW);

        uri = "api/v1/asset/rack/1234";
        REQUIRE (asset (uri) == asset_type::RACK);

        uri = "api/v1/asset/group/1234";
        REQUIRE (asset (uri) == asset_type::GROUP);

        uri = "api/v1/asset/device/1234";
        REQUIRE (asset (uri) == asset_type::DEVICE);
    } // SECTION ("asset () valid input")

    SECTION ("asset() invalid input") {
        std::string uri = "api/v1/asset//5";
        REQUIRE (asset (uri) == -1);

        uri = "api/v1/asset/";
        REQUIRE (asset (uri) == -1);

        uri = "api/v1/asset";
        REQUIRE (asset (uri) == -1);

        uri = "api/v2/asset/datacenter/5";
        REQUIRE (asset (uri) == -1);

        uri = "api/v1/asset/data center/5";
        REQUIRE (asset (uri) == 0);
    } // SECTION ("asset () invalid input")

}
