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
    "model",
    "manufacturer",
    "serial",
    "type",
    "device.description",
    "device.contact",
    "device.location",
    "device.part",
    "status",
    "ups.alarm",
    "ups.serial",
    "battery.date",
    "battery.type"
};

NUTDevice::NUTDevice() {  
    _change = false;
    _name = "";
}

NUTDevice::NUTDevice(std::string aName) {  
    _change = false;
    name(aName);
}

void NUTDevice::name(const std::string aName) {
    _name = aName;
}

std::string NUTDevice::name() const {
    return _name;
}

bool NUTDevice::changed() const {
    return _change;
}

void NUTDevice::changed(const bool status) {
    _change = status;
}

void NUTDevice::updatePhysics(std::string varName, float newValue) {
    // calculating round(newValue * 100) without math library
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

void NUTDevice::updatePhysics(std::string varName, std::vector<std::string> values) {
    if( values.size() == 1 ) {
        // don't know how to handle multiple values
        // multiple values would be probably nonsence
        try {
            float value = std::stof(values[0]);
            updatePhysics(varName,value);
        } catch (...) {}
    }
}

void NUTDevice::updateInventory(std::string varName, std::vector<std::string> values) {
    std::string inventory = "";
    for(size_t i = 0 ; i < values.size() ; ++i ) {
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

void NUTDevice::update(std::map<std::string,std::vector<std::string>> vars ) {
    for(size_t i = 0; i < physicsNUT.size(); ++i) {
        if( vars.count(physicsNUT[i]) ) {
            // variable found in received data
            std::vector<std::string> values = vars[physicsNUT[i]];
            updatePhysics( physicsBIOS[i], values );
        }
    }
    for(size_t i = 0; i < inventoryNUT.size(); ++i) {
        if( vars.count(inventoryNUT[i]) ) {
            // variable found in received data
            std::vector<std::string> values = vars[inventoryNUT[i]];
            updateInventory( inventoryBIOS[i], values );
        }
    }
}

std::string NUTDevice::itof(const long int X) const {
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

std::string NUTDevice::toString() const {
    std::string msg = "",val;
    for(auto it : _physics ){
        msg += "\"" + it.first + "\":" + itof(it.second ) + ", ";
    }
    for(auto it : _inventory ){
        val = it.second;
        std::replace(val.begin(), val.end(),'"',' ');
        msg += "\"" + it.first + "\":\"" + val + "\", ";
    }
    if( msg.size() > 2 ) {
        msg = msg.substr(0, msg.size()-2 );
    }
    return "{" + msg + "}";
}

std::map<std::string,std::string> NUTDevice::properties() const {
    std::map<std::string,std::string> map;
    for(auto it : _physics ){
        map[ it.first ] = itof(it.second);
    }
    for(auto it : _inventory ){
        map[ it.first ] = it.second;
    }
    return map;
}


bool NUTDevice::hasProperty(const std::string name) const {
    if( _physics.count( name ) != 0 ) {
        // this is a number and value exists
        return true;
    }
    if( _inventory.count( name ) != 0 ) {
        // this is a inventory string, value exists
        return true;
    }
    return false;
}

std::string NUTDevice::property(const std::string name) const {
    auto iterP = _physics.find(name);
    if( iterP != _physics.end() ) {
        // this is a number, value exists
        return itof(iterP->second);
    }
    auto iterI = _inventory.find(name);
    if( iterI != _inventory.end() ) {
        // this is a inventory string, value exists
        return iterI->second;
    }
    return "";
}

NUTDevice::~NUTDevice() {

}

NUTDeviceList::NUTDeviceList() {

}

void NUTDeviceList::updateDeviceList() {
    try {
        std::set<std::string> devs = nutClient.getDeviceNames();
        // add newly appeared devices
        for(auto &it : devs ) {
            // cout << *it << endl;
            if( _devices.count( it ) == 0 ) {
                _devices[it] = NUTDevice(it);
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

void NUTDeviceList::updateDeviceStatus() {
    try {
        for(auto &device : _devices ) {
            nut::Device nutDevice = nutClient.getDevice(device.first);
            device.second.update( nutDevice.getVariableValues());
        }
    } catch (...) {}
}

bool NUTDeviceList::connect() {
    try {
        nutClient.connect("localhost",3493);
    } catch (...) {}
    return nutClient.isConnected();
}

void NUTDeviceList::disconnect() {
    try {
        nutClient.disconnect();
    } catch (...) {}
}

void NUTDeviceList::update() {
    if( connect() ) {
        updateDeviceList();
        updateDeviceStatus();
        disconnect();
    }
}

size_t NUTDeviceList::size() {
    return _devices.size();
}

NUTDevice& NUTDeviceList::operator[](const std::string name) {
    return _devices[name];
}

std::map<std::string, NUTDevice>::iterator NUTDeviceList::begin() {
    return _devices.begin();
}

std::map<std::string, NUTDevice>::iterator NUTDeviceList::end() {
    return _devices.end();
}

bool NUTDeviceList::changed() const {

    for(auto  &it : _devices ) {
        if(it.second.changed() ) return true; 
    }
    return false;
}

NUTDeviceList::~NUTDeviceList() {
    disconnect();
}

