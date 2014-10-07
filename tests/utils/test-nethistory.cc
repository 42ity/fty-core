#include <catch.hpp>
#include "dbinit.h"
#include <nethistory.h>

TEST_CASE("net history getters1","[dbnethistory][constructor1][toString] \\
[getId][getUrl][getState][getTimestamp][getMac][getMask][getIp][getName] \\
[getCommand]")
{
    utils::NetHistory dbnethistory(url);
    time_t tmp_t = dbnethistory.getTimestamp();
    std::string timestr = ctime(&tmp_t);

    std::string expected =  "url="       + url                + ";" +
                            "id="        + std::to_string(-1) + ";" +
                            "state="     + osnew              + ";" +
                            "time="      + timestr            + ";" +
                            "mac="       + ""                 + ";" +
                            "mask="      + std::to_string(0)  + ";" +
                            "ip="        + ""                 + ";" +
                            "command="   + "z"                + ";" +
                            "name="      + ""                 ;

    REQUIRE( dbnethistory.getId() == -1 );
    REQUIRE( dbnethistory.getUrl() == url );
    REQUIRE( utils::objectStatetoString(dbnethistory.getState()) == osnew );
    REQUIRE( dbnethistory.getMac() == "" );
    REQUIRE( dbnethistory.getMask() == 0 );
    REQUIRE( dbnethistory.getIp() == "" );
    REQUIRE( dbnethistory.getCommand() == 'z' );
    REQUIRE( dbnethistory.getName() == "" );
    REQUIRE( dbnethistory.getTimestamp() == tmp_t );
    REQUIRE( dbnethistory.toString() == expected );
}

TEST_CASE("net history getters2","[dbnethistory][constructor1][toString][getId][getUrl][getState][getClientId][getDiscoveredDeviceId][getBlobData][getClientName]"){
    utils::NetHistory dbnethistory(url);
    time_t tmp_t = dbnethistory.getTimestamp();
    std::string timestr = ctime(&tmp_t);

    std::string expected =  "url="       + url                + ";" +
                            "id="        + std::to_string(-1) + ";" +
                            "state="     + osnew              + ";" +
                            "time="      + timestr            + ";" +
                            "mac="  + "" + ";" +
                            "mask="+ std::to_string(0)                 + ";" +
                            "ip="  + "" + ";" +
                            "command="  + "a"                 + ";"+
                            "name="  + ""                 ;
    dbnethistory.setMac("123456789AD");
    dbnethistory.setCommand('a');
    int n = dbnethistory.dbsave();
    REQUIRE( n == 1);
}

/*
TEST_CASE("net history getters2","[dbnethistory][constructor2][toString][getId][getUrl][getState][getClientId][getDiscoveredDeviceId][getBlobData][getClientName]"){
    //THIS ROW MUST BE IN DB
    std::string name = "nmap";
    utils::NetHistory dbnethistory(url,name);
    time_t tmp_t = dbnethistory.getTimestamp();
    std::string timestr = ctime(&tmp_t);

    std::string expected =  "url="       + url                + ";" +
                            "id="        + std::to_string(-1) + ";" +
                            "state="     + osnew              + ";" +
                            "time="      + timestr            + ";" +
                            "clientId="  + std::to_string(1)  + ";" +
                            "clientName="+ name               + ";" +
                            "deviceId="  + std::to_string(-1) + ";" +
                            "blobdata="  + ""                 ;

    REQUIRE( dbnethistory.toString() > "" );
    REQUIRE( dbnethistory.getId() == -1 );
    REQUIRE( dbnethistory.getUrl() == url );
    REQUIRE( utils::objectStatetoString(dbnethistory.getState()) == osnew );
    REQUIRE( dbnethistory.getClientId() == 1 );
    REQUIRE( dbnethistory.getDeviceDiscoveredId() == -1 );
    REQUIRE( dbnethistory.getBlobData() == "" );
    REQUIRE( dbnethistory.getClientName() == name );
    REQUIRE( dbnethistory.toString() == expected );
}

TEST_CASE("net history getters3","[dbnethistory][constructor3][toString][getId][getUrl][getState][getClientId][getDiscoveredDeviceId][getBlobData][getClientName]"){
    // THIS ROW MUST BE IN DB
    std::string name = "nmap";
    int newid = 1;
    utils::NetHistory dbnethistory(url,newid);
    time_t tmp_t = dbnethistory.getTimestamp();
    std::string timestr = ctime(&tmp_t);

    std::string expected =  "url="       + url                  + ";" +
                            "id="        + std::to_string(-1)   + ";" +
                            "state="     + osnew                + ";" +
                            "time="      + timestr              + ";" +
                            "clientId="  + std::to_string(newid)+ ";" +
                            "clientName="+ name                 + ";" +
                            "deviceId="  + std::to_string(-1)   + ";" +
                            "blobdata="  + ""                   ;

    REQUIRE( dbnethistory.toString() > "" );
    REQUIRE( dbnethistory.getId() == -1 );
    REQUIRE( dbnethistory.getUrl() == url );
    REQUIRE( utils::objectStatetoString(dbnethistory.getState()) == osnew );
    REQUIRE( dbnethistory.getClientId() == newid );
    REQUIRE( dbnethistory.getDeviceDiscoveredId() == -1 );
    REQUIRE( dbnethistory.getBlobData() == "" );
    REQUIRE( dbnethistory.getClientName() == name );
    REQUIRE( dbnethistory.toString() == expected );
}
*/

