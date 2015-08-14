#include <catch.hpp>

#include <iostream>
#include <czmq.h>

#include "common_msg.h"
#include "monitor.h"
#include "assetcrud.h"

#include "dbpath.h"
#include "log.h"

#include "cleanup.h"

TEST_CASE("Common messages: _generate_db_fail","[common][generate][db_fail][db]")
{
    uint32_t errnonew = 1;
    _scoped_common_msg_t* fail = generate_db_fail (errnonew, NULL,  NULL);
    REQUIRE ( fail );
    REQUIRE ( common_msg_id(fail) == COMMON_MSG_FAIL );
//    common_msg_print (fail);

    byte        errno1 = common_msg_errorno (fail);
    const char* errmsg = common_msg_errmsg  (fail);
    zhash_t *   erraux = common_msg_aux  (fail);
    REQUIRE ( errnonew == errno1 );
    REQUIRE ( erraux   == NULL );
    REQUIRE ( streq(errmsg, "")  );

    common_msg_destroy(&fail);
    REQUIRE ( fail == NULL );

    errnonew = 2;
    fail = generate_db_fail (errnonew, "",  NULL);
    REQUIRE( fail );
    REQUIRE ( common_msg_id(fail) == COMMON_MSG_FAIL );
//    common_msg_print (fail);

    errno1 = common_msg_errorno (fail);
    errmsg = common_msg_errmsg  (fail);
    erraux = common_msg_aux  (fail);
    REQUIRE ( errnonew == errno1 );
    REQUIRE ( erraux   == NULL );
    REQUIRE ( streq(errmsg, "")  );

    common_msg_destroy(&fail);
    REQUIRE ( fail == NULL );

    errnonew             = 3;
    char     errmsgnew[] = "abracadabra";
    _scoped_zhash_t* errauxnew   = zhash_new();
    char     keytag[]    = "mykey";
    char     value[]     = "myvalue";
    zhash_insert (errauxnew, keytag, value);

    fail = generate_db_fail (errnonew, errmsgnew, &errauxnew);
    REQUIRE ( fail );
    REQUIRE ( common_msg_id(fail) == COMMON_MSG_FAIL );
    // REQUIRE ( errauxnew == NULL );  // this doesn't work
//    common_msg_print (fail);
    
    errno1 = common_msg_errorno (fail);
    errmsg = common_msg_errmsg  (fail);
    erraux = common_msg_aux  (fail);
    REQUIRE ( errnonew == errno1 );
    REQUIRE ( streq( errmsg, errmsgnew) );
    // TODO how to compare zhash_t ??
    common_msg_destroy (&fail);
    REQUIRE ( fail == NULL );
}


TEST_CASE("Common messages: _generate_ok","[common][generate][db_ok][db]")
{
    uint32_t rowid = 11111111;
    _scoped_common_msg_t* okmsg = generate_ok (rowid, NULL);
    REQUIRE ( okmsg );
    
    REQUIRE ( common_msg_id (okmsg) == COMMON_MSG_DB_OK );
//    common_msg_print (okmsg);
    uint32_t rowid_ = common_msg_rowid (okmsg);
    REQUIRE ( rowid == rowid_ );

    common_msg_destroy (&okmsg);
    REQUIRE ( okmsg == NULL );
}
