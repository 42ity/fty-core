#include <catch.hpp>
#include "dbinit.h"
#include <databaseobject.h>

TEST_CASE("objectstate toString","[objectstate][toString]"){
    REQUIRE(utils::objectStatetoString(utils::ObjectState::OS_NEW) == "osnew");
    REQUIRE(utils::objectStatetoString(utils::ObjectState::OS_SELECTED) == "osselected");
    REQUIRE(utils::objectStatetoString(utils::ObjectState::OS_UPDATED) == "osupdated");
    REQUIRE(utils::objectStatetoString(utils::ObjectState::OS_DELETED) == "osdeleted");
    REQUIRE(utils::objectStatetoString(utils::ObjectState::OS_INSERTED) == "osinserted");
}

TEST_CASE("Databaseobject constructor+getters","[dbobj][constructor][toString][getId][getUrl][getState]"){
    utils::DataBaseObject dbobj(url);
    std::string expected =  "url="  + url                   + ";" +
                            "id="   + std::to_string(-1)    + ";" +
                            "state=" + osnew;
    REQUIRE(dbobj.toString() == expected);
    REQUIRE(dbobj.getId() == -1);
    REQUIRE(dbobj.getUrl() == url);
    REQUIRE(dbobj.getState() == utils::ObjectState::OS_NEW); 
}

TEST_CASE("Databaseobject dbsave","[dbobj][dbsave][dbdelete]"){
    utils::DataBaseObject dbobj(url);
    std::string expected =  "url="  + url                   + ";" +
                            "id="   + std::to_string(-1)    + ";" +
                            "state=" + osnew;
    int n = dbobj.dbdelete();
    REQUIRE( n == 0 );
    REQUIRE(dbobj.toString() == expected );
    
    expected =  "url="  + url                   + ";" +
                            "id="   + std::to_string(-1)    + ";" +
                            "state=" + osselected;
    n = dbobj.dbsave();
    REQUIRE(n == 1);
    REQUIRE(dbobj.toString() == expected);

    n = dbobj.dbdelete();
    REQUIRE(n == 1);
    expected =  "url="  + url                   + ";" +
                            "id="   + std::to_string(-1)    + ";" +
                            "state=" + utils::objectStatetoString(utils::ObjectState::OS_DELETED);
    REQUIRE(dbobj.toString() == expected);
}

//TODO
//clear
//destructor
//reload



