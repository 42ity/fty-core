#include <catch.hpp>
#include <iostream>
#include "dbinit.h"
#include <client.h>

TEST_CASE("client getters1","[dbdevtype][constructor1][toString][getId][getUrl][getState][getName]"){
    utils::Client dbclient(url);
    std::string expected =  "url="   + url                + ";" +
                            "id="    + std::to_string(-1) + ";" +
                            "state=" + osnew              + ";" +
                            "name="  + ""                 ;
    REQUIRE( dbclient.toString() == expected );
    REQUIRE( dbclient.getId()    == -1 );
    REQUIRE( dbclient.getUrl()   == url );
    REQUIRE( utils::objectStatetoString(dbclient.getState()) == osnew );
    REQUIRE( dbclient.getName() == "" );
}

TEST_CASE("client getters2","[dbdevtype][constructor2][toString][getId][getUrl][getState][getName]"){
    std::string name = "constructor";

    utils::Client dbclient(url,name);
    std::string expected =  "url="   + url                + ";" +
                            "id="    + std::to_string(-1) + ";" +
                            "state=" + osnew              + ";" +
                            "name="  + name               ;
    REQUIRE(dbclient.toString() == expected );
    REQUIRE(dbclient.getId()    == -1 );
    REQUIRE(dbclient.getUrl()   == url );
    REQUIRE(utils::objectStatetoString(dbclient.getState()) == osnew);
    REQUIRE(dbclient.getName() == name);
}

TEST_CASE("client selectbyname","[dbclient][select][byName]")
{
    //THIS RECORD SHOULD ALWAYS BEEN THERE
    std::string newname = "nmap";
    //name is unique
    utils::Client dbclient =  utils::Client(url);
    
    int n = dbclient.selectByName(newname);
    REQUIRE(n == 1);
   
    std::string expected = "url="    + url                + ";" +
                            "id="    + std::to_string(1)  + ";" +
                            "state=" + osselected         + ";" +
                            "name="  + newname            ;
    REQUIRE( dbclient.toString() == expected );

    //not found
    expected = dbclient.toString();
    n = dbclient.selectByName("not found");
    REQUIRE(n == 0);
    REQUIRE(dbclient.toString() == expected);
}

TEST_CASE("client selectbyid","[dbclient][select][byId]"){
    //THIS RECORD SHOULD ALWAYS BEEN THERE
    int newid = 1;
    std::string newname = "nmap";
    //name is unique
    utils::Client dbclient =  utils::Client(url);
    
    int n = dbclient.selectById(newid);
    REQUIRE(n == 1);
   
    std::string expected = "url="    + url                 + ";" +
                           "id="     + std::to_string(newid) + ";" +
                           "state="  + osselected            + ";" +
                           "name="   + newname               ;
    REQUIRE( dbclient.toString() == expected );

    //not found
    expected = dbclient.toString();
    n = dbclient.selectById(111111);
    REQUIRE(n == 0);
    REQUIRE(dbclient.toString() == expected);
}

TEST_CASE("client selectId","[dbclient][select][Id]"){
    std::string newname = "nmap";
    
    int n  =  utils::Client::selectId(url,newname);
    REQUIRE( n == 1); //it is an ID
    
    n  =  utils::Client::selectId(url,"not found");
    REQUIRE( n == -1); //it is an ID
}

TEST_CASE("client clear","[dbclient][clear]"){

    std::string newname = "nmap";
    
    utils::Client  dbclient =  utils::Client(url);
    std::string expected = dbclient.toString();
    
    int n = dbclient.selectByName(newname);
    REQUIRE( n == 1);

    dbclient.clear();
    REQUIRE(dbclient.toString() == expected);
}

TEST_CASE("client setName","[dbclient][setName]")
{
    utils::Client dbclient(url);
    std::string newname = "set_name";
    //OS_NEW set OS_NEW
    dbclient.setName(newname);
    REQUIRE(dbclient.getState() == utils::ObjectState::OS_NEW);
    REQUIRE(dbclient.getId() == -1 );
    
    //insert test info
    utils::Client newclient = utils::Client(url);
    newclient.setName(newname);
    int n = newclient.dbsave();
    REQUIRE( n == 1 );

    n = dbclient.selectByName(newname);
    REQUIRE( n == 1 );    //in DB must be one row for testing
    REQUIRE(dbclient.getState() == utils::ObjectState::OS_SELECTED);

    //OS_SELECTED set= OS_SELECTED
    dbclient.setName(newname);
    REQUIRE(dbclient.getState() == utils::ObjectState::OS_SELECTED);
    //OS_SELECTED set!= OS_UPDATED
    dbclient.setName(newname+newname);
    REQUIRE(dbclient.getState() == utils::ObjectState::OS_UPDATED);
    REQUIRE(dbclient.getName() == newname+newname);
    //OS_UPDATED set = OS_UPDATED
    dbclient.setName(newname+newname);
    REQUIRE(dbclient.getState() == utils::ObjectState::OS_UPDATED);
    REQUIRE(dbclient.getName() == newname+newname);
    //OS_UPDATED set!= OS_UPDATED
    dbclient.setName(newname+newname+newname);
    REQUIRE(dbclient.getState() == utils::ObjectState::OS_UPDATED);
    REQUIRE(dbclient.getName() == newname+newname+newname);
    
    n = dbclient.dbdelete();
    REQUIRE ( n == 1 );

    //OS_DELETED set= OS_DELETED
    std::string oldname = dbclient.getName();
    dbclient.setName(newname);
    REQUIRE(dbclient.getState() == utils::ObjectState::OS_DELETED);
    REQUIRE(dbclient.getName() == oldname);
    //OS_DELETED set!= OS_DELETED
    dbclient.setName(newname+newname);
    REQUIRE(dbclient.getState() == utils::ObjectState::OS_DELETED);
    REQUIRE(dbclient.getName() == oldname);
}

TEST_CASE("client reload ","[dbclient][reload]"){
    
    utils::Client dbclient(url);
    std::string newname = "reload";
    dbclient.setName(newname);
    std::string expected = dbclient.toString();
    
    int n = dbclient.reload();
    INFO("OS_NEW");
    REQUIRE(n == 0);
    REQUIRE(dbclient.toString() == expected );

    n = dbclient.dbsave();
    REQUIRE( n == 1);
    expected = dbclient.toString();

    INFO(dbclient.toString());
    n = dbclient.reload();
    INFO("OS_SELECTED");
    REQUIRE( n == 0);
    REQUIRE(dbclient.toString() == expected );

    dbclient.setName(newname+newname);
    n = dbclient.reload();
    INFO("OS_UPDATED");
    REQUIRE( n == 1);
    REQUIRE(dbclient.toString() == expected );
    
    n = dbclient.dbdelete();
    REQUIRE( n == 1);
    expected = dbclient.toString();

    n = dbclient.reload();
    INFO("OS_DELETED");
    REQUIRE( n == 0);
    REQUIRE(dbclient.toString() == expected );
}


TEST_CASE("client insert/delete ","[dbclient][save][insert][delete]"){
    utils::Client dbclient(url);
    std::string newname = "insert_delete";
    dbclient.setName(newname);
    int n = dbclient.dbsave();
    std::string expected;
    if ( n == 1 )
    {
        REQUIRE(dbclient.getId() > 0 );
        REQUIRE(utils::objectStatetoString(dbclient.getState()) == osselected);
        expected = dbclient.toString();
        n = dbclient.dbsave();
        REQUIRE( n == 0);
        REQUIRE(dbclient.toString() == expected);
        
        n = dbclient.dbdelete();
        if ( n == 0 )   // unreal situation
            FAIL("nothing was deleted"); 
        if ( n > 1 )    // unreal situation
            FAIL("more than one row was deleted");
        if ( n == 1 )
        {
            REQUIRE( utils::objectStatetoString(dbclient.getState()) == osdeleted);
            REQUIRE( dbclient.getId() == -1 );

            expected = dbclient.toString();
            n = dbclient.dbdelete();
            INFO(expected);
            REQUIRE( n == 0 );
            REQUIRE(dbclient.toString() == expected);
            n = dbclient.selectByName(newname);
            REQUIRE( n == 0 );
        }
    }
    else if ( n > 1)     // unreal situation
        FAIL("inserted more than one row");
    else if ( n == 0 )   // this could happen if there are some problems with db
        FAIL("nothing was inserted");
}

TEST_CASE("client insert long name ","[dbclient][save][insert][longname]"){
    utils::Client dbclient(url);
    
    std::string newname = "this should fail";
    for (unsigned int i = 0 ; i < dbclient.getNamesLength() ; i++ )
        newname+="a";

    dbclient.setName(newname);

    int n = dbclient.dbsave();
    REQUIRE( n == 0 );
}

TEST_CASE("client update","[dbclient][save][update]"){
    
    std::string newname = "update";
    utils::Client dbclient(url,newname);
    int n = dbclient.dbsave();
    REQUIRE( n == 1 );
    
    int newid = dbclient.getId();
    n = dbclient.selectById(newid);
    REQUIRE( n == 1 );
    
    dbclient.setName(newname+newname);
    REQUIRE(utils::objectStatetoString(dbclient.getState()) == osupdated);
    
    n = dbclient.dbsave();
    REQUIRE( n == 1 );
    REQUIRE(utils::objectStatetoString(dbclient.getState()) == osselected);
    std::string expected = "url="    + url                 + ";" +
                           "id="     + std::to_string(newid) + ";" +
                           "state="  + osselected            + ";" +
                           "name="   + newname+newname               ;
    REQUIRE(dbclient.toString() == expected );

    n = dbclient.selectById(newid);
    REQUIRE( n == 1 );
    REQUIRE(dbclient.toString() == expected );

    n = dbclient.dbdelete();
    REQUIRE( n == 1 );
}
