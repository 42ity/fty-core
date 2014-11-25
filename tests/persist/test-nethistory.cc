#include <catch.hpp>
#include "dbinit.h"
#include "nethistory.h"
#include "cidr.h"

TEST_CASE("net history getters","[dbnethistory][constructor1][toString] \\
[getId][getUrl][getState][getTimestamp][getMac][getMask][getIp][getName] \\
[getCommand][getAddress][db]")
{
    persist::NetHistory dbnethistory(url);
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
    REQUIRE( persist::objectStatetoString(dbnethistory.getState()) == osnew );
    REQUIRE( dbnethistory.getMac() == "" );
    REQUIRE( dbnethistory.getMask() == -1 );
    REQUIRE( dbnethistory.getIp() == "" );
    REQUIRE( dbnethistory.getCommand() == 'z' );
    REQUIRE( dbnethistory.getName() == "" );
    REQUIRE( dbnethistory.getTimestamp() == tmp_t );
    REQUIRE( dbnethistory.toString() == expected );
    shared::CIDRAddress nn();
    //TODO doesn't work
    //REQUIRE( dbnethistory.getAddress() == nn );
}

TEST_CASE("net history insert/delete ","[dbnethistory][save][insert][delete][db]"){
    persist::NetHistory dbnethistory(url);
    std::string newname = "insert_delete";
    shared::CIDRAddress newaddress("1.1.1.1",8);
    std::string newmac = "12:34:56:78:91:11";
    dbnethistory.setName(newname);
    dbnethistory.setAddress(newaddress);
    dbnethistory.setMac(newmac);
    REQUIRE( dbnethistory.getMac() == newmac );
    dbnethistory.setCommand('a');

    int n = dbnethistory.dbsave();

    if ( n == 1 )
    {
        REQUIRE(dbnethistory.getId() > 0 );
        REQUIRE(persist::objectStatetoString(dbnethistory.getState()) == osinserted);
        std::string expected = dbnethistory.toString();
        n = dbnethistory.dbsave();
        REQUIRE( n == 0 );
        REQUIRE( dbnethistory.toString() == expected );
        
        int newidd = dbnethistory.getId();   // save id for further control 
        n = dbnethistory.dbdelete();
        if ( n == 0 )   // unreal situation
            FAIL("nothing was deleted"); 
        if ( n > 1 )    // unreal situation
            FAIL("more than one row was deleted");
        if ( n == 1 )
        {
            REQUIRE( persist::objectStatetoString(dbnethistory.getState()) == osdeleted);
            REQUIRE( dbnethistory.getId() == -1 );

            expected = dbnethistory.toString();
            n = dbnethistory.dbdelete();
            INFO(expected);
            REQUIRE( n == 0 );
            REQUIRE(dbnethistory.toString() == expected);
            
            persist::NetHistory newnh(url);
            n = newnh.selectById(newidd);
            REQUIRE( n == 0 );  //row must be deleted from the db
        }
    }
    else if ( n > 1)     // unreal situation
        FAIL("inserted more than one row");
    else if ( n == 0 )   // this could happen if there are some problems with db
        FAIL("nothing was inserted");
}



TEST_CASE("net history select by id ","[dbnethistory][select][byId][db]"){
    
    persist::NetHistory dbnethistory(url);
    std::string newname = "selectById";
    shared::CIDRAddress newaddress("1.2.3.4",8);
    std::string newmac = "11:22:33:AA:91:11";
    dbnethistory.setName(newname);
    dbnethistory.setAddress(newaddress);
    dbnethistory.setMac(newmac);
    dbnethistory.setCommand('a');

    int n = dbnethistory.dbsave();
    REQUIRE( n == 1 );
    int newidd = dbnethistory.getId();   // save id for further control

    persist::NetHistory newnh(url);
    n = newnh.selectById(newidd);
    INFO(newnh.toString());
    REQUIRE( n == 1 );
    n = newnh.dbdelete();
    REQUIRE( n == 1);
}

TEST_CASE("net history select all","[dbnethistory][select][getHistory][db]"){
    std::vector<persist::NetHistory> history = persist::NetHistory::getHistory(url);
    bool result = true;
    for ( auto i = history.begin(); i != history.end(); i++ ) {
        result = result && ( (*i).getId() >= 7 ) && ( (*i).getId() <=10 ) ;
    }
    REQUIRE (result);
    REQUIRE (history.size() > 0);
}
