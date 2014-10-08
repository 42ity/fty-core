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

#ifndef _SRC_DRIVERS_NUT_DRIVER_H_
#define _SRC_DRIVERS_NUT_DRIVER_H_

#include <czmq.h>
#include <map>
#include <vector>
#include <nutclient.h>

void nut_actor(zsock_t *pipe, void *args);

/**
 * \class NUTUPS
 *
 * \brief Class for keeping status information of one UPS as it is presented by NUT.
 *
 * NUTUPS keeps inventory, status and measurement values of one UPS as it is presented
 * by NUT.
 */

class NUTUPS {
    friend class NUTUPSList;
 public:
    /**
     * \brief Creates new NUTUPS with empty set of values without name.
     */
    NUTUPS();

    /**
     * \brief Creates new NUTUPS with empty set of values with name (name corresponds
     * with NUTs /etc/ups/ups.conf
     */
    NUTUPS(std::string name);

    /**
     * \brief Method for obtaining UPS's name.
     */
    std::string name();

    /**
     * \brief Method for checking that some changes in UPS status happened.
     * \return bool
     * \see statusMessage()
     *
     * Method returns true if there are some changes in UPS since last
     * statusMessage has been called.
     */
    bool changed();
    
    /**
     * \brief Produces a std::string with UPS status in JSON format.
     * \return std::string
     * \see change()
     *
     * Method returns string with UPS status message. The flag _change is
     * set to false. Method returs the actual status allways, even if there is
     * no change.
     *
     *    if( UPS.change() ) {
     *        cout << UPS.statusMessage() << endl;
     *    }
     */
    std::string statusMessage();
    ~NUTUPS();
 private:

    /**
     * \brief sets the UPS name
     */
    void name(const std::string name);

    /**
     * \brief Updates physical or measurement value (like current or load) from float.
     *
     * Updates the value if new value is significantly differen (>5%). Flag _change is
     * set if new value is saved.
     */
    void updatePhysics(std::string varName, float newValue);

    /**
     * \brief Updates physical or measurement value from vector.
     *
     * Updates the value with first value from vector (NUT returns vectors of
     * values).
     */
    void updatePhysics(std::string varName, std::vector<std::string> values);

    /**
     * \brief Updates inventory value.
     *
     * Updates the value with values from vector (NUT returns vectors of
     * values). values are connected like "value1, value2, value3". Flag _change is
     * set if new value is different from old one.
     */
    void updateInventory(std::string varName, std::vector<std::string> values);

    /**
     * \brief Updates all values from NUT.
     */
    void update(std::map<std::string,std::vector<std::string>> vars );

    /**
     * \brief map of physical values.
     *
     * Values are multiplied by 100 and stored as integer 
     */
    std::map<std::string, long int> _physics;
    //! \brief map of inventory values
    std::map<std::string, std::string> _inventory;
    //! \brief flag, if there is change to be published
    bool _change;
    //! \brief ups name
    std::string _name;
    //! \brief Transformation of our integer (x100) back
    std::string itof(long int);
};

/**
 * \brief NUTUPSList is class for holding list of NUTUPS objects.
 */
class NUTUPSList {
 public:
    NUTUPSList();

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
    bool changed();

    /**
     * \brief returns the size of device list (number of upses)
     */
    unsigned int size();

    //! \brief get the NUTUPS object by name
    NUTUPS& operator[](std::string name);

    //! \brief get the iterators, to be able to go trough list of devices
    std::map<std::string, NUTUPS>::iterator begin();
    std::map<std::string, NUTUPS>::iterator end();

    ~NUTUPSList();
 private:
    //! \brief Connection to NUT daemon
    nut::TcpClient nutClient;

    //! \brief list of NUT devices
    std::map<std::string, NUTUPS> _devices;  

    //! \brief connect to NUT daemon
    void connect();

    //! \brief disconnect from NUT daemon
    void disconnect();

    //! \brief update list of NUT devices
    void updateDeviceList();

    //! \brief update status of NUT devices
    void updateDeviceStatus();
};

#endif // _SRC_DRIVERS_NUT_DRIVER_H_
