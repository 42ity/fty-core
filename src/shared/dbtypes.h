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

/*! \file   dbtypes.h
    \brief  Contains type definitions for database entities and some constants.
            Should be used for all manipulations with database.
    \author Alena Chernikava <alenachernikava@eaton.com>
*/

#ifndef SRC_PERSIST_DBTYPES_H_
#define SRC_PERSIST_DBTYPES_H_

#define SRCOUT_DESTIN_IS_NULL "999"
#define INPUT_POWER_CHAIN     1


// ----- table:  t_bios_asset_element ------------
// ----- column: id_asset_element ----------------
typedef uint32_t a_elmnt_id_t;

// ----- table:  t_bios_asset_ext_attributes -----
// ----- column: id_asset_ext_attribute ----------
typedef uint32_t a_ext_attr_id_t;

// ----- table:  t_bios_asset_group_relation -----
// ----- column: id_asset_group_relation ---------
typedef uint32_t a_grp_rltn_id_t;

// ----- table:  t_bios_asset_element_type -------
// ----- column: id_asset_element_type  ----------
// TODO tntdb can't manage uint8_t, so for now there is
// uint16_t
typedef uint16_t  a_elmnt_tp_id_t;

// ----- table:  t_bios_asset_device -------------
// ----- column: id_asset_device -----------------
typedef uint32_t a_dvc_id_t;

// ----- table:  t_bios_asset_device_type --------
// ----- column: id_asset_device_type ------------
// TODO tntdb can't manage uint8_t, so for now there is
// uint16_t
typedef uint16_t a_dvc_tp_id_t;

// ----- table:  t_bios_asset_link ---------------
// ----- column: id_link--------------------------
typedef uint32_t a_lnk_id_t;

// ----- table:  t_bios_asset_link_type ----------
// ----- column: id_asset_link_type --------------
typedef uint8_t  a_lnk_tp_id_t;

// ----- table:  t_bios_asset_link ---------------
// ----- column: src_out -------------------------
// TODO tntdb can't manage uint8_t, so for now there is
// uint16_t
typedef char  a_lnk_src_out_t[4];

// ----- table:  t_bios_asset_link ---------------
// ----- column: dest_in -------------------------
// TODO tntdb can't manage uint8_t, so for now there is
// uint16_t
typedef char  a_lnk_dest_in_t[4];

// ----- table:  t_bios_monitor_asset_relation ---
// ----- column: id_ma_relation ------------------
typedef uint32_t ma_rltn_id_t;

// ----- table:  t_bios_discovered_device --------
// ----- column: id_discovered_device ------------
typedef uint16_t m_dvc_id_t;

// ----- table:  t_bios_device_type --------------
// ----- column: id_device_type ------------------
// TODO tntdb can't manage uint8_t, so for now there is
// uint16_t
typedef uint16_t  m_dvc_tp_id_t;

// ----- table:  t_bios_client -------------------
// ----- column: id_client -----------------------
// TODO tntdb can't manage uint8_t, so for now there is
// uint16_t
typedef uint16_t  m_clnt_id_t;

// ----- table:  t_bios_client_info --------------
// ----- column: id_client_info ------------------
typedef uint64_t m_clnt_info_id_t;

// ----- table:  t_bios_measurements -------------
// ----- column: id_measurements -----------------
typedef uint64_t m_msrmnt_id_t;

// ----- table:  t_bios_measurements -------------
// ----- column: value ---------------------------
typedef  int64_t m_msrmnt_value_t;

// ----- table:  t_bios_measurement_type ---------
// ----- column: id ------------------------------
typedef uint16_t m_msrmnt_tp_id_t;

// ----- table:  t_bios_measurement_subtype ------
// ----- column: id ------------------------------
typedef uint16_t m_msrmnt_sbtp_id_t;

// ----- table:  t_bios_measurement_subtype ------
// ----- column: scale ---------------------------
// TODO tntdb can't manage uint8_t, so for now there is
// uint16_t
typedef int16_t   m_msrmnt_scale_t;

// ----- table:  t_bios_discovered_ip ------------
// ----- column: id_ip ---------------------------
typedef uint32_t m_ip_id_t;

// ----- table:  t_bios_net_history --------------
// ----- column: id_net_history ------------------
typedef uint32_t m_nt_hstr_id_t;

// ----- table:  t_bios_nethistory ---------------
// ----- column: mask ----------------------------
typedef uint8_t  m_nt_hstr_mask_t;

#endif // SRC_PERSIST_DVTYPES_H_
