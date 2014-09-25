#include <catch.hpp>

#include <databasetimeobject.h>

std::string urlbto = "mysql:db=box_utf8;user=root";

std::string osnewbto       = utils::objectStatetoString(utils::ObjectState::OS_NEW);
std::string osdeletedbto   = utils::objectStatetoString(utils::ObjectState::OS_DELETED);
std::string osupdatedbto   = utils::objectStatetoString(utils::ObjectState::OS_UPDATED);
std::string osselectedbto  = utils::objectStatetoString(utils::ObjectState::OS_SELECTED);
std::string osinsertedbto  = utils::objectStatetoString(utils::ObjectState::OS_INSERTED);

TEST_CASE("Databasetimeobject constructor+getters","[dbtobj][constructor][toString][getTimestampt]"){
    utils::DataBaseTimeObject dbobj(urlbto);
    REQUIRE(dbobj.getId() == -1);
    REQUIRE(dbobj.getUrl() == urlbto);
    REQUIRE(dbobj.getState() == utils::ObjectState::OS_NEW);
    REQUIRE(dbobj.getTimestamp() <= time(nullptr)); 
}

TEST_CASE("Databasetimeobject dbsave/dbdelete","[dbtobj][dbsave][dbdelete]"){
    utils::DataBaseTimeObject dbobj(urlbto);
    
    int n = dbobj.dbdelete();
    REQUIRE( n == 0 );
    REQUIRE(dbobj.getState() == utils::ObjectState::OS_NEW);

    n = dbobj.dbsave();
    REQUIRE(n == 1);
    REQUIRE(dbobj.getState() == utils::ObjectState::OS_INSERTED);

    n = dbobj.dbdelete();
    REQUIRE(n == 1);
    REQUIRE(dbobj.getState() == utils::ObjectState::OS_DELETED);
}

//TODO
//clear
//destructor
//reload

