#include <catch.hpp>

#include <clientinfo.h>

std::string urlcl = "mysql:db=box_utf8;user=root";

std::string osnewcl       = utils::objectStatetoString(utils::ObjectState::OS_NEW);
std::string osdeletedcl   = utils::objectStatetoString(utils::ObjectState::OS_DELETED);
std::string osupdatedcl   = utils::objectStatetoString(utils::ObjectState::OS_UPDATED);
std::string osselectedcl  = utils::objectStatetoString(utils::ObjectState::OS_SELECTED);
std::string osinsertedcl  = utils::objectStatetoString(utils::ObjectState::OS_INSERTED);

TEST_CASE("Client info+getters","[dbclientinfo][constructor][toString][getId][getUrl][getState][getName]"){
    utils::ClientInfo dbobj(urldcl);
    REQUIRE(dbobj.toString() > "" );
    REQUIRE(dbobj.getId() == -1 );
    REQUIRE(dbobj.getUrl() == urldcl );
    REQUIRE(utils::objectStatetoString(dbdevice.getState()) == osnewcl);
    REQUIRE(dbobj.getClientId() == -1);
    REQUIRE(dbobj.getDeviceDiscoveredId() == -1 );
    REQUIRE(dbobj.getBlobData() == "");
    REQUIRE(dbobj.getClientName() == "");
}

TEST_CASE("ClientInfo+getters1","[dbclientinfo][constructor]"){
    std::string name = "mymodule";
    utils::DeviceDiscovered dbdevice(urldev,name);
    REQUIRE(dbdevice.toString() == expected );
    REQUIRE(dbdevice.getId() == -1 );
    REQUIRE(dbdevice.getUrl() == urldev );
    REQUIRE(utils::objectStatetoString(dbdevice.getState()) == osnew);
    REQUIRE(dbdevice.getName() == name);
}

