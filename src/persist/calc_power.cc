/*
Copyright (C) 2015 Eaton

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <set>

#include "log.h"
#include "calc_power.h"

#include "asset_types.h"
#include "assetcrud.h"

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


/**
 *  \brief From set of links derives set of elements that at least once were
 *  a dest device in a link.
 *  link is: from to
 */
static std::set<a_elmnt_id_t>
    find_dests
        (const std::set <std::pair<a_elmnt_id_t, a_elmnt_id_t> > &links,
         a_elmnt_id_t element_id)
{
    std::set<a_elmnt_id_t> dests;
    
    for ( auto &one_link: links )
    {
        if ( std::get<0>(one_link) == element_id )
            dests.insert(std::get<1>(one_link));
    }
    return dests;
}


/**
 *  \brief An implementation of looking for a "border" devices
 *
 *  A border device is a device, that should be taken as a power
 *  source device at first turn
 */
static void
    update_border_devices
        (const std::map <a_elmnt_id_t, device_info_t> &dc_devices,
         const std::set <std::pair<a_elmnt_id_t, a_elmnt_id_t> > &links,
         std::set <device_info_t> &border_devices)
{
    LOG_START;

    std::set<device_info_t> new_border_devices;
    for ( auto &border_device: border_devices )
    {
        auto adevice_dests = find_dests (links, std::get<0>(border_device));
        for ( auto &adevice: adevice_dests )
        {
            auto it = dc_devices.find(adevice);
            if ( it != dc_devices.end() )
                new_border_devices.insert(dc_devices.find(adevice)->second);
            else
            {
                log_critical ("database is in inconsistent state");
                log_critical ("device(as element) %" PRIu32 " is not in dc",
                                                std::get<0>(border_device));
                assert(false);
            }
        }
    }
    border_devices.clear();
    border_devices.insert(new_border_devices.begin(),
                          new_border_devices.end());
    LOG_END;
}


/**
 *  \brief An implementation of the algorithm.
 *
 *  Take a first "smart" device in every powerchain thatis closest to "main"
 *  If device is not smart, try to look at upper level. Repeat until
 *  chain ends or until all chains are processed
 */
static std::vector<std::string>
    compute_total_power_v2(
        const std::map <a_elmnt_id_t, device_info_t> &devices_in_container,
        const std::set <std::pair<a_elmnt_id_t, a_elmnt_id_t> > &links,
        std::set <device_info_t> border_devices)
{
    LOG_START;

    std::vector <std::string> dvc{};

    if ( border_devices.empty() )
        return dvc;
    // it is not a good idea to delete from collection while iterating it
    std::set <device_info_t> todelete{};
    while ( !border_devices.empty() )
    {
        for ( auto &border_device: border_devices )
        {
            if ( is_ups(border_device) || is_epdu(border_device) )
            {
                dvc.push_back(std::get<1>(border_device));
                // remove from border
                todelete.insert(border_device);
                continue;
            }
            // NOT IMPLEMENTED
            //if ( is_it_device(border_device) )
            //{
            //    // remove from border
            //    // add to ipmi
            //}
        }
        for (auto &todel: todelete)
            border_devices.erase(todel);
        update_border_devices(devices_in_container, links, border_devices);
    }
    LOG_END;
    return dvc;
}


/**
 *  \brief For every container returns a list of its power sources
 */
static db_reply <std::map<std::string, std::vector<std::string> > >
    select_devices_total_power_container
        (tntdb::Connection  &conn,
         int8_t container_type_id)
{
    LOG_START;
    log_debug ("  container_type_id = %" PRIi8, container_type_id);
    // name of the container is mapped onto the vector of names of its power sources
    std::map<std::string, std::vector<std::string> > item{};
    db_reply <std::map<std::string, std::vector<std::string> > > ret =
                db_reply_new(item);

    // there is no need to do all in one select, so let's do it by steps
    // select all containers by type
    auto allContainers = select_asset_elements_by_type 
                                    (conn, container_type_id);

    if  ( allContainers.status == 0 )
    {
        ret.status     = 0;
        ret.msg        = allContainers.msg;
        ret.errtype    = allContainers.errtype;
        ret.errsubtype = allContainers.errsubtype;
        log_error ("some error appears, during selecting the containers");
        return ret;
    }
    // if there is no containers, then it is an error
    if  ( allContainers.item.empty() )
    {
        ret.status     = 0;
        ret.msg        = "there is no containers of requested type";
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_NOTFOUND;
        log_warning (ret.msg.c_str());
        return ret;
    }

    // go through every container and "compute" what should be summed up
    for ( auto &container : allContainers.item )
    {
        // select all devices in the container
        auto container_devices_set = select_asset_device_by_container
                                                        (conn, container.id);

        // here would be placed names of devices to summ up
        std::vector<std::string> result(0);

        if ( container_devices_set.status == 0 )
        {
            log_warning ("'%s': problems appeared in selecting devices",
                                                    container.name.c_str());
            // so return an empty set of power devices
            ret.item.insert(std::pair< std::string, std::vector<std::string> >
                                                    (container.name, result));
            continue;
        }
        if ( container_devices_set.item.empty() )
        {
            log_warning ("'%s': has no devices", container.name.c_str());
            // so return an empty set of power devices
            ret.item.insert(std::pair< std::string, std::vector<std::string> >
                                                    (container.name, result));
            continue;
        }

        // create a map, for better use
        std::map <a_elmnt_id_t, device_info_t> container_devices{};
        for ( auto &container_dev: container_devices_set.item )
        {
            a_elmnt_id_t tmp = std::get<0>(container_dev);
            container_devices.insert(std::pair<a_elmnt_id_t, device_info_t>
                                                        (tmp, container_dev));
        }

        auto links = select_links_by_container (conn, container.id);
        if ( links.status == 0 )
        {
            log_warning ("'%s': internal problems in links detecting",
                                                    container.name.c_str());
            // so return an empty set of power devices
            ret.item.insert(std::pair< std::string, std::vector<std::string> >
                                                    (container.name, result));
            continue;
        }

        if ( links.item.empty() )
        {
            log_warning ("'%s': has no power links", container.name.c_str());
            // so return an empty set of power devices
            ret.item.insert(std::pair< std::string, std::vector<std::string> >
                                                    (container.name, result));
            continue;
        }

        // the set of all border devices ("starting points")
        std::set <device_info_t> border_devices;
        // the set of all destination devices in selected links
        std::set <a_elmnt_id_t> dest_dvcs{};
        //  from (first)   to (second)
        //           +--------------+ 
        //  B________|______A__C    |
        //           |              |
        //           +--------------+
        //   B is out of the Container
        //   A is in the Container
        //   then A is border device
        for ( auto &oneLink : links.item )
        {
            log_debug ("  cur_link: %d->%d", oneLink.first, oneLink.second);
            auto it = container_devices.find (oneLink.first);
            if ( it == container_devices.end() )
                // if in the link first point is out of the Container,
                // the second definitely should be in Container,
                // otherwise it is not a "container"-link
            {
                border_devices.insert(
                            container_devices.find(oneLink.second)->second);
            }
            dest_dvcs.insert(oneLink.second);
        }
        //  from (first)   to (second)
        //           +-----------+ 
        //           |A_____C    |
        //           |           |
        //           +-----------+
        //   A is in the Container (from)
        //   C is in the Container (to)
        //   then A is border device
        //
        //   Algorithm: from all devices in the Container we will
        //   select only those that don't have an incoming links
        //   (they are not a destination device for any link)
        for ( auto &oneDevice : container_devices )
        {
            if ( dest_dvcs.find (oneDevice.first) == dest_dvcs.end() )
                border_devices.insert ( oneDevice.second );
        }

        result = compute_total_power_v2(container_devices, links.item,
                                        border_devices);
        ret.item.insert(std::pair< std::string, std::vector<std::string> >
                                                (container.name, result));
    }
    LOG_END;
    return ret;
}


db_reply <std::map<std::string, std::vector<std::string> > >
    select_devices_total_power_dcs
        (tntdb::Connection  &conn)
{
    return select_devices_total_power_container (conn, asset_type::DATACENTER);
}


db_reply <std::map<std::string, std::vector<std::string> > >
    select_devices_total_power_racks
        (tntdb::Connection  &conn)
{
    return select_devices_total_power_container (conn, asset_type::RACK);
}
