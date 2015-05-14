#include <catch.hpp>

#include "dbpath.h"
#include "log.h"

#include "db/alert.h"
#include "assetcrud.h"
#include "monitor.h"

using namespace persist;

TEST_CASE("t_bios_alert INSERT/DELETE #1","[db][CRUD][insert][delete][alert][crud_test.sql]")
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

    // before first insert
    auto reply_select = select_alert_all_opened (conn);
    REQUIRE ( reply_select.status == 1 );
    auto                oldsize_a = reply_select.item.size() ;

    // first insert
    auto reply_insert = insert_into_alert (conn, rule_name, priority, alert_state, description, notification, date_from);
    REQUIRE ( reply_insert.status == 1 );
    uint64_t rowid = reply_insert.rowid;
    CAPTURE (rowid);
    REQUIRE ( reply_insert.affected_rows == 1 );

    // check select
    reply_select = select_alert_all_opened (conn);
    REQUIRE ( reply_select.status == 1 );
    REQUIRE ( reply_select.item.size() == (oldsize_a + 1) );
    REQUIRE ( reply_select.item.at(oldsize_a).id == rowid );
    REQUIRE ( reply_select.item.at(oldsize_a).rule_name == std::string(rule_name) );
    REQUIRE ( reply_select.item.at(oldsize_a).alert_state == alert_state );
    REQUIRE ( reply_select.item.at(oldsize_a).description == std::string(description) );
    REQUIRE ( reply_select.item.at(oldsize_a).notification == notification );
    REQUIRE ( reply_select.item.at(oldsize_a).date_from == date_from );
    REQUIRE ( reply_select.item.at(oldsize_a).date_till == 0 );

    // must handle duplicate insert without insert
    reply_insert = insert_into_alert (conn, rule_name, priority, alert_state, description, notification, date_from);
    REQUIRE ( reply_insert.status == 1 );
    REQUIRE ( reply_insert.affected_rows == 0 );

    // first delete
    auto reply_delete = delete_from_alert (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 1 );
    REQUIRE ( reply_delete.status == 1 );

    // check select
    reply_select = select_alert_all_opened (conn);
    REQUIRE ( reply_select.status == 1 );
    REQUIRE ( reply_select.item.size() == oldsize_a );

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

    // before first insert
    auto reply_select = select_alert_all_closed (conn);
    REQUIRE ( reply_select.status == 1 );
    auto                oldsize_a = reply_select.item.size() ;

    // insert
    auto reply_insert = insert_into_alert (conn, rule_name, priority, alert_state, description, notification, date_from);
    uint64_t rowid = reply_insert.rowid;

    // update
    int64_t  date_till = 2059829;
    auto reply_update = update_alert_tilldate(conn, date_till, rowid);
    REQUIRE (reply_update.status == 1);
    REQUIRE (reply_update.affected_rows == 1);

    // check select
    reply_select = select_alert_all_closed (conn);
    REQUIRE ( reply_select.status == 1 );
    REQUIRE ( reply_select.item.size() == (oldsize_a+1) );
    REQUIRE ( reply_select.item.at(oldsize_a).id == rowid );
    REQUIRE ( reply_select.item.at(oldsize_a).rule_name == std::string(rule_name) );
    REQUIRE ( reply_select.item.at(oldsize_a).alert_state == alert_state );
    REQUIRE ( reply_select.item.at(oldsize_a).description == std::string(description) );
    REQUIRE ( reply_select.item.at(oldsize_a).notification == notification );
    REQUIRE ( reply_select.item.at(oldsize_a).date_from == date_from );
    REQUIRE ( reply_select.item.at(oldsize_a).date_till == date_till );

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

    // before first insert
    auto reply_select = select_alert_all_opened (conn);
    REQUIRE ( reply_select.status == 1 );
    auto                oldsize_a = reply_select.item.size() ;

    // insert
    auto reply_insert = insert_into_alert (conn, rule_name, priority, alert_state, description, notification, date_from);
    uint64_t rowid = reply_insert.rowid;

    // update
    int64_t  new_notification = 4;
    auto reply_update = update_alert_notification_byId(conn, new_notification, rowid);
    REQUIRE (reply_update.status == 1);
    REQUIRE (reply_update.affected_rows == 1);

    // check select
    reply_select = select_alert_all_opened (conn);
    REQUIRE ( reply_select.status == 1 );
    REQUIRE ( reply_select.item.size() == (oldsize_a + 1) );
    REQUIRE ( reply_select.item.at(oldsize_a).id == rowid );
    REQUIRE ( reply_select.item.at(oldsize_a).rule_name == std::string(rule_name) );
    REQUIRE ( reply_select.item.at(oldsize_a).alert_state == alert_state );
    REQUIRE ( reply_select.item.at(oldsize_a).description == std::string(description) );
    REQUIRE ( reply_select.item.at(oldsize_a).notification == ( new_notification | notification ) );
    REQUIRE ( reply_select.item.at(oldsize_a).date_from == date_from );
    REQUIRE ( reply_select.item.at(oldsize_a).date_till == 0 );

    // delete
    delete_from_alert (conn, rowid);

    log_close();
}


TEST_CASE("t_bios_alert_device INSERT/DELETE #4","[db][CRUD][insert][delete][alert_device][crud_test.sql]")
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
    auto reply_insert_alert = insert_into_alert (conn, rule_name, priority, alert_state, description, notification, date_from);
    uint64_t rowid_alert = reply_insert_alert.rowid;
    CAPTURE ( rowid_alert );

    //insert element
    const char *element_name = "test_element_name4";
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


    // before first insert
    auto reply_select = select_alert_devices (conn, rowid_alert);
    REQUIRE ( reply_select.status == 1 );
    auto                oldsize_a = reply_select.item.size() ;

    // first insert
    auto reply_insert1 = insert_into_alert_device (conn, rowid_alert, element_name);
    REQUIRE ( reply_insert1.status == 1 );
    uint64_t rowid = reply_insert1.rowid;
    CAPTURE ( rowid );
    REQUIRE ( reply_insert1.affected_rows == 1 );

    // check select
    reply_select = select_alert_devices (conn, rowid_alert);
    REQUIRE ( reply_select.status == 1 );
    REQUIRE ( reply_select.item.size() == (oldsize_a + 1) );
    REQUIRE ( reply_select.item.at(oldsize_a) == rowid_device );

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
    REQUIRE ( reply_select.item.size() == oldsize_a );

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


TEST_CASE("t_bios_alert_device INSERT/DELETE #5","[db][CRUD][insert][delete][alert_device][crud_test.sql]")
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
    auto reply_insert_alert = insert_into_alert (conn, rule_name, priority, alert_state, description, notification, date_from);
    uint64_t rowid_alert = reply_insert_alert.rowid;
    CAPTURE ( rowid_alert );

    //insert element
    const char *element_name1 = "test_element_name5.1";
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

    // before first insert
    auto reply_select = select_alert_devices (conn, rowid_alert);
    REQUIRE ( reply_select.status == 1 );
    auto                oldsize_d = reply_select.item.size() ;

    // first insert
    std::vector<std::string> names;
    names.push_back(std::string(element_name1));
    names.push_back(std::string(element_name2));

    auto reply_insert1 = insert_into_alert_devices (conn, rowid_alert, names);
    REQUIRE ( reply_insert1.status == 1 );
    REQUIRE ( reply_insert1.affected_rows == 2 );

    // check select
    reply_select = select_alert_devices (conn, rowid_alert);
    REQUIRE ( reply_select.status == 1 );
    REQUIRE ( reply_select.item.size() == (oldsize_d + 2) );
    bool first  = ( reply_select.item.at(oldsize_d) == rowid_device1 ) && ( reply_select.item.at(oldsize_d+1) == rowid_device2 );
    bool second = ( reply_select.item.at(oldsize_d) == rowid_device2 ) && ( reply_select.item.at(oldsize_d+1) == rowid_device1 );

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
    const char *element_name1 = "test_element_name5.1";
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

    // before first insert
    auto reply_select1 = select_alert_all_opened (conn);
    REQUIRE ( reply_select1.status == 1 );
    auto                oldsize_a = reply_select1.item.size() ;

    // first insert
    std::vector<std::string> names;
    names.push_back(std::string(element_name1));
    names.push_back(std::string(element_name2));

    // insert alert
    const char         *rule_name = "this is the GREAT rule name5";
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
    bool first  = ( reply_select.item.at(0) == rowid_device1 ) && ( reply_select.item.at(1) == rowid_device2 );
    bool second = ( reply_select.item.at(0) == rowid_device2 ) && ( reply_select.item.at(1) == rowid_device1 );
    REQUIRE ( (first || second) );

    reply_select1 = select_alert_all_opened (conn);
    REQUIRE ( reply_select1.status == 1 );
    REQUIRE ( reply_select1.item.size() == (oldsize_a + 1) );
    REQUIRE ( reply_select1.item.at(oldsize_a).id == rowid_alert );
    REQUIRE ( reply_select1.item.at(oldsize_a).rule_name == std::string(rule_name) );
    REQUIRE ( reply_select1.item.at(oldsize_a).alert_state == alert_state );
    REQUIRE ( reply_select1.item.at(oldsize_a).description == std::string(description) );
    REQUIRE ( reply_select1.item.at(oldsize_a).notification == notification );
    REQUIRE ( reply_select1.item.at(oldsize_a).date_from == date_from );
    REQUIRE ( reply_select1.item.at(oldsize_a).date_till == 0 );

    // delete
    delete_from_alert_device_byalert (conn, rowid_alert);
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

TEST_CASE("t_bios_alert UPDATE end date #7","[db][CRUD][update][alert][crud_test.sql]")
{
    log_open ();

    log_info ("=============== ALERT: ALERT UPDATE end date #7 ==================");

    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );
    
    const char         *rule_name = "this is the GREAT rule name6";
    a_elmnt_pr_t        priority  = 1;
    m_alrt_state_t      alert_state = 1;
    const char         *description = "very small description";
    m_alrt_ntfctn_t     notification = 12;
    int64_t             date_from = 2049829;

    // before first insert
    auto reply_select = select_alert_all_closed (conn);
    REQUIRE ( reply_select.status == 1 );
    auto                oldsize_a = reply_select.item.size() ;

    // insert
    auto reply_insert = insert_into_alert (conn, rule_name, priority, alert_state, description, notification, date_from);
    uint64_t rowid = reply_insert.rowid;

    // update
    int64_t  date_till = 2059829;
    auto reply_update = update_alert_tilldate_by_rulename (conn, date_till, rule_name);
    REQUIRE (reply_update.status == 1);
    REQUIRE (reply_update.affected_rows == 1);

    // check select
    reply_select = select_alert_all_closed (conn);
    REQUIRE ( reply_select.status == (oldsize_a + 1) );
    REQUIRE ( reply_select.item.size() == 1 );
    REQUIRE ( reply_select.item.at(oldsize_a).id == rowid );
    REQUIRE ( reply_select.item.at(oldsize_a).rule_name == std::string(rule_name) );
    REQUIRE ( reply_select.item.at(oldsize_a).alert_state == alert_state );
    REQUIRE ( reply_select.item.at(oldsize_a).description == std::string(description) );
    REQUIRE ( reply_select.item.at(oldsize_a).notification == notification );
    REQUIRE ( reply_select.item.at(oldsize_a).date_from == date_from );
    REQUIRE ( reply_select.item.at(oldsize_a).date_till == date_till );

    // delete
    delete_from_alert (conn, rowid);
    
    log_close();
}
