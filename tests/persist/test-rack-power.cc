#include <catch.hpp>

#include <iostream>
#include <czmq.h>

#include "dbpath.h"
#include "log.h"

#include "assettopology.h"
#include "assetmsg.h"
#include "monitor.h"
#include "calc_power.h"


TEST_CASE("Rack power #1","[db][power][rack][calc][rack_power.sql]")
{
    log_open();
//    log_set_level(LOG_DEBUG);

    log_info ("=============== RACK POWER #1 ==================");
    
    m_clnt_id_t         client_id  = 2; // mymodule
    m_msrmnt_tp_id_t    type_id    = 3; // realpower
    m_msrmnt_sbtp_id_t  subtype_id = 1; // default

    // fill DB with measurements
    std::set < m_dvc_id_t > ids;
    REQUIRE_NOTHROW( ids.insert(convert_asset_to_monitor(url.c_str(), 8003)) );
    REQUIRE_NOTHROW( ids.insert(convert_asset_to_monitor(url.c_str(), 8004)) );
    REQUIRE_NOTHROW( ids.insert(convert_asset_to_monitor(url.c_str(), 8005)) );
    REQUIRE_NOTHROW( ids.insert(convert_asset_to_monitor(url.c_str(), 8002)) );
    REQUIRE_NOTHROW( ids.insert(convert_asset_to_monitor(url.c_str(), 8001)) );

    for ( auto &device_id: ids )
       REQUIRE_NOTHROW( generate_measurements (url.c_str(), client_id, device_id, type_id, subtype_id, 300) );
    
    // calculate total rack power
    zmsg_t* res = REQUIRE_NOTHROW( calc_total_rack_power (url.c_str(), 8000) );
    
    REQUIRE ( is_compute_msg (res) );

    compute_msg_t* res_compute = compute_msg_decode (&res);
    zhash_t* results = compute_msg_get_results (res_compute);

    m_msrmnt_value_t value;
    m_msrmnt_scale_t scale;
    a_elmnt_id_t num_missed;
    int rv  = REQUIRE_NOTHROW( compute_result_value_get(results, &value) );
    int rv1 = REQUIRE_NOTHROW( compute_result_scale_get(results, &scale) );
    int rv2 = REQUIRE_NOTHROW( compute_result_num_missed_get(results, &num_missed) );

    REQUIRE ( !rv );
    REQUIRE ( !rv1 );
    REQUIRE ( !rv2 );

    REQUIRE ( value == 8001+2000+GEN_MEASUREMENTS_MAX );
    REQUIRE ( scale == -1 );
    REQUIRE ( num_missed == 0 );

    zmsg_destroy (&res);
    log_close();
}

TEST_CASE("Rack power #2","[db][power][rack][calc][rack_power.sql]")
{
    log_open();
 //   log_set_level(LOG_DEBUG);

    log_info ("=============== RACK POWER #2 ==================");
    
    m_clnt_id_t         client_id  = 2; // mymodule
    m_msrmnt_tp_id_t    type_id    = 3; // realpower
    m_msrmnt_sbtp_id_t  subtype_id = 1; // default

    // expected devices to summ up
    std::set <m_dvc_id_t > expected_ids;
    REQUIRE_NOTHROW( expected_ids.insert(convert_asset_to_monitor(url.c_str(), 8007)) );
    REQUIRE_NOTHROW( expected_ids.insert(convert_asset_to_monitor(url.c_str(), 8008)) );
    
    m_msrmnt_value_t expected_value = 0;
    for ( auto &device_id: expected_ids)
        expected_value += device_id + GEN_MEASUREMENTS_MAX;

    // set of all devices in rack
    std::set < m_dvc_id_t > ids;
    REQUIRE_NOTHROW( ids.insert(convert_asset_to_monitor(url.c_str(), 8009)) );
    REQUIRE_NOTHROW( ids.insert(convert_asset_to_monitor(url.c_str(), 8010)) );
    REQUIRE_NOTHROW( ids.insert(convert_asset_to_monitor(url.c_str(), 8011)) );
    REQUIRE_NOTHROW( ids.insert(convert_asset_to_monitor(url.c_str(), 8012)) );
    REQUIRE_NOTHROW( ids.insert(expected_ids.begin(), expected_ids.end()) );

    // fill DB with measurements
    for ( auto &device_id: ids )
       REQUIRE_NOTHROW( generate_measurements (url.c_str(), client_id, device_id, type_id, subtype_id, 300) );
    
    // calculate total rack power
    zmsg_t* res = REQUIRE_NOTHROW( calc_total_rack_power (url.c_str(), 8006) );
    
    REQUIRE ( is_compute_msg (res) );

    compute_msg_t* res_compute = compute_msg_decode (&res);
    zhash_t* results = compute_msg_get_results (res_compute);

    m_msrmnt_value_t value;
    m_msrmnt_scale_t scale;
    a_elmnt_id_t num_missed;
    int rv  = REQUIRE_NOTHROW( compute_result_value_get(results, &value) );
    int rv1 = REQUIRE_NOTHROW( compute_result_scale_get(results, &scale) );
    int rv2 = REQUIRE_NOTHROW( compute_result_num_missed_get(results, &num_missed) );

    REQUIRE ( !rv );
    REQUIRE ( !rv1 );
    REQUIRE ( !rv2 );
    
    REQUIRE ( value == expected_value );
    REQUIRE ( scale == -1 );
    REQUIRE ( num_missed == 0 );

    zmsg_destroy (&res);
    log_close();
}

TEST_CASE("Rack power #3","[db][power][rack][calc][rack_power.sql]")
{
    log_open();
  //  log_set_level(LOG_DEBUG);

    log_info ("=============== RACK POWER #3 ==================");
    
    m_clnt_id_t         client_id  = 2; // mymodule
    m_msrmnt_tp_id_t    type_id    = 3; // realpower
    m_msrmnt_sbtp_id_t  subtype_id = 1; // default

    // expected devices to summ up
    std::set <m_dvc_id_t > expected_ids;
    REQUIRE_NOTHROW( expected_ids.insert(convert_asset_to_monitor(url.c_str(), 8015)) );
    REQUIRE_NOTHROW( expected_ids.insert(convert_asset_to_monitor(url.c_str(), 8020)) );
    REQUIRE_NOTHROW( expected_ids.insert(convert_asset_to_monitor(url.c_str(), 8022)) );
    
    m_msrmnt_value_t expected_value = 0;
    for ( auto &device_id: expected_ids)
        expected_value += device_id + GEN_MEASUREMENTS_MAX;

    // set of all devices in rack
    std::set < m_dvc_id_t > ids;
    REQUIRE_NOTHROW( ids.insert(convert_asset_to_monitor(url.c_str(), 8016)) );
    REQUIRE_NOTHROW( ids.insert(convert_asset_to_monitor(url.c_str(), 8014)) );
    REQUIRE_NOTHROW( ids.insert(convert_asset_to_monitor(url.c_str(), 8017)) );
    REQUIRE_NOTHROW( ids.insert(convert_asset_to_monitor(url.c_str(), 8021)) );
    REQUIRE_NOTHROW( ids.insert(convert_asset_to_monitor(url.c_str(), 8019)) );
    REQUIRE_NOTHROW( ids.insert(convert_asset_to_monitor(url.c_str(), 8018)) );
    REQUIRE_NOTHROW( ids.insert(expected_ids.begin(), expected_ids.end()) );

    // fill DB with measurements
    for ( auto &device_id: ids )
       REQUIRE_NOTHROW( generate_measurements (url.c_str(), client_id, device_id, type_id, subtype_id, 300) );
    
    // calculate total rack power
    zmsg_t* res = REQUIRE_NOTHROW( calc_total_rack_power (url.c_str(), 8013) );
    
    REQUIRE ( is_compute_msg (res) );

    compute_msg_t* res_compute = compute_msg_decode (&res);
    zhash_t* results = compute_msg_get_results (res_compute);

    m_msrmnt_value_t value;
    m_msrmnt_scale_t scale;
    a_elmnt_id_t num_missed;
    int rv  = REQUIRE_NOTHROW( compute_result_value_get(results, &value) );
    int rv1 = REQUIRE_NOTHROW( compute_result_scale_get(results, &scale) );
    int rv2 = REQUIRE_NOTHROW( compute_result_num_missed_get(results, &num_missed) );

    REQUIRE ( !rv );
    REQUIRE ( !rv1 );
    REQUIRE ( !rv2 );
    
    REQUIRE ( value == expected_value );
    REQUIRE ( scale == -1 );
    REQUIRE ( num_missed == 0 );

    zmsg_destroy (&res);
    log_close();
}
    
TEST_CASE("Rack power #4","[db][power][rack][calc][rack_power.sql]")
{
    log_open();
  //  log_set_level(LOG_DEBUG);

    log_info ("=============== RACK POWER #4 ==================");
    
    m_clnt_id_t         client_id  = 2; // mymodule
    m_msrmnt_tp_id_t    type_id    = 3; // realpower
    m_msrmnt_sbtp_id_t  subtype_id = 1; // default

    // expected devices to summ up
    std::set <m_dvc_id_t > expected_ids;
    REQUIRE_NOTHROW( expected_ids.insert(convert_asset_to_monitor(url.c_str(), 8025)) );
    REQUIRE_NOTHROW( expected_ids.insert(convert_asset_to_monitor(url.c_str(), 8026)) );
    REQUIRE_NOTHROW( expected_ids.insert(convert_asset_to_monitor(url.c_str(), 8029)) );
    
    m_msrmnt_value_t expected_value = 0;
    for ( auto &device_id: expected_ids)
        expected_value += device_id + GEN_MEASUREMENTS_MAX;

    // set of all devices in rack
    std::set < m_dvc_id_t > ids;
    REQUIRE_NOTHROW( ids.insert(convert_asset_to_monitor(url.c_str(), 8027)) );
    REQUIRE_NOTHROW( ids.insert(convert_asset_to_monitor(url.c_str(), 8024)) );
    REQUIRE_NOTHROW( ids.insert(convert_asset_to_monitor(url.c_str(), 8028)) );
    REQUIRE_NOTHROW( ids.insert(convert_asset_to_monitor(url.c_str(), 8030)) );
    // ids.insert(convert_asset_to_monitor(url.c_str(), 8031)); this element has no measurement
    REQUIRE_NOTHROW( ids.insert(expected_ids.begin(), expected_ids.end()) );

    // fill DB with measurements
    for ( auto &device_id: ids )
       REQUIRE_NOTHROW( generate_measurements (url.c_str(), client_id, device_id, type_id, subtype_id, 300) );
    
    // calculate total rack power
    zmsg_t* res = REQUIRE_NOTHROW( calc_total_rack_power (url.c_str(), 8023) );
    
    REQUIRE ( is_compute_msg (res) );

    compute_msg_t* res_compute = compute_msg_decode (&res);
    zhash_t* results = compute_msg_get_results (res_compute);

    m_msrmnt_value_t value;
    m_msrmnt_scale_t scale;
    a_elmnt_id_t num_missed;
    int rv  = REQUIRE_NOTHROW( compute_result_value_get(results, &value) );
    int rv1 = REQUIRE_NOTHROW( compute_result_scale_get(results, &scale) );
    int rv2 = REQUIRE_NOTHROW( compute_result_num_missed_get(results, &num_missed) );

    REQUIRE ( !rv );
    REQUIRE ( !rv1 );
    REQUIRE ( !rv2 );
    
    REQUIRE ( value == expected_value );
    REQUIRE ( scale == -1 );
    REQUIRE ( num_missed == 0 );

    zmsg_destroy (&res);
    log_close();
}

