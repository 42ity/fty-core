#include "nut-driver.h"

#include <nutclient.h>
#include <iostream>
#include <algorithm>

using namespace std;

static const std::vector<std::string> physicsNUT {
    "ups.temperature",
    "ups.load",
    "ups.realpower",
    "output.voltage",
    "output.current",
    "output.L1-N.voltage",
    "output.L1.realpower",
    "output.L1.current",
    "output.L2-N.voltage",
    "output.L2.realpower",
    "output.L2.current",
    "output.L3-N.voltage",
    "output.L3.realpower",
    "output.L3.current",
    "batery.charge",
    "outlet.realpower"
};

static const std::vector<std::string> physicsBIOS {
    "ups.temperature",
    "ups.load",
    "ups.realpower",
    "output.voltage",
    "output.current",
    "output.L1-N.voltage",
    "output.L1.realpower",
    "output.L1.current",
    "output.L2-N.voltage",
    "output.L2.realpower",
    "output.L2.current",
    "output.L3-N.voltage",
    "output.L3.realpower",
    "output.L3.current",
    "batery.charge",
    "outlet.realpower"
};

static const std::vector<std::string> inventoryNUT {
    "device.model",
    "device.mfr",
    "device.serial",
    "device.type",
    "device.description",
    "device.contact",
    "device.location",
    "device.part",
    "ups.status",
    "ups.alarm",
    "ups.serial",
    "battery.date",
    "battery.type"
};

static const std::vector<std::string> inventoryBIOS {
    "device.model",
    "device.mfr",
    "device.serial",
    "device.type",
    "device.description",
    "device.contact",
    "device.location",
    "device.part",
    "ups.status",
    "ups.alarm",
    "ups.serial",
    "battery.date",
    "battery.type"
};

NUTUPS::NUTUPS() {  
    _change = false;
    _name = "";
}

NUTUPS::NUTUPS(std::string aName) {  
    _change = false;
    name(aName);
}

void NUTUPS::name(const std::string aName) {
    _name = aName;
}

std::string NUTUPS::name() {
    return _name;
}

bool NUTUPS::changed() {
    return _change;
}

void NUTUPS::updatePhysics(std::string varName, float newValue) {
    long int newValueInt = ((newValue * 100) + 0.5);
    if( _physics.count( varName ) == 0 ) {
        // this is new value
        _physics[ varName ] = newValueInt;
        _change = true;
    } else {
        long int oldValue = _physics[ varName ];
        if( oldValue == newValueInt ) return ;
        try {
            if( abs( oldValue - newValueInt ) * 100 / oldValue > 5 ) {
                // significant change
                _physics[ varName ] = newValueInt;
                _change = true;
            }
        } catch(...) {
            // probably division by 0
            _physics[ varName ] = newValueInt;
            _change = true;     
        }
    }
}

void NUTUPS::updatePhysics(std::string varName, std::vector<std::string> values) {
    if( values.size() == 1 ) {
        // don't know how to handle multiple values
        // multiple values would be probably nonsence
        try {
            float value = std::stof(values[0]);
            updatePhysics(varName,value);
        } catch (...) {}
    }
}

void NUTUPS::updateInventory(std::string varName, std::vector<std::string> values) {
    std::string inventory = "";
    for(unsigned int i = 0 ; i < values.size() ; ++i ) {
        inventory += values[i];
        if( i < values.size() -1 ) {
            inventory += ", ";
        }
    }
    // inventory now looks like "value1, value2, value3"
    if( _inventory.count( varName ) == 0 ) {
        // this is new value
        _inventory[ varName ] = inventory;
        _change = true;
    } else {
        std::string oldValue = _inventory[ varName ];
        if( oldValue != inventory ) {
            _inventory[ varName ] = inventory;
            _change = true;
        }
    }
}

void NUTUPS::update(std::map<std::string,std::vector<std::string>> vars ) {
    for(unsigned int i = 0; i < physicsNUT.size(); ++i) {
        // cout << "update " << physicsNUT[i] << endl;
        if( vars.count(physicsNUT[i]) ) {
            // variable found in received data
            std::vector<std::string> values = vars[physicsNUT[i]];
            updatePhysics( physicsBIOS[i], values );
        }
    }
    for(unsigned int i = 0; i < inventoryNUT.size(); ++i) {
        // cout << "update " << inventoryNUT[i] << endl;
        if( vars.count(inventoryNUT[i]) ) {
            // variable found in received data
            std::vector<std::string> values = vars[inventoryNUT[i]];
            updateInventory( inventoryBIOS[i], values );
        }
    }
}

std::string NUTUPS::itof(long int X) {
    std::string num,dec;

    num = std::to_string( X / 100 );
    dec = std::to_string( X % 100 );
    if( dec.size() == 1 ) dec = "0" + dec;
    if( dec == "00" ) {
        return num;
    } else {
        return num + "." + dec;
    }
}

std::string NUTUPS::statusMessage() {
    std::string msg = "",val;
    for(auto it = _physics.begin(); it != _physics.end(); ++it){
        msg += "\"" + it->first + "\":" + itof(it->second ) + ", ";
    }
    for(auto it = _inventory.begin(); it != _inventory.end(); ++it){
        val = it->second;
        std::replace(val.begin(), val.end(),'"',' ');
        msg += "\"" + it->first + "\":\"" + val + "\", ";
    }
    if( msg.size() > 2 ) {
        msg = msg.substr(0, msg.size()-2 );
    }
    _change = false;
    return "{" + msg + "}";
}

NUTUPS::~NUTUPS() {

}

NUTUPSList::NUTUPSList() {

}

void NUTUPSList::updateDeviceList() {
    try {
        std::set<std::string> devs = nutClient.getDeviceNames();
        // add newly appeared devices
        for(auto it = devs.begin(); it != devs.end(); ++it ) {
            // cout << *it << endl;
            if( _devices.count( *it ) == 0 ) {
                _devices[*it] = NUTUPS(*it);
            }
        }
        // remove missing devices
        auto it = _devices.begin();
        while( it != _devices.end() ) {
            if( devs.count(it->first) == 0 ){
                auto toBeDeleted = it;
                ++it;
                _devices.erase(toBeDeleted);
            } else {
                ++it;
            }
        }
    } catch (...) {}
}

void NUTUPSList::updateDeviceStatus() {
    try {
        for(auto device = _devices.begin(); device != _devices.end(); ++device) {
            nut::Device nutDevice = nutClient.getDevice(device->first);
            device->second.update( nutDevice.getVariableValues());
        }
    } catch (...) {}
}

void NUTUPSList::connect() {
    try {
        nutClient.connect("localhost",3493);
    } catch (...) {}
}

void NUTUPSList::disconnect() {
    try {
        nutClient.disconnect();
    } catch (...) {}
}

void NUTUPSList::update() {
    connect();
    updateDeviceList();
    updateDeviceStatus();
    disconnect();
}

/**
   
void NUTUPSList::doMessage() {
    for(auto it = _devices.begin(); it != _devices.end(); ++it) {
        if( it->second.changed() ) {
            cout << it->second.name() << " " << it->second.statusMessage() << endl;
        }
    }
}
*/

unsigned int NUTUPSList::size() {
    return _devices.size();
}

NUTUPS& NUTUPSList::operator[](std::string name) {
    return _devices[name];
}

std::map<std::string, NUTUPS>::iterator NUTUPSList::begin() {
    return _devices.begin();
}

std::map<std::string, NUTUPS>::iterator NUTUPSList::end() {
    return _devices.end();
}

bool NUTUPSList::changed() {

    for(auto  it = _devices.begin() ; it != _devices.end(); ++it) {
        if(it->second.changed() ) return true; 
    }
    return false;
}


NUTUPSList::~NUTUPSList() {
    disconnect();
}


#include <thread>
#include <chrono>

int main(int argc, char *argv[]) {
    NUTUPSList mups;

    while(true) {
        mups.update();
        mups.doMessage();
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    return 0;
}


