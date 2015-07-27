#include <catch.hpp>

#include "dbpath.h"
#include "log.h"

#include "db/alerts.h"
#include "assetcrud.h"
#include "monitor.h"

using namespace persist;


// TODO: add errtype checks
// use sections in tests
TEST_CASE("t_bios_alert INSERT/DELETE/SELECT #1","[db][CRUD][insert][delete][select][alert][crud_test.sql]")
{
    log_open ();

    log_info ("=============== ALERT: ALERT DELETE/INSERT #1 ==================");

    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );

    const char         *rule_name = "this is the GREAT rule name1";
    a_elmnt_pr_t        priority  = 1;
    m_alrt_state_t      alert_state = 1;
    const char         *description = "very small description";
    m_alrt_ntfctn_t     notification = 12;
    int64_t             date_from = 2049829;
    a_elmnt_id_t        dc_id = 1;

    // first insert
    auto reply_insert = insert_into_alert (conn, rule_name, priority, alert_state, description, notification, date_from, dc_id);
    REQUIRE ( reply_insert.status == 1 );
    uint64_t rowid = reply_insert.rowid;
    CAPTURE ( rowid );
    REQUIRE ( rowid >0 );
    REQUIRE ( reply_insert.affected_rows == 1 );

    // check select
    auto reply_select = select_alert_last_byRuleName (conn, rule_name);
    REQUIRE ( reply_select.status == 1 );
    REQUIRE ( reply_select.item.id == rowid );
    REQUIRE ( reply_select.item.rule_name == std::string(rule_name) );
    REQUIRE ( reply_select.item.alert_state == alert_state );
    REQUIRE ( reply_select.item.description == std::string(description) );
    REQUIRE ( reply_select.item.notification == notification );
    REQUIRE ( reply_select.item.date_from == date_from );
    REQUIRE ( reply_select.item.date_till == 0 );

    // check select, another function
    reply_select = select_alert_byRuleNameDateFrom (conn, rule_name, date_from);
    REQUIRE ( reply_select.status == 1 );
    REQUIRE ( reply_select.item.id == rowid );
    REQUIRE ( reply_select.item.rule_name == std::string(rule_name) );
    REQUIRE ( reply_select.item.alert_state == alert_state );
    REQUIRE ( reply_select.item.description == std::string(description) );
    REQUIRE ( reply_select.item.notification == notification );
    REQUIRE ( reply_select.item.date_from == date_from );
    REQUIRE ( reply_select.item.date_till == 0 );

    // check select, one more another function
    auto reply_select_all = select_alert_all_opened (conn);
    REQUIRE ( reply_select_all.status == 1 );
    REQUIRE ( reply_select_all.item.size() == 1 );
    REQUIRE ( reply_select_all.item.at(0).id == rowid );
    REQUIRE ( reply_select_all.item.at(0).rule_name == std::string(rule_name) );
    REQUIRE ( reply_select_all.item.at(0).alert_state == alert_state );
    REQUIRE ( reply_select_all.item.at(0).description == std::string(description) );
    REQUIRE ( reply_select_all.item.at(0).notification == notification );
    REQUIRE ( reply_select_all.item.at(0).date_from == date_from );
    REQUIRE ( reply_select_all.item.at(0).date_till == 0 );

    // must handle duplicate insert without insert
    reply_insert = insert_into_alert (conn, rule_name, priority, alert_state, description, notification, date_from, dc_id);
    REQUIRE ( reply_insert.status == 1 );
    REQUIRE ( reply_insert.affected_rows == 0 );

    // first delete
    auto reply_delete = delete_from_alert (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 1 );
    REQUIRE ( reply_delete.status == 1 );

    // check select
    reply_select_all = select_alert_all_opened (conn);
    REQUIRE ( reply_select_all.status == 1 );
    REQUIRE ( reply_select_all.item.size() == 0 );

    // must handle second delete without crash
    reply_delete = delete_from_alert (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 0 );
    REQUIRE ( reply_delete.status == 1 );

    log_close();
}

TEST_CASE("t_bios_alert UPDATE end date #2","[db][CRUD][update][alert][crud_test.sql]")
{
    log_open ();

    log_info ("=============== ALERT: ALERT UPDATE end date #2 ==================");

    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );

    const char         *rule_name = "this is the GREAT rule name2";
    a_elmnt_pr_t        priority  = 1;
    m_alrt_state_t      alert_state = 1;
    const char         *description = "very small description";
    m_alrt_ntfctn_t     notification = 12;
    int64_t             date_from = 2049829;
    a_elmnt_id_t        dc_id = 1;

    // insert
    auto reply_insert = insert_into_alert (conn, rule_name, priority, alert_state, description, notification, date_from, dc_id);
    uint64_t rowid = reply_insert.rowid;
    CAPTURE( rowid );

    // update
    int64_t  date_till = 2059829;
    auto reply_update = update_alert_tilldate(conn, date_till, rowid);
    REQUIRE ( reply_update.status == 1 );
    REQUIRE ( reply_update.affected_rows == 1 );
    REQUIRE ( reply_update.rowid == rowid );

    // check select
    auto reply_select = select_alert_last_byRuleName (conn, rule_name);
    REQUIRE ( reply_select.status == 1 );
    REQUIRE ( reply_select.item.id == rowid );
    REQUIRE ( reply_select.item.rule_name == std::string(rule_name) );
    REQUIRE ( reply_select.item.alert_state == alert_state );
    REQUIRE ( reply_select.item.description == std::string(description) );
    REQUIRE ( reply_select.item.notification == notification );
    REQUIRE ( reply_select.item.date_from == date_from );
    REQUIRE ( reply_select.item.date_till == date_till );

    // check select, one more another function
    auto reply_select_all = select_alert_all_closed (conn);
    REQUIRE ( reply_select_all.status == 1 );
    REQUIRE ( reply_select_all.item.size() == 1 );
    REQUIRE ( reply_select_all.item.at(0).id == rowid );
    REQUIRE ( reply_select_all.item.at(0).rule_name == std::string(rule_name) );
    REQUIRE ( reply_select_all.item.at(0).alert_state == alert_state );
    REQUIRE ( reply_select_all.item.at(0).description == std::string(description) );
    REQUIRE ( reply_select_all.item.at(0).notification == notification );
    REQUIRE ( reply_select_all.item.at(0).date_from == date_from );
    REQUIRE ( reply_select_all.item.at(0).date_till == date_till );

    // delete
    delete_from_alert (conn, rowid);
    
    // insert
    reply_insert = insert_into_alert (conn, rule_name, priority, alert_state, description, notification, date_from, dc_id);
    rowid = reply_insert.rowid;

    // update
    reply_update = update_alert_tilldate_by_rulename(conn, date_till + 5, rule_name);
    REQUIRE ( reply_update.status == 1 );
    REQUIRE ( reply_update.affected_rows == 1 );
    REQUIRE ( reply_update.rowid == rowid );

    // check select
    reply_select = select_alert_last_byRuleName (conn, rule_name);
    REQUIRE ( reply_select.status == 1 );
    REQUIRE ( reply_select.item.id == rowid );
    REQUIRE ( reply_select.item.rule_name == std::string(rule_name) );
    REQUIRE ( reply_select.item.alert_state == alert_state );
    REQUIRE ( reply_select.item.description == std::string(description) );
    REQUIRE ( reply_select.item.notification == notification );
    REQUIRE ( reply_select.item.date_from == date_from );
    REQUIRE ( reply_select.item.date_till == date_till + 5 );
    
    // delete
    delete_from_alert (conn, rowid);

    log_close();
}

TEST_CASE("t_bios_alert UPDATE notification #3","[db][CRUD][update][alert][crud_test.sql]")
{
    log_open ();

    log_info ("=============== ALERT: ALERT UPDATE notification #3 ==================");

    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );

    const char         *rule_name = "this is the GREAT rule name3";
    a_elmnt_pr_t        priority  = 1;
    m_alrt_state_t      alert_state = 1;
    const char         *description = "very small description";
    m_alrt_ntfctn_t     notification = 1;
    int64_t             date_from = 2049829;
    a_elmnt_id_t        dc_id = 1;

    // insert
    auto reply_insert = insert_into_alert (conn, rule_name, priority, alert_state, description, notification, date_from, dc_id);
    uint64_t rowid = reply_insert.rowid;

    // update
    int64_t  new_notification = 4;
    auto reply_update = update_alert_notification_byId(conn, new_notification, rowid);
    REQUIRE (reply_update.status == 1);
    REQUIRE (reply_update.affected_rows == 1);
    REQUIRE (reply_update.rowid == rowid);

    // check select
    auto reply_select = select_alert_last_byRuleName (conn, rule_name);
    REQUIRE ( reply_select.status == 1 );
    REQUIRE ( reply_select.item.id == rowid );
    REQUIRE ( reply_select.item.rule_name == std::string(rule_name) );
    REQUIRE ( reply_select.item.alert_state == alert_state );
    REQUIRE ( reply_select.item.description == std::string(description) );
    REQUIRE ( reply_select.item.notification == ( new_notification | notification ) );
    REQUIRE ( reply_select.item.date_from == date_from );
    REQUIRE ( reply_select.item.date_till == 0 );
    notification = new_notification | notification;
 
    // update
    new_notification = 8;
    reply_update = update_alert_notification_byRuleName(conn, new_notification, rule_name);
    REQUIRE (reply_update.status == 1);
    REQUIRE (reply_update.affected_rows == 1);
    REQUIRE (reply_update.rowid == rowid);

    // check select
    reply_select = select_alert_last_byRuleName (conn, rule_name);
    REQUIRE ( reply_select.status == 1 );
    REQUIRE ( reply_select.item.id == rowid );
    REQUIRE ( reply_select.item.rule_name == std::string(rule_name) );
    REQUIRE ( reply_select.item.alert_state == alert_state );
    REQUIRE ( reply_select.item.description == std::string(description) );
    REQUIRE ( reply_select.item.notification == ( new_notification | notification ) );
    REQUIRE ( reply_select.item.date_from == date_from );
    REQUIRE ( reply_select.item.date_till == 0 );

    // delete
    delete_from_alert (conn, rowid);

    log_close();
}


TEST_CASE("t_bios_alert_device INSERT/DELETE #4","[db][CRUD][insert][delete][alert][alert_device][crud_test.sql]")
{
    log_open ();

    log_info ("=============== ALERT DEVICE: ALERT DEVICE DELETE/INSERT #4 ==================");

    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );

    // insert alert
    const char         *rule_name = "this is the GREAT rule name4";
    a_elmnt_pr_t        priority  = 1;
    m_alrt_state_t      alert_state = 1;
    const char         *description = "very small description";
    m_alrt_ntfctn_t     notification = 12;
    int64_t             date_from = 2049829;
    a_elmnt_id_t        dc_id = 1;

    auto reply_insert_alert = insert_into_alert (conn, rule_name, priority, alert_state, description, notification, date_from, dc_id);
    uint64_t rowid_alert = reply_insert_alert.rowid;
    CAPTURE ( rowid_alert );

    //insert element
    const char      *element_name = "test_element_name4";
    a_elmnt_tp_id_t  element_type_id = 6;
    a_elmnt_id_t     parent_id = 0;
    const char      *status = "active";
    a_elmnt_pr_t     priority_el = 4;
    a_elmnt_bc_t     bc = 3;
    auto reply_insert_element = insert_into_asset_element (conn, element_name, element_type_id, parent_id, status, priority_el, bc);
    uint64_t rowid_element = reply_insert_element.rowid;
    CAPTURE ( rowid_element );

    //insert device discovered
    m_dvc_tp_id_t device_type_id = 1;
    //common_msg_t* 
    auto o_reply_insert_device = insert_disc_device(url.c_str(), device_type_id, element_name);
    uint64_t rowid_device = common_msg_rowid (o_reply_insert_device);

    //insert monitor_asset relation
    auto reply_insert_ma = insert_into_monitor_asset_relation (conn, rowid_device, rowid_element);
    uint64_t rowid_ma = reply_insert_ma.rowid;

    // first insert
    auto reply_insert1 = insert_into_alert_device (conn, rowid_alert, element_name);
    REQUIRE ( reply_insert1.status == 1 );
    uint64_t rowid = reply_insert1.rowid;
    CAPTURE ( rowid );
    REQUIRE ( reply_insert1.affected_rows == 1 );

    // check select
    auto reply_select = select_alert_devices (conn, rowid_alert);
    REQUIRE ( reply_select.status == 1 );
    REQUIRE ( reply_select.item.size() == 1 );
    REQUIRE ( reply_select.item.at(0) == rowid_device );

    // must handle duplicate insert without insert
    reply_insert1 = insert_into_alert_device (conn, rowid_alert, element_name);
    REQUIRE ( reply_insert1.status == 1 );
    REQUIRE ( reply_insert1.affected_rows == 0 );

    // first delete
    auto reply_delete = delete_from_alert_device (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 1 );
    REQUIRE ( reply_delete.status == 1 );

    // check select
    reply_select = select_alert_devices (conn, rowid_alert);
    REQUIRE ( reply_select.status == 1 );
    REQUIRE ( reply_select.item.size() == 0 );

    // must handle second delete without crash
    reply_delete = delete_from_alert_device (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 0 );
    REQUIRE ( reply_delete.status == 1 );

    //delete alert
    delete_from_alert (conn, rowid_alert);
    //delete ma
    delete_monitor_asset_relation (conn, rowid_ma);
    //delete device
    delete_disc_device (url.c_str(), rowid_device);
    //delete element
    delete_asset_element (conn, rowid_element);

    log_close();
}


TEST_CASE("t_bios_alert_device INSERT/DELETE #5","[db][CRUD][insert][alert][delete][alert_device][crud_test.sql]")
{
    log_open ();

    log_info ("=============== ALERT DEVICE: ALERT DEVICE INSERT_deviceS #5 ==================");

    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );

    // insert alert
    const char         *rule_name = "this is the GREAT rule name5";
    a_elmnt_pr_t        priority  = 1;
    m_alrt_state_t      alert_state = 1;
    const char         *description = "very small description";
    m_alrt_ntfctn_t     notification = 12;
    int64_t             date_from = 2049829;
    a_elmnt_id_t        dc_id = 1;

    auto reply_insert_alert = insert_into_alert (conn, rule_name, priority, alert_state, description, notification, date_from, dc_id);
    uint64_t rowid_alert = reply_insert_alert.rowid;
    CAPTURE ( rowid_alert );

    //insert element
    const char      *element_name1 = "test_element_name5.1";
    a_elmnt_tp_id_t  element_type_id = 6;
    a_elmnt_id_t     parent_id = 0;
    const char      *status = "active";
    a_elmnt_pr_t     priority_el = 4;
    a_elmnt_bc_t     bc = 3;
    auto reply_insert_element1 = insert_into_asset_element (conn, element_name1, element_type_id, parent_id, status, priority_el, bc);
    uint64_t rowid_element1 = reply_insert_element1.rowid;
    CAPTURE ( rowid_element1 );

    //insert device discovered
    m_dvc_tp_id_t device_type_id = 1;
    //common_msg_t* 
    auto o_reply_insert_device1 = insert_disc_device(url.c_str(), device_type_id, element_name1);
    uint64_t rowid_device1 = common_msg_rowid (o_reply_insert_device1);
    CAPTURE ( rowid_device1 );

    //insert monitor_asset relation
    auto reply_insert_ma1 = insert_into_monitor_asset_relation (conn, rowid_device1, rowid_element1);
    uint64_t rowid_ma1 = reply_insert_ma1.rowid;

    //insert element
    const char *element_name2 = "test_element_name5.2";
    auto reply_insert_element2 = insert_into_asset_element (conn, element_name2, element_type_id, parent_id, status, priority_el, bc);
    uint64_t rowid_element2 = reply_insert_element2.rowid;
    CAPTURE ( rowid_element2 );

    //insert device discovered
    auto o_reply_insert_device2 = insert_disc_device(url.c_str(), device_type_id, element_name2);
    uint64_t rowid_device2 = common_msg_rowid (o_reply_insert_device2);
    CAPTURE ( rowid_device2 );

    //insert monitor_asset relation
    auto reply_insert_ma2 = insert_into_monitor_asset_relation (conn, rowid_device2, rowid_element2);
    uint64_t rowid_ma2 = reply_insert_ma2.rowid;

    // first insert
    std::vector<std::string> names;
    names.push_back(std::string(element_name1));
    names.push_back(std::string(element_name2));

    auto reply_insert1 = insert_into_alert_devices (conn, rowid_alert, names);
    REQUIRE ( reply_insert1.status == 1 );
    REQUIRE ( reply_insert1.affected_rows == 2 );

    // check select
    auto reply_select = select_alert_devices (conn, rowid_alert);
    REQUIRE ( reply_select.status == 1 );
    REQUIRE ( reply_select.item.size() == 2 );
    bool first  = ( reply_select.item.at(0) == rowid_device1 ) && ( reply_select.item.at(1) == rowid_device2 );
    bool second = ( reply_select.item.at(0) == rowid_device2 ) && ( reply_select.item.at(1) == rowid_device1 );

    REQUIRE ( (first || second) );

    // must handle duplicate insert without insert
    reply_insert1 = insert_into_alert_devices (conn, rowid_alert, names);
    REQUIRE ( reply_insert1.status == 1 );
    REQUIRE ( reply_insert1.affected_rows == 0 );

    // delete
    auto reply_delete = delete_from_alert_device_byalert (conn, rowid_alert);
    REQUIRE ( reply_delete.affected_rows == 2 );
    REQUIRE ( reply_delete.status == 1 );
    
    //delete alert
    delete_from_alert (conn, rowid_alert);
    //delete ma
    delete_monitor_asset_relation (conn, rowid_ma1);
    delete_monitor_asset_relation (conn, rowid_ma2);
    //delete device
    delete_disc_device (url.c_str(), rowid_device1);
    delete_disc_device (url.c_str(), rowid_device2);
    //delete element
    delete_asset_element (conn, rowid_element1);
    delete_asset_element (conn, rowid_element2);

    log_close();
}


TEST_CASE("insert_alert_new #6","[db][CRUD][insert][delete][alert][crud_test.sql]")
{
    log_open ();

    log_info ("=============== insert ALERT NEW: #6 ==================");

    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );

    //insert element
    const char      *element_name1 = "test_element_name6.1";
    a_elmnt_tp_id_t  element_type_id = 6;
    a_elmnt_id_t     parent_id = 0;
    const char      *status = "active";
    a_elmnt_pr_t     priority_el = 4;
    a_elmnt_bc_t     bc = 3;
    a_dvc_tp_id_t    asset_device_type_id = 5;
    const char      *asset_device_type_name = "server";

    std::vector <link_t> links{};
    std::set <a_elmnt_id_t> groups{};
    
    auto reply_insert_element1 = insert_device
       (conn, links, groups, element_name1, parent_id,
        NULL, asset_device_type_id, asset_device_type_name,
        status, priority_el, bc);

    uint64_t rowid_element1 = reply_insert_element1.rowid;
    CAPTURE ( rowid_element1 );

    //insert element
    const char *element_name2 = "test_element_name6.2";
    auto reply_insert_element2 = insert_device
       (conn, links, groups, element_name2, parent_id,
        NULL, asset_device_type_id, asset_device_type_name,
        status, priority_el, bc);
    uint64_t rowid_element2 = reply_insert_element2.rowid;
    CAPTURE ( rowid_element2 );

    // first insert
    std::vector<std::string> names;
    names.push_back(std::string(element_name1));
    names.push_back(std::string(element_name2));

    // insert alert
    const char         *rule_name = "this is the GREAT rule name6";
    a_elmnt_pr_t        priority  = 1;
    m_alrt_state_t      alert_state = 1;
    const char         *description = "very small description";
    m_alrt_ntfctn_t     notification = 12;
    int64_t             date_from = 2049829;
    auto reply_insert_alert = insert_new_alert (conn, rule_name, priority, alert_state, description, notification, date_from, names);
    REQUIRE ( reply_insert_alert.status == 1 );
    REQUIRE ( reply_insert_alert.affected_rows == 1 );
    uint64_t rowid_alert = reply_insert_alert.rowid;
    CAPTURE ( rowid_alert );

    // check select
    auto reply_select = select_alert_devices (conn, rowid_alert);
    REQUIRE ( reply_select.status == 1 );
    REQUIRE ( reply_select.item.size() == 2 );
    auto rowid_device1 = reply_select.item.at(0);
    auto rowid_device2 = reply_select.item.at(1);

    auto reply_select1 = select_alert_last_byRuleName (conn, rule_name);
    REQUIRE ( reply_select1.status == 1 );
    REQUIRE ( reply_select1.item.id == rowid_alert );
    REQUIRE ( reply_select1.item.rule_name == std::string(rule_name) );
    REQUIRE ( reply_select1.item.alert_state == alert_state );
    REQUIRE ( reply_select1.item.description == std::string(description) );
    REQUIRE ( reply_select1.item.notification == notification );
    REQUIRE ( reply_select1.item.date_from == date_from );
    REQUIRE ( reply_select1.item.date_till == 0 );
    REQUIRE ( reply_select1.item.device_ids.size() == 2 );
    bool first  = ( reply_select1.item.device_ids.at(0) == rowid_device1 ) && ( reply_select1.item.device_ids.at(1) == rowid_device2 );
    bool second = ( reply_select1.item.device_ids.at(0) == rowid_device2 ) && ( reply_select1.item.device_ids.at(1) == rowid_device1 );
    REQUIRE ( ( first || second ) );

    // delete
    delete_from_alert_device_byalert (conn, rowid_alert);
    delete_from_alert (conn, rowid_alert);

    delete_device (conn, rowid_element1);
    delete_device (conn, rowid_element2);

    log_close();
}

TEST_CASE("t_bios_alert INSERT Fail #7","[db][CRUD][insert][alert][crud_test.sql][wrong_input]")
{
    log_open ();

    log_info ("=============== ALERT: ALERT INSERT #7 ==================");

    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );

    const char         *rule_name = NULL;
    a_elmnt_pr_t        priority  = 1;
    m_alrt_state_t      alert_state = 1;
    const char         *description = "very small description";
    m_alrt_ntfctn_t     notification = 12;
    int64_t             date_from = 2049829;
    a_elmnt_id_t        dc_id = 1;

    // first insert
    auto reply_insert = insert_into_alert (conn, rule_name, priority, alert_state, description, notification, date_from, dc_id);
    REQUIRE ( reply_insert.status == 0 );
    REQUIRE ( reply_insert.affected_rows == 0 );
    REQUIRE ( reply_insert.errtype == DB_ERR );
    REQUIRE ( reply_insert.errsubtype == DB_ERROR_BADINPUT );
    REQUIRE ( strlen(reply_insert.msg) != 0 );

    log_close();
}

TEST_CASE("t_bios_alert INSERT Null description #8","[db][CRUD][insert][alert][crud_test.sql]")
{
    log_open ();

    log_info ("=============== ALERT: ALERT INSERT #8 ==================");

    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );

    const char         *rule_name = "cool name1";
    a_elmnt_pr_t        priority  = 1;
    m_alrt_state_t      alert_state = 1;
    const char         *description = NULL;
    m_alrt_ntfctn_t     notification = 12;
    int64_t             date_from = 2049829;
    a_elmnt_id_t        dc_id = 1;

    // insert
    auto reply_insert = insert_into_alert (conn, rule_name, priority, alert_state, description, notification, date_from, dc_id);
    REQUIRE ( reply_insert.status == 1 );
    REQUIRE ( reply_insert.affected_rows == 1 );
    uint64_t rowid = reply_insert.rowid;

    // delete
    delete_from_alert (conn, rowid);

    log_close();
}

TEST_CASE("t_bios_alert UPDATE notification by id #9","[db][CRUD][update][alert][crud_test.sql][wrong_input]")
{
    log_open ();

    log_info ("=============== ALERT: ALERT UPDATE by ID #9 ==================");

    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );

    // update
    int64_t  new_notification = 4;
    m_alrt_id_t id = 999;
    auto reply_update = update_alert_notification_byId(conn, new_notification, id);
    REQUIRE (reply_update.status == 1);
    REQUIRE (reply_update.affected_rows == 0);
    REQUIRE (reply_update.rowid == 0);

    log_close();
}

TEST_CASE("t_bios_alert UPDATE tilldate by rulename #10","[db][CRUD][update][alert][crud_test.sql][wrong_input]")
{
    log_open ();

    log_info ("=============== ALERT: ALERT UPDATE by ID #10 ==================");

    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );

    // update
    int64_t  date_till = 2059829;
    const char *rule_name = "abra-cadabra";
    auto reply_update = update_alert_tilldate_by_rulename (conn, date_till, rule_name);
    REQUIRE (reply_update.status == 1);
    REQUIRE (reply_update.affected_rows == 0);
    REQUIRE (reply_update.rowid == 0);

    log_close();
}

