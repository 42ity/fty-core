#include <catch.hpp>
#include "dbinit.h"
#include <clientinfo.h>

TEST_CASE("Client info+getters","[dbclientinfo][constructor][toString][getId][getUrl][getState][getName]"){
    utils::ClientInfo dbobj(url);
    REQUIRE(dbobj.toString() > "" );
    REQUIRE(dbobj.getId() == -1 );
    REQUIRE(dbobj.getUrl() == url );
    REQUIRE(utils::objectStatetoString(dbobj.getState()) == osnew);
    REQUIRE(dbobj.getClientId() == -1);
    REQUIRE(dbobj.getDeviceDiscoveredId() == -1 );
    REQUIRE(dbobj.getBlobData() == "");
    REQUIRE(dbobj.getClientName() == "");
}

TEST_CASE("ClientInfo+getters1","[dbclientinfo][constructor]"){
    std::string name = "mymodule";
    utils::ClientInfo dbobj(url,name);
    REQUIRE(dbobj.getId() == -1 );
    REQUIRE(dbobj.getUrl() == url );
    REQUIRE(utils::objectStatetoString(dbobj.getState()) == osnew);
}

