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
    REQUIRE(dbobj.getName() == "unknown");
}

TEST_CASE("Device discovered+getters1","[dbclientinfo][constructor][toString][getId][getUrl][getState][getName]"){
    std::string name = "myname";
    utils::DeviceDiscovered dbdevice(urldev,name);
    std::string expected =  "url="  + urldev                + ";" +
                            "id="   + std::to_string(-1)    + ";" +
                            "state=" + osnew                + ";" +
                            "name=" + name;
    REQUIRE(dbdevice.toString() == expected );
    REQUIRE(dbdevice.getId() == -1 );
    REQUIRE(dbdevice.getUrl() == urldev );
    REQUIRE(utils::objectStatetoString(dbdevice.getState()) == osnew);
    REQUIRE(dbdevice.getName() == name);
}

