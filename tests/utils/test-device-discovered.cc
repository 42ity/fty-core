#include <catch.hpp>

#include <device_discovered.h>

std::string urldev = "mysql:db=box_utf8;user=root";

std::string osnew       = utils::objectStatetoString(utils::ObjectState::OS_NEW);
std::string osdeleted   = utils::objectStatetoString(utils::ObjectState::OS_DELETED);
std::string osupdated   = utils::objectStatetoString(utils::ObjectState::OS_UPDATED);
std::string osselected  = utils::objectStatetoString(utils::ObjectState::OS_SELECTED);

TEST_CASE("Device discovered static","[dbdevice][clientname][getDetailedClientName]")
{
    REQUIRE(utils::DeviceDiscovered::getDetailedClientName() == MODULE_ADMIN);
}

TEST_CASE("Device discovered+getters","[dbdevice][constructor][toString][getId][getUrl][getState][getName]"){
    utils::DeviceDiscovered dbdevice(urldev);
    std::string expected =  "url="  + urldev                + ";" +
                            "id="   + std::to_string(-1)    + ";" +
                            "state=" + osnew                + ";" +
                            "name=" + "unknown";
    REQUIRE(dbdevice.toString() == expected );
    REQUIRE(dbdevice.getId() == -1 );
    REQUIRE(dbdevice.getUrl() == urldev );
    REQUIRE(utils::objectStatetoString(dbdevice.getState()) == osnew);
    REQUIRE(dbdevice.getName() == "unknown");
}

TEST_CASE("Device discovered+getters1","[dbdevice][constructor][toString][getId][getUrl][getState][getName]"){
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

TEST_CASE("Device discovered selectbyname","[dbdevice][select][byname]"){
    
    
    std::string newname = "select_device";
    
    std::vector<utils::DeviceDiscovered> dbdevices =  utils::DeviceDiscovered::selectByName(urldev,newname);

    REQUIRE(dbdevices.size() == 2);
   
    std::string expected1 = "url="  + urldev                + ";" +
                            "id="   + std::to_string(1)     + ";" +
                            "state=" + osselected           + ";" +
                            "name=" + newname;
    
    std::string expected2 = "url="  + urldev                + ";" +
                            "id="   + std::to_string(2)     + ";" +
                            "state=" + osselected           + ";" +
                            "name=" + newname;

    utils::DeviceDiscovered dbdevice1 = dbdevices.front();
    utils::DeviceDiscovered dbdevice2 = dbdevices.back();
    
    bool f1 =  (dbdevice1.toString() == expected1)  &&  ( dbdevice2.toString() == expected2);
    bool f2 =  (dbdevice1.toString() == expected2)  &&  ( dbdevice2.toString() == expected1);
    bool result = f1 || f2;
    REQUIRE(result);

    newname = "not_exist";
    dbdevices =  utils::DeviceDiscovered::selectByName(urldev,newname);
    REQUIRE(dbdevices.size() == 0 );

}


TEST_CASE("Device discovered setName","[dbdevice][setName]")
{
    utils::DeviceDiscovered dbdevice(urldev);
    std::string newname = "set_name";
    //OS_NEW set OS_NEW
    dbdevice.setName(newname);
    REQUIRE(dbdevice.getState() == utils::ObjectState::OS_NEW);
   
    std::vector<utils::DeviceDiscovered> dbdevices =  utils::DeviceDiscovered::selectByName(urldev,newname);
    //in DB must be one row for testing
    REQUIRE(dbdevices.size() > 0 );
    dbdevice = dbdevices.front();
    REQUIRE(dbdevice.getState() == utils::ObjectState::OS_SELECTED);

    //OS_SELECTED set= OS_SELECTED
    dbdevice.setName(newname);
    REQUIRE(dbdevice.getState() == utils::ObjectState::OS_SELECTED);
    //OS_SELECTED set!= OS_UPDATED
    dbdevice.setName(newname+newname);
    REQUIRE(dbdevice.getState() == utils::ObjectState::OS_UPDATED);
    //OS_UPDATED set= OS_UPDATED
    dbdevice.setName(newname+newname);
    REQUIRE(dbdevice.getState() == utils::ObjectState::OS_UPDATED);
    //OS_UPDATED set!= OS_UPDATED
    dbdevice.setName(newname+newname+newname);
    REQUIRE(dbdevice.getState() == utils::ObjectState::OS_UPDATED);
    
    dbdevices =  utils::DeviceDiscovered::selectByName(urldev,newname);
    dbdevice = dbdevices.front();
    dbdevice.dbdelete();
    
    //OS_DELETED set= OS_DELETED
    dbdevice.setName(newname);
    REQUIRE(dbdevice.getState() == utils::ObjectState::OS_DELETED);
    //OS_DELETED set!= OS_DELETED
    dbdevice.setName(newname+newname);
    REQUIRE(dbdevice.getState() == utils::ObjectState::OS_DELETED);
    
    //return to consistent state
    utils::DeviceDiscovered newdevice = utils::DeviceDiscovered(urldev,newname);
    int n = newdevice.dbsave();
    REQUIRE( n == 1 );
}


TEST_CASE("Device discovered insert/delete ","[dbdevice][save][insert][delete]"){
    utils::DeviceDiscovered dbdevice(urldev);
    std::string newname = "insert_delete";
    dbdevice.setName(newname);
    int n = dbdevice.dbsave();
    if ( n == 1 )
    {
        REQUIRE(dbdevice.getId() > 0 );
        REQUIRE( utils::objectStatetoString(dbdevice.getState()) == osselected);
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

    std::vector<utils::DeviceDiscovered> dbdevices =  utils::DeviceDiscovered::selectByName(urldev,newname);
    REQUIRE(dbdevices.size() == 0 );
}

TEST_CASE("Device discovered insert long name ","[dbdevice][save][insert][longname]"){
    utils::DeviceDiscovered dbdevice(urldev);
    
    std::string newname = "this should fail";
    for (int i = 0 ; i < dbdevice.getNamesLength() ; i++ )
        newname+="a";

    dbdevice.setName(newname);

    int n = dbdevice.dbsave();
    REQUIRE( n == 0 );
}


TEST_CASE("Device discovered update","[dbdevice][save][update]"){
    
    std::string newname = "update";
    utils::DeviceDiscovered dbdevice(urldev,newname);
    int n = dbdevice.dbsave();
    REQUIRE( n == 1 );
    n = dbdevice.selectById(dbdevice.getId());
    REQUIRE( n == 1 );
    
    dbdevice.setName(newname+newname);
    
    std::string expected = "id="    + std::to_string(dbdevice.getId())     + ";";
    
    n = dbdevice.dbsave();
    REQUIRE( n == 1 );
    REQUIRE(utils::objectStatetoString(dbdevice.getState()) == osselected);

    utils::DeviceDiscovered dbdeviceupd(urldev);
    n = dbdeviceupd.selectById(dbdevice.getId());
    REQUIRE(n == 1);
    
    REQUIRE( dbdeviceupd.toString() == dbdevice.toString());

    n = dbdevice.dbdelete();
    REQUIRE( n == 1 );
}


TEST_CASE("Device discovered select by Id","[dbdevice][select][byId]"){
    
    std::string newname = "select_by_id";
    utils::DeviceDiscovered dbdevice(urldev,newname);
    int n = dbdevice.dbsave();
    REQUIRE( n == 1 );

    // select one row
    utils::DeviceDiscovered dbdevicesel(urldev);
    n = dbdevicesel.selectById(dbdevice.getId());
    REQUIRE( n == 1 );
    REQUIRE(utils::objectStatetoString(dbdevicesel.getState()) == osselected );
    REQUIRE (dbdevice.toString() == dbdevicesel.toString());
     
    n = dbdevice.dbdelete();
    REQUIRE( n == 1 );
    
    //select nothing
    dbdevice = utils::DeviceDiscovered(urldev);
    n = dbdevice.selectById(111111);
    REQUIRE ( n == 0 );

}
//TODO
//clear
//destructor



