#include <catch.hpp>

#include <iostream>
#include <czmq.h>

#include "dbpath.h"
#include <map>
#include "log.h"

#include "assettopology.h"
#include "assetmsg.h"
#include "monitor.h"
#include "dbtypes.h"
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

    m_dvc_id_t yy = 0;
    /*REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8003));
    ids.insert (yy);
    REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8004));
    ids.insert (yy);
    REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8005));
    ids.insert (yy);*/
    REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8002));
    ids.insert (yy);
    REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8001));
    ids.insert (yy);

    for ( auto &device_id: ids )
       generate_measurements (url.c_str(), client_id, device_id, type_id, subtype_id, 300, device_id + GEN_MEASUREMENTS_MAX);
    
    // calculate total rack power
    zmsg_t* res = calc_total_rack_power (url.c_str(), 8000);
    
    REQUIRE ( is_compute_msg (res) );

    compute_msg_t* res_compute = compute_msg_decode (&res);
    zhash_t* results = compute_msg_get_results (res_compute);

    m_msrmnt_value_t value;
    m_msrmnt_scale_t scale;
    a_elmnt_id_t num_missed;
    int rv  = compute_result_value_get(results, &value);
    int rv1 = compute_result_scale_get(results, &scale);
    int rv2 = compute_result_num_missed_get(results, &num_missed);

    REQUIRE ( !rv );
    REQUIRE ( !rv1 );
    REQUIRE ( !rv2 );

    REQUIRE ( value == 8001+2000+GEN_MEASUREMENTS_MAX );
    REQUIRE ( scale == -1 );
    REQUIRE ( num_missed == 3 );

    zmsg_destroy (&res);
    log_close();
}

TEST_CASE("Rack power #2","[db][power][rack][calc][rack_power.sql]")
{
    log_open();
   log_set_level(LOG_DEBUG);

    log_info ("=============== RACK POWER #2 ==================");
    
    m_clnt_id_t         client_id  = 2; // mymodule
    m_msrmnt_tp_id_t    type_id    = 3; // realpower
    m_msrmnt_sbtp_id_t  subtype_id = 1; // default

    // expected devices to summ up
    std::set <m_dvc_id_t > expected_ids;
    expected_ids.insert(convert_asset_to_monitor(url.c_str(), 8007));
    expected_ids.insert(convert_asset_to_monitor(url.c_str(), 8008));
    
    m_msrmnt_value_t expected_value = 0;
    for ( auto &device_id: expected_ids)
        expected_value += device_id + GEN_MEASUREMENTS_MAX;

    // set of all devices in rack
    std::set < m_dvc_id_t > ids;
//    ids.insert(convert_asset_to_monitor(url.c_str(), 8009));
//    ids.insert(convert_asset_to_monitor(url.c_str(), 8010));
//    ids.insert(convert_asset_to_monitor(url.c_str(), 8011));
//    ids.insert(convert_asset_to_monitor(url.c_str(), 8012));
    ids.insert(expected_ids.begin(), expected_ids.end());

    // fill DB with measurements
    for ( auto &device_id: ids )
       generate_measurements (url.c_str(), client_id, device_id, type_id, subtype_id, 300, device_id + GEN_MEASUREMENTS_MAX);
    
    // calculate total rack power
    zmsg_t* res = calc_total_rack_power (url.c_str(), 8006);
    
    REQUIRE ( is_compute_msg (res) );

    compute_msg_t* res_compute = compute_msg_decode (&res);
    zhash_t* results = compute_msg_get_results (res_compute);

    m_msrmnt_value_t value;
    m_msrmnt_scale_t scale;
    a_elmnt_id_t num_missed;
    int rv  = compute_result_value_get(results, &value);
    int rv1 = compute_result_scale_get(results, &scale);
    int rv2 = compute_result_num_missed_get(results, &num_missed);

    REQUIRE ( !rv );
    REQUIRE ( !rv1 );
    REQUIRE ( !rv2 );
    
    REQUIRE ( value == expected_value );
    REQUIRE ( scale == -1 );
    REQUIRE ( num_missed == 4 );

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
    expected_ids.insert(convert_asset_to_monitor(url.c_str(), 8015));
    expected_ids.insert(convert_asset_to_monitor(url.c_str(), 8020));
    expected_ids.insert(convert_asset_to_monitor(url.c_str(), 8022));
    
    m_msrmnt_value_t expected_value = 0;
    for ( auto &device_id: expected_ids)
        expected_value += device_id + GEN_MEASUREMENTS_MAX;

    // set of all devices in rack
    std::set < m_dvc_id_t > ids;
//    ids.insert(convert_asset_to_monitor(url.c_str(), 8016));
//    ids.insert(convert_asset_to_monitor(url.c_str(), 8014));
//    ids.insert(convert_asset_to_monitor(url.c_str(), 8017));
    ids.insert(convert_asset_to_monitor(url.c_str(), 8021));
//    ids.insert(convert_asset_to_monitor(url.c_str(), 8019));
//    ids.insert(convert_asset_to_monitor(url.c_str(), 8018));
    ids.insert(expected_ids.begin(), expected_ids.end());

    // fill DB with measurements
    for ( auto &device_id: ids )
       generate_measurements (url.c_str(), client_id, device_id, type_id, subtype_id, 300, device_id + GEN_MEASUREMENTS_MAX);
    
    // calculate total rack power
    zmsg_t* res = calc_total_rack_power (url.c_str(), 8013);
    
    REQUIRE ( is_compute_msg (res) );

    compute_msg_t* res_compute = compute_msg_decode (&res);
    zhash_t* results = compute_msg_get_results (res_compute);

    m_msrmnt_value_t value;
    m_msrmnt_scale_t scale;
    a_elmnt_id_t num_missed;
    int rv  = compute_result_value_get(results, &value);
    int rv1 = compute_result_scale_get(results, &scale);
    int rv2 = compute_result_num_missed_get(results, &num_missed);

    REQUIRE ( !rv );
    REQUIRE ( !rv1 );
    REQUIRE ( !rv2 );
    
    REQUIRE ( value == expected_value );
    REQUIRE ( scale == -1 );
    REQUIRE ( num_missed == 4 );

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
    expected_ids.insert(convert_asset_to_monitor(url.c_str(), 8025));
    expected_ids.insert(convert_asset_to_monitor(url.c_str(), 8026));
    expected_ids.insert(convert_asset_to_monitor(url.c_str(), 8029));
    
    m_msrmnt_value_t expected_value = 0;
    for ( auto &device_id: expected_ids)
        expected_value += device_id + GEN_MEASUREMENTS_MAX;

    // set of all devices in rack
    std::set < m_dvc_id_t > ids;
//    ids.insert(convert_asset_to_monitor(url.c_str(), 8027));
//    ids.insert(convert_asset_to_monitor(url.c_str(), 8024));
//    ids.insert(convert_asset_to_monitor(url.c_str(), 8028));
//    ids.insert(convert_asset_to_monitor(url.c_str(), 8030));
    // ids.insert(convert_asset_to_monitor(url.c_str(), 8031)); this element has no measurement
    ids.insert(expected_ids.begin(), expected_ids.end());

    // fill DB with measurements
    for ( auto &device_id: ids )
       generate_measurements (url.c_str(), client_id, device_id, type_id, subtype_id, 300, device_id + GEN_MEASUREMENTS_MAX);
    
    // calculate total rack power
    zmsg_t* res = calc_total_rack_power (url.c_str(), 8023);
    
    REQUIRE ( is_compute_msg (res) );

    compute_msg_t* res_compute = compute_msg_decode (&res);
    zhash_t* results = compute_msg_get_results (res_compute);

    m_msrmnt_value_t value;
    m_msrmnt_scale_t scale;
    a_elmnt_id_t num_missed;
    int rv  = compute_result_value_get(results, &value);
    int rv1 = compute_result_scale_get(results, &scale);
    int rv2 = compute_result_num_missed_get(results, &num_missed);

    REQUIRE ( !rv );
    REQUIRE ( !rv1 );
    REQUIRE ( !rv2 );
    
    REQUIRE ( value == expected_value );
    REQUIRE ( scale == -1 );
    REQUIRE ( num_missed == 3 );

    zmsg_destroy (&res);
    log_close();
}
    
TEST_CASE("Rack power #5","[db][power][rack][calc][rack_power.sql]")
{
    log_open();
  //  log_set_level(LOG_DEBUG);

    log_info ("=============== RACK POWER #5 ==================");
    
    m_clnt_id_t         client_id  = 2; // mymodule
    m_msrmnt_tp_id_t    type_id    = 3; // realpower
    m_msrmnt_sbtp_id_t  subtype_id = 1; // default

    // expected devices to summ up
    std::set <m_dvc_id_t > expected_ids;
    m_dvc_id_t yy = 0;
    REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8102) );
    expected_ids.insert(yy);
    REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8103) );
    expected_ids.insert(yy);
    
    m_msrmnt_value_t expected_value = 0;
    for ( auto &device_id: expected_ids)
        expected_value += device_id + GEN_MEASUREMENTS_MAX;

    // set of devices we want to generate measurements for
    std::set < m_dvc_id_t > ids;

    REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8104) );
    ids.insert(yy);
    REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8105) );
    ids.insert(yy);
    REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8106) );
    ids.insert(yy);
    REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8107) );
    ids.insert(yy);
    ids.insert(expected_ids.begin(), expected_ids.end());

    // fill DB with measurements
    for ( auto &device_id: ids )
       generate_measurements (url.c_str(), client_id, device_id, type_id, subtype_id, 300, device_id + GEN_MEASUREMENTS_MAX);
    
    // calculate total rack power
    zmsg_t* res = calc_total_rack_power (url.c_str(), 8101);
    
    REQUIRE ( is_compute_msg (res) );

    compute_msg_t* res_compute = compute_msg_decode (&res);
    zhash_t* results = compute_msg_get_results (res_compute);

    m_msrmnt_value_t value;
    m_msrmnt_scale_t scale;
    a_elmnt_id_t num_missed;
    int rv  = compute_result_value_get(results, &value);
    int rv1 = compute_result_scale_get(results, &scale);
    int rv2 = compute_result_num_missed_get(results, &num_missed);

    REQUIRE ( !rv );
    REQUIRE ( !rv1 );
    REQUIRE ( !rv2 );
    
    REQUIRE ( value == expected_value );
    REQUIRE ( scale == -1 );
    REQUIRE ( num_missed == 0 );

    zmsg_destroy (&res);
    log_close();
}

TEST_CASE("Rack power #6","[db][power][rack][calc][rack_power.sql]")
{
    log_open();
  //  log_set_level(LOG_DEBUG);

    log_info ("=============== RACK POWER #6 ==================");
    
    m_clnt_id_t         client_id  = 2; // mymodule
    m_msrmnt_tp_id_t    type_id    = 3; // realpower
    m_msrmnt_sbtp_id_t  subtype_id = 1; // default

    // expected devices to summ up
    std::set <m_dvc_id_t > expected_ids;
    m_dvc_id_t yy = 0;
    REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8110) );
    expected_ids.insert(yy);
    REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8111) );
    expected_ids.insert(yy);
    
    m_msrmnt_value_t expected_value = 0;
    for ( auto &device_id: expected_ids)
        expected_value += device_id + GEN_MEASUREMENTS_MAX;

    // set of devices we want to generate measurements for
    std::set < m_dvc_id_t > ids;
    REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8109));
    ids.insert(yy);
    REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8112));
    ids.insert(yy);
    REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8112));
    ids.insert(yy);
    REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8114));
    ids.insert(yy);
    REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8115));
    ids.insert(yy);
    ids.insert(expected_ids.begin(), expected_ids.end());

    // fill DB with measurements
    for ( auto &device_id: ids )
       generate_measurements (url.c_str(), client_id, device_id, type_id, subtype_id, 300, device_id + GEN_MEASUREMENTS_MAX);
    
    // calculate total rack power
    zmsg_t* res = calc_total_rack_power (url.c_str(), 8108);
    
    REQUIRE ( is_compute_msg (res) );

    compute_msg_t* res_compute = compute_msg_decode (&res);
    zhash_t* results = compute_msg_get_results (res_compute);

    m_msrmnt_value_t value;
    m_msrmnt_scale_t scale;
    a_elmnt_id_t num_missed;
    int rv  = compute_result_value_get(results, &value);
    int rv1 = compute_result_scale_get(results, &scale);
    int rv2 = compute_result_num_missed_get(results, &num_missed);

    REQUIRE ( !rv );
    REQUIRE ( !rv1 );
    REQUIRE ( !rv2 );
    
    REQUIRE ( value == expected_value );
    REQUIRE ( scale == -1 );
    REQUIRE ( num_missed == 0 );

    zmsg_destroy (&res);
    log_close();
}

TEST_CASE("Rack power #7","[db][power][rack][calc][rack_power.sql]")
{
    log_open();
  //  log_set_level(LOG_DEBUG);

    log_info ("=============== RACK POWER #7 ==================");
    
    m_clnt_id_t         client_id  = 2; // mymodule
    m_msrmnt_tp_id_t    type_id    = 3; // realpower
    m_msrmnt_sbtp_id_t  subtype_id = 1; // default

    // expected devices to summ up
    std::set <m_dvc_id_t > expected_ids;
    m_dvc_id_t yy = 0;
    REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8118) );
    expected_ids.insert(yy);
    REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8119) );
    expected_ids.insert(yy);
    
    m_msrmnt_value_t expected_value = 0;
    for ( auto &device_id: expected_ids)
        expected_value += device_id + GEN_MEASUREMENTS_MAX;

    // set of devices we want to generate measurements for
    std::set < m_dvc_id_t > ids;
        REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8122) );
    ids.insert(yy);
    REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8123) );
    ids.insert(yy);
    REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8124) );
    ids.insert(yy);
    REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8125) );
    ids.insert(yy);
    // there is no measurements about PDU
    // REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8121) );
    // ids.insert(yy);
    // REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8120) );
    // ids.insert(yy);
    // there is no measurements about main
    // REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8117) );
    // ids.insert(yy);
    ids.insert(expected_ids.begin(), expected_ids.end());

    // fill DB with measurements
    for ( auto &device_id: ids )
       generate_measurements (url.c_str(), client_id, device_id, type_id, subtype_id, 300, device_id + GEN_MEASUREMENTS_MAX);
    
    // calculate total rack power
    zmsg_t* res = calc_total_rack_power (url.c_str(), 8116);
    
    REQUIRE ( is_compute_msg (res) );

    compute_msg_t* res_compute = compute_msg_decode (&res);
    zhash_t* results = compute_msg_get_results (res_compute);

    m_msrmnt_value_t value;
    m_msrmnt_scale_t scale;
    a_elmnt_id_t num_missed;
    int rv  = compute_result_value_get(results, &value);
    int rv1 = compute_result_scale_get(results, &scale);
    int rv2 = compute_result_num_missed_get(results, &num_missed);

    REQUIRE ( !rv );
    REQUIRE ( !rv1 );
    REQUIRE ( !rv2 );
    
    REQUIRE ( value == expected_value );
    REQUIRE ( scale == -1 );
    REQUIRE ( num_missed == 0 );

    zmsg_destroy (&res);
    log_close();
}
/*
TEST_CASE("Rack power #8","[db][power][rack][calc][rack_power.sql][iii][.]")
{
    log_open();
    log_set_level(LOG_DEBUG);

    log_info ("=============== RACK POWER #8 ==================");
    
    m_clnt_id_t         client_id  = 2; // mymodule
    m_msrmnt_tp_id_t    type_id    = 3; // realpower
    m_msrmnt_sbtp_id_t  subtype_id = 1; // default
    m_msrmnt_sbtp_id_t  subtype_id_PSU1 = 5; // PSU1
    m_msrmnt_sbtp_id_t  subtype_id_PSU2 = 9; // PSU2

    // sets of expected returned IT-device ids
    std::set < a_elmnt_id_t > expected_dvc_ids_1_PSU_IPMI = {8130, 8131};
    std::set < a_elmnt_id_t > expected_dvc_ids_2_PSU_IPMI = {8132, 8133};
    std::set < a_elmnt_id_t > expected_dvc_ids_NO_IPMI    = {};

    // sets of expected returned power device ids
    std::set < a_elmnt_id_t > expected_power_dvc = {};

    // ids of expected devices to summ up
    std::set < a_elmnt_id_t > expected_ids_;
    expected_ids_.insert(expected_power_dvc.begin(),expected_power_dvc.end());
    expected_ids_.insert(expected_dvc_ids_1_PSU_IPMI.begin(), expected_dvc_ids_1_PSU_IPMI.end());
    expected_ids_.insert(expected_dvc_ids_2_PSU_IPMI.begin(), expected_dvc_ids_2_PSU_IPMI.end());
    expected_ids_.insert(expected_dvc_ids_NO_IPMI.begin()   , expected_dvc_ids_NO_IPMI.end());

    // a counterpart for expected devices in monitor part
    std::map < a_elmnt_id_t, m_dvc_id_t > expected_ids;
    // find counterparts and generate measurements and calculate expected value
    m_msrmnt_value_t expected_value = 0;
    for ( auto &dvc_id: expected_ids_ )
    {
        // counterpart
        m_dvc_id_t yy = 0;
        INFO (dvc_id);
        REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), dvc_id) );
        expected_ids.insert( std::pair < a_elmnt_id_t, m_dvc_id_t >(dvc_id, yy));

        // simulate measurements //
        //    total
        m_msrmnt_value_t val_total = dvc_id + GEN_MEASUREMENTS_MAX;
        generate_measurements (url.c_str(), client_id, yy, type_id, subtype_id, 300, val_total);
        //    1PSU
        if ( expected_dvc_ids_1_PSU_IPMI.count(dvc_id) == 1 )
            generate_measurements (url.c_str(), client_id, yy, type_id, subtype_id_PSU1, 300, val_total);
        //    2PSU
        if ( expected_dvc_ids_2_PSU_IPMI.count(dvc_id) == 1 )
        {
            m_msrmnt_value_t val_psu1 = 10;
            m_msrmnt_value_t val_psu2 = val_total - val_psu1;
            generate_measurements (url.c_str(), client_id, yy, type_id, subtype_id_PSU2, 300, val_psu1);
            generate_measurements (url.c_str(), client_id, yy, type_id, subtype_id_PSU2, 300, val_psu2);
        }
        expected_value += val_total;
    }
   
    // set of all devices in rack, not expected to return
    std::set < m_dvc_id_t > ids;
    m_dvc_id_t yy = 0;
    REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8128) );
    ids.insert(yy);
    REQUIRE_NOTHROW ( yy = convert_asset_to_monitor(url.c_str(), 8129) );
    ids.insert(yy);
//    ids.insert(convert_asset_to_monitor(url.c_str(), 8127)); // there is no measurements about main
    
    // calculate total rack power
    zmsg_t* res = calc_total_rack_power (url.c_str(), 8126);
    
    REQUIRE ( is_compute_msg (res) );

    compute_msg_t* res_compute = compute_msg_decode (&res);
    zhash_t* results = compute_msg_get_results (res_compute);

    m_msrmnt_value_t value;
    m_msrmnt_scale_t scale;
    a_elmnt_id_t num_missed;
    int rv  = compute_result_value_get(results, &value);
    int rv1 = compute_result_scale_get(results, &scale);
    int rv2 = compute_result_num_missed_get(results, &num_missed);

    REQUIRE ( !rv );
    REQUIRE ( !rv1 );
    REQUIRE ( !rv2 );
    
    REQUIRE ( value == expected_value );
    REQUIRE ( scale == -1 );
    REQUIRE ( num_missed == 0 );

    zmsg_destroy (&res);
    log_close();
}
*/

TEST_CASE("Rack power #9","[db][power][rack][calc][rack_power.sql][.]")
{
    log_open();
    //log_set_level(LOG_DEBUG);

    log_info ("=============== RACK POWER #9 ==================");
    
    m_clnt_id_t         client_id  = 2; // mymodule
    m_msrmnt_tp_id_t    type_id    = 3; // realpower
    m_msrmnt_sbtp_id_t  subtype_id = 1; // default

    // expected devices to summ up
    std::set <m_dvc_id_t > expected_ids;
    expected_ids.insert(convert_asset_to_monitor(url.c_str(), 8137));
    
    m_msrmnt_value_t expected_value = 0;
    for ( auto &device_id: expected_ids)
        expected_value += device_id + GEN_MEASUREMENTS_MAX;

    // set of all devices in rack
    std::set < m_dvc_id_t > ids;
//    ids.insert(convert_asset_to_monitor(url.c_str(), 8136));
// no impi -> no measurements about devices
//    ids.insert(convert_asset_to_monitor(url.c_str(), 8138));
//    ids.insert(convert_asset_to_monitor(url.c_str(), 8139));
//    ids.insert(convert_asset_to_monitor(url.c_str(), 8140));
// there is no measurements about main
//    ids.insert(convert_asset_to_monitor(url.c_str(), 8134));    
    ids.insert(expected_ids.begin(), expected_ids.end());

    // fill DB with measurements
    for ( auto &device_id: ids )
       generate_measurements (url.c_str(), client_id, device_id, type_id, subtype_id, 300, device_id + GEN_MEASUREMENTS_MAX);
    
    // calculate total rack power
    zmsg_t* res = calc_total_rack_power (url.c_str(), 8134);
    
    REQUIRE ( is_compute_msg (res) );

    compute_msg_t* res_compute = compute_msg_decode (&res);
    zhash_t* results = compute_msg_get_results (res_compute);

    m_msrmnt_value_t value;
    m_msrmnt_scale_t scale;
    a_elmnt_id_t num_missed;
    int rv  = compute_result_value_get(results, &value);
    int rv1 = compute_result_scale_get(results, &scale);
    int rv2 = compute_result_num_missed_get(results, &num_missed);

    REQUIRE ( !rv );
    REQUIRE ( !rv1 );
    REQUIRE ( !rv2 );
    
    REQUIRE ( value == expected_value );
    REQUIRE ( scale == -1 );
    REQUIRE ( num_missed == 0 );

    zmsg_destroy (&res);
    log_close();
}
