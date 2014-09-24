#include <catch.hpp>

#include <clientinfo.h>

std::string urlcl = "mysql:db=box_utf8;user=root";

std::string osnewcl       = utils::objectStatetoString(utils::ObjectState::OS_NEW);
std::string osdeletedcl   = utils::objectStatetoString(utils::ObjectState::OS_DELETED);
std::string osupdatedcl   = utils::objectStatetoString(utils::ObjectState::OS_UPDATED);
std::string osselectedcl  = utils::objectStatetoString(utils::ObjectState::OS_SELECTED);
std::string osinsertedcl  = utils::objectStatetoString(utils::ObjectState::OS_INSERTED);

TEST_CASE("Client info+getters","[dbclientinfo][constructor][toString][getId][getUrl][getState][getName]"){
    utils::ClientInfo dbobj(urlcl);
    REQUIRE(dbobj.toString() > "" );
    REQUIRE(dbobj.getId() == -1 );
    REQUIRE(dbobj.getUrl() == urlcl );
    REQUIRE(utils::objectStatetoString(dbobj.getState()) == osnewcl);
    REQUIRE(dbobj.getClientId() == -1);
    REQUIRE(dbobj.getDeviceDiscoveredId() == -1 );
    REQUIRE(dbobj.getBlobData() == "");
    REQUIRE(dbobj.getClientName() == "");
}

TEST_CASE("ClientInfo+getters1","[dbclientinfo][constructor]"){
    std::string name = "mymodule";
    utils::ClientInfo dbobj(urlcl,name);
    REQUIRE(dbobj.getId() == -1 );
    REQUIRE(dbobj.getUrl() == urlcl );
    REQUIRE(utils::objectStatetoString(dbobj.getState()) == osnewcl);
}

