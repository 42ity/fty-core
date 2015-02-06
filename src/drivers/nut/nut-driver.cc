#include "nut-driver.h"

#include <nutclient.h>
#include <iostream>
#include <algorithm>

using namespace std;

namespace drivers
{
namespace nut
{

/*
 * See the meaning of variables at NUT project
 * http://www.networkupstools.org/docs/user-manual.chunked/apcs01.html
 */
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
    "battery.charge",
    "outlet.realpower",
    "outlet.#.current",
    "outlet.#.voltage",
    "outlet.#.realpower"
};

static const std::vector<std::string> physicsBIOS {
    "temperature.default",
    "load.default",
    "realpower.default",
    "voltage.output",
    "current.output",
    "voltage.output.L1-N",
    "realpower.output.L1",
    "current.output.L1",
    "voltage.output.L2-N",
    "realpower.output.L2",
    "current.output.L2",
    "voltage.output.L3-N",
    "realpower.output.L3",
    "current.output.L3",
    "charge.battery",
    "realpower.outlet",
    "current.outlet.#",
    "voltage.outlet.#",
    "realpower.outlet.#"
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
    _name = "";
}

NUTDevice::NUTDevice(const char *aName) {  
    name(aName);
}

NUTDevice::NUTDevice(const std::string& aName) {  
    name(aName);
}

void NUTDevice::name(const std::string& aName) {
    _name = aName;
}

std::string NUTDevice::name() const {
    return _name;
}

/**
 * change getters  
 */
bool NUTDevice::changed() const {
    for(auto &it : _physics ){
        if(it.second.changed) return true;
    }
    for(auto &it : _inventory ){
        if(it.second.changed) return true;
    }
    return false;
}

bool NUTDevice::changed(const char *name) const {
    auto iterP = _physics.find(name);
    if( iterP != _physics.end() ) {
        // this is a number, value exists
        return iterP->second.changed;
    }
    auto iterI = _inventory.find(name);
    if( iterI != _inventory.end() ) {
        // this is a inventory string, value exists
        return iterI->second.changed;
    }
    return false;
}

bool NUTDevice::changed(const std::string &name) const {
    return changed(name.c_str());
}

/**
 * change setters
 */
void NUTDevice::setChanged(const bool status) {
    for(auto &it : _physics ){
        it.second.changed = status;
    }
    for(auto &it : _inventory ){
        it.second.changed = status;
    }
}

void NUTDevice::setChanged(const char *name, const bool status) {
    auto iterP = _physics.find(name);
    if( iterP != _physics.end() ) {
        // this is a number, value exists
        iterP->second.changed = status;
    }
    auto iterI = _inventory.find(name);
    if( iterI != _inventory.end() ) {
        // this is a inventory string, value exists
        iterI->second.changed = status;
    }
}
    
void NUTDevice::setChanged(const std::string& name,const bool status){
    setChanged(name.c_str(),status);
}


void NUTDevice::updatePhysics(const std::string& varName, const float newValue) {
    long int newValueInt = round(newValue * 100);
    if( _physics.count( varName ) == 0 ) {
        // this is new value
        struct NUTPhysicalValue pvalue;
        pvalue.changed = true;
        pvalue.value = newValueInt;
        _physics[ varName ] = pvalue;
    } else {
        long int oldValue = _physics[ varName ].value;
        if( oldValue == newValueInt ) return ;
        try {
            if( (oldValue == 0.0 ) || ( abs( (oldValue - newValueInt ) * 100 / oldValue ) > 5 ) ) {
                // significant change
                _physics[ varName ].value = newValueInt;
                _physics[ varName ].changed = true;
            }
        } catch(...) {
            // probably division by 0
            struct NUTPhysicalValue pvalue;
            pvalue.changed = true;
            pvalue.value = newValueInt;
            _physics[ varName ] = pvalue;
        }
    }
}

void NUTDevice::updatePhysics(const std::string& varName, std::vector<std::string>& values) {
    if( values.size() == 1 ) {
        // don't know how to handle multiple values
        // multiple values would be probably nonsence
        try {
            float value = std::stof(values[0]);
            updatePhysics(varName,value);
        } catch (...) {}
    }
}

void NUTDevice::updateInventory(const std::string& varName, std::vector<std::string>& values) {
    std::string inventory = "";
    for(size_t i = 0 ; i < values.size() ; ++i ) {
        inventory += values[i];
        if( i < values.size() -1 ) {
            inventory += ", ";
        }
    }
    // inventory now looks like "value1, value2, value3"
    // NUT bug type pdu => epdu
    if( varName == "type" && inventory == "pdu" ) { inventory = "epdu"; }
    if( _inventory.count( varName ) == 0 ) {
        // this is new value
        struct NUTInventoryValue ivalue;
        ivalue.changed = true;
        ivalue.value = inventory;
        _inventory[ varName ] = ivalue;
    } else {
        if( _inventory[ varName ].value != inventory ) {
            _inventory[ varName ].value = inventory;
            _inventory[ varName ].changed = true;
        }
    }
}

void NUTDevice::update(std::map<std::string,std::vector<std::string>> vars ) {
    for(size_t i = 0; i < physicsNUT.size(); ++i) {
        if( vars.count(physicsNUT[i]) ) {
            // variable found in received data
            std::vector<std::string> values = vars[physicsNUT[i]];
            updatePhysics( physicsBIOS[i], values );
        } else {
            // iterating numbered items in physics
            // like outlet.1.voltage, outlet.2.voltage, ...
            int x = physicsNUT[i].find(".#."); // is always in the middle: outlet.1.realpower
            int y = physicsBIOS[i].find(".#"); // can be at the end: outlet.voltage.#
            if( x > 0 && y > 0 ) {
                // this is something like outlet.#.realpower
                std::string nutprefix = physicsNUT[i].substr(0,x+1);
                std::string nutsuffix = physicsNUT[i].substr(x+2);
                std::string biosprefix = physicsBIOS[i].substr(0,y+1);
                std::string biossuffix = physicsBIOS[i].substr(y+2);
                std::string nutname,biosname;
                int i = 1;
                while(true) {
                    nutname = nutprefix + std::to_string(i) + nutsuffix;
                    biosname = biosprefix + std::to_string(i) + biossuffix;
                    if( vars.count(nutname) == 0 ) break; // variable out of scope
                    // variable found
                    std::vector<std::string> values = vars[nutname];
                    updatePhysics(biosname,values);
                    ++i;
                }
            }
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
    std::string num,dec,sig;
    long int AX;

    if( X < 0 ) {
        sig = "-";
    } else {         
        sig = "";
    }
    AX = abs(X);
    num = std::to_string( AX / 100 );
    dec = std::to_string( AX % 100 );
    if( dec.size() == 1 ) dec = "0" + dec;
    if( dec == "00" ) {
        return sig + num;
    } else {
        return sig + num + "." + dec;
    }
}

std::string NUTDevice::toString() const {
    std::string msg = "",val;
    for(auto it : _physics ){
        msg += "\"" + it.first + "\":" + itof(it.second.value) + ", ";
    }
    for(auto it : _inventory ){
        val = it.second.value;
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
        map[ it.first ] = itof(it.second.value);
    }
    for(auto it : _inventory ){
        map[ it.first ] = it.second.value;
    }
    return map;
}

std::map<std::string,int> NUTDevice::physics(bool onlyChanged) const {
    std::map<std::string,int> map;
    for(auto it : _physics ){
        if( ( ! onlyChanged ) || it.second.changed ) {
            map[ it.first ] = it.second.value;
        }
    }
    return map;
}


bool NUTDevice::hasProperty(const char *name) const {
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

bool NUTDevice::hasProperty(const std::string& name) const {
    return hasProperty(name.c_str());
}

bool NUTDevice::hasPhysics(const char *name) const {
    if( _physics.count( name ) != 0 ) {
        // this is a number and value exists
        return true;
    }
    return false;
}

bool NUTDevice::hasPhysics(const std::string& name) const {
    return hasPhysics(name.c_str());
}


    
std::string NUTDevice::property(const char *name) const {
    auto iterP = _physics.find(name);
    if( iterP != _physics.end() ) {
        // this is a number, value exists
        return itof(iterP->second.value);
    }
    auto iterI = _inventory.find(name);
    if( iterI != _inventory.end() ) {
        // this is a inventory string, value exists
        return iterI->second.value;
    }
    return "";
}

std::string NUTDevice::property(const std::string& name) const {
    return property(name.c_str());
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
            nutclient::Device nutDevice = nutClient.getDevice(device.first);
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

} // namespace drivers::nut
} // namespace drivers
