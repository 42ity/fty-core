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
            Michal Vyskocil  <michalvyskocil@eaton.com>
*/

#include <tntdb/connect.h>
#include <tntdb/error.h>
#include <tntdb/value.h>
#include <tntdb/result.h>

#include "log.h"
#include "defs.h"
#include "calc_power.h"

#include "asset_types.h"
#include "persist_error.h"
#include "assetcrud.h"
#include "monitor.h"

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
    log_info ("start ");

    auto devices    = power_topology.first;
    auto powerlinks = power_topology.second;
    
    log_debug ("number of devices is %ld ", devices.size());
    log_debug ("number of powerlinks is %ld ", powerlinks.size());

    // start_device can be
    // - IT device     - if it is the first call
    // - Power device  - if it is a recursive call, because somewhere before
    //                      we found an PDU power source
   
    // TODO add max_recursive_level

    // result sets of devices to ask for a power measurement
    std::set < device_info_t > pow_src_epdu;
    std::set < device_info_t > pow_src_ups;
    std::set < device_info_t > pow_src_it_device;
   
    // process links
    // one power link correcpond to one PSU (power supply unit)
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

        log_debug ("start to process ");
        log_debug ("src_id is %d ", std::get<0>(src_device));
        log_debug (" src_type_id is %d ", std::get<3>(src_device));
        log_debug (" src_type_name is %s ", std::get<2>(src_device).c_str());
        log_debug (" src_name is %s ", std::get<1>(src_device).c_str());

        if ( is_epdu (src_device) )
            pow_src_epdu.insert(src_device);
        else if ( is_ups (src_device) )
            pow_src_ups.insert(src_device);
        else if ( is_pdu (src_device) )  
        {
            // select_power_topology_to  PDU
            try{
                // select only one step back => false
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
            catch (const bios::NotFound &e) {
                // TODO od prvniho vyberu z database uz probehlo spousta casu,
                // a ten device mohl nekdo smazat z DB.
                // for now ignore such device
            }
            catch (const bios::InternalDBError &e) {
                // propagate error to higher level
                log_warning ("abnormal end with '%s' ", e.what());
                throw bios::InternalDBError(e.what());
            }
            catch (const bios::ElementIsNotDevice &e) {
                log_error ("Database is corrupted, in power chain there is a"
                            "non device with asset id %d \n",  
                            std::get<0>(src_device));
                // TODO
                // for now ignore this element
            }
        }
        else
        {   
            // TODO issue some warning, because this is not normal
            // that device directly is not connected to epdu/pdu/ups
            if ( is_it_device (src_device) )
                log_warning ("device with asset id %d is not connected "
                        "to epdu, ups, pdu \n", std::get<0>(src_device));
            pow_src_it_device.insert (start_device);
        }
    }
    log_info ("end ");
    return std::make_tuple (pow_src_epdu, pow_src_ups, pow_src_it_device);
}
    
std::set <device_info_t> select_rack_devices(const char* url, 
                                             a_elmnt_id_t element_id)
{
    // ASSUMPTION
    // specified element_id is already in DB and has type asset_type::RACK
    log_info ("start ");
    std::set <device_info_t> result_set;
    try{
        tntdb::Connection conn = tntdb::connectCached(url); 
               
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
        log_info ("end");
        // TODO
        // result_set is empty if:
        //  - someone removed a rack from DB (but this should never happen)
        //  - there is no any device in a rack
        return result_set;
    }
    catch (const std::exception &e) {
        log_warning ("abnormal end with '%s'", e.what());
        throw bios::InternalDBError(e.what());
    }
}

a_elmnt_tp_id_t select_element_type (const char* url, 
                                     a_elmnt_id_t asset_element_id)
{
    log_info ("start ");
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
        log_info ("end ");
        return asset_element_type_id;
    }
    catch (const tntdb::NotFound &e) {
        log_info ("specified element %d was not found ", asset_element_id);
        // element with specified id was not found
        return 0;
    }
    catch (const std::exception &e){
        log_warning ("abnormal end with '%s' ", e.what());
        throw bios::InternalDBError(e.what());
    }
}

// TODO quality
power_sources_t select_pow_src_for_device (const char* url, 
                                                device_info_t adevice)
{
    power_sources_t power_sources;
    auto pow_top_to = select_power_topology_to 
               (url, std::get<0>(adevice), INPUT_POWER_CHAIN, false);
    auto new_power_srcs = extract_power_sources
               (url, pow_top_to, adevice);
    log_debug ("end: normal");
    return new_power_srcs;
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
    char buff[20];
    sprintf(buff, "%f", value);
    zhash_insert (results, "value_d", buff);
    log_debug ("conv value from %f to '%s' ", value, buff);
}

int compute_result_value_get (zhash_t *results, double *value)
{
    char* value_str = (char *) zhash_lookup (results, "value_d");
    int r = convert_str_to_double (value_str, value);
    log_debug ("conv value from '%s' to %f ", value_str, *value);
    return r;
}

void compute_result_value_set (zhash_t *results, m_msrmnt_value_t value)
{
    // 21 = 20+1 , 20 charecters has a uint64_t
    char buff[21];
    sprintf(buff, "%ld", value);
    zhash_insert (results, "value", buff);
    log_debug ("conv value from %ld to '%s' ", value, buff);
}

int compute_result_value_get (zhash_t *results, m_msrmnt_value_t *value)
{
    char* value_str = (char *) zhash_lookup (results, "value");
    int r = sscanf(value_str ,"%ld", value);
    log_debug ("conv value from '%s' to %ld ", value_str, *value);
    return (r == 0 ? 1:0);
}

void compute_result_scale_set (zhash_t *results, m_msrmnt_scale_t scale)
{
    // 21 = 20+1 , 20 charecters has a uint64_t
    char buff[21];
    sprintf(buff, "%d", scale);
    zhash_insert (results, "scale", buff);
    log_debug ("conv scale from %hd to '%s' ", scale, buff);
}

int compute_result_scale_get (zhash_t *results, m_msrmnt_scale_t *scale)
{
    char* value_str = (char *) zhash_lookup (results, "scale");
    int r = sscanf(value_str ,"%hd", scale);
    log_debug ("conv scale from '%s' to %hd ", value_str, *scale);
    return (r == 0 ? 1:0);
}

void compute_result_num_missed_set (zhash_t *results, a_elmnt_id_t num_missed)
{
    // 21 = 20+1 , 20 charecters has a uint64_t
    char buff[21];
    sprintf(buff, "%d", num_missed);
    zhash_insert (results, "num_missed", buff);
    log_debug ("conv num_missed from %d to '%s' ", num_missed, buff);
}

int compute_result_num_missed_get (zhash_t *results, a_elmnt_id_t *num_missed)
{
    char* value_str = (char *) zhash_lookup (results, "num_missed");
    int r = sscanf(value_str ,"%u", num_missed);
    log_debug ("conv num_missed from '%s' to %u ", value_str, *num_missed);
    return (r == 0 ? 1:0);
}

zmsg_t* calc_total_rack_power (const char *url, a_elmnt_id_t rack_element_id)
{
    log_info ("start ");

    // check if specified device has a rack type
    try{
        a_elmnt_id_t type_id = select_element_type (url, rack_element_id);
        if ( type_id == 0 )
        {
            log_info ("end: %d rack not found ", rack_element_id);
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_NOTFOUND, 
                                    "specified rack was not found", NULL);
        }
        if (  type_id != asset_type::RACK ) 
        {
            log_info ("end: %d isn't rack ", rack_element_id);
            return common_msg_encode_fail(BIOS_ERROR_DB, DB_ERROR_BADINPUT, 
                                    "specified element is not a rack", NULL);
        }
    }
    catch (const bios::InternalDBError &e)
    {
        log_warning ("end: abnormal with '%s' ", e.what());
        return common_msg_encode_fail(BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                    e.what(), NULL);
    }

    // continue, select all devices in a rack
    auto rack_devices = select_rack_devices(url, rack_element_id);

    auto ret_results = compute_total_rack_power_v1 (url, rack_devices, 300);

    // transform numbers to string and fill hash
    zhash_t* result = zhash_new();
    zhash_autofree (result);
    compute_result_value_set (result, ret_results.power);
    compute_result_scale_set (result, ret_results.scale);
    // TODO process missed ids
    // TODO quality
    compute_result_num_missed_set (result, ret_results.missed.size());

    // fill the return message
    zmsg_t* retmsg = compute_msg_encode_return_computation(result);
    return retmsg;
}

/**
 *  \brief rescale number to the new scale - upscalling loses information
 *
 *  assert's to prevent integer overflow
 */
static m_msrmnt_value_t s_rescale(m_msrmnt_value_t value, 
                m_msrmnt_scale_t old_scale, m_msrmnt_scale_t new_scale)
{
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

/**
 *  \brief add a value with a respect to the scale - downscale all the time
 */
static void s_add_scale(rack_power_t& ret, m_msrmnt_value_t value, 
                                                m_msrmnt_scale_t scale)
{
    if (ret.scale > scale) {
        ret.power = s_rescale(ret.power, ret.scale, scale);
        ret.scale = scale;
    }
    ret.power += s_rescale(value, ret.scale, scale);
}

static std::string s_generate_in_clause(
                            const std::map<m_dvc_id_t, a_elmnt_id_t>& map)
{
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

static rack_power_t
    doA(const char *url,
        const std::set <device_info_t> &upses,
        const std::set <device_info_t> &epdus,
        const std::set <device_info_t> &dvc,
        uint32_t max_age)
{
    log_info("start real calculation");
    assert ( max_age != 0 );

    // set of all asset_id-es to be computed
    std::set < a_elmnt_id_t > all_asset_ids{};
    
    //FIXME: this is stupid - aren't there union operations on sets in std C++?
    for (const device_info_t& d : upses)  
        all_asset_ids.insert(device_info_id(d));
    for (const device_info_t& d : epdus)  
        all_asset_ids.insert(device_info_id(d));
    for (const device_info_t& d : dvc)   
        all_asset_ids.insert(device_info_id(d));

    rack_power_t ret{0, 0, 255, all_asset_ids};
    if ( all_asset_ids.empty() )
    {
        log_debug ("end: no devices to compute was recieved");
        return ret;
    }
    
    // FIXME: asking for mapping each time sounds slow
    // convert id from asset part to monitor part
    std::map<m_dvc_id_t, a_elmnt_id_t> idmap;
    for ( a_elmnt_id_t asset_id : all_asset_ids ) 
    {
        try
        {
            m_dvc_id_t dev_id = convert_asset_to_monitor(url, asset_id);
            idmap[dev_id] = asset_id;
        }
        catch (const bios::NotFound &e) {
            log_info("asset element %d notfound, ignore it", asset_id);
        }
        catch (const bios::ElementIsNotDevice &e) {
            log_info("asset element %d is not a device, ignore it", asset_id);
        }
        catch (bios::MonitorCounterpartNotFound &e ) {
            log_warning("monitor counterpart for the %d was not found,"
                                            " ignore it", asset_id);
        }
        // ATTENTION: if internal, leave it to upper level
    }
    if ( idmap.empty() ) // no id was correctly converted
    {
        log_debug ("end: all devices were converted to monitor part"
                                                            "with errors");
        return ret;
    }

    // NOTE: cannot cache as SQL query is not stable
        
    // XXX: SQL placeholders can't deal with list of values, 
    // as we're using plain int, there is no issue in generating 
    // SQL by hand
        
    //TODO: read realpower.default from database, 
    // SELECT v.id as id_subkey, v.id_type as id_key 
    // FROM v_bios_measurement_subtypes v 
    // WHERE name='default' AND typename='realpower';
    try
    {
        tntdb::Connection conn = tntdb::connect(url);
        // v_..._last contain only last measures
        // TODO check its correctness
        tntdb::Statement st = conn.prepare(
            " SELECT"
            "    v.id_discovered_device, v.value, v.scale"
            " FROM"
            "   v_bios_client_info_measurements_last v"
            " WHERE"
            "   v.id_discovered_device IN (" 
                    + s_generate_in_clause(idmap) + ")"  // XXX
//                        "   AND v.id_key=3 AND v.id_subkey=1 "   // TODO
            "   AND v.id_key=3 AND v.id_subkey IN (1,5) "    
            "   AND v.timestamp BETWEEN"
            "       DATE_SUB(UTC_TIMESTAMP(), INTERVAL :seconds SECOND)"
            "       AND UTC_TIMESTAMP()"
        );

        tntdb::Result result = st.set("seconds", max_age).
                                  select();
        log_debug("rows selected %d", result.size());
        for ( auto &row: result )
        {
            m_dvc_id_t dev_id = 0;
            row[0].get(dev_id);
            ret.missed.erase(idmap[dev_id]);  //<- no assert needed, 
            // invalid value will raise an exception, 
            // converts from device id back to asset id

            m_msrmnt_value_t value = 0;
            row[1].get(value);

            m_msrmnt_scale_t scale = 0;
            row[2].get(scale);

            s_add_scale(ret, value, scale);
        }
    }
    catch (const std::exception &e) {
        log_warning ("end: abnormal with '%s'", e.what());
        throw bios::InternalDBError(e.what());
    }
    log_debug ("end: normal");
    return ret;
}

/**
 * FIXME: leave three arguments - one per device type, maybe in the future 
 * we'll use it, or change
 * TODO: ask for id_key/id_subkey, 
 * see measurement_id_t nut_get_measurement_id(const std::string &name) 
 * TODO: quality computation - to be defined, leave with 255
 */
rack_power_t
compute_total_rack_power_v1(
        const char *url,
        const std::set <device_info_t> &rack_devices,
        uint32_t max_age)
{
    log_info ("start");
    std::set <device_info_t> dvc = {};
    for ( auto &adevice : rack_devices )
        if (is_it_device(adevice))
            dvc.insert(adevice);

    log_debug ("number of IT devices is %ld", dvc.size());
    auto ret = doA (url, {}, {}, dvc, max_age);
    power_sources_t power_sources;
    log_debug ("number of IT devices informations is missing for is %ld", 
                                                        ret.missed.size());
    // for every missed value try to find its power path
    for ( auto &bdevice: ret.missed )
    {
        device_info_t src_device;
        // find more info about device from device set
        for ( auto &adevice : dvc )
        {
            if ( std::get<0>(adevice) == bdevice )
            {
                src_device = adevice;
                break;
            }
        }
        log_debug ("missed device processed:");
        log_debug ("device_id = %d", std::get<0>(src_device));
        log_debug ("device_name = %s", std::get<1>(src_device).c_str());
        log_debug ("device type name = %s", std::get<2>(src_device).c_str());
        log_debug ("device type_id = %d",std::get<3>(src_device));

        auto new_power_srcs = select_pow_src_for_device (url, src_device);

        // in V1 if power_source should be taken in account iff 
        // it is in rack
        
        log_debug ("power sources for device %d", std::get<0>(src_device));
        log_debug ("num of epdus is %ld", std::get<0>(new_power_srcs).size());
        log_debug ("num of upses is %ld", std::get<1>(new_power_srcs).size());
        log_debug ("num of devices is %ld", std::get<2>(new_power_srcs).size());
        
        for ( auto &bdevice: std::get<0>(new_power_srcs) )
            if ( rack_devices.count(bdevice) == 1 )
                std::get<0>(power_sources).insert (bdevice);
            else
            {
                // decrese quality
            }

        for ( auto &bdevice: std::get<1>(new_power_srcs) )
            if ( rack_devices.count(bdevice) == 1 )
                std::get<1>(power_sources).insert (bdevice);
            else
            {
                // decrese quality
            }

        for ( auto &bdevice: std::get<2>(new_power_srcs) )
            if ( rack_devices.count(bdevice) == 1 )
                std::get<2>(power_sources).insert (bdevice);
            else
            {
                // decrese quality
            }
    }
    log_debug ("total power sources");
    log_debug ("num of epdus is %ld", std::get<0>(power_sources).size());
    log_debug ("num of upses is %ld", std::get<1>(power_sources).size());
    log_debug ("num of devices is %ld", std::get<2>(power_sources).size());
    auto ret2 = doA (url, std::get<1>(power_sources), 
                          std::get<0>(power_sources),
                          std::get<2>(power_sources), max_age);
    s_add_scale (ret, ret2.power, ret2.scale);
    // TODO manage quality + missed
    log_debug ("end: power = %ld, scale = %d", ret.power, ret.scale);
    return ret;
}

std::set < a_elmnt_id_t > find_racks (zframe_t* frame, 
                                                m_dvc_tp_id_t parent_type_id)
{
    log_debug ("start");
    std::set < a_elmnt_id_t > result;
    
    byte *buffer = zframe_data (frame);
    assert ( buffer );
    
    zmsg_t* zmsg = zmsg_decode (buffer, zframe_size (frame));
    assert ( zmsg );
     
    zmsg_t* pop = NULL;
    zframe_t* fr;
    while ( ( pop = zmsg_popmsg (zmsg) ) != NULL )
    { 
        asset_msg_t *item = asset_msg_decode (&pop); // zmsg_t is freed
        asset_msg_print(item);
        assert ( item );
         
        // process rooms
        if ( parent_type_id == asset_type::DATACENTER )
        {
            fr = asset_msg_rooms (item);
            auto result1 = find_racks (fr, asset_msg_type(item));
            result.insert (result1.begin(), result1.end());
        }
        
        // process rows
        if ( ( parent_type_id == asset_type::DATACENTER ) ||
             ( parent_type_id == asset_type::ROOM ) )
        {
            fr = asset_msg_rows (item);
            auto result1 = find_racks (fr, asset_msg_type(item));
            result.insert (result1.begin(), result1.end());
        }
        
        // process racks
        if ( ( parent_type_id == asset_type::DATACENTER ) ||
             ( parent_type_id == asset_type::ROOM ) ||
             ( parent_type_id == asset_type::ROW ) )
        {
            fr = asset_msg_racks (item);
            auto result1 = find_racks (fr, asset_msg_type(item)); 
            result.insert (result1.begin(), result1.end());
        }
        
        result.insert (asset_msg_element_id (item));
        
        asset_msg_destroy (&item);
        assert ( pop == NULL );
    }
    zmsg_destroy (&zmsg);
    log_debug ("end");
    return result;
}


// DC POWER
zmsg_t* calc_total_dc_power (const char *url, a_elmnt_id_t dc_element_id)
{
    log_info ("start \n");
    // check if specified device has a dc type
    try{
        a_elmnt_id_t type_id = select_element_type (url, dc_element_id);
        if ( type_id == 0 )
        {
            log_info ("end: %d DC was not found", dc_element_id);
            return common_msg_encode_fail (BIOS_ERROR_DB, DB_ERROR_NOTFOUND, 
                                    "specified DC was not found", NULL);
        }
        if ( type_id != asset_type::DATACENTER ) 
        {
            log_info ("end: %d isn't DC", dc_element_id);
            return common_msg_encode_fail(BIOS_ERROR_DB, DB_ERROR_BADINPUT, 
                                    "specified element is not a DC", NULL);
        }
    }
    catch (const bios::InternalDBError &e)
    {
        log_warning ("end: abnormal with '%s'", e.what());
        return common_msg_encode_fail(BIOS_ERROR_DB, DB_ERROR_INTERNAL, 
                                    e.what(), NULL);
    }

    // continue, select all racks in a dc
    // unfortunatly there is no structure where to store topology, so 
    // it is directly stored in message
    // here we need to select und unpack it
    
    // fill the getmsg
    asset_msg_t* getmsg = asset_msg_new (ASSET_MSG_GET_LOCATION_FROM);
    asset_msg_set_element_id (getmsg, dc_element_id);
    asset_msg_set_recursive (getmsg, true);
    asset_msg_set_filter_type (getmsg, asset_type::RACK);
    log_debug("get_msg filled");

    // get topology
    zmsg_t* dc_topology = get_return_topology_from (url, getmsg);
    asset_msg_t* item = asset_msg_decode (&dc_topology);
    log_debug("get_msg decoded");

    // find all ids of rack in topology
    // process rooms
    log_debug("start look racks in rooms");
    zframe_t* fr = asset_msg_rooms (item);
    std::set <a_elmnt_id_t > rack_ids = find_racks (fr, asset_msg_type(item));
    log_debug("end look racks in rooms");
    log_debug("number of racks found: %ld", rack_ids.size());
        
    // process rows
    log_debug("start look racks in rows");
    fr = asset_msg_rows (item);
    auto tmp = find_racks (fr, asset_msg_type(item));
    rack_ids.insert(tmp.begin(), tmp.end());
    log_debug("end look racks in rows");
    log_debug("total number of racks found: %ld", rack_ids.size());
        
    // process racks
    log_debug("strat look racks");
    fr = asset_msg_racks (item);
    auto tmp1 = find_racks (fr, asset_msg_type(item)); 
    rack_ids.insert(tmp1.begin(), tmp1.end());
    log_debug("end look racks");
    log_debug("total number of racks found: %ld", rack_ids.size());

    // set of results for every rack
    std::vector<rack_power_t> ret_results_rack{rack_ids.size()};
    size_t i = 0;
    for ( auto &rack_id: rack_ids )
    {
        // select all devices in a rack
        log_debug("start process the rack: %d", rack_id);
        auto rack_devices = select_rack_devices(url, rack_id);

        // calc sum
        auto aresult = compute_total_rack_power_v1 (
                url, rack_devices, 
                300);
        ret_results_rack.insert(ret_results_rack.begin(), aresult);
        log_debug("end process the rack: %d with value = %ld and scale = %d", rack_id, ret_results_rack[i].power, ret_results_rack[i].scale);
    }
    log_debug("here 11");

    // calc the total summ
    rack_power_t ret_result{0, 0, 255, rack_ids};
    for ( auto &one_rack : ret_results_rack )
        s_add_scale(ret_result, one_rack.power, one_rack.scale);
    log_debug("total: value = %ld, scale = %d", ret_result.power, ret_result.scale);

    // transform numbers to string and fill hash
    zhash_t* result = zhash_new();
    zhash_autofree (result);
    compute_result_value_set (result, ret_result.power);
    compute_result_scale_set (result, ret_result.scale);
    // TODO process missed ids
    compute_result_num_missed_set (result, ret_result.missed.size());

    // fill the return message
    zmsg_t* retmsg = compute_msg_encode_return_computation(result);
    log_debug("end: normal");
    return retmsg;
}
