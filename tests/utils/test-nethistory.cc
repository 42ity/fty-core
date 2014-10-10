#include <catch.hpp>
#include "dbinit.h"
#include "nethistory.h"
#include "cidr.h"

TEST_CASE("net history getters","[dbnethistory][constructor1][toString] \\
[getId][getUrl][getState][getTimestamp][getMac][getMask][getIp][getName] \\
[getCommand][getAddress]")
{
    utils::db::NetHistory dbnethistory(url);
    time_t tmp_t = dbnethistory.getTimestamp();
    std::string timestr = ctime(&tmp_t);

    std::string expected =  "url="       + url                + ";" +
                            "id="        + std::to_string(-1) + ";" +
                            "state="     + osnew              + ";" +
                            "time="      + timestr            + ";" +
                            "mac="       + ""                 + ";" +
                            "address="   + ""                 + ";" +
                            "command="   + "z"                + ";" +
                            "name="      + ""                 ;
    
    REQUIRE( dbnethistory.getId() == -1 );
    REQUIRE( dbnethistory.getUrl() == url );
    REQUIRE( utils::db::objectStatetoString(dbnethistory.getState()) == osnew );
    REQUIRE( dbnethistory.getMac() == "" );
    REQUIRE( dbnethistory.getMask() == -1 );
    REQUIRE( dbnethistory.getIp() == "" );
    REQUIRE( dbnethistory.getCommand() == 'z' );
    REQUIRE( dbnethistory.getName() == "" );
    REQUIRE( dbnethistory.getTimestamp() == tmp_t );
    REQUIRE( dbnethistory.toString() == expected );
    utils::CIDRAddress nn();
    //TODO doesn't work
    //REQUIRE( dbnethistory.getAddress() == nn );
}

TEST_CASE("net history insert/delete ","[dbnethistory][save][insert][delete]"){
    utils::db::NetHistory dbnethistory(url);
    std::string newname = "insert_delete";
    utils::CIDRAddress newaddress("1.1.1.1",8);
    dbnethistory.setName(newname);
    dbnethistory.setAddress(newaddress);
    dbnethistory.setMac("123456789111");
    dbnethistory.setCommand('a');
    int n = dbnethistory.dbsave();
    std::string expected;
    if ( n == 1 )
    {
        REQUIRE(dbnethistory.getId() > 0 );
        REQUIRE(utils::db::objectStatetoString(dbnethistory.getState()) == osinserted);
        expected = dbnethistory.toString();
        n = dbnethistory.dbsave();
        REQUIRE( n == 0 );
        REQUIRE( dbnethistory.toString() == expected );
        
        n = dbnethistory.dbdelete();
        if ( n == 0 )   // unreal situation
            FAIL("nothing was deleted"); 
        if ( n > 1 )    // unreal situation
            FAIL("more than one row was deleted");
        if ( n == 1 )
        {
            REQUIRE( utils::db::objectStatetoString(dbnethistory.getState()) == osdeleted);
            REQUIRE( dbnethistory.getId() == -1 );

            expected = dbnethistory.toString();
            n = dbnethistory.dbdelete();
            INFO(expected);
            REQUIRE( n == 0 );
            REQUIRE(dbnethistory.toString() == expected);
            //check if there is really row was deleted
        }
    }
    else if ( n > 1)     // unreal situation
        FAIL("inserted more than one row");
    else if ( n == 0 )   // this could happen if there are some problems with db
        FAIL("nothing was inserted");

    //TODO check unreal situation
}

