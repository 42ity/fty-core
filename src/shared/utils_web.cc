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
#include <cstring>
#include <ostream>
#include <limits>
#include <mutex>
#include <cxxtools/jsonformatter.h>
#include <cxxtools/convert.h>
#include <cxxtools/regex.h>
#include <cxxtools/serializationinfo.h>
#include <cxxtools/split.h>

#include "utils_web.h"

namespace utils {

uint32_t
string_to_element_id (const std::string& string) {

    size_t pos = 0;
    unsigned long long u = std::stoull (string, &pos);
    if (pos != string.size ()) {
        throw std::invalid_argument ("string_to_element_id");
    }
    if (u == 0 || u >  std::numeric_limits<uint32_t>::max()) {
        throw std::out_of_range ("string_to_element_id");
    }
    uint32_t element_id = static_cast<uint32_t> (u);
    return element_id;
}

namespace json {    

std::string escape (const char *string) {
    if (!string)
        return "(null_ptr)";

    std::string after;
    std::string::size_type length = strlen (string);
    after.reserve (length * 2);

/*
    Quote from http://www.json.org/
    -------------------------------
    Char
    any-Unicode-character-except-"-or-\-or-control-character:
        \"
        \\
        \/
        \b
        \f
        \n
        \r
        \t
        \u four-hex-digits 
    ------------------------------
*/

    for (std::string::size_type i = 0; i < length; ++i) {
        char c = string[i];
        if (c == '"') {
            after.append ("\\\"");
        }
        else if (c =='\b') {
            after.append ("\\\\b");
        }
        else if (c =='\f') {
            after.append ("\\\\f");
        }
        else if (c == '\n') {
            after.append ("\\\\n");
        }
        else if (c == '\r') {
            after.append ("\\\\r");
        }
        else if (c == '\t') {
            after.append ("\\\\t");
        }
        else if (c == '\\') {
            after.append ("\\\\");
        }
        else if (static_cast<unsigned char>(c) >= 0x80 || static_cast<unsigned char>(c) < 0x20) {
            // Code below this comment is taken from
            //      https://github.com/maekitalo/cxxtools
            //      JsonFormatter::stringOut(const std::string& str) {}
            //      Author: Tommi Mäkitalo (tommi@tntnet.org)
            after.append("\\u");
            static const char hex[] = "0123456789abcdef";
            uint32_t v = static_cast<unsigned char>(c);

            for (uint32_t s = 16; s > 0; s -= 4)
            {
                after += hex[(v >> (s - 4)) & 0xf];
            }
        }
        else {
            after += c;
        }
    }
    return after;
}

std::string escape (const std::string& before) {
    return escape (before.c_str ());
}


// Note: At the time of re-writing from defect cxxtools::JsonFormatter I decided
// to go with the simplest solution for create_error_json functions (after all,
// our error template is not that complex). Shall some person find this ugly
// and repulsive enough, there is a working cxxtools::JsonSerialize solution
// ready at the following link:
// http://stash.mbt.lab.etn.com/projects/BIOS/repos/core/pull-requests/1094/diff#src/web/src/error.cc

std::string
create_error_json (const std::string& message, uint32_t code) {
    return std::string (
"{\n"
"\t\"errors\": [\n"
"\t\t{\n"
"\t\t\t\"message\": ").append (jsonify (message)).append (",\n"
"\t\t\t\"code\": ").append (jsonify (code)).append ("\n"
"\t\t}\n"
"\t]\n"
"}\n"
);
}

std::string
create_error_json (const std::string& message, uint32_t code, const std::string& debug) {
    return std::string (
"{\n"
"\t\"errors\": [\n"
"\t\t{\n"
"\t\t\t\"message\": ").append (jsonify (message)).append (",\n"
"\t\t\t\"debug\": ").append (jsonify (debug)).append (",\n"
"\t\t\t\"code\": ").append (jsonify (code)).append ("\n"
"\t\t}\n"
"\t]\n"
"}\n"
);
}

std::string
create_error_json (std::vector <std::tuple<uint32_t, std::string, std::string>> messages) {
    std::string result =
"{\n"
"\t\"errors\": [\n";

    for (const auto &it : messages) {
        result.append (
"\t\t{\n"
"\t\t\t\"message\": "
        );
        result.append (jsonify (std::get<1>(it))).append (
",\n");
        if (std::get<2>(it) != "") {
        result.append (
"\t\t\t\"debug\": "
        );
        result.append (jsonify (std::get<2>(it))).append (
",\n");
        }
        result.append (
"\t\t\t\"code\": "
        );
        result.append (jsonify (std::get<0>(it))).append (
"\n"
"\t\t},\n"
        );
    }
    // remove ,\n
    result.pop_back ();
    result.pop_back ();

    result.append (
"\n\t]\n"
"}\n"
);
    return result;
}

std::string jsonify (double t)
{
    if (isnan(t))
        return "null";
    return std::to_string (t);
}

} // namespace utils::json

namespace config {
const char *
get_mapping (const std::string& key)
{
    const static std::map <const std::string, const std::string> config_mapping = {
        // general
        {"BIOS_SNMP_COMMUNITY_NAME",    "snmp/community"},
        // nut
        {"BIOS_NUT_POLLING_INTERVAL",   "nut/polling_interval"},
        // agent-smtp
        {"BIOS_SMTP_SERVER",            "smtp/server"},
        {"BIOS_SMTP_PORT",              "smtp/port"},
        {"BIOS_SMTP_ENCRYPT",           "smtp/encryption"},
        {"BIOS_SMTP_VERIFY_CA",         "smtp/verify_ca"},
        {"BIOS_SMTP_USER",              "smtp/user"},
        {"BIOS_SMTP_PASSWD",            "smtp/password"},
        {"BIOS_SMTP_FROM",              "smtp/from"},
        {"BIOS_SMTP_SMS_GATEWAY",       "smtp/smsgateway"},
        {"BIOS_SMTP_USE_AUTHENTICATION", "smtp/use_auth"},
        // agent-ms
        {"BIOS_METRIC_STORE_AGE_RT",    "store/rt"},
        {"BIOS_METRIC_STORE_AGE_15m",   "store/15m"},
        {"BIOS_METRIC_STORE_AGE_30m",   "store/30m"},
        {"BIOS_METRIC_STORE_AGE_1h",    "store/1h"},
        {"BIOS_METRIC_STORE_AGE_8h",    "store/8h"},
        {"BIOS_METRIC_STORE_AGE_24h",   "store/24h"},
        {"BIOS_METRIC_STORE_AGE_7d",    "store/7d"},
        {"BIOS_METRIC_STORE_AGE_30d",   "store/30d"}
    };
    if (config_mapping.find (key) == config_mapping.end ())
        return key.c_str ();
    return config_mapping.at (key).c_str (); 
}

const char *
get_path (const std::string& key)
{
    if (key.find ("BIOS_SMTP_") == 0) 
    {
        return "/etc/agent-smtp/bios-agent-smtp.cfg";
    }
    else
    if (key.find ("BIOS_METRIC_STORE_") == 0)
    {
        return "/etc/bios-agent-ms/bios-agent-ms.cfg";
    }
    else
    if (key.find ("BIOS_NUT_") == 0)
    {
        return "/etc/agent-nut/bios-agent-nut.cfg";
    }
    // general config file
    return "/etc/default/bios.cfg";
}

// put new value to zconfig_t - if key has / inside, it create the
// hierarchy automatically.
//
// If c_value is NULL, it simply create hierarchy
static zconfig_t*
s_zconfig_put (zconfig_t *config, const std::string& key, const char* c_value)
{
    std::vector <std::string> keys;
    cxxtools::split ('/', key, std::back_inserter (keys));

    zconfig_t *cfg = config;
    for (const std::string& k: keys)
    {
        if (!zconfig_locate (cfg, k.c_str ()))
            cfg = zconfig_new (k.c_str (), cfg);
        else
            cfg = zconfig_locate (cfg, k.c_str ());
    }

    if (c_value)
        zconfig_set_value (cfg, c_value);

    return cfg;
}

static void
assert_key (const std::string& key)
{
    static std::string key_format_string = "^[-._a-zA-Z0-9/]+$";
    static cxxtools::Regex key_format(key_format_string);
    if (!key_format.match (key)) {
        std::string msg = "to satisfy format " + key_format_string;
        bios_throw ("request-param-bad", key.c_str (), key.c_str (), msg.c_str ());
    }

}

static void
assert_value (const std::string& key, const std::string& value)
{
    static std::string value_format_string = "^[[:blank:][:alnum:][:punct:]]*$";
    static cxxtools::Regex value_format(value_format_string);
    if (!value_format.match (value)) {
        std::string msg2 = "to satisfy format " + value_format_string;
        bios_throw ("request-param-bad", key.c_str (), value.c_str (), msg2.c_str ());
    }

}

void
json2zpl (
        std::map <std::string, zconfig_t*> &roots,
        const cxxtools::SerializationInfo &si,
        std::lock_guard <std::mutex> &lock)
{
    static const std::string slash {"/"};

    if (si.category () != cxxtools::SerializationInfo::Object)
        bios_throw ("bad-request-document", "Root of json request document must be an object");

    for (const auto& it: si) {

        assert_key (it.name ());

        // this is a support for legacy input document, please drop it
        bool legacy_format = it.name () == "config"
                        && it.category () == cxxtools::SerializationInfo::Category::Object;
        if (legacy_format)
        {
            cxxtools::SerializationInfo fake_si;
            cxxtools::SerializationInfo fake_value = si.getMember ("config"). getMember ("value");
            std::string name;
            si.getMember ("config"). getMember ("key"). getValue (name);

            if (fake_value.category () == cxxtools::SerializationInfo::Category::Value)
            {
                std::string value;
                fake_value.getValue (value);
                fake_si.addMember (name) <<= value;
            }
            else
            if (fake_value.category () == cxxtools::SerializationInfo::Category::Array)
            {
                std::vector <std::string> values;
                fake_value >>= values;
                fake_si.addMember (name) <<= values;
            }
            else
            {
                std::string msg = "Value of " + name + " must be string or array of strings.";
                bios_throw ("bad-request-document", msg.c_str ());
            }
            json2zpl (roots, fake_si, lock);
            continue;
        }

        std::string key;
        key = it.name ();

        std::string file_path = get_path (key);
        if (roots.count (file_path) == 0) {
            zconfig_t *root = zconfig_load (file_path.c_str ());
            if (!root)
                root = zconfig_new ("root", NULL);
            roots [file_path] = root;
        }

        zconfig_t *cfg = roots [file_path];

        if (it.category () == cxxtools::SerializationInfo::Category::Value)
        {
            std::string value;
            it.getValue (value);
            s_zconfig_put (cfg, get_mapping (it.name ()), value.c_str ());
        }
        else
        if (it.category () == cxxtools::SerializationInfo::Category::Array)
        {
            std::vector <std::string> values;
            it >>= values;
            size_t i = 0;

            zconfig_t *array_root = zconfig_locate (cfg, get_mapping (it.name ().c_str ()));
            if (array_root) {
                zconfig_print (array_root);
                zconfig_t *i = zconfig_child (array_root);
                while (i) {
                    zconfig_set_value (i, NULL);
                    i = zconfig_next (i);
                }
            }

            for (const auto& value : values) {
                assert_value (it.name (), value);
                std::string name =
                    get_mapping (it.name ()) + slash + std::to_string(i);
                s_zconfig_put (cfg, name.c_str (), value.c_str ());
                i++;
            }
        }
        else {
            std::string msg = "Value of " + it.name () + " must be string or array of strings.";
            bios_throw ("bad-request-document", msg.c_str ());
        }
    }
}

} // namespace utils::config

} // namespace utils

