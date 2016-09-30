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
        {"BIOS_SMTP_PASSWD",            "smtp/passwd"},
        {"BIOS_SMTP_FROM",              "smtp/from"},
        {"BIOS_SMTP_SMS_GATEWAY",       "smtp/smsgateway"},
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

zconfig_t*
json2zpl (
        zconfig_t *root,
        const cxxtools::SerializationInfo &si)
{
    for (const auto& it: si) {

        bool legacy_path = it.name () == "config"
                        && it.category () == cxxtools::SerializationInfo::Category::Object;

        zconfig_t *cfg;
        if (!legacy_path)
            cfg = s_zconfig_put (root, get_mapping (it.name ()), NULL);

        // this is a support for legacy input document, please drop it
        if (legacy_path)
        {
            cxxtools::SerializationInfo fake_si;
            cxxtools::SerializationInfo fake_value = si.getMember ("config"). getMember ("value");
            std::string name;
            si.getMember ("config"). getMember ("key"). getValue (name);
            name = get_mapping (name);

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
            json2zpl (root, fake_si);
            continue;
        }
        else
        if (it.category () == cxxtools::SerializationInfo::Category::Value)
        {
            std::string value;
            it.getValue (value);
            zconfig_set_value (cfg, value.c_str ());
        }
        else
        if (it.category () == cxxtools::SerializationInfo::Category::Array)
        {
            std::vector <std::string> values;
            it >>= values;
            size_t i = 0;
            for (const auto& value : values) {
                 s_zconfig_put (cfg, std::to_string (i).c_str (), value.c_str ());
                 i++;
            }
        }
    }

    return root;
}
} // namespace utils::config

} // namespace utils

