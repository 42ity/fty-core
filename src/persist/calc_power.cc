#include "calc_power.h"
#include "dbtypes.h"
#include "dbpath.h"
#include "log.h"

#define DEVICE_TYPE_EPDU "epdu"
#define DEVICE_TYPE_PDU "pdu"
#define DEVICE_TYPE_UPS "ups"

bool is_epdu (const device_info_t &device)
{
    if ( std::get<3>(device) == DEVICE_TYPE_EPDU )
        return true;
    else
        return false;
}

bool is_pdu (const device_info_t &device)
{
    if ( std::get<2>(device) == DEVICE_TYPE_PDU )
        return true;
    else
        return false;
}

bool is_ups (const device_info_t &device)
{
    if ( std::get<2>(device) == DEVICE_TYPE_UPS )
        return true;
    else
        return false;
}


//TODO move to map also
std::set < device_info_t > doA ( std::pair < std::set < device_info_t >, 
                                            std::set < powerlink_info_t > > power_topology, 
                               device_info_t start_device )
{
    // start_device can be it_device, so it is the first call
    // or power device, because somewhere before we found an PDU power device
   
    //result set of powersources
    //can be
    //UPS
    //EPDU
    //IT_device
    std::set < device_info_t > power_sources;

    auto devices = power_topology.first;
   
    // process links
    for ( auto &link : power_topology.second )
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
        // TODO add device_type_id to returned set
        if ( is_epdu (src_device) ||  is_ups (src_device) )
            power_sources.insert(src_device);
        else if ( is_pdu (src_device) )  
        {
            // ??? da se spocitat power z PDU???
            // select power_topology for PDU, that powers this PDU
            try{
                auto pdu_power_topology = select_power_topology_to 
                     (url.c_str(), std::get<0>(src_device), INPUT_POWER_CHAIN, false);
                auto pdu_power_source = doA (pdu_power_topology, src_device);
          
                power_sources.insert (pdu_power_source.begin(), pdu_power_source.end());
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
            power_sources.insert (start_device);
        }
    }

    return power_sources;
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
            assert ( device_name );

            std::string device_type_name = "";
            row[2].get(device_type_name);
            asser2t ( device_type_name );

            a_dvc_tp_id_t device_type_id = 0;
            row[3].get(device_type_id);
            assert ( device_type_id );
            
            result_set.insert (std::make_tuple(device_asset_id, device_name, 
                                               device_type_name, device_type_id));
        }
        log_info ("end\n");
        return res;
    }
    catch (const std::exception &e) {
        log_warning ("abort with err = '%s'\n", e.what());
        result_set.erase();
        throw bios::InternalDBError;
    }
}
                     
common_msg_t* calc_total_rack_power (const char *url, a_elmnt_id_t rack_element_id)
{
    // check if specified device has a rack type
    if ( select_element_type (url, rack_element_id) == asset_type::RACK )
        return generate_db_fail (DB_ERROR_BADINPUT, "specified element is not a rack", NULL);

    
}
