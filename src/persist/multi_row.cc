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
 * \file multi_row.cc
 * \author GeraldGuillaume <GeraldGuillaume@Eaton.com>
 * \brief \brief  manage multi rows insertion cache
 */
#include "log.h"
#include "multi_row.h"
#include <ctime>


namespace persist {
    
    MultiRowCache::MultiRowCache (){
        _max_row = MAX_ROW_DEFAULT;
        char *env_max_row = getenv (EV_DBSTORE_MAX_ROW);
        if( env_max_row!=NULL ){
            int max_row = atoi(env_max_row);
            if( max_row<=0 ) _max_row = MAX_ROW_DEFAULT;
            else _max_row = (uint32_t)max_row;
            log_info("use %s %d as max row insertion bulk limit",EV_DBSTORE_MAX_ROW,_max_row);
        }
        
        _max_delay_s = MAX_DELAY_DEFAULT;
        char *env_max_delay = getenv (EV_DBSTORE_MAX_DELAY);
        if( env_max_delay!=NULL ){
            int max_delay_s = atoi(env_max_delay);
            if( max_delay_s<=0 ) _max_delay_s = MAX_DELAY_DEFAULT;
            else _max_delay_s=(uint32_t)max_delay_s;
            log_info("use %s %ds as max delay before multi row insertion",EV_DBSTORE_MAX_DELAY,_max_delay_s);
        }
        
    }

void
    MultiRowCache::push_back(
        int64_t time,
        m_msrmnt_value_t value,
        m_msrmnt_scale_t scale,
        m_msrmnt_tpc_id_t topic_id )
{
        //multiple row insertion request
        char val[50];
        sprintf(val,"(%" PRIu64 ",%" PRIi32 ",%" PRIi16 ",%" PRIi16 ")",time,value,scale,topic_id );
        _row_cache.push_back(val);
        //check if it is the first one => if yes, memory the timestamp
        if(_row_cache.size()==1)
        {
            _first_ms = get_clock_ms();
        }
}

bool 
    MultiRowCache::is_ready_for_insert()
{
    //time to display stat ?
    long now_ms = get_clock_ms();
    long elapsed_periodic_ms = (now_ms - _first_ms);
    //every period seconds display current total row count and the trend over the last periodic_display second
    return (_row_cache.size()>=_max_row || elapsed_periodic_ms >= _max_delay_s * 1000 );
    
}
/* return INSERT query or empty string if no value in cache available*/
string
    MultiRowCache::get_insert_query()
{
    string query;
    if(_row_cache.size()==0)return query;
    query = "INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES ";
    for (list<string>::iterator value=_row_cache.begin(); value != _row_cache.end(); ){
         query += *value;
        if( ++value != _row_cache.end())query+=",";
    }
    query+=" ON DUPLICATE KEY UPDATE value=VALUES(value),scale=VALUES(scale) ";

    log_debug("query %s",query.c_str());
    return query;
}

        
long 
    MultiRowCache::get_clock_ms()
{
    struct timeval time;
    gettimeofday(&time, NULL); // Get Time
    return (time.tv_sec * 1000) + (time.tv_usec / 1000);
}       
    
}

