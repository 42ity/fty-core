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

#include "utils++.h"

// this file exists only to have syntax highlighting correct
// to be included in datacenter_indicators.ecpp

// ATTENTION!!! for trends:
//  first one should be average, second one is raw measurement
static const std::map<std::string, const std::string> PARAM_TO_SRC = {
    {"power", "realpower.default"},
    {"avg_power_last_day", "realpower.default_arithmetic_mean_24h"},
    {"avg_power_last_week", "<zero>"},
    {"avg_power_last_month", "<zero>"},

    {"min_power_last_day", "realpower.default_arithmetic_min_24h"},
    {"min_power_last_week", "<zero>"},
    {"min_power_last_month", "<zero>"},

    {"max_power_last_day", "realpower.default_arithmetic_max_24h"},
    {"max_power_last_week", "<zero>"},
    {"max_power_last_month", "<zero>"},

    {"trend_power_last_day", "realpower.default_arithmetic_mean_24h/realpower.default"},
    {"trend_power_last_week", "<zero>"},
    {"trend_power_last_month", "<zero>"},

    {"temperature", "temperature.TH%"},
    {"avg_temperature_last_day", R"(temperature.TH%\_arithmetic\_mean\_24h%)"},
    {"avg_temperature_last_week", "<zero>"},
    {"avg_temperature_last_month", "<zero>"},

    {"min_temperature_last_day", R"(temperature.TH%\_arithmetic\_min\_24h%)"},
    {"min_temperature_last_week", "<zero>"},
    {"min_temperature_last_month", "<zero>"},

    {"max_temperature_last_day", R"(temperature.TH%\_arithmetic\_max\_24h%)"},
    {"max_temperature_last_week", "<zero>"},
    {"max_temperature_last_month", "<zero>"},

    {"trend_temperature_last_day", R"(temperature.TH%\_arithmetic\_mean\_24h%/temperature.TH%)"},
    {"trend_temperature_last_week", "<zero>"},
    {"trend_temperature_last_month", "<zero>"},

    {"humidity", "humidity.TH%"},
    {"avg_humidity_last_day", R"(humidity.TH%\_arithmetic\_mean\_24h%)"},
    {"avg_humidity_last_week", "<zero>"},
    {"avg_humidity_last_month", "<zero>"},

    {"min_humidity_last_day", R"(humidity.TH%\_arithmetic\_min\_24h%)"},
    {"min_humidity_last_week", "<zero>"},
    {"min_humidity_last_month", "<zero>"},

    {"max_humidity_last_day", R"(humidity.TH%\_arithmetic\_max\_24h%)"},
    {"max_humidity_last_week", "<zero>"},
    {"max_humidity_last_month", "<zero>"},

    {"trend_humidity_last_day", R"(humidity.TH%\_arithmetic\_mean\_24h%/humidity.TH%)"},
    {"trend_humidity_last_week", "<zero>"},
    {"trend_humidity_last_month", "<zero>"}
};


double
    get_dc_raw(
        tntdb::Connection& conn,
        const std::string& src,
        a_elmnt_id_t id,
        std::map<const std::string, double> &cache)
{
    int step = 0;
    if ( src.find("24h") != std::string::npos )
        step = 24*60*60;
    else if ( src.find("15m") != std::string::npos )
        step = 60*15;

    double value = 0;
    if ( step != 0 )
    {
        // here we are, if we are looking for some aggregated data
        bool fuzzy = false;
        // determine, if we need to use like or not
        if ( src.find('%') != std::string::npos )
            fuzzy = true;

        int rv = persist::select_last_aggregated_by_element_by_src_by_step(conn, id, src, step, value, fuzzy);
        if ( rv != 0 )
        {
            log_debug ("not computed, take 0");
        }
    }
    else
    {
        // not an aggregate -> current -> 10 minutes back
        m_msrmnt_value_t val = 0;
        m_msrmnt_scale_t scale = 0;
        reply_t reply = persist::select_measurement_last_web_byElementId (conn, src, id, val, scale, 10);
        if ( reply.rv != 0 )
        {
            log_debug ("not computed, take 0");
        }
        else
        {
            value = val * pow(10, scale);
        }
    }
    cache.insert(std::make_pair(src, value));
    return value;
}


double
get_dc_trend(
    tntdb::Connection& conn,
    const std::string& src,
    a_elmnt_id_t id,
    std::map<const std::string, double> &cache)
{
    if (src == "<zero>")
        return 0.0f;

    std::vector<std::string> items;
    cxxtools::split('/', src, std::back_inserter(items));
    if( items.size() != 2 ) return 0.0f;

    double value_actual = 0.0f;
    double value_average = 0.0f;
    auto it = cache.find(items.at(1));
    if ( it != cache.cend() )
    {
        value_actual = it->second;
    }
    else
    {
        value_actual = get_dc_raw (conn, items.at(1), id, cache);
        cache.insert(std::make_pair(src, value_actual));
    }

    it = cache.find(items.at(0));
    if ( it != cache.cend() )
    {
        value_average = it->second;
    }
    else
    {
        value_average = get_dc_raw(conn, items.at(0), id, cache);
        cache.insert(std::make_pair(src, value_average));
    }

    double val = 0.0f;
    if ( value_average != 0 )
        val = round( (value_actual - value_average ) / ( value_average ) * 1000.0f ) / 10.0f ;
    cache.insert(std::make_pair(src, val));
    return val;
}


double
get_dc_indicator(
    tntdb::Connection& conn,
    const std::string& key,
    a_elmnt_id_t id,
    std::map<const std::string, double> &cache)
{
    // Assumption: key is ok.
    const std::string src = PARAM_TO_SRC.at(key); //XXX: operator[] does not work here!

    auto it = cache.find(src);
    if ( it != cache.cend() )
        return it->second;

    if (src == "<zero>")
    {
        cache.insert(std::make_pair(key, 0.0f));
        return 0.0f;
    }

    if (key.substr(0,5) == "trend") {
        return get_dc_trend(conn, src, id, cache);
    }
    return get_dc_raw(conn, src, id, cache);
}

static bool
s_is_valid_param(const std::string& p)
{
    return PARAM_TO_SRC.count(p) != 0;
}

std::string 
s_get_valid_param () {
    return utils::join_keys_map (PARAM_TO_SRC, ", ");
}


#endif // SRC_WEB_SRC_DCI_IMPL_H
