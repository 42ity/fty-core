/*
Copyright (C) 2016 Eaton

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/*! \file   multi_row.h
    \brief  manage multi rows insertion cache
    \author GeraldGuillaume <GeraldGuillaume@Eaton.com>
 */
#ifndef SRC_PERSIST_MULTI_ROW_H
#define SRC_PERSIST_MULTI_ROW_H

#include "dbhelpers.h"
#include <list>

#define MAX_ROW_DEFAULT   1000
#define MAX_DELAY_DEFAULT 10

#define EV_DBSTORE_MAX_ROW   "BIOS_DBSTORE_MAX_ROW"
#define EV_DBSTORE_MAX_DELAY "BIOS_DBSTORE_MAX_DELAY"

using namespace std;

namespace persist {

class MultiRowCache {
    public:
        MultiRowCache ();
        MultiRowCache ( const int32_t max_row,const int32_t max_delay_s )
        {
            _max_row=max_row;
            _max_delay_s=max_delay_s;
        }
        

        void push_back(
            int64_t time,
            m_msrmnt_value_t value,
            m_msrmnt_scale_t scale,
            m_msrmnt_tpc_id_t topic_id );

        /*
         * \brief check one of those conditions : 
         *  number of values > _max_row
         * or delay between first value and now > _max_delay_s
         */
        bool is_ready_for_insert();
        
        string get_insert_query();
        
        void clear(){_row_cache.clear();}
        
        uint32_t get_max_row(){return _max_row;}
        
        uint32_t get_max_delay(){return _max_delay_s;}
        

    private:
        list<string> _row_cache;
        uint32_t _max_delay_s; 
        uint32_t _max_row;
        
        long get_clock_ms();
        long _first_ms;
};

}
#endif // #define SRC_PERSIST_



