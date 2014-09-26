#include <catch.hpp>
#include <iostream>

#include <devicetype.h>

std::string urldt = "mysql:db=box_utf8;user=root";

std::string osnewdt       = utils::objectStatetoString(utils::ObjectState::OS_NEW);
std::string osdeleteddt   = utils::objectStatetoString(utils::ObjectState::OS_DELETED);
std::string osupdateddt   = utils::objectStatetoString(utils::ObjectState::OS_UPDATED);
std::string osselecteddt  = utils::objectStatetoString(utils::ObjectState::OS_SELECTED);
std::string osinserteddt  = utils::objectStatetoString(utils::ObjectState::OS_INSERTED);

TEST_CASE("device type getters1","[dbdevtype][constructor1][toString][getId][getUrl][getState][getName]"){
    utils::DeviceType dbdevicetype(urldt);
    std::string expected =  "url="   + urldt                + ";" +
                            "id="    + std::to_string(-1)   + ";" +
                            "state=" + osnewdt              + ";" +
                            "name="  + ""                   ;
    REQUIRE(dbdevicetype.toString() == expected );
    REQUIRE(dbdevicetype.getId()    == -1 );
    REQUIRE(dbdevicetype.getUrl()   == urldt );
    REQUIRE(utils::objectStatetoString(dbdevicetype.getState()) == osnewdt);
    REQUIRE(dbdevicetype.getName() == "");
}

TEST_CASE("device type getters2","[dbdevtype][constructor2][toString][getId][getUrl][getState][getName]"){

    std::string name = "constructor";

    utils::DeviceType dbdevicetype(urldt,name);
    std::string expected =  "url="   + urldt                + ";" +
                            "id="    + std::to_string(-1)   + ";" +
                            "state=" + osnewdt              + ";" +
                            "name="  + name                 ;
    REQUIRE(dbdevicetype.toString() == expected );
    REQUIRE(dbdevicetype.getId()    == -1 );
    REQUIRE(dbdevicetype.getUrl()   == urldt );
    REQUIRE(utils::objectStatetoString(dbdevicetype.getState()) == osnewdt);
    REQUIRE(dbdevicetype.getName() == name);
}

TEST_CASE("device type selectbyname","[dbdevicetype][select][byName]"){
    //THIS RECORD SHOULD ALWAYS BEEN THERE
    std::string newname = "not_classified";
    //name is unique
    utils::DeviceType dbdevicetype =  utils::DeviceType(urldt);
    
    int n = dbdevicetype.selectByName(newname);
    REQUIRE(n == 1);
   
    std::string expected = "url="    + urldt                + ";" +
                            "id="    + std::to_string(1)    + ";" +
                            "state=" + osselecteddt           + ";" +
                            "name="  + newname              ;
    REQUIRE( dbdevicetype.toString() == expected );

    //not found
    expected = dbdevicetype.toString();
    n = dbdevicetype.selectByName("not found");
    REQUIRE(n == 0);
    REQUIRE(dbdevicetype.toString() == expected);
}

TEST_CASE("device type selectbyid","[dbdevicetype][select][byId]"){
    //THIS RECORD SHOULD ALWAYS BEEN THERE
    int newid = 1;
    std::string newname = "not_classified";
    //name is unique
    utils::DeviceType dbdevicetype =  utils::DeviceType(urldt);
    
    int n = dbdevicetype.selectById(newid);
    REQUIRE(n == 1);
   
    std::string expected = "url="    + urldt                 + ";" +
                           "id="     + std::to_string(newid) + ";" +
                           "state="  + osselecteddt            + ";" +
                           "name="   + newname               ;
    REQUIRE( dbdevicetype.toString() == expected );

    //not found
    expected = dbdevicetype.toString();
    n = dbdevicetype.selectById(111111);
    REQUIRE(n == 0);
    REQUIRE(dbdevicetype.toString() == expected);
}

TEST_CASE("device type selectId","[dbdevicetype][select][Id]"){
    std::string newname = "not_classified";
    
    int n  =  utils::DeviceType::selectId(urldt,newname);
    REQUIRE( n == 1); //it is an ID
    
    n  =  utils::DeviceType::selectId(urldt,"not found");
    REQUIRE( n == -1); //it is an ID
}

TEST_CASE("device type clear","[dbdevicetype][clear]"){

    std::string newname = "not_classified";
    
    utils::DeviceType  dbdevicetype =  utils::DeviceType(urldt);
    std::string expected = dbdevicetype.toString();
    
    int n = dbdevicetype.selectByName(newname);
    REQUIRE( n == 1);

    dbdevicetype.clear();
    REQUIRE(dbdevicetype.toString() == expected);
}

TEST_CASE("device type setName","[dbdevicetype][setName]")
{
    utils::DeviceType dbdevicetype(urldt);
    std::string newname = "set_name";
    //OS_NEW set OS_NEW
    dbdevicetype.setName(newname);
    REQUIRE(dbdevicetype.getState() == utils::ObjectState::OS_NEW);
    REQUIRE(dbdevicetype.getId() == -1 );
    
    //insert test info
    utils::DeviceType newdevicetype = utils::DeviceType(urldt);
    newdevicetype.setName(newname);
    int n = newdevicetype.dbsave();
    REQUIRE( n == 1 );

    n = dbdevicetype.selectByName(newname);
    REQUIRE( n == 1 );    //in DB must be one row for testing
    REQUIRE(dbdevicetype.getState() == utils::ObjectState::OS_SELECTED);

    //OS_SELECTED set= OS_SELECTED
    dbdevicetype.setName(newname);
    REQUIRE(dbdevicetype.getState() == utils::ObjectState::OS_SELECTED);
    //OS_SELECTED set!= OS_UPDATED
    dbdevicetype.setName(newname+newname);
    REQUIRE(dbdevicetype.getState() == utils::ObjectState::OS_UPDATED);
    REQUIRE(dbdevicetype.getName() == newname+newname);
    //OS_UPDATED set = OS_UPDATED
    dbdevicetype.setName(newname+newname);
    REQUIRE(dbdevicetype.getState() == utils::ObjectState::OS_UPDATED);
    REQUIRE(dbdevicetype.getName() == newname+newname);
    //OS_UPDATED set!= OS_UPDATED
    dbdevicetype.setName(newname+newname+newname);
    REQUIRE(dbdevicetype.getState() == utils::ObjectState::OS_UPDATED);
    REQUIRE(dbdevicetype.getName() == newname+newname+newname);
    
    n = dbdevicetype.dbdelete();
    REQUIRE ( n == 1 );

    //OS_DELETED set= OS_DELETED
    std::string oldname = dbdevicetype.getName();
    dbdevicetype.setName(newname);
    REQUIRE(dbdevicetype.getState() == utils::ObjectState::OS_DELETED);
    REQUIRE(dbdevicetype.getName() == oldname);
    //OS_DELETED set!= OS_DELETED
    dbdevicetype.setName(newname+newname);
    REQUIRE(dbdevicetype.getState() == utils::ObjectState::OS_DELETED);
    REQUIRE(dbdevicetype.getName() == oldname);
}

TEST_CASE("device type reload ","[dbdevicetype][reload]"){
    
    utils::DeviceType dbdevicetype(urldt);
    std::string newname = "reload";
    dbdevicetype.setName(newname);
    std::string expected = dbdevicetype.toString();
    
    int n = dbdevicetype.reload();
    INFO("OS_NEW");
    REQUIRE(n == 0);
    REQUIRE(dbdevicetype.toString() == expected );

    n = dbdevicetype.dbsave();
    REQUIRE( n == 1);
    expected = dbdevicetype.toString();

    INFO(dbdevicetype.toString());
    n = dbdevicetype.reload();
    INFO("OS_SELECTED");
    REQUIRE( n == 0);
    REQUIRE(dbdevicetype.toString() == expected );

    dbdevicetype.setName(newname+newname);
    n = dbdevicetype.reload();
    INFO("OS_UPDATED");
    REQUIRE( n == 1);
    REQUIRE(dbdevicetype.toString() == expected );
    
    n = dbdevicetype.dbdelete();
    REQUIRE( n == 1);
    expected = dbdevicetype.toString();

    n = dbdevicetype.reload();
    INFO("OS_DELETED");
    REQUIRE( n == 0);
    REQUIRE(dbdevicetype.toString() == expected );
}


TEST_CASE("device type insert/delete ","[dbdevicetype][save][insert][delete]"){
    utils::DeviceType dbdevicetype(urldt);
    std::string newname = "insert_delete";
    dbdevicetype.setName(newname);
    int n = dbdevicetype.dbsave();
    std::string expected;
    if ( n == 1 )
    {
        REQUIRE(dbdevicetype.getId() > 0 );
        REQUIRE(utils::objectStatetoString(dbdevicetype.getState()) == osselecteddt);
        expected = dbdevicetype.toString();
        n = dbdevicetype.dbsave();
        REQUIRE( n == 0);
        REQUIRE(dbdevicetype.toString() == expected);
        
        n = dbdevicetype.dbdelete();
        if ( n == 0 )   // unreal situation
            FAIL("nothing was deleted");
        if ( n > 1 )    // unreal situation
            FAIL("more than one row was deleted");
        if ( n == 1 )
        {
            REQUIRE( utils::objectStatetoString(dbdevicetype.getState()) == osdeleteddt);
            REQUIRE( dbdevicetype.getId() == -1 );

            expected = dbdevicetype.toString();
            n = dbdevicetype.dbdelete();
            INFO(expected);
            REQUIRE( n == 0 );
            REQUIRE(dbdevicetype.toString() == expected);
            n = dbdevicetype.selectByName(newname);
            REQUIRE( n == 0 );
        }
    }
    else if ( n > 1)     // unreal situation
        FAIL("inserted more than one row");
    else if ( n == 0 )   // this could happen if there are some problems with db
        FAIL("nothing was inserted");
}

TEST_CASE("device type insert long name ","[dbdevicetype][save][insert][longname]"){
    utils::DeviceType dbdevicetype(urldt);
    
    std::string newname = "this should fail";
    for (int i = 0 ; i < dbdevicetype.getNamesLength() ; i++ )
        newname+="a";

    dbdevicetype.setName(newname);

    int n = dbdevicetype.dbsave();
    REQUIRE( n == 0 );
}

TEST_CASE("device type update","[dbdevicetype][save][update]"){
    
    std::string newname = "update";
    utils::DeviceType dbdevicetype(urldt,newname);
    int n = dbdevicetype.dbsave();
    REQUIRE( n == 1 );
    
    int newid = dbdevicetype.getId();
    n = dbdevicetype.selectById(newid);
    REQUIRE( n == 1 );
    
    dbdevicetype.setName(newname+newname);
    REQUIRE(utils::objectStatetoString(dbdevicetype.getState()) == osupdateddt);
    
    n = dbdevicetype.dbsave();
    REQUIRE( n == 1 );
    REQUIRE(utils::objectStatetoString(dbdevicetype.getState()) == osselecteddt);
    std::string expected = "url="    + urldt                 + ";" +
                           "id="     + std::to_string(newid) + ";" +
                           "state="  + osselecteddt            + ";" +
                           "name="   + newname+newname               ;
    REQUIRE(dbdevicetype.toString() == expected );

    n = dbdevicetype.selectById(newid);
    REQUIRE( n == 1 );
    REQUIRE(dbdevicetype.toString() == expected );

    n = dbdevicetype.dbdelete();
    REQUIRE( n == 1 );
}
