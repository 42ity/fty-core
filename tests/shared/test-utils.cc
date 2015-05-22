#include <catch.hpp>

#include "defs.h"
#include "utils.h"

TEST_CASE("str_eq","[utils][streq]"){
    CHECK(str_eq(NULL, NULL));
    CHECK_FALSE(str_eq("a", NULL));
    CHECK_FALSE(str_eq(NULL, "b"));
    CHECK(str_eq("a", "a"));
    CHECK_FALSE(str_eq("yyy","bb"));
    CHECK_FALSE(str_eq("yyy","yy"));
    CHECK_FALSE(str_eq("YY","yy"));
    CHECK_FALSE(str_eq("\0",NULL));
    CHECK(str_eq("","\0")); 
} 


TEST_CASE("str_bool","[str_bool][utils]") {
    CHECK(str_eq(str_bool(true), "true"));
    CHECK(str_eq(str_bool(false), "false"));
    CHECK_FALSE(str_eq(str_bool(true),"True"));
    CHECK_FALSE(str_eq(str_bool(false),"False"));
}

TEST_CASE("safe_str","[safe_str][utils]"){
    CHECK(str_eq(safe_str(NULL), "(null)"));
    CHECK(str_eq(safe_str("a"), "a"));
    CHECK(str_eq(safe_str("\0"), ""));
}

TEST_CASE ("average_step_seconds", "[average_step_seconds][utils][average]") {

    SECTION ("bad arguments") {
        CHECK ( average_step_seconds ("15s") == -1 );
        CHECK ( average_step_seconds ("15h") == -1 );
        CHECK ( average_step_seconds ("15d") == -1 );

        CHECK ( average_step_seconds ("30h") == -1 );
        CHECK ( average_step_seconds ("30d") == -1 );

        CHECK ( average_step_seconds ("1m") == -1 );
        CHECK ( average_step_seconds ("24m") == -1 );
        CHECK ( average_step_seconds ("24d") == -1 );

        CHECK ( average_step_seconds ("1d") == -1 );
        CHECK ( average_step_seconds ("1440m") == -1 );
        CHECK ( average_step_seconds ("2d") == -1 );
    }
    SECTION ("correct execution") {
        CHECK ( average_step_seconds ("15m") == 900 );
        CHECK ( average_step_seconds ("30m") == 1800 );
        CHECK ( average_step_seconds ("1h") == 3600 );
        CHECK ( average_step_seconds ("24h") == 86400 );
    }
}

TEST_CASE ("average_extend_left_margin", "[utils][average][time]") {
// TODO: Very important, make other types, unusual and corner cases
    SECTION ("8h") {
        CHECK ( average_extend_left_margin (1426611600, "8h") == 1426607700 ); // 2015-03-17 17:00:00 overlap => 2015-03-17 15:55:00
        CHECK ( average_extend_left_margin (1426618800, "8h") == 1426607700 ); // 2015-03-17 19:00:00 overlap => 2015-03-17 15:55:00
        CHECK ( average_extend_left_margin (1426593600, "8h") == 1426578900 ); // 2015-03-17 12:00:00 overlap => 2015-03-17 07:55:00
        CHECK ( average_extend_left_margin (1426622400, "8h") == 1426607700 ); // 2015-03-17 20:00:00 overlap => 2015-03-17 15:55:00
        CHECK ( average_extend_left_margin (1426608000, "8h") == 1426578900 ); // 2015-03-17 16:00:00         => 2015-03-17 07:55:00
        CHECK ( average_extend_left_margin (1426550400, "8h") == 1426521300 ); // 2015-03-17 00:00:00         => 2015-03-16 15:55:00

        CHECK ( average_extend_left_margin (1426554015, "8h") == 1426550100 ); // 2015-03-17 01:00:15         => 2015-03-16 23:55:00
        CHECK ( average_extend_left_margin (1426608115, "8h") == 1426607700 ); // 2015-03-17 16:01:55
        CHECK ( average_extend_left_margin (1426636799, "8h") == 1426607700 ); // 2015-03-17 23:59:59

        CHECK ( average_extend_left_margin (1426266000, "8h") == 1426262100 ); // 2015-03-13 17:00:00
    }
    SECTION ("15m") {
        CHECK ( average_extend_left_margin (951865200, "15m") == 951864000 ); // 2000-02-29 23:00:00 
        CHECK ( average_extend_left_margin (951865380, "15m") == 951864900 ); // 2000-02-29 23:03:00 overlap
        CHECK ( average_extend_left_margin (951869063, "15m") == 951868500 ); // 2000-03-01 00:04:23 overlap
    }
}

TEST_CASE ("datetime_to_calendar", "[utils][time]") {
    SECTION ("bad arguments") {
        CHECK ( datetime_to_calendar (NULL) == -1 );
        CHECK ( datetime_to_calendar ("") == -1 );
        CHECK ( datetime_to_calendar ("20150419181523") == -1 ); // missing utc timezone specifier
        CHECK ( datetime_to_calendar ("20150419181523z") == -1 ); // wrong small case
        CHECK ( datetime_to_calendar ("2015041918152Z") == -1 ); // one number short
    }
    SECTION ("correct execution") {
        CHECK ( datetime_to_calendar ("20150301100532Z") == 1425204332 );
        CHECK ( datetime_to_calendar ("20150304235959Z") == 1425513599 );
        CHECK ( datetime_to_calendar ("19991231235959Z") == 946684799 );
        CHECK ( datetime_to_calendar ("20000101000000Z") == 946684800 );
        CHECK ( datetime_to_calendar ("20000131235959Z") == 949363199 );
        CHECK ( datetime_to_calendar ("20000201000000Z") == 949363200 );
        CHECK ( datetime_to_calendar ("20000229235959Z") == 951868799 );
        CHECK ( datetime_to_calendar ("20000301000000Z") == 951868800 );
        CHECK ( datetime_to_calendar ("20000526135504Z") == 959349304 );
        CHECK ( datetime_to_calendar ("20000430235959Z") == 957139199 );
        CHECK ( datetime_to_calendar ("20000501000000Z") == 957139200 );
        CHECK ( datetime_to_calendar ("20001231235959Z") == 978307199 );
        CHECK ( datetime_to_calendar ("20010101000000Z") == 978307200 );
        CHECK ( datetime_to_calendar ("20010731235959Z") == 996623999 );
        CHECK ( datetime_to_calendar ("20010801000000Z") == 996624000 );
        CHECK ( datetime_to_calendar ("20010930235959Z") == 1001894399 );
        CHECK ( datetime_to_calendar ("20011001000000Z") == 1001894400 );
    }
}

TEST_CASE ("average_first_since", "[utils][average][time]") {
    SECTION ("bad args") {
        CHECK ( average_first_since (-1 , "8h") == -1 );
        CHECK ( average_first_since (12 , "76m") == -1 );
    }

    SECTION ("correct invocation") {
        // 1430174399 == Mon Apr 27 22:39:59 UTC 2015                       expected
        CHECK ( average_first_since (1430174399, "8h") == 1430179200 );  // 2015-04-28 00:00:00
        CHECK ( average_first_since (1430174399, "15m") == 1430174700 ); // 2015-04-27 22:45:00
        CHECK ( average_first_since (1430174399, "30m") == 1430175600 ); // 2015-04-27 23:00:00
        CHECK ( average_first_since (1430174399, "1h") == 1430175600 );  // 2015-04-27 23:00:00

        // 1407801599 == 2014-08-11 23:59:59 UTC                       expected
        CHECK ( average_first_since (1407801599, "8h") == 1407801600 );  // 2014-08-12 00:00:00
        CHECK ( average_first_since (1407801599, "15m") == 1407801600 );
        CHECK ( average_first_since (1407801599, "30m") == 1407801600 );
        CHECK ( average_first_since (1407801599, "1h") == 1407801600 );  

        // 1430180776 == 00:26:16 2015-04-28 UTC                       expected
        CHECK ( average_first_since (1430180776, "8h") == 1430208000 ); // 2015-04-28 08:00:00
        CHECK ( average_first_since (1430180776, "1h") == 1430182800 ); // 2015-04-28 01:00:00
        CHECK ( average_first_since (1430180776, "30m") == 1430181000 ); // 2015-04-28 00:30:00
        CHECK ( average_first_since (1430180776, "15m") == 1430181000 ); // 2015-04-28 00:30:00

        // exact
        // 1430179200 == 00:00:00 2015-04-28 UTC
        CHECK ( average_first_since (1430179200, "8h") == 1430208000 );  // 2015-04-28 08:00:00
        CHECK ( average_first_since (1430179200, "15m") == 1430180100 );  // 2015-04-28 00:15:00
        CHECK ( average_first_since (1430179200, "1h") == 1430182800 );  // 2015-04-28 01:00:00

        // 1430208000 == 08:00:00 2015-04-28 UTC
        CHECK ( average_first_since (1430208000, "8h") == 1430236800 ); // 2015-04-28 16:00:00
        CHECK ( average_first_since (1430208000, "1h") == 1430211600 ); // 2015-04-28 09:00:00
        CHECK ( average_first_since (1430208000, "15m") == 1430208900 ); // 2015-04-28 08:15:00

        // 1430183700 == 01:15:00 2015-04-28 UTC
        CHECK ( average_first_since (1430183700, "8h") == 1430208000 ); // 2015-04-28 08:00:00
        CHECK ( average_first_since (1430183700, "1h") == 1430186400 ); // 2015-04-28 02:00:00
        CHECK ( average_first_since (1430183700, "15m") == 1430184600 ); // 2015-04-28 01:30:00
    }
}

TEST_CASE("addi32_overflow", "[utils][overflow]") {
    int32_t a, b, value;
    bool ret;

    ret = addi32_overflow(22, 20, &value);
    CHECK(ret);
    CHECK(value == 42);

    ret = addi32_overflow(INT32_MAX, 0, &value);
    CHECK(ret);
    CHECK(value == INT32_MAX);

    ret = addi32_overflow(INT32_MAX, 2, &value);
    CHECK(!ret);
    CHECK(value == INT32_MAX); // old value

    ret = addi32_overflow(66, -24, &value);
    CHECK(ret);
    CHECK(value == 42);

    ret = addi32_overflow(INT32_MIN, -1, &value);
    CHECK(!ret);
    CHECK(value == 42); // old value
}

TEST_CASE("bsi32_rescale","[utils][bs_rescale]"){

    int32_t value;
    bool ret;

    // upscale
    ret = bsi32_rescale(42, 0, 1, &value);
    CHECK(ret);
    CHECK(value == 4);

    ret = bsi32_rescale(42, 0, 3, &value);
    CHECK(ret);
    CHECK(value == 0);

    // down
    ret = bsi32_rescale(42, 0, -3, &value);
    CHECK(ret);
    CHECK(value == 42000);

    // underflow
    ret = bsi32_rescale(42, 0, -128, &value);
    CHECK(!ret);
    CHECK(value == 42000); //<<< just the previous value

    // overflow
    ret = bsi32_rescale(42, 0, 128, &value);
    CHECK(!ret);
    CHECK(value == 42000); //<<< just the previous value

    // upscale - negative
    ret = bsi32_rescale(-42, 0, 1, &value);
    CHECK(ret);
    CHECK(value == -4);

    // downscale - negative
    ret = bsi32_rescale(-42, 0, -3, &value);
    CHECK(ret);
    CHECK(value == -42000);

    // underflow - negative
    ret = bsi32_rescale(INT32_MIN / 10, 0, -2, &value);
    CHECK(!ret);
    CHECK(value == -42000); //<<< just the previous value
    
}

TEST_CASE("bsi32_add","[utils][bs_add]"){

    int32_t value;
    int8_t scale;
    bool ret;

    // add with the same scale is easy ...
    ret = bsi32_add(22, 0, 20, 0, &value, &scale);
    CHECK(ret);
    CHECK(value == 42);
    CHECK(scale == 0);

    // ... but we do support any arbitrary scale
    ret = bsi32_add(40, 0, 20, -1, &value, &scale);
    CHECK(ret);
    CHECK(value == 420);
    CHECK(scale == -1);

    // ... and we check overflows
    ret = bsi32_add(40, 0, 20, -128, &value, &scale);
    CHECK(!ret);
    CHECK(value == 420);    //<<< just the previous value
    CHECK(scale == -1);     //<<< just the previous value

    // ... and we check overflows
    ret = bsi32_add(INT32_MAX, 0, 42, 0, &value, &scale);
    CHECK(!ret);
    CHECK(value == 420);    //<<< just the previous value
    CHECK(scale == -1);     //<<< just the previous value

    // negative numbers
    ret = bsi32_add(64, 0, -22, 0, &value, &scale);
    CHECK(ret);
    CHECK(value == 42);
    CHECK(scale == 0);

    // negative numbers - underflows
    ret = bsi32_add(INT32_MIN, 0, -22, 0, &value, &scale);
    CHECK(!ret);
    CHECK(value == 42);    //<<< just the previous value
    CHECK(scale == 0);     //<<< just the previous value
    return;
}
