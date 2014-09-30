#include <catch.hpp>
#include "dbinit.h"
#include <clientinfo.h>

TEST_CASE("client info getters1","[dbclientinfo][constructor1][toString][getId][getUrl][getState][getClientId][getDiscoveredDeviceId][getBlobData][getClientName]"){
    utils::ClientInfo dbclientinfo(url);
    time_t tmp_t = dbclientinfo.getTimestamp();
    std::string timestr = ctime(&tmp_t);

    std::string expected =  "url="       + url                + ";" +
                            "id="        + std::to_string(-1) + ";" +
                            "state="     + osnew              + ";" +
                            "time="      + timestr            + ";" +
                            "clientId="  + std::to_string(-1) + ";" +
                            "clientName="+ ""                 + ";" +
                            "deviceId="  + std::to_string(-1) + ";" +
                            "blobdata="  + ""                 ;

    REQUIRE(dbclientinfo.toString() > "" );
    REQUIRE(dbclientinfo.getId() == -1 );
    REQUIRE(dbclientinfo.getUrl() == url );
    REQUIRE(utils::objectStatetoString(dbclientinfo.getState()) == osnew);
    REQUIRE(dbclientinfo.getClientId() == -1);
    REQUIRE(dbclientinfo.getDeviceDiscoveredId() == -1 );
    REQUIRE(dbclientinfo.getBlobData() == "");
    REQUIRE(dbclientinfo.getClientName() == "");
    REQUIRE( dbclientinfo.toString() == expected );
}

TEST_CASE("client info getters2","[dbclientinfo][constructor2][toString][getId][getUrl][getState][getClientId][getDiscoveredDeviceId][getBlobData][getClientName]"){
    //THIS ROW MUST BE IN DB
    std::string name = "nmap";
    utils::ClientInfo dbclientinfo(url,name);
    time_t tmp_t = dbclientinfo.getTimestamp();
    std::string timestr = ctime(&tmp_t);

    std::string expected =  "url="       + url                + ";" +
                            "id="        + std::to_string(-1) + ";" +
                            "state="     + osnew              + ";" +
                            "time="      + timestr            + ";" +
                            "clientId="  + std::to_string(1)  + ";" +
                            "clientName="+ name               + ";" +
                            "deviceId="  + std::to_string(-1) + ";" +
                            "blobdata="  + ""                 ;

    REQUIRE( dbclientinfo.toString() > "" );
    REQUIRE( dbclientinfo.getId() == -1 );
    REQUIRE( dbclientinfo.getUrl() == url );
    REQUIRE( utils::objectStatetoString(dbclientinfo.getState()) == osnew );
    REQUIRE( dbclientinfo.getClientId() == 1 );
    REQUIRE( dbclientinfo.getDeviceDiscoveredId() == -1 );
    REQUIRE( dbclientinfo.getBlobData() == "" );
    REQUIRE( dbclientinfo.getClientName() == name );
    REQUIRE( dbclientinfo.toString() == expected );
}

TEST_CASE("client info getters3","[dbclientinfo][constructor3][toString][getId][getUrl][getState][getClientId][getDiscoveredDeviceId][getBlobData][getClientName]"){
    // THIS ROW MUST BE IN DB
    std::string name = "nmap";
    int newid = 1;
    utils::ClientInfo dbclientinfo(url,newid);
    time_t tmp_t = dbclientinfo.getTimestamp();
    std::string timestr = ctime(&tmp_t);

    std::string expected =  "url="       + url                  + ";" +
                            "id="        + std::to_string(-1)   + ";" +
                            "state="     + osnew                + ";" +
                            "time="      + timestr              + ";" +
                            "clientId="  + std::to_string(newid)+ ";" +
                            "clientName="+ name                 + ";" +
                            "deviceId="  + std::to_string(-1)   + ";" +
                            "blobdata="  + ""                   ;

    REQUIRE( dbclientinfo.toString() > "" );
    REQUIRE( dbclientinfo.getId() == -1 );
    REQUIRE( dbclientinfo.getUrl() == url );
    REQUIRE( utils::objectStatetoString(dbclientinfo.getState()) == osnew );
    REQUIRE( dbclientinfo.getClientId() == newid );
    REQUIRE( dbclientinfo.getDeviceDiscoveredId() == -1 );
    REQUIRE( dbclientinfo.getBlobData() == "" );
    REQUIRE( dbclientinfo.getClientName() == name );
    REQUIRE( dbclientinfo.toString() == expected );
}

