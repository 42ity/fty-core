#include <catch.hpp>
#include <iostream>
#include "dbinit.h"
#include <device_discovered.h>

TEST_CASE("Device discovered static","[dbdevice][clientname][getDetailedClientName][2]")
{

    REQUIRE(utils::DeviceDiscovered::getDetailedClientName() == MODULE_ADMIN);
}

TEST_CASE("Device discovered+getters1","[dbdevice][constructor1][toString][getId][getUrl][getState][getName][getDeviceTypeId][3]")
{
    utils::DeviceDiscovered dbdevice(url);

    std::string expected =  "url="          + url                + ";" +
                            "id="           + std::to_string(-1) + ";" +
                            "state="        + osnew              + ";" +
                            "name="         + "unknown"          + ";" +
                            "devicetypeid=" + std::to_string(-1) ;

    REQUIRE( dbdevice.toString() == expected );
    REQUIRE( dbdevice.getId() == -1 );
    REQUIRE( dbdevice.getUrl() == url );
    REQUIRE( utils::objectStatetoString(dbdevice.getState()) == osnew );
    REQUIRE( dbdevice.getName() == "unknown" );
    REQUIRE( dbdevice.getDeviceTypeId() == -1 );
}

TEST_CASE("Device discovered+getters2","[dbdevice][constructor2][toString][getId][getUrl][getState][getName][getDeviceTypeId][4]")
{
    std::string name = "myname";
    utils::DeviceDiscovered dbdevice(url, name);

    std::string expected =  "url="          + url                + ";" +
                            "id="           + std::to_string(-1) + ";" +
                            "state="        + osnew              + ";" +
                            "name="         + name               + ";" +
                            "devicetypeid=" + std::to_string(-1) ;

    REQUIRE( dbdevice.toString() == expected );
    REQUIRE( dbdevice.getId() == -1 );
    REQUIRE( dbdevice.getUrl() == url );
    REQUIRE( utils::objectStatetoString(dbdevice.getState()) == osnew );
    REQUIRE( dbdevice.getName() == name );
    REQUIRE( dbdevice.getDeviceTypeId() == -1 );
}

TEST_CASE("Device discovered selectbyname","[dbdevice][select][byName][5]")
{    
    std::string newname = "select_device";
    
    std::vector <utils::DeviceDiscovered> dbdevices =  
                utils::DeviceDiscovered::selectByName(url, newname);

    REQUIRE( dbdevices.size() == 2 );
   
    std::string expected1 = "url="          + url                + ";" +
                            "id="           + std::to_string(1)  + ";" +
                            "state="        + osselected         + ";" +
                            "name="         + newname            + ";" +
                            "devicetypeid=" + std::to_string(1)  ;
    
    std::string expected2 = "url="          + url               + ";" +
                            "id="           + std::to_string(2) + ";" +
                            "state="        + osselected        + ";" +
                            "name="         + newname           + ";" +
                            "devicetypeid=" + std::to_string(1) ;

    utils::DeviceDiscovered dbdevice1 = dbdevices.front();
    utils::DeviceDiscovered dbdevice2 = dbdevices.back();
    INFO(dbdevice1.toString());
    INFO(dbdevice2.toString());
    
    bool f1 =  (dbdevice1.toString() == expected1)  
        &&  ( dbdevice2.toString() == expected2);
    bool f2 =  (dbdevice1.toString() == expected2)  
        &&  ( dbdevice2.toString() == expected1);
    bool result = f1 || f2;
    REQUIRE(result);

    newname = "not_exist";
    dbdevices =  utils::DeviceDiscovered::selectByName(url,newname);
    REQUIRE(dbdevices.size() == 0 );
}


TEST_CASE("Device discovered setName/setDeviceType","[dbdevice][setName][setDeviceType][2]")
{
    utils::DeviceDiscovered dbdevice(url);
    std::string newname = "set_name";
    //OS_NEW set OS_NEW
    dbdevice.setName(newname);
    REQUIRE(dbdevice.getState() == utils::ObjectState::OS_NEW);
    REQUIRE(dbdevice.getId() == -1 );
    
    //insert test info
    utils::DeviceDiscovered newdevice = utils::DeviceDiscovered(url);
    newdevice.setName(newname);
    newdevice.setDeviceTypeId(1);
    int n = newdevice.dbsave();
    REQUIRE( n == 1 );

    std::vector<utils::DeviceDiscovered> dbdevices =  utils::DeviceDiscovered::selectByName(url,newname);
    //in DB must be one row for testing
    REQUIRE(dbdevices.size() == 1);
    dbdevice = dbdevices.front();
    REQUIRE(dbdevice.getState() == utils::ObjectState::OS_SELECTED);

    //OS_SELECTED set= OS_SELECTED
    dbdevice.setName(newname);
    REQUIRE(dbdevice.getState() == utils::ObjectState::OS_SELECTED);
    //OS_SELECTED set!= OS_UPDATED
    dbdevice.setName(newname+newname);
    REQUIRE(dbdevice.getState() == utils::ObjectState::OS_UPDATED);
    REQUIRE(dbdevice.getName() == newname+newname);
    //OS_UPDATED set= OS_UPDATED
    dbdevice.setName(newname+newname);
    REQUIRE(dbdevice.getState() == utils::ObjectState::OS_UPDATED);
    REQUIRE(dbdevice.getName() == newname+newname);
    //OS_UPDATED set!= OS_UPDATED
    dbdevice.setName(newname+newname+newname);
    REQUIRE(dbdevice.getState() == utils::ObjectState::OS_UPDATED);
    REQUIRE(dbdevice.getName() == newname+newname+newname);

    n = dbdevice.dbdelete();
    REQUIRE ( n == 1 );
    
    //OS_DELETED set= OS_DELETED  
    std::string oldname = dbdevice.getName();
    dbdevice.setName(newname);
    REQUIRE(dbdevice.getState() == utils::ObjectState::OS_DELETED);
    REQUIRE(dbdevice.getName() == oldname);
    //OS_DELETED set!= OS_DELETED
    dbdevice.setName(newname+newname);
    REQUIRE(dbdevice.getState() == utils::ObjectState::OS_DELETED);
    REQUIRE(dbdevice.getName() == oldname);
    }

TEST_CASE("Device discovered insert/delete ","[dbdevice][save][insert][delete][3]"){
    utils::DeviceDiscovered dbdevice(url);
    std::string newname = "insert_delete";
    dbdevice.setName(newname);
    dbdevice.setDeviceTypeId(1);
    int n = dbdevice.dbsave();
    if ( n == 1 )
    {
        REQUIRE(dbdevice.getId() > 0 );
        REQUIRE(utils::objectStatetoString(dbdevice.getState()) == osselected);
    }
    if ( n > 1)     // unreal situation
        FAIL("inserted more than one row");
    if ( n == 0 )    // this could happen if there are some problems with db
        FAIL("nothing was inserted");

    n = dbdevice.dbdelete();
    if ( n == 0 )
        FAIL("nothing was deleted");
    if ( n > 1 )
        FAIL("more than one row was deleted");
    if ( n == 1 )
        REQUIRE( utils::objectStatetoString(dbdevice.getState()) == osdeleted);

    std::vector<utils::DeviceDiscovered> dbdevices =  utils::DeviceDiscovered::selectByName(url,newname);
    REQUIRE(dbdevices.size() == 0 );
}

TEST_CASE("Device discovered insert long name ","[dbdevice][save][insert][longname]"){
    utils::DeviceDiscovered dbdevice(url);
    
    std::string newname = "this should fail";
    for (unsigned int i = 0 ; i < dbdevice.getNamesLength() ; i++ )
        newname+="a";

    dbdevice.setName(newname);

    int n = dbdevice.dbsave();
    REQUIRE( n == 0 );
}


TEST_CASE("Device discovered update","[dbdevice][save][update]"){
    
    std::string newname = "update";
    utils::DeviceDiscovered dbdevice(url,newname);
    int n = dbdevice.dbsave();
    REQUIRE(n == 0 );
    dbdevice.setDeviceTypeId(1);
    n = dbdevice.dbsave();
    REQUIRE( n == 1 );
    n = dbdevice.selectById(dbdevice.getId());
    REQUIRE( n == 1 );
    
    dbdevice.setName(newname+newname);
    
    std::string expected = "id="    + std::to_string(dbdevice.getId())     + ";";
    
    n = dbdevice.dbsave();
    REQUIRE( n == 1 );
    REQUIRE(utils::objectStatetoString(dbdevice.getState()) == osselected);

    utils::DeviceDiscovered dbdeviceupd(url);
    n = dbdeviceupd.selectById(dbdevice.getId());
    REQUIRE(n == 1);
    
    REQUIRE( dbdeviceupd.toString() == dbdevice.toString());

    n = dbdevice.dbdelete();
    REQUIRE( n == 1 );
}


TEST_CASE("Device discovered select by Id","[dbdevice][select][byId][1]")
{
    
    std::string newname = "select_by_id";
    utils::DeviceDiscovered dbdevice(url,newname);
    int n = dbdevice.dbsave();
    REQUIRE( n == 0 );
    dbdevice.setDeviceTypeId(1);
    n = dbdevice.dbsave();
    REQUIRE( n == 1 );
    
    // select one row
    utils::DeviceDiscovered dbdevicesel(url);
    n = dbdevicesel.selectById(dbdevice.getId());
    REQUIRE( n == 1 );
    REQUIRE(utils::objectStatetoString(dbdevicesel.getState()) == osselected );
    REQUIRE (dbdevice.toString() == dbdevicesel.toString());
     
    n = dbdevice.dbdelete();
    REQUIRE( n == 1 );
    
    //select nothing
    dbdevice = utils::DeviceDiscovered(url);
    n = dbdevice.selectById(111111);
    REQUIRE ( n == 0 );
}
//TODO
//clear
//destructor



