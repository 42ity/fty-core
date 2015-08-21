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
 * \file dci_impl.h
 * \author Michal Vyskocil <MichalVyskocil@Eaton.com>
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \brief Not yet documented file
 */
#ifndef SRC_WEB_SRC_DCI_IMPL_H
#define SRC_WEB_SRC_DCI_IMPL_H

// this file exists only to have syntax highlighting correct
// to be included in datacenter_indicators.ecpp

static const std::map<const std::string, const std::string> PARAM_TO_SRC = {
    {"power", "realpower.default"},
    {"avg_power_last_day", "realpower.default_arithmetic_mean_24h"},
    {"avg_power_last_week", "<zero>"},
    {"avg_power_last_month", "<zero>"},

    {"min_power_last_day", "<zero>"},
    {"min_power_last_week", "<zero>"},
    {"min_power_last_month", "<zero>"},

    {"max_power_last_day", "<zero>"},
    {"max_power_last_week", "<zero>"},
    {"max_power_last_month", "<zero>"},

    {"trend_power_last_day", R"(realpower.default\_arithmetic\_mean\_24h/realpower.default)"},
    {"trend_power_last_week", "<zero>"},
    {"trend_power_last_month", "<zero>"},

    {"temperature", "temperature.TH%"},
    {"avg_temperature_last_day", R"(temperature.TH%\_arithmetic\_mean\_24h%)"},
    {"avg_temperature_last_week", "<zero>"},
    {"avg_temperature_last_month", "<zero>"},

    {"min_temperature_last_day", "<zero>"},
    {"min_temperature_last_week", "<zero>"},
    {"min_temperature_last_month", "<zero>"},

    {"max_temperature_last_day", "<zero>"},
    {"max_temperature_last_week", "<zero>"},
    {"max_temperature_last_month", "<zero>"},

    {"trend_temperature_last_day", R"(temperature.TH%\_arithmetic\_mean\_24h%/temperature.TH%)"},
    {"trend_temperature_last_week", "<zero>"},
    {"trend_temperature_last_month", "<zero>"},

    {"humidity", "humidity.TH%"},
    {"avg_humidity_last_day", R"(humidity.TH%\_arithmetic\_mean\_24h%)"},
    {"avg_humidity_last_week", "<zero>"},
    {"avg_humidity_last_month", "<zero>"},

    {"min_humidity_last_day", "<zero>"},
    {"min_humidity_last_week", "<zero>"},
    {"min_humidity_last_month", "<zero>"},

    {"max_humidity_last_day", "<zero>"},
    {"max_humidity_last_week", "<zero>"},
    {"max_humidity_last_month", "<zero>"},

    {"trend_humidity_last_day", R"(humidity.TH%\_arithmetic\_mean\_24h%/humidity.TH%)"},
    {"trend_humidity_last_week", "<zero>"},
    {"trend_humidity_last_month", "<zero>"}
};

float
get_dc_percentage(
    tntdb::Connection& conn,
    const std::string& src,
    a_elmnt_id_t id,
    std::map<const std::string, double> &key_value)
{
    m_msrmnt_value_t value;
    m_msrmnt_scale_t scale;

    auto it = key_value.find(src);
    double val = 0.0f;
    if ( it == key_value.cend() )
    {
        int minutes_back = 10;
        if ( src.find("24h") != std::string::npos )
            minutes_back = 24*60;

        reply_t reply = persist::select_measurement_last_web_byTopicLike(conn, src, value, scale, minutes_back);
        if ( reply.rv == 0 )
            val = value * pow (10, scale);
        key_value.insert(std::make_pair(src, val));
    }
    else
        val = it->second;
    return val;
}

float
get_dc_measurement(
    tntdb::Connection& conn,
    const std::string& src,
    a_elmnt_id_t id,
    std::map<const std::string, double> &key_value,
    int minutes_back)
{
    m_msrmnt_value_t value;
    m_msrmnt_scale_t scale;

    auto it = key_value.find(src);
    double val = 0.0f;
    if ( it == key_value.cend() )
    {
        reply_t reply = persist::select_measurement_last_web_byElementId(conn, src, id, value, scale, minutes_back);
        if (reply.rv == 0)
            val = value * pow(10,scale);
        key_value.insert(std::make_pair(src, val));
    }
    else
        val = it->second;
    return val;
}



float
get_dc_trend(
    tntdb::Connection& conn,
    const std::string& src,
    a_elmnt_id_t id,
    std::map<const std::string, double> &key_value)
{
    if (src == "<zero>")
        return 0.0f;

    std::vector<std::string> items;
    cxxtools::split('/', src, std::back_inserter(items));
    if( items.size() != 2 ) return 0.0f;

    double value_actual = 0.0f;
    double value_average = 0.0f;
    auto it = key_value.find(items.at(1));
    if ( it == key_value.cend() )
    {
        // not in cache
        if ( items.at(1).find('%') != std::string::npos)
        {
            value_actual = get_dc_percentage(conn, items.at(1), id, key_value);
        }
        else
        {
            int minutes_back = 10;
            if ( items.at(1).find("24h") != std::string::npos )
                minutes_back = 24*60;
            value_actual = get_dc_measurement(conn, items.at(1), id, key_value, minutes_back);
        }
        key_value.insert(std::make_pair(src, value_actual));
    }
    else
    {
        // found in cache
        value_actual = it->second;
    }

    it = key_value.find(items.at(0));
    if ( it == key_value.cend() )
    {
        // not in cache
        if ( items.at(0).find('%') != std::string::npos)
        {
            value_average = get_dc_percentage(conn, items.at(0), id, key_value);
        }
        else
        {
            int minutes_back = 10;
            if ( items.at(0).find("24h") != std::string::npos )
                minutes_back = 24*60;
            value_average = get_dc_measurement(conn, items.at(0), id, key_value, minutes_back);
        }
        key_value.insert(std::make_pair(src, value_average));
    }
    else
    {
        // found in cache
        value_average = it->second;
    }

    double val = 0.0f;
    if ( value_average != 0 )
        val = round( (value_actual - value_average ) / ( value_average ) * 1000.0f ) / 10.0f ;
    key_value.insert(std::make_pair(src, val));
    return val;
}


float
get_dc_indicator(
    tntdb::Connection& conn,
    const std::string& key,
    a_elmnt_id_t id,
    std::map<const std::string, double> &key_value)
{
    const std::string& src = PARAM_TO_SRC.at(key); //XXX: operator[] does not work here!

    if (src == "<zero>")
    {
        key_value.insert(std::make_pair(key, 0.0f));
        return 0.0f;
    }

    std::cout << "key: " << key << "src: " << src << std::endl;
    if (key.substr(0,5) == "trend") {
        return get_dc_trend(conn, src, id, key_value);
    }
    if (src.find('%') != std::string::npos) {
        return get_dc_percentage(conn, src, id, key_value);
    }
    if (src.find("24h") != std::string::npos) {
        return get_dc_measurement(conn, src, id, key_value, 24*60);
    }
    return get_dc_measurement(conn, src, id, key_value, 10);
}

static bool
s_is_valid_param(const std::string& p)
{
    return PARAM_TO_SRC.count(p) != 0;
}


#endif // SRC_WEB_SRC_DCI_IMPL_H
