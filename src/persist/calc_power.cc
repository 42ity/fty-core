#include "calc_power.h"
#include "dbtypes.h"
#include "dbpath.h"
#include "log.h"
#include "persist_error.h"

#include <tntdb/connect.h>
#include <tntdb/row.h>
#include <tntdb/error.h>
#include <tntdb/value.h>
#include <tntdb/result.h>

#include "common_msg.h"
#include "monitor.h"
#include "defs.h"
#include "asset_types.h"
#include "log.h"

#define DEVICE_TYPE_EPDU 3
#define DEVICE_TYPE_PDU 4
#define DEVICE_TYPE_UPS 1

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


//TODO move to map also
std::tuple < std::set < device_info_t >, 
             std::set < device_info_t >, 
             std::set < device_info_t>  >
    doA ( std::pair < std::set < device_info_t >, 
                      std::set < powerlink_info_t > > power_topology, 
          device_info_t start_device )
{
    log_info ("start \n");

    auto devices = power_topology.first;
    auto powerlinks = power_topology.second;
    
    log_debug ("number of devices is %ld \n", devices.size());
    log_debug ("number of powerdevices is %ld \n", powerlinks.size());

    // start_device can be
    // - IT device     - if it is the first call
    // - Power device  - if it is a recursive call, because somewhere before
    //                      we found an PDU power source
   
    // a result sets of devices to ask for a power measurement
    std::set < device_info_t > pow_src_epdu;
    std::set < device_info_t > pow_src_ups;
    std::set < device_info_t > pow_src_it_device;
   
    // process links
    for ( auto &link : powerlinks )
    {
        // get src device id
        a_elmnt_id_t src_id = std::get<0>(link);

        // find more info about device
        // TODO rewrite devices to MAP!!!!! for better effectivity
        device_info_t src_device;
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
                     (url.c_str(), std::get<0>(src_device), INPUT_POWER_CHAIN, false);
                auto pdu_pow_srcs = doA (pdu_pow_top, src_device);
          
                pow_src_epdu.insert (std::get<0>(pdu_pow_srcs).begin(), std::get<0>(pdu_pow_srcs).end());
                pow_src_ups.insert (std::get<1>(pdu_pow_srcs).begin(), std::get<1>(pdu_pow_srcs).end());
                pow_src_it_device.insert (std::get<2>(pdu_pow_srcs).begin(), std::get<2>(pdu_pow_srcs).end());
            }
            // TODO what should we do with the errors
            catch (const bios::NotFound &e) {
            }
            catch (const bios::InternalDBError &e) {
            }
            catch (const bios::ElementIsNotDevice &e) {
            }
        }
        else
        {   
            //issue some warning, because this is not normal
            pow_src_it_device.insert (start_device);
        }
    }

    log_info ("end \n");
    return std::make_tuple (pow_src_epdu, pow_src_ups, pow_src_it_device);
}
    
std::set <device_info_t> select_rack_devices(const char* url, a_elmnt_id_t element_id)
{
    // ASSUMPTION
    // specified element_id is already in DB and has type asset_type::RACK
    std::set <device_info_t> result_set;
    try{
        tntdb::Connection conn = tntdb::connectCached(url); 
               
        // TODO get rid of duplicate device processing
        tntdb::Statement st = conn.prepareCached(
                " SELECT"
                "    v.id, v.name, v1.name as type_name, v1.id_asset_device_type"
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
        return result_set;
    }
    catch (const std::exception &e) {
        log_warning ("abort with err = '%s'\n", e.what());
        throw bios::InternalDBError(e.what());
    }
}

a_elmnt_tp_id_t select_element_type (const char* url, a_elmnt_id_t asset_element_id)
{
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
        return asset_element_type_id;
    }
    catch (const tntdb::NotFound &e) {
        // element with specified id was not found
        return 0;
    }
    catch (const std::exception &e){
        // log_warning ("abort with err = '%s'\n", e.what());
        throw bios::InternalDBError(e.what());
    }
}
                     
common_msg_t* calc_total_rack_power (const char *url, a_elmnt_id_t rack_element_id)
{
    // check if specified device has a rack type
    try{
        a_elmnt_id_t type_id = select_element_type (url, rack_element_id);
        if ( type_id == 0 )
            return generate_db_fail (DB_ERROR_NOTFOUND, "specified element was not found", NULL);
        if (  type_id != asset_type::RACK ) 
            return generate_db_fail (DB_ERROR_BADINPUT, "specified element is not a rack", NULL);
        return NULL;
    }
    catch ( bios::InternalDBError() )
    {
        return generate_db_fail(DB_ERROR_INTERNAL,"",NULL);
    }
}
