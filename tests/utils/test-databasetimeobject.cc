#include <catch.hpp>
#include "dbinit.h"
#include <databasetimeobject.h>

TEST_CASE("Databasetimeobject constructor+getters","[dbj][constructor][toString][getTimestamp]"){
    utils::DataBaseTimeObject dbobj(url);
    REQUIRE(dbobj.getId() == -1);
    REQUIRE(dbobj.getUrl() == url);
    REQUIRE(dbobj.getState() == utils::ObjectState::OS_NEW);
    REQUIRE(dbobj.getTimestamp() <= time(nullptr)); 
}

TEST_CASE("Databasetimeobject dbsave/dbdelete","[dbj][dbsave][dbdelete]"){
    utils::DataBaseTimeObject dbobj(url);
    
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

