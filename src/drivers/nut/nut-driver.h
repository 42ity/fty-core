/*
Copyright (C) 2014 Eaton
 
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*! \file nut-driver.h
    \brief Classes for communicating with NUT daemon
    \author Tomas Halman <tomashalman@eaton.com>
*/

#ifndef SRC_DRIVERS_NUT_NUT_DRIVER_H_
#define SRC_DRIVERS_NUT_NUT_DRIVER_H_

#include <map>
#include <vector>
#include <nutclient.h>
#include <czmq.h>

namespace nutclient = nut;

namespace drivers
{
namespace nut
{

struct NUTInventoryValue {
    bool changed;
    std::string value;
};

struct NUTPhysicalValue {
    bool changed;
    long int value;
};
 
/**
 * \class NUTDevice
 *
 * \brief Class for keeping status information of one UPS/ePDU/... as it is presented by NUT.
 *
 * NUTDevice keeps inventory, status and measurement values of one device as it is presented
 * by NUT.
 */

class NUTDevice {
    friend class NUTDeviceList;
 public:
    /**
     * \brief Creates new NUTDevice with empty set of values without name.
     */
    NUTDevice();

    /**
     * \brief Creates new NUTDevice with empty set of values with name (name corresponds
     * with NUTs /etc/ups/ups.conf
     */
    NUTDevice(const char *name);
    NUTDevice(const std::string& name);

    /**
     * \brief Method for obtaining device's name.
     */
    std::string name() const;

    /**
     * \brief Method for checking that some changes in device status happened.
     * \return bool
     *
     * Method returns true if there are some changes in device since last
     * statusMessage has been called.
     */
    bool changed() const;
    bool changed(const char *name) const;
    bool changed(const std::string& name) const;

    /**
     * \brief Method for setting the change status.
     */
    void changed(const bool status);
    void changed(const char *name, const bool status);
    void changed(const std::string& name,const bool status);

    /**
     * \brief Produces a std::string with device status in JSON format.
     * \return std::string
     * \see change()
     *
     * Method returns string with device status message. Method returs the
     * actual status in json like std::string.
     *
     *    if( UPS.change() ) {
     *        cout << UPS.toString() << endl;
     *        UPS.changed(false);
     *    }
     */
    std::string toString() const;

    /**
     * \brief method checks whether this device reports particular property.
     * \return bool, true if property exists
     */
    bool hasProperty(const char *name) const;
    bool hasProperty(const std::string& name) const;
    bool hasPhysics(const char *name) const;
    bool hasPhysics(const std::string& name) const;
    std::map<std::string,int> physics(bool onlyChanged) const;

    /**
     * \brief method returns particular device property.
     * \return std::string, property value as a string or empty
     *         string ("") if property doesn't exists
     *
     *    if( UPS.hasProperty("voltage") ) {
     *        cout << "voltage " << UPS.property("voltage") << "\n";
     *    } else {
     *        cout << "voltage unknown\n";
     *    }
     */
    std::string property(const char *name) const;
    std::string property(const std::string& name) const;

    /**
     * \brief method returns all discovered properties of device.
     * \return std::map<std::string,std::string> property values
     *
     * Method transforms all properties (physical and inventory) to
     * map. Numeric values are converted to strings using itof() method.
     */
    std::map<std::string,std::string> properties() const;
    ~NUTDevice();
 private:

    /**
     * \brief sets the device name
     */
    void name(const std::string& name);

    /**
     * \brief Updates physical or measurement value (like current or load) from float.
     *
     * Updates the value if new value is significantly differen (>5%). Flag _change is
     * set if new value is saved.
     */
    void updatePhysics(const std::string& varName, const float newValue);

    /**
     * \brief Updates physical or measurement value from vector.
     *
     * Updates the value with first value from vector (NUT returns vectors of
     * values).
     */
    void updatePhysics(const std::string& varName, std::vector<std::string>& values);

    /**
     * \brief Updates inventory value.
     *
     * Updates the value with values from vector (NUT returns vectors of
     * values). values are connected like "value1, value2, value3". Flag _change is
     * set if new value is different from old one.
     */
    void updateInventory(const std::string& varName, std::vector<std::string>& values);

    /**
     * \brief Updates all values from NUT.
     */
    void update(std::map<std::string,std::vector<std::string>> vars );

    /**
     * \brief map of physical values.
     *
     * Values are multiplied by 100 and stored as integer 
     */
    std::map<std::string, NUTPhysicalValue> _physics;
    //! \brief map of inventory values
    std::map<std::string, NUTInventoryValue> _inventory;
    //! \brief device name
    std::string _name;
    //! \brief Transformation of our integer (x100) back
    std::string itof(const long int) const;
};

/**
 * \brief NUTDeviceList is class for holding list of NUTDevice objects.
 */
class NUTDeviceList {
 public:
    NUTDeviceList();

    /**
     * \brief Reads status information from NUT daemon.
     *
     * Method reads values from NUT and updates information of particular
     * devices. Newly discovered davices are added to list, removed devices
     * are also removed from list.
     */
    void update();

    /**
     * \brief Returns true if there is at least one device claiming change.
     */
    bool changed() const;

    /**
     * \brief returns the size of device list (number of devices)
     */
    size_t size();

    //! \brief get the NUTDevice object by name
    NUTDevice& operator[](const std::string name);

    //! \brief get the iterators, to be able to go trough list of devices
    std::map<std::string, NUTDevice>::iterator begin();
    std::map<std::string, NUTDevice>::iterator end();

    ~NUTDeviceList();
 private:
    //! \brief Connection to NUT daemon
    nutclient::TcpClient nutClient;

    //! \brief list of NUT devices
    std::map<std::string, NUTDevice> _devices;  

    //! \brief connect to NUT daemon
    bool connect();

    //! \brief disconnect from NUT daemon
    void disconnect();

    //! \brief update list of NUT devices
    void updateDeviceList();

    //! \brief update status of NUT devices
    void updateDeviceStatus();
};

} // namespace drivers::nut
} // namespace drivers

#endif // SRC_DRIVERS_NUT_NUT_DRIVER_H_

