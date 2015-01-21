/*
Copyright (C) 2015 Eaton
 
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

/*! \file calc_power.cc
    \brief Functions for calculating a total rack and DC power from 
     database values.
    \author Alena Chernikava <alenachernikava@eaton.com>
*/

#include <sstream>
#include <cstdint>

#include <tntdb/connect.h>
#include <tntdb/row.h>
#include <tntdb/error.h>
#include <tntdb/value.h>
#include <tntdb/result.h>

#include "log.h"
#include "defs.h"
#include "asset_types.h"
#include "persist_error.h"
#include "assetmsg.h"

#include "monitor.h"
#include "calc_power.h"

bool is_epdu (const device_info_t &device)
{
    if ( std::get<3>(device) == DEVICE_TYPE_EPDU )
        return true;
    else
        return false;
}

bool is_pdu (const device_info_t &device)
{
    if ( std::get<3>(device) == DEVICE_TYPE_PDU )
        return true;
    else
        return false;
}

bool is_ups (const device_info_t &device)
{
    if ( std::get<3>(device) == DEVICE_TYPE_UPS )
        return true;
    else
        return false;
}

bool is_it_device (const device_info_t &device)
{
    auto device_type_id = std::get<3>(device);
    // TODO add all IT devices
    if ( device_type_id == DEVICE_TYPE_SERVER )
        return true;
    else
        return false;
}

power_sources_t
    extract_power_sources ( const char* url,
          std::pair < std::set < device_info_t >, 
                      std::set < powerlink_info_t > > power_topology, 
          device_info_t start_device )
{
    log_info ("start \n");

    auto devices = power_topology.first;
    auto powerlinks = power_topology.second;
    
    log_debug ("number of devices is %ld \n", devices.size());
    log_debug ("number of powerlinks is %ld \n", powerlinks.size());

    // start_device can be
    // - IT device     - if it is the first call
    // - Power device  - if it is a recursive call, because somewhere before
    //                      we found an PDU power source
   
    // TODO add max_recursive_level

    // a result sets of devices to ask for a power measurement
    std::set < device_info_t > pow_src_epdu;
    std::set < device_info_t > pow_src_ups;
    std::set < device_info_t > pow_src_it_device;
   
    // process links
    for ( auto &link : powerlinks )
    {
        // get src device id
        a_elmnt_id_t src_id = std::get<0>(link);

        device_info_t src_device;
        // find more info about device from device set
        for ( auto &adevice : devices )
        {
            if ( std::get<0>(adevice) == src_id )
            {
                src_device = adevice;
                break;
            }
        }

        log_debug (" start to process \n");
        log_debug ( "src_id is %d \n", std::get<0>(src_device));
        log_debug (" src_type_id is %d \n", std::get<3>(src_device));
        log_debug (" src_type_name is %s \n", std::get<2>(src_device).c_str());
        log_debug (" src_name is %s \n", std::get<1>(src_device).c_str());

        if ( is_epdu (src_device) )
            pow_src_epdu.insert(src_device);
        else if ( is_ups (src_device) )
            pow_src_ups.insert(src_device);
        else if ( is_pdu (src_device) )  
        {
            // ??? da se spocitat power z PDU???
            // select_power_topology_to  PDU
            try{
                auto pdu_pow_top = select_power_topology_to 
                     (url, std::get<0>(src_device), INPUT_POWER_CHAIN, false);
                auto pdu_pow_srcs = extract_power_sources 
                                            (url, pdu_pow_top, src_device);
                pow_src_epdu.insert (std::get<0>(pdu_pow_srcs).begin(), 
                                     std::get<0>(pdu_pow_srcs).end());
                pow_src_ups.insert (std::get<1>(pdu_pow_srcs).begin(), 
                                    std::get<1>(pdu_pow_srcs).end());
                pow_src_it_device.insert (std::get<2>(pdu_pow_srcs).begin(), 
                                          std::get<2>(pdu_pow_srcs).end());
            }
            // TODO what should we do with the errors
            catch (const bios::NotFound &e) {
                // TODO od prvniho vyberu z database uz probehlo spousta casu,
                // a ten device mohl nekdo smazat z DB.
                // for now ignore such device
            }
            catch (const bios::InternalDBError &e) {
                // propagate error to higher level
                log_warning ("abnormal end with %s \n", e.what());
                throw bios::InternalDBError(e.what());
            }
            catch (const bios::ElementIsNotDevice &e) {
                log_error ("Database is corrupted, in power chain there is a"
                            "non device");
                // TODO
                // for now ignore this element
            }
        }
        else
        {   
            // TODO issue some warning, because this is not normal
            log_warning ("device is not connected to epdu, ups, pdu \n");
            pow_src_it_device.insert (start_device);
        }
    }
    log_info ("end \n");
    return std::make_tuple (pow_src_epdu, pow_src_ups, pow_src_it_device);
}
    
std::set <device_info_t> select_rack_devices(const char* url, 
                                             a_elmnt_id_t element_id)
{
    // ASSUMPTION
    // specified element_id is already in DB and has type asset_type::RACK
    log_info ("start \n");
    std::set <device_info_t> result_set;
    try{
        tntdb::Connection conn = tntdb::connectCached(url); 
               
        // TODO get rid of duplicate device processing
        tntdb::Statement st = conn.prepareCached(
                " SELECT"
                "   v.id, v.name, v1.name as type_name,"
                "   v1.id_asset_device_type"
                " FROM"
                "   v_bios_asset_element v"
                " INNER JOIN v_bios_asset_device v1"
                "   ON v1.id_asset_element = v.id"
                " WHERE v.id_parent = :elementid"
        );
        
        // Could return more than one row
        tntdb::Result result = st.set("elementid", element_id).
                                  select();
        log_debug("rows selected %d\n", result.size());

        for ( auto &row: result )
        {
            a_elmnt_id_t device_asset_id = 0;
            row[0].get(device_asset_id);
            assert ( device_asset_id );
            
            std::string device_name = "";
            row[1].get(device_name);
            assert ( !device_name.empty() );

            std::string device_type_name = "";
            row[2].get(device_type_name);
            assert ( !device_type_name.empty() );

            a_dvc_tp_id_t device_type_id = 0;
            row[3].get(device_type_id);
            assert ( device_type_id );
            
            result_set.insert (std::make_tuple(device_asset_id, device_name, 
                                device_type_name, device_type_id));
        }
        log_info ("end\n");
        // TODO
        // result_set is empty if:
        //  - someone removed a rack from DB (but this should never happen)
        //  - there is no any device in a rack
        return result_set;
    }
    catch (const std::exception &e) {
        log_warning ("abnormal end with '%s'\n", e.what());
        throw bios::InternalDBError(e.what());
    }
}

a_elmnt_tp_id_t select_element_type (const char* url, 
                                     a_elmnt_id_t asset_element_id)
{
    log_info ("start \n");
    assert (asset_element_id);
    try{
        tntdb::Connection conn = tntdb::connectCached(url);
        tntdb::Statement st = conn.prepareCached(
            " SELECT"
            "    v.id_type"
            " FROM"
            "   v_bios_asset_element v"
            " WHERE v.id = :id"
        );
    
        tntdb::Value val = st.set("id", asset_element_id).
                              selectValue();
        
        a_elmnt_tp_id_t asset_element_type_id = 0;
        val.get(asset_element_type_id);
        assert ( asset_element_type_id );
        log_info ("end \n");
        return asset_element_type_id;
    }
    catch (const tntdb::NotFound &e) {
        log_info ("specified element was not found \n");
        // element with specified id was not found
        return 0;
    }
    catch (const std::exception &e){
        log_warning ("abnormal end with '%s'\n", e.what());
        throw bios::InternalDBError(e.what());
    }
}

power_sources_t choose_power_sources (const char* url, 
                                        std::set <device_info_t> rack_devices)
{
    power_sources_t power_sources;

    for (auto &adevice: rack_devices)
        if ( is_it_device (adevice) )
        {
            auto pow_top_to = select_power_topology_to 
                        (url, std::get<0>(adevice), INPUT_POWER_CHAIN, false);
            auto new_power_srcs = extract_power_sources
                        (url, pow_top_to, adevice);
            
            // in V1 if power_source should be taken in account iff 
            // it is in rack
            for ( auto &bdevice: std::get<0>(new_power_srcs) )
                if ( rack_devices.count(bdevice) == 1 )
                    std::get<0>(power_sources).insert (bdevice); 
            
            for ( auto &bdevice: std::get<1>(new_power_srcs) )
                if ( rack_devices.count(bdevice) == 1 )
                    std::get<1>(power_sources).insert (bdevice);
            
            for ( auto &bdevice: std::get<2>(new_power_srcs) )
                if ( rack_devices.count(bdevice) == 1 )
                    std::get<2>(power_sources).insert (bdevice);
        }
    return power_sources;
}

int convert_str_to_double (const char* value_str, double *value)
{
    if ( value_str == NULL )
        return -1;
    else
    {
        char *p;
        errno = 0; // strtod could change this value
        *value = strtod(value_str, &p);
        
        // (p == vulue_str) detects if the input string was empty
        if ( ( p == value_str ) || ( *p != '\0' ) )
            return -3;
        // Value is ok, if pointer points to the end of the string

        if ( errno == ERANGE ) // the value is out of range
            return -2;

        return 0;
    }
}

void compute_result_value_set (zhash_t *results, double value)
{
    log_debug ("value is %f \n", value);
    char buff[20];
    sprintf(buff, "%f", value);
    log_debug ("converted value is %s \n", buff);
    zhash_insert (results, "value_d", buff);
}

int compute_result_value_get (zhash_t *results, double *value)
{
    char* value_str = (char *) zhash_lookup (results, "value_d");
    int r = convert_str_to_double (value_str, value);
    return r;
}

void compute_result_value_set (zhash_t *results, m_msrmnt_value_t value)
{
    // 21 = 20+1 , 20 charecters has a uint64_t
    log_debug ("value is %ld \n", value);
    char buff[21];
    sprintf(buff, "%ld", value);
    zhash_insert (results, "value", buff);
    log_debug ("converted value is %s \n", buff);
}

int compute_result_value_get (zhash_t *results, m_msrmnt_value_t *value)
{
    char* value_str = (char *) zhash_lookup (results, "value");
    log_debug ("In hash there is %s value \n", value_str);
    int r = sscanf(value_str ,"%ld", value);
    log_debug ("Converted value is %ld \n", *value);
    return r;
}

void compute_result_scale_set (zhash_t *results, m_msrmnt_scale_t scale)
{
    // 21 = 20+1 , 20 charecters has a uint64_t
    log_debug ("scale is %d \n", scale);
    char buff[21];
    sprintf(buff, "%d", scale);
    zhash_insert (results, "scale", buff);
    log_debug ("converted scale is %s \n", buff);
}

int compute_result_scale_get (zhash_t *results, m_msrmnt_scale_t *scale)
{
    char* value_str = (char *) zhash_lookup (results, "scale");
    int r = sscanf(value_str ,"%hd", scale);
    return r;
}

zmsg_t* calc_total_rack_power (const char *url, a_elmnt_id_t rack_element_id)
{
    log_info ("start \n");
    // check if specified device has a rack type
    try{
        a_elmnt_id_t type_id = select_element_type (url, rack_element_id);
        if ( type_id == 0 )
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_NOTFOUND, 
                                    "specified element was not found", NULL);
        if (  type_id != asset_type::RACK ) 
            return common_msg_encode_fail(BIOS_ERROR_DB, DB_ERROR_BADINPUT, 
                                    "specified element is not a rack", NULL);
    }
    catch (const bios::InternalDBError &e)
    {
        return common_msg_encode_fail(BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                    e.what(), NULL);
    }

    // continue, select all devices in a rack
    auto rack_devices = select_rack_devices(url, rack_element_id);

    // find all power sources for in-rack devices
    power_sources_t power_sources = choose_power_sources(url, rack_devices);

    // calc sum
    

    // transform number to string
    m_msrmnt_scale_t scale = -1;
    m_msrmnt_value_t value = 999993;

    zhash_t* result = zhash_new();
    zhash_autofree (result);
    compute_result_value_set (result, value);
    compute_result_scale_set (result, scale);
    

    m_msrmnt_scale_t scale_get = 8;
    m_msrmnt_value_t value_get = 88;
    int r = compute_result_value_get (result, &value_get);
    int r1 = compute_result_scale_get (result, &scale_get);
   
    log_debug("errcode: %d, this is value: %ld \n", r, value_get);
    log_debug("errcode: %d, this is scale: %d \n", r1, scale_get);
    
    // fill the return message
    zmsg_t* retmsg = compute_msg_encode_return_computation(result);

    log_debug ("start to print \n");
    log_debug ("ePDU \n");
    for (auto &adevice: std::get<0>(power_sources))
    {
        log_debug ("%d \n", std::get<0>(adevice));
    }
    log_debug ("UPS \n");
    for (auto &adevice: std::get<1>(power_sources))
    {
        log_debug ("%d \n", std::get<0>(adevice));
    }
    log_debug ("IT devices \n");
    for (auto &adevice: std::get<2>(power_sources))
    {
        log_debug ("%d \n", std::get<0>(adevice));
    }
    log_info ("end \n");
    return retmsg;
}

//! \brief rescale number to the new scale - upscalling loses information
//
// assert's to prevent integer overflow
static m_msrmnt_value_t s_rescale(m_msrmnt_value_t value, m_msrmnt_scale_t old_scale, m_msrmnt_scale_t new_scale) {
    if (old_scale == new_scale) {
        return value;
    }

    if (old_scale > new_scale) {
        for (auto i = 0; i != abs(old_scale - new_scale); i++) {
            assert(value < (INT64_MAX / 10));
            value *= 10;
        }
        return value;
    }

    for (auto i = 0; i != abs(old_scale - new_scale); i++) {
        value /= 10;
    }
    return value;

}

//! \brief add a value with a respect to the scale - downscale all the time
static void s_add_scale(rack_power_t& ret, m_msrmnt_value_t value, m_msrmnt_scale_t scale) {
    if (ret.scale > scale) {
        ret.power = s_rescale(ret.power, ret.scale, scale);
        ret.scale = scale;
    }
    ret.power += s_rescale(value, ret.scale, scale);
}

static std::string s_generate_in_clause(const std::map<m_dvc_id_t, a_elmnt_id_t>& map) {
    std::ostringstream o;
    size_t i = 0;
    for (const auto el: map) {
        o << el.first;
        i++;
        if (i != map.size()) {
            o << ", ";
        }
    }
    return o.str();
}

//! \brief compute total rack power V1
//
// FIXME: leave three arguments - one per device type, maybe in the future we'll use it, or change
// TODO: ask for id_key/id_subkey, see measurement_id_t nut_get_measurement_id(const std::string &name) 
// TODO: quality computation - to be defined, leave with 255
//
// \param upses - list of ups'es
// \param epdus - list of epdu's
// \param devs - list of devices
// \param max_age - maximum age we'd like to take into account in secs
// \return rc_power_t - total power, quality of metric and list of id's, which were requested, but missed

rack_power_t
compute_total_rack_power_v1(
        const char *url,
        std::set<device_info_t> upses,
        std::set<device_info_t> epdus,
        std::set<device_info_t> devs,
        uint32_t max_age) {

    log_info("start \n");

    assert(max_age != 0);

    std::set<a_elmnt_id_t> all_asset_ids{};
    //FIXME: this is stupid - aren't there union operations on sets in std C++?
    for (const device_info_t& d : upses)  all_asset_ids.insert(device_info_id(d));
    for (const device_info_t& d : epdus)  all_asset_ids.insert(device_info_id(d));
    for (const device_info_t& d : devs)   all_asset_ids.insert(device_info_id(d));
    rack_power_t ret{0, 0, 255, all_asset_ids};

    try{
        // FIXME: asking for mapping each time sounds slow
        std::map<m_dvc_id_t, a_elmnt_id_t> idmap;
        for (a_elmnt_id_t asset_id : all_asset_ids) {
            m_dvc_id_t dev_id = convert_asset_to_monitor(url, asset_id);
            idmap[dev_id] = asset_id;
        }

        //NOTE: cannot cache as SQL query is not stable
        tntdb::Connection conn = tntdb::connect(url);
        tntdb::Statement st = conn.prepare(
            " SELECT"
            "    v.id_discovered_device, v.value, v.scale"
            " FROM"
            "   v_bios_client_info_measurements_last v"
            " WHERE"
            "   v.id_discovered_device IN (" + s_generate_in_clause(idmap) + ")"  //XXX: SQL placeholders can't deal with list of values, as we're using plain int, there is no issue in generating SQL by hand
            "   AND v.id_key=3 AND v.id_subkey=1 "  //TODO: read realpower.default from database, SELECT v.id as id_subkey, v.id_type as id_key FROM v_bios_measurement_subtypes v WHERE name='default' AND typename='realpower';
            "   AND v.timestamp BETWEEN"
            "       DATE_SUB(UTC_TIMESTAMP(), INTERVAL :seconds SECOND)"
            "       AND UTC_TIMESTAMP()"
        );

        tntdb::Result result = st.set("seconds", max_age).
                              select();
        log_debug("rows selected %d\n", result.size());
        for ( auto &row: result )
        {
            m_dvc_id_t dev_id = 0;
            row[0].get(dev_id);
            ret.missed.erase(idmap[dev_id]);  //<- no assert needed, invalid value will raise an exception, converts from device id back to asset id

            m_msrmnt_value_t value = 0;
            row[1].get(value);

            m_msrmnt_scale_t scale = 0;
            row[2].get(scale);

            s_add_scale(ret, value, scale);
        }

    }
    catch (const std::exception &e) {
        log_warning ("abnormal end with '%s'\n", e.what());
        throw bios::InternalDBError(e.what());
    }

    return ret;
}
