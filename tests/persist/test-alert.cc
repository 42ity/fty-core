#include <catch.hpp>

#include "dbpath.h"
#include "log.h"

#include "alert.h"

TEST_CASE("t_bios_alert INSERT/DELETE #1","[db][CRUD][insert][delete][alert][crud_test.sql]")
{
    log_open ();

    log_info ("=============== ALERT: ALERT DELETE/INSERT #1 ==================");
    
    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );
    
    const char         *rule_name = "this is the GREAT rule name";
    a_elmnt_pr_t        priority  = 1;
    m_alrt_state_t      alert_state = 1;
    const char         *description = "very small description";
    m_alrt_ntfctn_t     notification = 12;
    int64_t             date_from = 2049829;

    // first insert
    auto reply_insert = insert_into_alert (conn, rule_name, priority, alert_state, description, notification, date_from);
    REQUIRE ( reply_insert.status == 1 );
    uint64_t rowid = reply_insert.rowid;
    CAPTURE (rowid);
    REQUIRE ( reply_insert.affected_rows == 1 );

    // check select
    auto reply_select = select_alert_all_opened (conn);
    REQUIRE ( reply_select.status == 1 );
    REQUIRE ( reply_select.item.size() == 1 );
    REQUIRE ( reply_select.item.at(0).id == rowid );
    // TODO overit po polozkach

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
    REQUIRE ( reply_select.item.size() == 0 );
    
    // must handle second delete without crash
    reply_delete = delete_from_alert (conn, rowid);
    REQUIRE ( reply_delete.affected_rows == 0 );
    REQUIRE ( reply_delete.status == 1 );

    log_close();
}
