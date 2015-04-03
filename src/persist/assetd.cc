/*
Copyright (C) 2014-2015 Eaton
 
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

/*! \file assetcr.cc
    \brief Pure DB API for delete for different tables

    \author Alena Chernikava <alenachernikava@eaton.com>
*/

#include <tntdb/connect.h>
#include <tntdb/row.h>
#include <tntdb/result.h>
#include <tntdb/error.h>
#include <tntdb/transaction.h>

#include "log.h"
#include "defs.h"
#include "assetcrud.h"
#include "monitor.h"
#include "persist_error.h"

// ATTENTION: there is no easy way of getting last deleted id,
// and there is no requirements to do this.
// Then for every succesfull delete statement
// 0 would be return as rowid

/////////////////////// DELETE ///////////////////////////////////////////
//=============================================================================
db_reply_t
    delete_asset_device
        (tntdb::Connection &conn, 
         a_elmnt_id_t asset_element_id)
{
    LOG_START;
    log_debug ("asset_element_id = %" PRIu32, asset_element_id);
    
    db_reply_t ret = db_reply_new();
    
    // input parameters control 
    if ( asset_element_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of asset_element_id is not allowed";
        log_error ("end: %s, %s", "ignore delete", ret.msg);
        return ret;
    }
    log_debug ("input parameters are correct");
  
    try{
        tntdb::Statement st = conn.prepareCached(
            " DELETE FROM"
            "   t_bios_asset_device"
            " WHERE"
            "   id_asset_element = :element"
        );
    
        ret.affected_rows = st.set("element", asset_element_id).
                               execute();
        log_debug ("[t_bios_asset_element]: was deleted %" 
                                PRIu64 " rows", ret.affected_rows);
        if ( ( ret.affected_rows == 1 ) || ( ret.affected_rows == 0 ) )
        {
            ret.status = 1;
            LOG_END;
            return ret;
        }
        else
        {
            ret.status     = 0;
            ret.errtype    = DB_ERR;
            ret.errsubtype = DB_ERROR_DELETEFAIL;
            ret.msg        = "unexpected number of rows was deleted";
            log_error ("end: %s", ret.msg);
            return ret;
        } 
    }
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

//=============================================================================
// ATTENTION: in theory there could exist more than one link 
// between two devices
db_reply_t
    delete_asset_link
        (tntdb::Connection &conn, 
         a_elmnt_id_t asset_element_id_src,
         a_elmnt_id_t asset_element_id_dest)
{
    LOG_START;
    log_debug ("  asset_element_id_src = %" PRIu32, asset_element_id_src);
    log_debug ("  asset_element_id_dest = %" PRIu32, asset_element_id_dest);

    db_reply_t ret = db_reply_new();

    // input parameters control 
    if ( asset_element_id_src == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of asset_element_id_src is not allowed";
        log_error ("end: %s, %s", "ignore delete", ret.msg);
        return ret;
    }
    if ( asset_element_id_src == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of asset_element_id_src is not allowed";
        log_error ("end: %s, %s", "ignore delete", ret.msg);
        return ret;
    }
    log_debug ("input parameters are correct");

    try{
        tntdb::Statement st = conn.prepareCached(
            " DELETE"
            "   t_bios_asset_link"
            " FROM"
            "   t_bios_asset_link"
            "   INNER JOIN v_bios_asset_link"
            " WHERE"
            "   t_bios_asset_link.id_asset_device_src = "
            "           v_bios_asset_link.id_asset_device_src AND"
            "   t_bios_asset_link.id_asset_device_dest = "
            "           v_bios_asset_link.id_asset_device_dest AND"
            "   v_bios_asset_link.id_asset_element_src = :src AND"
            "   v_bios_asset_link.id_asset_element_dest = :dest"
        );

        ret.affected_rows = st.set("src", asset_element_id_src).
                               set("dest", asset_element_id_dest).
                               execute();
        log_debug ("[t_bios_asset_link]: was deleted %"
                                    PRIu64 " rows", ret.affected_rows);
        ret.status = 1;
        LOG_END;
        return ret;
    }
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

//=============================================================================
db_reply_t
    delete_asset_links_all
        (tntdb::Connection &conn,
         a_elmnt_id_t asset_element_id)
{
    LOG_START;
    log_debug ("  asset_element_id = %" PRIu32, asset_element_id);

    db_reply_t ret = db_reply_new();

    // input parameters control
    if ( asset_element_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of asset_element_id is not allowed";
        log_error ("end: %s, %s", "ignore delete", ret.msg);
        return ret;
    }
    log_debug ("input parameters are correct");

    try{
        tntdb::Statement st = conn.prepareCached(
            " DELETE"
            "   t_bios_asset_link"
            " FROM"
            "   t_bios_asset_link"
            "   INNER JOIN t_bios_asset_device"
            " WHERE"
            "   t_bios_asset_device.id_asset_element = :element AND"
            "   ( ( t_bios_asset_link.id_asset_device_src = "
            "               t_bios_asset_device.id_asset_device) OR"
            "     ( t_bios_asset_link.id_asset_device_dest = "
            "               t_bios_asset_device.id_asset_device) )"
        );

        ret.affected_rows = st.set("element", asset_element_id).
                               execute();
        log_debug ("[t_bios_asset_link]: was deleted %"
                                PRIu64 " rows", ret.affected_rows);
        ret.status = 1;
        LOG_END;
        return ret;
    }
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

//=============================================================================
db_reply_t
    delete_asset_links_to
        (tntdb::Connection &conn, 
         a_dvc_id_t asset_device_id)
// ATTENTION: asset_device_id is from t_bios_asset_device
// and it is NOT from t_bios_asset_element;
{
    LOG_START;
    log_debug ("  asset_device_id = %" PRIu32, asset_device_id);

    db_reply_t ret = db_reply_new();

    // input parameters control
    if ( asset_device_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of asset_device_id is not allowed";
        log_error ("end: %s, %s", "ignore delete", ret.msg);
        return ret;
    }
    log_debug ("input parameters are correct");

    try{
        tntdb::Statement st = conn.prepareCached(
            " DELETE FROM"
            "   t_bios_asset_link"
            " WHERE"
            "   id_asset_device_dest = :dest"
        );

        ret.affected_rows = st.set("dest", asset_device_id).
                               execute();
        log_debug ("[t_bios_asset_link]: was deleted %"
                                PRIu64 " rows", ret.affected_rows);
        ret.status = 1;
        LOG_END;
        return ret;
    }
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

//=============================================================================
db_reply_t
    delete_asset_links_from
        (tntdb::Connection &conn, 
         a_dvc_id_t asset_device_id)
// ATTENTION: asset_device_id is from t_bios_asset_device
// and it is NOT from t_bios_asset_element;
{
    LOG_START;
    log_debug ("  asset_device_id = %" PRIu32, asset_device_id);

    db_reply_t ret = db_reply_new();

    // input parameters control
    if ( asset_device_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of asset_device_id is not allowed";
        log_error ("end: %s, %s", "ignore delete", ret.msg);
        return ret;
    }
    log_debug ("input parameters are correct");

    try{
        tntdb::Statement st = conn.prepareCached(
            " DELETE FROM"
            "   t_bios_asset_link"
            " WHERE"
            "   id_asset_device_src = :src"
        );
    
        ret.affected_rows = st.set("src", asset_device_id).
                               execute();
        log_debug ("[t_bios_asset_link]: was deleted %"
                                PRIu64 " rows", ret.affected_rows);
        ret.status = 1;
        LOG_END;
        return ret;
    }
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

//=============================================================================
db_reply_t
    delete_asset_group_links
        (tntdb::Connection &conn, 
         a_elmnt_id_t asset_group_id)
{
    LOG_START;
    log_debug ("  asset_group_id = %" PRIu32, asset_group_id);

    db_reply_t ret = db_reply_new();

    // input parameters control
    if ( asset_group_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of asset_group_id is not allowed";
        log_error ("end: %s, %s", "ignore delete", ret.msg);
        return ret;
    }
    log_debug ("input parameters are correct");

    try{
        tntdb::Statement st = conn.prepareCached(
            " DELETE FROM"
            "   t_bios_asset_group_relation"
            " WHERE"
            "   id_asset_group = :grp"
        );
    
        ret.affected_rows = st.set("grp", asset_group_id).
                               execute();
        log_debug ("[t_bios_asset_group_relation]: was deleted %"
                                PRIu64 " rows", ret.affected_rows);
        ret.status = 1;
        LOG_END;
        return ret;
    }
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

//=============================================================================
db_reply_t
    delete_asset_ext_attribute
        (tntdb::Connection &conn, 
         const char   *keytag,
         a_elmnt_id_t  asset_element_id)
{
    LOG_START;
    log_debug ("keytag = '%s'", keytag);
    log_debug ("asset_element_id = %" PRIu32, asset_element_id);

    db_reply_t ret = db_reply_new();

    // input parameters control
    if ( !is_ok_keytag (keytag) )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "unexpected value of keytag";
        log_error ("end: %s, %s", "ignore delete", ret.msg);
        return ret;
    }
    if ( asset_element_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of asset_element_id is not allowed";
        log_error ("end: %s, %s", "ignore delete", ret.msg);
        return ret;
    }
    log_debug ("input parameters are correct");

    try{
        tntdb::Statement st = conn.prepareCached(
            " DELETE FROM"
            "   t_bios_asset_ext_attributes"
            " WHERE"
            "   keytag = :keytag AND"
            "   id_asset_element = :element"
        );

        ret.affected_rows = st.set("keytag", keytag).
                               set("element", asset_element_id).
                               execute();
        log_debug("[t_bios_asset_ext_attributes]: was deleted %"
                                PRIu64 " rows", ret.affected_rows);
        if ( ( ret.affected_rows == 1 ) || ( ret.affected_rows == 0 ) )
        {
            ret.status = 1;
            LOG_END;
            return ret;
        }
        else
        {
            ret.status     = 0;
            ret.errtype    = DB_ERR;
            ret.errsubtype = DB_ERROR_DELETEFAIL;
            ret.msg        = "unexpected number of rows was deleted";
            log_error ("end: %s", ret.msg);
            return ret;
        }
    }
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

//=============================================================================
db_reply_t
    delete_asset_ext_attributes
        (tntdb::Connection &conn, 
         a_elmnt_id_t asset_element_id)
{
    LOG_START;
    log_debug ("asset_element_id = %" PRIu32, asset_element_id);
   
    db_reply_t ret = db_reply_new();

    // input parameters control
    if ( asset_element_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of asset_element_id is not allowed";
        log_error ("end: %s, %s", "ignore delete", ret.msg);
        return ret;
    }
    log_debug ("input parameters are correct");
 
    try{
        tntdb::Statement st = conn.prepareCached(
            " DELETE FROM"
            "   t_bios_asset_ext_attributes"
            " WHERE"
            "   id_asset_element = :element"
        );
    
        ret.affected_rows = st.set("element", asset_element_id).
                               execute();
        log_debug("[t_bios_asset_ext_attributes]: was deleted %"
                                PRIu64 " rows", ret.affected_rows);
        ret.status = 1;
        LOG_END;
        return ret;
    } 
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

//=============================================================================
db_reply_t
    delete_asset_element
        (tntdb::Connection &conn, 
         a_elmnt_id_t asset_element_id)
{
    LOG_START;
    log_debug ("asset_element_id = %" PRIu32, asset_element_id);
      
    db_reply_t ret = db_reply_new();

    // input parameters control
    if ( asset_element_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of asset_element_id is not allowed";
        log_error ("end: %s, %s", "ignore delete", ret.msg);
        return ret;
    }
    log_debug ("input parameters are correct");

    try{
        tntdb::Statement st = conn.prepareCached(
            " DELETE FROM"
            "   t_bios_asset_element"
            " WHERE"
            "   id_asset_element = :element"
        );
    
        ret.affected_rows  = st.set("element", asset_element_id).
                                execute();
        log_debug("[t_bios_asset_element]: was deleted %"
                                PRIu64 " rows", ret.affected_rows);
        if ( ( ret.affected_rows == 1 ) || ( ret.affected_rows == 0 ) )
        {
            ret.status = 1;
            LOG_END;
            return ret;
        }
        else
        {
            ret.status     = 0;
            ret.errtype    = DB_ERR;
            ret.errsubtype = DB_ERROR_DELETEFAIL;
            ret.msg        = "unexpected number of rows was deleted";
            log_error ("end: %s", ret.msg);
            return ret;
        }
    }
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

//=============================================================================
db_reply_t
    delete_asset_element_from_asset_groups
        (tntdb::Connection &conn, 
         a_elmnt_id_t asset_element_id)
{
    LOG_START;
    log_debug ("asset_element_id = %" PRIu32, asset_element_id);
   
    db_reply_t ret = db_reply_new();

    // input parameters control
    if ( asset_element_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of asset_element_id is not allowed";
        log_error ("end: %s, %s", "ignore delete", ret.msg);
        return ret;
    }
    log_debug ("input parameters are correct");

    try{
        tntdb::Statement st = conn.prepareCached(
            " DELETE FROM"
            "   t_bios_asset_group_relation"
            " WHERE"
            "   id_asset_element = :element"
        );
    
        ret.affected_rows = st.set("element", asset_element_id).
                               execute();
        log_debug("[t_bios_asset_group_relation]: was deleted %"
                                PRIu64 " rows", ret.affected_rows);
        ret.status = 1;
        LOG_END;
        return ret;
    } 
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

//=============================================================================
db_reply_t
    delete_asset_element_from_asset_group
        (tntdb::Connection &conn, 
         a_elmnt_id_t asset_group_id,
         a_elmnt_id_t asset_element_id)
{
    LOG_START;
    log_debug ("  asset_group_id = %" PRIu32, asset_group_id);
    log_debug ("  asset_element_id = %" PRIu32, asset_element_id);
       
    db_reply_t ret = db_reply_new();

    // input parameters control
    if ( asset_element_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of asset_element_id is not allowed";
        log_error ("end: %s, %s", "ignore delete", ret.msg);
        return ret;
    }
    if ( asset_group_id == 0 )
    {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_BADINPUT;
        ret.msg        = "0 value of asset_group_id is not allowed";
        log_error ("end: %s, %s", "ignore delete", ret.msg);
        return ret;
    }
    log_debug ("input parameters are correct");
 
    try{
        tntdb::Statement st = conn.prepareCached(
            " DELETE FROM"
            "   t_bios_asset_group_relation"
            " WHERE"
            "   id_asset_group = :grp AND"
            "   id_asset_element = :element"
        );
    
        ret.affected_rows = st.set("grp", asset_group_id).
                               set("element", asset_element_id).
                               execute();
        log_debug("[t_bios_asset_group_relation]: was deleted %"
                                PRIu64 " rows", ret.affected_rows);
        if ( ( ret.affected_rows == 1 ) || ( ret.affected_rows == 0 ) )
        {
            ret.status = 1;
            LOG_END;
            return ret;
        }
        else
        {
            ret.status     = 0;
            ret.errtype    = DB_ERR;
            ret.errsubtype = DB_ERROR_DELETEFAIL;
            ret.msg        = "unexpected number of rows was deleted";
            log_error ("end: %s", ret.msg);
            return ret;
        }
    } 
    catch (const std::exception &e) {
        ret.status     = 0;
        ret.errtype    = DB_ERR;
        ret.errsubtype = DB_ERROR_INTERNAL;
        ret.msg        = e.what();
        LOG_END_ABNORMAL(e);
        return ret;
    }
}

//=============================================================================
db_reply_t
    delete_dc_room_row_rack
        (tntdb::Connection &conn,
        a_elmnt_id_t element_id)
{
    LOG_START;
    tntdb::Transaction trans(conn);

    auto reply_delete1 = delete_asset_ext_attributes (conn, element_id);
    if ( reply_delete1.affected_rows == 0 )
    {
        trans.rollback();
        log_info ("end: error occured during deleting ext attributes");
        return reply_delete1;
    }
    
    auto reply_delete2 = delete_asset_element_from_asset_groups
                                                        (conn, element_id);
    if ( reply_delete2.status == 0 )
    {
        trans.rollback();
        log_info ("end: error occured during removing from groups");
        return reply_delete2;
    }

    auto reply_delete3 = delete_asset_element (conn, element_id);
    if ( reply_delete3.status == 0 )
    {
        trans.rollback();
        log_info ("end: error occured during removing element");
        return reply_delete3;
    }
 
    trans.commit();
    LOG_END;
    return reply_delete3;
}

//=============================================================================
db_reply_t
    delete_group
        (tntdb::Connection &conn,
         a_elmnt_id_t element_id)
{
    LOG_START;
    tntdb::Transaction trans(conn);

    auto reply_delete1 = delete_asset_ext_attributes (conn, element_id);
    if ( reply_delete1.status == 0 )
    {
        trans.rollback();
        log_info ("end: error occured during deleting ext attributes");
        return reply_delete1;
    }
 
    auto reply_delete2 = delete_asset_group_links (conn, element_id);
    if ( reply_delete2.status == 0 )
    {
        trans.rollback();
        log_info ("end: error occured during removing from groups");
        return reply_delete2;
    }

    auto reply_delete3 = delete_asset_element (conn, element_id);
    if ( reply_delete3.status == 0 )
    {
        trans.rollback();
        log_info ("end: error occured during removing element");
        return reply_delete3;
    }
 
    trans.commit();
    LOG_END;
    return reply_delete3;
}

//=============================================================================
db_reply_t
    delete_device
        (tntdb::Connection &conn,
         a_elmnt_id_t element_id)
{
    LOG_START;
    tntdb::Transaction trans(conn);

    // delete m_a_relation ????

    auto reply_delete1 = delete_asset_ext_attributes (conn, element_id);
    if ( reply_delete1.status == 0 )
    {
        trans.rollback();
        log_info ("end: error occured during deleting ext attributes");
        return reply_delete1;
    }
 
    auto reply_delete2 = delete_asset_group_links (conn, element_id);
    if ( reply_delete2.status == 0 )
    {
        trans.rollback();
        log_info ("end: error occured during removing from groups");
        return reply_delete2;
    }

    // links ????
    auto reply_delete3 = delete_asset_links_all (conn, element_id);
    if ( reply_delete3.status == 0 )
    {
        trans.rollback();
        log_info ("end: error occured during removing links");
        return reply_delete3;
    }

    auto reply_delete4 = delete_asset_device (conn, element_id);
    if ( reply_delete4.status == 0 )
    {
        trans.rollback();
        log_info ("end: error occured during removing element");
        return reply_delete4;
    }

    auto reply_delete5 = delete_asset_element (conn, element_id);
    if ( reply_delete5.status == 0 )
    {
        trans.rollback();
        log_info ("end: error occured during removing element");
        return reply_delete5;
    }
 
    trans.commit();
    LOG_END;
    return reply_delete5;
}
