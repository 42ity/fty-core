/*
 *
 * Copyright (C) 2015 Eaton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*!
 * \file test-helpers.cc
 * \author Karol Hrdina <KarolHrdina@Eaton.com>
 * \brief Not yet documented file
 */
#include <catch.hpp>
#include "helpers.h"

TEST_CASE ("unicode related stuff", "[helpers]") {

    static const std::vector <std::string> strings {
        "Žluťou-čký kůň@",
        "\u0441?sma__l \xf0\x90\x90\xa8@\ufe56",
        "abcd-ef%",
        "Žluťoučký kůň-"
    };

    std::vector <char> exclude {
        '_',
        '%',
        '@'
    }; 

    SECTION ("utf8_contains_chars") {
        CHECK ( utf8_contains_chars (strings[0], exclude) == 1 );
        CHECK ( utf8_contains_chars (strings[1], exclude) == 1 );
        CHECK ( utf8_contains_chars (strings[2], exclude) == 1 );
        CHECK ( utf8_contains_chars (strings[3], exclude) == 0 );
    }
}

TEST_CASE ("check_element_identifier", "[helpers]") {

    http_errors_t errors;
    std::string value;
    bool res;
    uint32_t element_id;

    SECTION ("bad input") {
        errors.errors.clear (); value = ""; element_id = 0; res = false;
        CHECK_NOTHROW ( res = check_element_identifier ("element_id", value, element_id, errors) );
        CHECK ( res == false );
        CHECK ( errors.errors.size () == 1 );
        CHECK ( element_id == 0 );

        errors.errors.clear (); value = "as/lvm';oi9j'vwn\uA66A\n\n"; element_id = 0; res = false;
        CHECK_NOTHROW ( res = check_element_identifier ("element_id", value, element_id, errors) );
        CHECK ( res == false );
        CHECK ( errors.errors.size () == 1 );
        CHECK ( element_id == 0 );

        errors.errors.clear (); value = "12s12314"; element_id = 0; res = false;
        CHECK_NOTHROW ( res = check_element_identifier ("element_id", value, element_id, errors) );
        CHECK ( res == false );
        CHECK ( errors.errors.size () == 1 );
        CHECK ( element_id == 0 );

        errors.errors.clear (); value = "s12314"; element_id = 0; res = false;
        CHECK_NOTHROW ( res = check_element_identifier ("element_id", value, element_id, errors) );
        CHECK ( res == false );
        CHECK ( errors.errors.size () == 1 );
        CHECK ( element_id == 0 );

        errors.errors.clear (); value = "123 14"; element_id = 0; res = false;
        CHECK_NOTHROW ( res = check_element_identifier ("element_id", value, element_id, errors) );
        CHECK ( res == false );
        CHECK ( errors.errors.size () == 1 );
        CHECK ( element_id == 0 );

        errors.errors.clear (); value = "0"; element_id = 0; res = false;
        CHECK_NOTHROW ( res = check_element_identifier ("element_id", value, element_id, errors) );
        CHECK ( res == false );
        CHECK ( errors.errors.size () == 1 );
        CHECK ( element_id == 0 );

        errors.errors.clear (); value = "0"; element_id = 0; res = false;
        CHECK_NOTHROW ( res = check_element_identifier ("element_id", value, element_id, errors) );
        CHECK ( res == false );
        CHECK ( errors.errors.size () == 1 );
        CHECK ( element_id == 0 );

        errors.errors.clear (); value = "4294967296"; element_id = 0; res = false;
        CHECK_NOTHROW ( res = check_element_identifier ("element_id", value, element_id, errors) );
        CHECK ( res == false );
        CHECK ( errors.errors.size () == 1 );
        CHECK ( element_id == 0 );

        errors.errors.clear (); value = "124294967294"; element_id = 0; res = false;
        CHECK_NOTHROW ( res = check_element_identifier ("element_id", value, element_id, errors) );
        CHECK ( res == false );
        CHECK ( errors.errors.size () == 1 );
        CHECK ( element_id == 0 );
        
        errors.errors.clear (); value = "1111111111111111111111111111111111111111111111111111111111124294967294"; element_id = 0; res = false;
        CHECK_NOTHROW ( res = check_element_identifier ("element_id", value, element_id, errors) );
        CHECK ( res == false );
        CHECK ( errors.errors.size () == 1 );
        CHECK ( element_id == 0 );
    }

    SECTION ("valid input") {
        errors.errors.clear (); value = "1"; element_id = 0; res = false;
        CHECK_NOTHROW ( res = check_element_identifier ("element_id", value, element_id, errors) );
        CHECK ( res == true );
        CHECK ( errors.errors.size () == 0 );
        CHECK ( element_id == 1 );

        errors.errors.clear (); value = "4294967295"; element_id = 0; res = false;
        CHECK_NOTHROW ( res = check_element_identifier ("element_id", value, element_id, errors) );
        CHECK ( res == true );
        CHECK ( errors.errors.size () == 0 );
        CHECK ( element_id == UINT_MAX );

        errors.errors.clear (); value = "12"; element_id = 0; res = false;
        CHECK_NOTHROW ( res = check_element_identifier ("element_id", value, element_id, errors) );
        CHECK ( res == true );
        CHECK ( errors.errors.size () == 0 );
        CHECK ( element_id == 12 );
        
        errors.errors.clear (); value = "1321"; element_id = 0; res = false;
        CHECK_NOTHROW ( res = check_element_identifier ("element_id", value, element_id, errors) );
        CHECK ( res == true );
        CHECK ( errors.errors.size () == 0 );
        CHECK ( element_id == 1321 );
    }

    SECTION ("text check") {
        errors.errors.clear (); value = "This-is_valid01.rule"; element_id = 0; res = false;
        CHECK_NOTHROW ( res = check_regex_text ("hey", value , "^[-._a-z0-9]{1,255}$", errors) );
        CHECK ( res == true );
        CHECK ( errors.errors.size () == 0 );

        errors.errors.clear ();
        CHECK_NOTHROW ( res = check_regex_text ("hey", value, "^[-._a-z0-9]{100,255}$" , errors) );
        CHECK ( res == false );
        CHECK ( errors.errors.size () == 1 );

        errors.errors.clear ();
        CHECK_NOTHROW ( res = check_regex_text ("hey", value, "^[-._a-z0-9]{1,2}$" , errors) );
        CHECK ( res == false );
        CHECK ( errors.errors.size () == 1 );
    }
}

