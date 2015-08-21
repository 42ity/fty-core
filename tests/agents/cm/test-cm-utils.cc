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
 * \file test-cm-utils.cc
 * \author Karol Hrdina <KarolHrdina@Eaton.com>
 * \author Tomas Halman <TomasHalman@Eaton.com>
 * \brief Not yet documented file
 */
#include <catch.hpp>
#include <stdio.h>

#include "defs.h"
#include "str_defs.h"
#include "utils.h"
#include "cm-utils.h"

namespace cm = computation;

TEST_CASE ("cm::web::sample_weight","[agent-cm][computation][average][sample_weight]")
{
    SECTION ("bad args") {
        // begin > end
        CHECK ( cm::web::sample_weight (2, 1) == -1 );
        CHECK ( cm::web::sample_weight (1, 0) == -1 );
        CHECK ( cm::web::sample_weight (0, -1) == -1 );
        CHECK ( cm::web::sample_weight (1426204882, 1426204881) == -1 );
        CHECK ( cm::web::sample_weight (-4, -5) == -1 );
        CHECK ( cm::web::sample_weight (1433753329, 1426634118) == -1 );
        CHECK ( cm::web::sample_weight (1433753329, -5) == -1 );
        CHECK ( cm::web::sample_weight (-90549882, -157658382) == -1 );
        // begin = end
        CHECK ( cm::web::sample_weight (1430173900, 1430173900) == -1 );
        CHECK ( cm::web::sample_weight (0, 0) == -1 );
        CHECK ( cm::web::sample_weight (-1, -1) == -1 );
        CHECK ( cm::web::sample_weight (-90549882, -90549882) == -1 );
    }
    SECTION ("positive values") {
        int64_t ts = 14134242829;
        CHECK ( cm::web::sample_weight (ts, ts + AGENT_NUT_REPEAT_INTERVAL_SEC) == AGENT_NUT_REPEAT_INTERVAL_SEC );
        CHECK ( cm::web::sample_weight (ts, ts + AGENT_NUT_REPEAT_INTERVAL_SEC - 1) == AGENT_NUT_REPEAT_INTERVAL_SEC - 1 );
        CHECK ( cm::web::sample_weight (ts, ts + AGENT_NUT_REPEAT_INTERVAL_SEC - 2) == AGENT_NUT_REPEAT_INTERVAL_SEC - 2 );
        CHECK ( cm::web::sample_weight (ts, ts + AGENT_NUT_REPEAT_INTERVAL_SEC/2) == AGENT_NUT_REPEAT_INTERVAL_SEC/2 );
        CHECK ( cm::web::sample_weight (ts, ts + 1) == 1 );
        CHECK ( cm::web::sample_weight (ts, ts + 2) == 2 );
        CHECK ( cm::web::sample_weight (ts, ts + 10) == 10 );
        CHECK ( cm::web::sample_weight (ts, ts + 50) == 50 );
        CHECK ( cm::web::sample_weight (ts, ts + AGENT_NUT_REPEAT_INTERVAL_SEC + 1) == 1 );
        CHECK ( cm::web::sample_weight (ts, ts + AGENT_NUT_REPEAT_INTERVAL_SEC + 2) == 1 );
        CHECK ( cm::web::sample_weight (ts, ts + AGENT_NUT_REPEAT_INTERVAL_SEC + 3) == 1 );
        CHECK ( cm::web::sample_weight (ts, ts + AGENT_NUT_REPEAT_INTERVAL_SEC + 10) == 1 );
        CHECK ( cm::web::sample_weight (ts, ts + 2*AGENT_NUT_REPEAT_INTERVAL_SEC) == 1 );
    }
    SECTION ("negative values") {   
        int64_t ts = -2428397;
        CHECK ( cm::web::sample_weight (ts, ts + AGENT_NUT_REPEAT_INTERVAL_SEC) == AGENT_NUT_REPEAT_INTERVAL_SEC );
        CHECK ( cm::web::sample_weight (ts, ts + AGENT_NUT_REPEAT_INTERVAL_SEC - 1) == AGENT_NUT_REPEAT_INTERVAL_SEC - 1 );
        CHECK ( cm::web::sample_weight (ts, ts + AGENT_NUT_REPEAT_INTERVAL_SEC - 2) == AGENT_NUT_REPEAT_INTERVAL_SEC - 2 );
        CHECK ( cm::web::sample_weight (ts, ts + AGENT_NUT_REPEAT_INTERVAL_SEC/2) == AGENT_NUT_REPEAT_INTERVAL_SEC/2 );
        CHECK ( cm::web::sample_weight (ts, ts + 1) == 1 );
        CHECK ( cm::web::sample_weight (ts, ts + 2) == 2 );
        CHECK ( cm::web::sample_weight (ts, ts + 10) == 10 );
        CHECK ( cm::web::sample_weight (ts, ts + 50) == 50 );
        CHECK ( cm::web::sample_weight (ts, ts + AGENT_NUT_REPEAT_INTERVAL_SEC + 1) == 1 );
        CHECK ( cm::web::sample_weight (ts, ts + AGENT_NUT_REPEAT_INTERVAL_SEC + 2) == 1 );
        CHECK ( cm::web::sample_weight (ts, ts + AGENT_NUT_REPEAT_INTERVAL_SEC + 3) == 1 );
        CHECK ( cm::web::sample_weight (ts, ts + AGENT_NUT_REPEAT_INTERVAL_SEC + 10) == 1 );
        CHECK ( cm::web::sample_weight (ts, ts + 2*AGENT_NUT_REPEAT_INTERVAL_SEC) == 1 );

    }
    SECTION ("misc") {
        CHECK ( cm::web::sample_weight (0, 1) == 1 );
        CHECK ( cm::web::sample_weight (-2, -1) == 1 );
        CHECK ( cm::web::sample_weight (-50500, -50425) == 75 );
        CHECK ( cm::web::sample_weight (1433110045, 1433110090) == 45 );
    }
}

TEST_CASE ("cm::web::solve_left_margin","[agent-cm][computation][average][solve_left_margin]")
{

    SECTION ("bad args") {
        std::map <int64_t, double> samples;
        CHECK (samples.empty ());
        cm::web::solve_left_margin (samples, 1426204882);
        CHECK (samples.empty ());

        samples.emplace (std::make_pair (1430173900, 12.4)); // 2015-04-28 00:31:40
        CHECK (samples.size () == 1);
        cm::web::solve_left_margin (samples, -1);
        CHECK (samples.size () == 1);
    }

    SECTION ("correct 1") {
        // start: 2015-04-27 22:31:39 (1430173899)
        // extended_start                start
        //       <                         <
        //                                      x
 
        // one second after
        std::map <int64_t, double> samples;
        samples.emplace (std::make_pair (1430173900, 12.4)); // 2015-04-27 22:31:40

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 1 );
        CHECK ( samples.begin ()->first == 1430173900 );
        CHECK ( samples.begin ()->second == 12.4 );

        // farther
        samples.clear ();
        samples.emplace (std::make_pair (1430174292, 12.4)); // 2015-04-27 22:38:12

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 1 );
        CHECK ( samples.begin ()->first == 1430174292 );
        CHECK ( samples.begin ()->second == 12.4 );

        // more points
        samples.clear ();
        samples.emplace (std::make_pair (1430173899 + 1 , 12.4));
        samples.emplace (std::make_pair (1430173899 + 10 , 12.5));
        samples.emplace (std::make_pair (1430173899 + 10 + AGENT_NUT_REPEAT_INTERVAL_SEC , 12.6));

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 3 );
        auto it = samples.begin ();
        CHECK ( it->first == 1430173899 + 1 );
        CHECK ( it->second == 12.4 );
        ++it;
        CHECK ( it->first == 1430173899 + 10 );
        CHECK ( it->second == 12.5 );
        ++it;
        CHECK ( it->first == 1430173899 + 10 + AGENT_NUT_REPEAT_INTERVAL_SEC );
        CHECK ( it->second == 12.6 );
    }
    SECTION ("correct 2") {
        // start: 2015-04-27 22:31:39 (1430173899)
        // extended_start                start
        //       <                         <
        //                                 x
        std::map <int64_t, double> samples;
        samples.emplace (std::make_pair (1430173899, 12.4)); // 2015-04-27 22:31:39

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 1 );
        CHECK ( samples.begin ()->first == 1430173899 );
        CHECK ( samples.begin ()->second == 12.4 );

        // more points
        samples.clear ();
        samples.emplace (std::make_pair (1430173899, 12.4));
        samples.emplace (std::make_pair (1430173899 + 10 , 12.5));
        samples.emplace (std::make_pair (1430173899 + 10 + AGENT_NUT_REPEAT_INTERVAL_SEC , 12.6));

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 3 );
        auto it = samples.begin ();
        CHECK ( it->first == 1430173899 );
        CHECK ( it->second == 12.4 );
        ++it;
        CHECK ( it->first == 1430173899 + 10 );
        CHECK ( it->second == 12.5 );
        ++it;
        CHECK ( it->first == 1430173899 + 10 + AGENT_NUT_REPEAT_INTERVAL_SEC );
        CHECK ( it->second == 12.6 );
    }
    SECTION ("correct 3") {
        // start: 2015-04-27 22:31:39 (1430173899)
        // extended_start                start
        //       <                         <
        //                             x    

        // one second before 
        std::map <int64_t, double> samples;
        samples.emplace (std::make_pair (1430173898, 12.4)); // 2015-04-27 22:31:38

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 0 );

        // somewhere in between
        samples.clear ();
        samples.emplace (std::make_pair (1430173899 - (AGENT_NUT_REPEAT_INTERVAL_SEC/2), 12.4));

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 0 );

        // one second after extended start
        samples.clear ();
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC + 1, 12.4));

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 0 );
        CHECK ( samples.empty () );
 
        // more points
        samples.clear ();
        samples.emplace (std::make_pair (1430173899 - 1, 12.4));
        samples.emplace (std::make_pair (1430173899 - 10 , 12.5));
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC/2 , 12.6));

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 0 );
    }
    SECTION ("correct 4") {
        // start: 2015-04-27 22:31:39 (1430173899)
        // extended_start                start
        //       <                         <
        //       x                          

        std::map <int64_t, double> samples;
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC, 12.4)); // 2015-04-27 22:26:39

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 0 );
    }
    SECTION ("correct 5") {
        // start: 2015-04-27 22:31:39 (1430173899)
        // extended_start                start
        //       <                         <
        //   x                                

        // one second before extended start
        std::map <int64_t, double> samples;
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC - 1, 12.4)); // 2015-04-27 22:26:39

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 0 );

        // farther 
        samples.clear ();
        samples.emplace (std::make_pair (1430173899 - 2*AGENT_NUT_REPEAT_INTERVAL_SEC, 12.4));

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 0 );

        // more points
        samples.clear ();
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC - 1, 12.4));
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC - 10 , 12.5));
        samples.emplace (std::make_pair (1430173899 - 2*AGENT_NUT_REPEAT_INTERVAL_SEC, 12.6));

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 0 );

        samples.clear ();
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC, 12.4));
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC - 10 , 12.5));
        samples.emplace (std::make_pair (1430173899 - 2*AGENT_NUT_REPEAT_INTERVAL_SEC, 12.6));

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 0 );

    }
    SECTION ("correct 6") {
        // start: 2015-04-27 22:31:39 (1430173899)
        // extended_start                start
        //       <                         <
        //                                 x     p

        // p one second after start
        std::map <int64_t, double> samples;
        samples.emplace (std::make_pair (1430173899, 15.1));
        samples.emplace (std::make_pair (1430173900, 12.4)); // p := 2015-04-27 22:31:40 

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 2 );
        CHECK ( samples.begin ()->first == 1430173899 );
        CHECK ( samples.begin ()->second == 15.1 );

        // p farther away
        samples.clear ();
        samples.emplace (std::make_pair (1430173899, 15.2));
        samples.emplace (std::make_pair (1430174320, 12.4)); // p := 2015-04-27 22:38:40 

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 2 );
        CHECK ( samples.begin ()->first == 1430173899 );
        CHECK ( samples.begin ()->second == 15.2 );
    }
    SECTION ("correct 7") {
        // start: 2015-04-27 22:31:39 (1430173899)
        // extended_start                start
        //       <                         <
        //                x                      p 

        // p one second after start
        std::map <int64_t, double> samples;
        // x exactly NUT_REPEAT_INTERVAL from p
        samples.emplace (std::make_pair (1430173900 - AGENT_NUT_REPEAT_INTERVAL_SEC, 15.1));
        samples.emplace (std::make_pair (1430173900, 12.4)); // p := 2015-04-27 22:31:40 

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 2 );
        auto it = samples.begin ();
        CHECK ( it->first == 1430173899 );
        CHECK ( it->second == 15.1 );
        ++it;
        CHECK ( it->first == 1430173900 );
        CHECK ( it->second == 12.4 );
        
        // x somewhere in between (p - NUT_REPEAT_INTERVAL, start) 
        samples.clear ();
        samples.emplace (std::make_pair (1430173900 - (AGENT_NUT_REPEAT_INTERVAL_SEC/2), 15.2));
        samples.emplace (std::make_pair (1430173900, 12.4)); // p :=  2015-04-27 22:31:40 

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 2 );
        it = samples.begin ();
        CHECK ( it->first == 1430173899 );
        CHECK ( it->second == 15.2 );
        ++it;
        CHECK ( it->first == 1430173900);
        CHECK ( it->second == 12.4 );

        // more points
        samples.clear ();
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC/2, 12.5));
        samples.emplace (std::make_pair (1430173899 - (AGENT_NUT_REPEAT_INTERVAL_SEC/2) + 10 , 12.6));
        samples.emplace (std::make_pair (1430173899 - (AGENT_NUT_REPEAT_INTERVAL_SEC/2) - 10, 12.7));
        samples.emplace (std::make_pair (1430173900, 12.4)); // p :=  2015-04-27 22:31:40 
        
        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 2 );
        it = samples.begin ();
        CHECK ( it->first == 1430173899 ); // 2015-04-27 22:31:39
        CHECK ( it->second == 12.6 );
        ++it;
        CHECK ( it->first == 1430173900);
        CHECK ( it->second == 12.4 );

        // p farther away
        // x exactly NUT_REPEAT_INTERVAL from p
        samples.clear ();
        samples.emplace (std::make_pair (1430173995 - AGENT_NUT_REPEAT_INTERVAL_SEC, 15.1));
        samples.emplace (std::make_pair (1430173995, 12.4)); // p := 2015-04-27 22:33:15 

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 2 );
        it = samples.begin ();
        CHECK ( it->first == 1430173899 ); // start
        CHECK ( it->second == 15.1 );
        ++it;
        CHECK ( it->first == 1430173995 ); // start
        CHECK ( it->second == 12.4 );

        // x somewhere in between (p - NUT_REPEAT_INTERVAL, start) 
        samples.clear ();
        samples.emplace (std::make_pair (1430173995 - (AGENT_NUT_REPEAT_INTERVAL_SEC/2), 15.2));
        samples.emplace (std::make_pair (1430173995, 12.4)); // p := 2015-04-27 22:33:15 

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 2 );
        it = samples.begin ();
        CHECK ( it->first == 1430173899 ); // 2015-04-27 22:31:39
        CHECK ( it->second == 15.2 );
        ++it;
        CHECK ( it->first == 1430173995 ); // start
        CHECK ( it->second == 12.4 );

        // x one second after exteded start
        samples.clear ();
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC + 1, 15.3));
        samples.emplace (std::make_pair (1430173995, 12.4)); // p :=  2015-04-27 22:31:40 

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 1 );
        CHECK ( samples.begin ()->first == 1430173995 ); // 2015-04-27 22:31:39
        CHECK ( samples.begin ()->second == 12.4 );

    }   
    SECTION ("correct 8") {
        // start: 2015-04-27 22:31:39 (1430173899)
        // extended_start                start
        //       <                         <
        //       x                               p 

        // p one second after start
        std::map <int64_t, double> samples;
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC, 15.1));
        samples.emplace (std::make_pair (1430173900, 12.4)); // p := 2015-04-27 22:31:40 

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 1 );
        CHECK ( samples.begin ()->first == 1430173900 ); // start
        CHECK ( samples.begin ()->second == 12.4 );

        // p farther away
        samples.clear ();
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC, 15.2));
        samples.emplace (std::make_pair (1430173995, 12.4)); // p := 2015-04-27 22:33:15 

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 1 );
        CHECK ( samples.begin ()->first == 1430173995 ); // start
        CHECK ( samples.begin ()->second == 12.4 );


    }
    SECTION ("correct 9") {
        // start: 2015-04-27 22:31:39 (1430173899)
        // extended_start                start
        //       <                         <
        //   x                                   p 

        // p one second after start
        std::map <int64_t, double> samples;
        // x one second before extended 
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC - 1, 15.1));
        samples.emplace (std::make_pair (1430173900, 12.4)); // p := 2015-04-27 22:31:40 

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 1 );
        CHECK ( samples.begin ()->first == 1430173900 );
        CHECK ( samples.begin ()->second == 12.4 );

        // x farther
        samples.clear ();
        samples.emplace (std::make_pair (1430173900 - 2*AGENT_NUT_REPEAT_INTERVAL_SEC, 15.2));
        samples.emplace (std::make_pair (1430173900, 12.4)); // p :=  2015-04-27 22:31:40 

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 1 );
        CHECK ( samples.begin ()->first == 1430173900 ); // 2015-04-27 22:31:39
        CHECK ( samples.begin ()->second == 12.4 );

        // p farther away
        // x one second before extended 
        samples.clear ();
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC - 1, 15.1));
        samples.emplace (std::make_pair (1430173995, 12.4)); // p := 2015-04-27 22:33:15 

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 1 );
        CHECK ( samples.begin ()->first == 1430173995 ); 
        CHECK ( samples.begin ()->second == 12.4 );

        // x farther 
        samples.clear ();
        samples.emplace (std::make_pair (1430173900 - 2*AGENT_NUT_REPEAT_INTERVAL_SEC, 15.2));
        samples.emplace (std::make_pair (1430173995, 12.4)); // p := 2015-04-27 22:33:15 

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 1 );
        CHECK ( samples.begin ()->first == 1430173995 );
        CHECK ( samples.begin ()->second == 12.4 );

        // more points
        samples.clear ();
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC - 1, 12.5));
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC - 10, 12.6));
        samples.emplace (std::make_pair (1430173899 - 2*AGENT_NUT_REPEAT_INTERVAL_SEC, 12.7));
        samples.emplace (std::make_pair (1430173995, 12.4)); // p :=  2015-04-27 22:33:15
        
        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 1 );
        auto it = samples.begin ();
        CHECK ( it->first == 1430173995 );
        CHECK ( it->second == 12.4 );

    }
    SECTION ("correct 10") {
        // start: 2015-04-27 22:31:39 (1430173899)
        // extended_start                start
        //       <                         <
        //                  x              x       

        // somewhere in between
        std::map <int64_t, double> samples;
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC/2, 15.1));
        samples.emplace (std::make_pair (1430173899, 12.4));

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 1 );
        CHECK ( samples.begin ()->first == 1430173899 );
        CHECK ( samples.begin ()->second == 12.4 );

        // one second after extended start
        samples.clear ();
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC + 1, 15.2));
        samples.emplace (std::make_pair (1430173899, 12.4));

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 1 );
        CHECK ( samples.begin ()->first == 1430173899 );
        CHECK ( samples.begin ()->second == 12.4 );

        // more points
        samples.clear ();
        samples.emplace (std::make_pair (1430173899, 12.4));
        samples.emplace (std::make_pair (1430173899 - (AGENT_NUT_REPEAT_INTERVAL_SEC/2) - 10, 12.5));
        samples.emplace (std::make_pair (1430173899 - (AGENT_NUT_REPEAT_INTERVAL_SEC/2), 12.6));
        samples.emplace (std::make_pair (1430173899 - (AGENT_NUT_REPEAT_INTERVAL_SEC/2) + 10, 12.7));

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 1 );
        CHECK ( samples.begin ()->first == 1430173899 );
        CHECK ( samples.begin ()->second == 12.4 );
 
    }
    SECTION ("correct 11") {
        // start: 2015-04-27 22:31:39 (1430173899)
        // extended_start                start
        //       <                         <
        //       x                         x

        std::map <int64_t, double> samples;
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC, 15.1));
        samples.emplace (std::make_pair (1430173899, 12.4));

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 1 );
        CHECK ( samples.begin ()->first == 1430173899 );
        CHECK ( samples.begin ()->second == 12.4 );
    }
    SECTION ("correct 12") {
        // start: 2015-04-27 22:31:39 (1430173899)
        // extended_start                start
        //       <                         <
        //   x                             x

        std::map <int64_t, double> samples;
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC - 1, 15.1));
        samples.emplace (std::make_pair (1430173899, 12.4));

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 1 );
        CHECK ( samples.begin ()->first == 1430173899 );
        CHECK ( samples.begin ()->second == 12.4 );

        samples.clear ();
        samples.emplace (std::make_pair (1430173899 - 2*AGENT_NUT_REPEAT_INTERVAL_SEC, 15.2));
        samples.emplace (std::make_pair (1430173899, 12.4));

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 1 );
        CHECK ( samples.begin ()->first == 1430173899 );
        CHECK ( samples.begin ()->second == 12.4 );

        // more points
        samples.clear ();
        samples.emplace (std::make_pair (1430173899, 12.4));
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC - 1, 12.5));
        samples.emplace (std::make_pair (1430173899 - 2*AGENT_NUT_REPEAT_INTERVAL_SEC, 12.6));
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC - 10, 12.7));

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 1 );
        CHECK ( samples.begin ()->first == 1430173899 );
        CHECK ( samples.begin ()->second == 12.4 );


    }   
    SECTION ("correct 13") {
        // start: 2015-04-27 22:31:39 (1430173899)
        // extended_start                start
        //       <                         <
        //   x   x                       
        std::map <int64_t, double> samples;
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC, 15.1));
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC - 1, 15.2));

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 0 );

        samples.clear ();
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC, 15.1));
        samples.emplace (std::make_pair (1430173899 - 2*AGENT_NUT_REPEAT_INTERVAL_SEC, 15.2));

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 0 );

        // more points
        samples.clear ();
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC, 12.5));
        samples.emplace (std::make_pair (1430173899 - 2*AGENT_NUT_REPEAT_INTERVAL_SEC, 12.6));
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC - 10, 12.7));
        samples.emplace (std::make_pair (1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC - 1, 12.8));

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 0 );
    }
    SECTION ("correct 14") {
        // start: 2015-04-27 22:31:39 (1430173899)
        // extended_start                start
        //       <                         <
    } 
    SECTION ("correct 15") {
        // start: 2015-04-27 22:31:39 (1430173899)
        // extended_start                start
        //       <                         <
        //                x                x    x

        std::map <int64_t, double> samples;
        
        samples.emplace (std::make_pair (1430173899 - (AGENT_NUT_REPEAT_INTERVAL_SEC/2), 12.1));
        samples.emplace (std::make_pair (1430173899, 12.4));
        samples.emplace (std::make_pair (1430173900, 15.1));
        samples.emplace (std::make_pair (1430174159, 15.2)); // 22:35:59
        samples.emplace (std::make_pair (1430173899 + 2*AGENT_NUT_REPEAT_INTERVAL_SEC, 15.3));

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 4 );
        auto it = samples.begin ();
        CHECK ( it->first == 1430173899 );
        CHECK ( it->second == 12.4 );
        ++it;
        CHECK ( it->first == 1430173900 );
        CHECK ( it->second == 15.1 );
        ++it;
        CHECK ( it->first == 1430174159 );
        CHECK ( it->second == 15.2 );
        ++it;
        CHECK ( it->first == 1430173899 + 2*AGENT_NUT_REPEAT_INTERVAL_SEC );
        CHECK ( it->second == 15.3 );

        samples.clear ();
        samples.emplace (std::make_pair (1430173899 - (AGENT_NUT_REPEAT_INTERVAL_SEC/2), 12.1));
        samples.emplace (std::make_pair (1430173899 - (AGENT_NUT_REPEAT_INTERVAL_SEC/2) + 50, 12.2));
        samples.emplace (std::make_pair (1430173899, 12.4));
        samples.emplace (std::make_pair (1430173899 + AGENT_NUT_REPEAT_INTERVAL_SEC/2, 15.1));

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC); 
        CHECK ( samples.size () == 2 );
        it = samples.begin ();
        CHECK ( it->first == 1430173899 );
        CHECK ( it->second == 12.4 );
        ++it;
        CHECK ( it->first == 1430173899 + AGENT_NUT_REPEAT_INTERVAL_SEC/2 );
        CHECK ( it->second == 15.1 );
    }
    SECTION ("correct 16") {
        // start:  2015-04-27 22:35:06 (1430174106)
        //  extended_start                start
        //        <                         <
        //   x       x     x      x
        std::map <int64_t, double> samples;
        samples.emplace (std::make_pair (1430174106 - (2*AGENT_NUT_REPEAT_INTERVAL_SEC), 1.13));
        samples.emplace (std::make_pair (1430174106 - 50, 1.17));
        samples.emplace (std::make_pair (1430174106 - 42, 1.18));
        samples.emplace (std::make_pair (1430174106 - 39, 1.19));
        samples.emplace (std::make_pair (1430174106 - 10, 1.20));
        samples.emplace (std::make_pair (1430174106 - 1, 1.21));

        cm::web::solve_left_margin (samples, 1430174106 - AGENT_NUT_REPEAT_INTERVAL_SEC);
        CHECK ( samples.size () == 0 );

        // start: 2015-04-27 22:31:39 (1430173899)
        // extended_start                start
        //       <                         <
        //   x       x     x      x
        
        samples.emplace (std::make_pair (1430173899 - (3*AGENT_NUT_REPEAT_INTERVAL_SEC), 11.9));
        samples.emplace (std::make_pair (1430173899 - (2*AGENT_NUT_REPEAT_INTERVAL_SEC), 11.9));
        samples.emplace (std::make_pair (1430173899 - (AGENT_NUT_REPEAT_INTERVAL_SEC/2), 12.1));
        samples.emplace (std::make_pair (1430173899 - (AGENT_NUT_REPEAT_INTERVAL_SEC/2) + 10, 12.2));
        samples.emplace (std::make_pair (1430173899 - (AGENT_NUT_REPEAT_INTERVAL_SEC/2) - 10, 12.3));
        samples.emplace (std::make_pair (1430173899 - 10, 12.4));

        cm::web::solve_left_margin (samples, 1430173899 - AGENT_NUT_REPEAT_INTERVAL_SEC);
        CHECK ( samples.size () == 0 );

    }
    SECTION ("correct 17") {
        //  start: 2015-04-28  00:35:05  (1430181305)
        //  extended_start                start
        //        <                         <
        //   x    x        x    x           x  x
        std::map <int64_t, double> samples;
        samples.emplace (std::make_pair (1430181305 - AGENT_NUT_REPEAT_INTERVAL_SEC - 175, 1.13));
        samples.emplace (std::make_pair (1430181305 - AGENT_NUT_REPEAT_INTERVAL_SEC - 1, -1.14));
        samples.emplace (std::make_pair (1430181305 - AGENT_NUT_REPEAT_INTERVAL_SEC, 12.4)); 
        samples.emplace (std::make_pair (1430181305 - AGENT_NUT_REPEAT_INTERVAL_SEC/2, 10.21)); 
        samples.emplace (std::make_pair (1430181305 - AGENT_NUT_REPEAT_INTERVAL_SEC + 50, 119.03)); 
        samples.emplace (std::make_pair (1430181305 - AGENT_NUT_REPEAT_INTERVAL_SEC - 1, 3.0482)); 
        samples.emplace (std::make_pair (1430181305, 7.02));
        samples.emplace (std::make_pair (1430181305 + AGENT_NUT_REPEAT_INTERVAL_SEC/2, 17.32));

        cm::web::solve_left_margin (samples, 1430181305 - AGENT_NUT_REPEAT_INTERVAL_SEC); // 2015-04-28 00:35:05
        CHECK ( samples.size () == 2 );
        auto it = samples.begin ();
        CHECK ( it->first == 1430181305 );
        CHECK ( it->second ==  7.02 );
        ++it;
        CHECK ( it->first == 1430181305 + AGENT_NUT_REPEAT_INTERVAL_SEC/2 );
        CHECK ( it->second ==  17.32 );
    }
    SECTION ("correct 18") {
        //  start: 2015-04-28  00:35:05  (1430181305)
        //  extended_start                start
        //        <                         <
        //   x    x        x    x      x        x
        std::map <int64_t, double> samples;
        samples.emplace (std::make_pair (1430181305 - AGENT_NUT_REPEAT_INTERVAL_SEC - 100, 1.13));
        samples.emplace (std::make_pair (1430181305 - AGENT_NUT_REPEAT_INTERVAL_SEC, 12.4));
        samples.emplace (std::make_pair (1430181305 - AGENT_NUT_REPEAT_INTERVAL_SEC + 65, 10.21));
        samples.emplace (std::make_pair (1430181305 - AGENT_NUT_REPEAT_INTERVAL_SEC + 128, 119.03));
        samples.emplace (std::make_pair (1430181305 - AGENT_NUT_REPEAT_INTERVAL_SEC/2 + 10, 12.12));
        samples.emplace (std::make_pair (1430181305 - AGENT_NUT_REPEAT_INTERVAL_SEC/10, 3.0482));
        samples.emplace (std::make_pair (1430181305 + AGENT_NUT_REPEAT_INTERVAL_SEC/10, 7.02));
        samples.emplace (std::make_pair (1430181305 + AGENT_NUT_REPEAT_INTERVAL_SEC, 17.1)); // 2015-04-28 00:40:22

        cm::web::solve_left_margin (samples, 1430181305 - AGENT_NUT_REPEAT_INTERVAL_SEC );
        CHECK ( samples.size () == 3 );
        auto it = samples.begin ();
        CHECK ( it->first == 1430181305 );
        CHECK ( it->second ==  3.0482 );
        ++it;
        CHECK ( it->first == 1430181305 +  AGENT_NUT_REPEAT_INTERVAL_SEC/10 );
        CHECK ( it->second ==  7.02 );       
        ++it;
        CHECK ( it->first == 1430181305 + AGENT_NUT_REPEAT_INTERVAL_SEC );
        CHECK ( it->second ==  17.1 );       
    }
    SECTION ("correct 19") {
        //  start: 2015-04-28  00:35:05  (1430181305)
        //  extended_start                start
        //        <                         <
        //   x            x    x      x        x
        std::map <int64_t, double> samples;
        samples.emplace (std::make_pair (1430181305 - 2*AGENT_NUT_REPEAT_INTERVAL_SEC, 1.13));
        samples.emplace (std::make_pair (1430181305 - AGENT_NUT_REPEAT_INTERVAL_SEC/2 - 10, 10.21));
        samples.emplace (std::make_pair (1430181305 - AGENT_NUT_REPEAT_INTERVAL_SEC/2, 12.12)); 
        samples.emplace (std::make_pair (1430181305 - AGENT_NUT_REPEAT_INTERVAL_SEC/2 + 125, 119.03));
        samples.emplace (std::make_pair (1430181305 + 35, 7.02));
        samples.emplace (std::make_pair (1430181305 + 2*AGENT_NUT_REPEAT_INTERVAL_SEC, 28.7));

        cm::web::solve_left_margin (samples, 1430181305 - AGENT_NUT_REPEAT_INTERVAL_SEC);
        CHECK ( samples.size () == 3 );
        auto it = samples.begin ();
        CHECK ( it->first == 1430181305 );
        CHECK ( it->second ==  119.03 );
        ++it;
        CHECK ( it->first == 1430181305 + 35 );
        CHECK ( it->second ==  7.02 );       
        ++it;
        CHECK ( it->first == 1430181305 + 2*AGENT_NUT_REPEAT_INTERVAL_SEC );
        CHECK ( it->second ==  28.7 );       

        samples.clear ();
        samples.emplace (std::make_pair (1430181305 - 2*AGENT_NUT_REPEAT_INTERVAL_SEC, 1.13));
        samples.emplace (std::make_pair (1430181305 - AGENT_NUT_REPEAT_INTERVAL_SEC/2 - 10, 10.21));
        samples.emplace (std::make_pair (1430181305 - AGENT_NUT_REPEAT_INTERVAL_SEC/2 + 125, 119.03));
        samples.emplace (std::make_pair (1430181305 - AGENT_NUT_REPEAT_INTERVAL_SEC/2, 12.12)); 
        samples.emplace (std::make_pair (1430181305 + 35, 7.02));
        samples.emplace (std::make_pair (1430181305 + 2*AGENT_NUT_REPEAT_INTERVAL_SEC, 28.7));

        cm::web::solve_left_margin (samples, 1430181305 - AGENT_NUT_REPEAT_INTERVAL_SEC);
        CHECK ( samples.size () == 3 );
        it = samples.begin ();
        CHECK ( it->first == 1430181305 );
        CHECK ( it->second ==  119.03 );
        ++it;
        CHECK ( it->first == 1430181305 + 35 );
        CHECK ( it->second ==  7.02 );       
        ++it;
        CHECK ( it->first == 1430181305 + 2*AGENT_NUT_REPEAT_INTERVAL_SEC );
        CHECK ( it->second ==  28.7 );       

        //  start:  2015-04-27 22:35:06 (1430174106)
        //  extended_start                start
        //        <                         <
        //   x       x     x      x             p ( > repeat interval from last x) x 
        samples.clear ();
        samples.emplace (std::make_pair (1430181306 - 2*AGENT_NUT_REPEAT_INTERVAL_SEC, 1.13));
        samples.emplace (std::make_pair (1430181306 - AGENT_NUT_REPEAT_INTERVAL_SEC/2 - 10, 10.21));
        samples.emplace (std::make_pair (1430181306 - AGENT_NUT_REPEAT_INTERVAL_SEC/2 - 1, 12.12)); 
        samples.emplace (std::make_pair (1430181306 + AGENT_NUT_REPEAT_INTERVAL_SEC/2, 7.02));
        samples.emplace (std::make_pair (1430181306 + 2*AGENT_NUT_REPEAT_INTERVAL_SEC, 28.7));

        cm::web::solve_left_margin (samples, 1430181306 - AGENT_NUT_REPEAT_INTERVAL_SEC);
        CHECK ( samples.size () == 2 );
        it = samples.begin ();
        CHECK ( it->first == 1430181306 + AGENT_NUT_REPEAT_INTERVAL_SEC/2 );
        CHECK ( it->second == 7.02 );
        ++it;
        CHECK ( it->first == 1430181306 + 2*AGENT_NUT_REPEAT_INTERVAL_SEC );
        CHECK ( it->second ==  28.7 );       
    }
}

TEST_CASE ("cm::web::calculate","[agent-cm][computation][average][calculate]")
{
    std::map <int64_t, double> samples;
    double result = -0.1;
    // we are trying to make this generic, but there is a baseline and some fractions are entered directly
    REQUIRE ( AGENT_NUT_REPEAT_INTERVAL_SEC >= 300 ); 
    const uint32_t RI = AGENT_NUT_REPEAT_INTERVAL_SEC;

    samples.emplace (std::make_pair (1430063833, 12.6 )); // 2015-04-26 15:57:13
    samples.emplace (std::make_pair (1430141537, 12.7 )); // 2015-04-27 13:32:17
    samples.emplace (std::make_pair (1430141295, 12.5 )); // 2015-04-27 13:28:15
    samples.emplace (std::make_pair (1430150400, 12.4 )); // 2015-04-27 16:00:00
    samples.emplace (std::make_pair (1430240580, 12.3 )); // 2015-04-28 17:03:00

    samples.emplace (std::make_pair (1430017260, 10 ));  // 2015-04-26 03:01:00
    samples.emplace (std::make_pair (1430017477, 100 )); // 2015-04-26 03:04:37
    samples.emplace (std::make_pair (1430017777, 98 ));  // 2015-04-26 03:09:37
    samples.emplace (std::make_pair (1430017932, 15 ));  // 2015-04-26 03:12:12
    samples.emplace (std::make_pair (1430018100, 10000 ));  // 2015-04-26 03:15:00

    samples.emplace (std::make_pair (1430036100, 50));  // 2015-04-26 08:15:00
    samples.emplace (std::make_pair (1430036252, 60));  // 2015-04-26 08:17:32
    samples.emplace (std::make_pair (1430036532, 70));  // 2015-04-26 08:22:12
    samples.emplace (std::make_pair (1430049660, 10));  // 2015-04-26 12:01:12
    samples.emplace (std::make_pair (1430049877, 100));  // 2015-04-26 12:04:37
    samples.emplace (std::make_pair (1430050177, 15));  // 2015-04-26 12:09:37
    samples.emplace (std::make_pair (1430050426, 170));  // 2015-04-26 12:13:46
    samples.emplace (std::make_pair (1430061124, 13));  // 2015-04-26 15:12:04
    samples.emplace (std::make_pair (1430061420, 4));  // 2015-04-26 15:17:00
    samples.emplace (std::make_pair (1430063839, 50));  // 2015-04-26 15:57:19
    samples.emplace (std::make_pair (1430064000, 10000));  // 2015-04-26 16:00:00

    // same, but with no end -> needs to add
    samples.emplace (std::make_pair (1429930860, 10 ));  // 2015-04-25 03:01:00
    samples.emplace (std::make_pair (1429931077, 100 )); // 2015-04-25 03:04:37
    samples.emplace (std::make_pair (1429931377, 98 ));  // 2015-04-25 03:09:37
    samples.emplace (std::make_pair (1429931532, 15 ));  // 2015-04-25 03:12:12
    samples.emplace (std::make_pair (1429931759, 10000 ));  // 2015-04-25 03:15:59
   
    samples.emplace (std::make_pair (1429975020, 4));  // 2015-04-25 15:17:00
    samples.emplace (std::make_pair (1429977439, 50));  // 2015-04-25 15:57:19
    samples.emplace (std::make_pair (1429977739, 10000));  // 2015-04-25 16:02:19
    
    // data - no point on end, but last too far
    samples.emplace (std::make_pair (1429890558, 4));  // 2015-04-24 15:49:18
    samples.emplace (std::make_pair (1429890889, 50));  // 2015-04-24 15:54:49
    samples.emplace (std::make_pair (1429891401, 10000));  // 2015-04-24 16:03:21
   
    // TODO: data - no point on end, last close, but gap on right

    SECTION ("bad args") {
        double result_prev = 0;

        samples. clear ();
        ////////////////
        // Empty samples
        result_prev = result;
        CHECK ( cm::web::calculate (samples, 1, 2, "min", result) == 1 );
        CHECK ( result == result_prev );

        ///////////////////////////////////
        // begin timestamp >= end timestamp
        samples.emplace (std::make_pair (1000, 12.231));
        result_prev = result;
        CHECK ( cm::web::calculate (samples, 3, 2, "min", result) == -1 );
        CHECK ( result == result_prev );

        result_prev = result;
        CHECK ( cm::web::calculate (samples, 1430174410, 1430174410, "min", result) == -1 );
        CHECK ( result == result_prev );

        result_prev = result;
        CHECK ( cm::web::calculate (samples, 1430174410, 1430174409, "min", result) == -1 );
        CHECK ( result == result_prev );

        result_prev = result;
        CHECK ( cm::web::calculate (samples, 1, -2, "min", result) == -1 );
        CHECK ( result == result_prev );
        
        ////////////////////////////////////////////////////////
        // There is no sample with timestamp of start or greater
        samples.clear ();
        samples.emplace (std::make_pair (1420070400, 100.123)); // 2015-01-01 00:00:00

        // 2015-01-01 00:00:01  ->  2015-04-26 17:46:40
        result_prev = result;
        CHECK ( cm::web::calculate (samples, 1420070401, 1430070400, "max", result) == 1 );
        CHECK ( result == result_prev );

        // 2015-06-09 07:39:14  ->  2015-06-12 13:57:02
        result_prev = result;
        CHECK ( cm::web::calculate (samples, 1433835554, 1434117422, "arithmetic_mean", result) == 1 );
        CHECK ( result == result_prev );

        //////////////////////////////////////
        // First sample is after end timestamp
        
        // 2014-06-15 13:57:02  ->  2014-06-15 11:22:05 
        result_prev = result;
        CHECK ( cm::web::calculate (samples, 1402581422, 1402831325, "min", result) == 1 );
        CHECK ( result == result_prev );
         
        // 2015-12-24 01:15:43  ->  2014-12-31 23:59:59
        result_prev = result;
        CHECK ( cm::web::calculate (samples, 1419383743, 1420070399, "max", result) == 1 );
        CHECK ( result == result_prev );
        
        // 2014-05-23 00:04:11  ->  2015-01-01 00:00:00
        result_prev = result;
        CHECK ( cm::web::calculate (samples, 1400803451, 1420070400, "arithmetic_mean", result) == 1 );
        CHECK ( result == result_prev );
    }

    SECTION ("no measurement on end timestamp - no measurement after") {
        // The last measurement should therefore have weight: 1
        // and no emplace should happen
        std::map <int64_t, double> samples_section;
        std::size_t size = 0;
        double result_prev = 0;
         
        samples_section.emplace (std::make_pair (1420070400, 564 )); // 2015-01-01 00:00:00

        samples_section.emplace (std::make_pair (1420084902, 564 )); // 2015-01-01 04:01:42
        samples_section.emplace (std::make_pair (1420084902 + AGENT_NUT_REPEAT_INTERVAL_SEC - 1, 257 ));
        samples_section.emplace (std::make_pair (1420084902 + 2*AGENT_NUT_REPEAT_INTERVAL_SEC - 1, 723 ));
        samples_section.emplace (std::make_pair (1420084902 + 2*AGENT_NUT_REPEAT_INTERVAL_SEC -1 + AGENT_NUT_REPEAT_INTERVAL_SEC/2, 443 ));
        samples_section.emplace (std::make_pair (1420084902 + 3*AGENT_NUT_REPEAT_INTERVAL_SEC  + AGENT_NUT_REPEAT_INTERVAL_SEC/2, 1002 ));

        samples_section.emplace (std::make_pair (1420093397, 931 )); // 2015-01-01 06:23:17
        samples_section.emplace (std::make_pair (1420093625, 321 )); // 2015-01-01 06:27:05
        samples_section.emplace (std::make_pair (1420103618, 52 )); // 2015-01-01 09:13:38
        samples_section.emplace (std::make_pair (1420125678, 1073 )); // 2015-01-01 15:21:18
        samples_section.emplace (std::make_pair (1420156800, 1012 )); // 2015-01-02 00:00:00
        samples_section.emplace (std::make_pair (1420159500, 327 )); // 2015-01-02 00:45:00
        samples_section.emplace (std::make_pair (1420160234, 437 )); // 2015-01-02 00:57:14
        samples_section.emplace (std::make_pair (1420160356, 673 )); // 2015-01-02 00:59:16

        size = samples_section.size ();

        // 2015-01-01 00:00:00  ->  2015-01-01 00:15:00  
        CHECK ( cm::web::calculate (samples_section, 1420070400, 1420071300, "arithmetic_mean", result) == 0 );
        CHECK ( result == Approx (564) );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420070400, 1420071300, "min", result) == 0 );
        CHECK ( result == Approx (564) );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420070400, 1420071300, "max", result) == 0 );
        CHECK ( result == Approx (564) );
        CHECK ( samples_section.size () == size );


        // empty 15 min
        // 2015-01-01 05:15:00  ->  2015-01-01 05:30:00
        result_prev = result;
        CHECK ( cm::web::calculate (samples_section, 1420089300, 1420090200, "arithmetic_mean", result) == 1 );
        CHECK ( result == result_prev );
        CHECK ( samples_section.size () == size );

        result_prev = result;
        CHECK ( cm::web::calculate (samples_section, 1420089300, 1420090200, "min", result) == 1 );
        CHECK ( result == result_prev );
        CHECK ( samples_section.size () == size );

        result_prev = result;
        CHECK ( cm::web::calculate (samples_section, 1420089300, 1420090200, "max", result) == 1 );
        CHECK ( result == result_prev );
        CHECK ( samples_section.size () == size );

        // 2015-01-02 00:30:00  ->  2015-01-02 00:45:00
        result_prev = result;
        CHECK ( cm::web::calculate (samples_section, 1420158600, 1420159500, "arithmetic_mean", result) == 1 );
        CHECK ( result == result_prev );
        CHECK ( samples_section.size () == size );

        result_prev = result;
        CHECK ( cm::web::calculate (samples_section, 1420158600, 1420159500, "min", result) == 1 );
        CHECK ( result == result_prev );
        CHECK ( samples_section.size () == size );

        result_prev = result;
        CHECK ( cm::web::calculate (samples_section, 1420158600, 1420159500, "max", result) == 1 );
        CHECK ( result == result_prev );
        CHECK ( samples_section.size () == size );

        // empty 30 min
        // 2015-01-01 07:30:00  ->  2015-01-01 08:00:00
        result_prev = result;
        CHECK ( cm::web::calculate (samples_section, 1420097400, 1420099200, "arithmetic_mean", result) == 1 );
        CHECK ( result == result_prev );
        CHECK ( samples_section.size () == size );

        result_prev = result;
        CHECK ( cm::web::calculate (samples_section, 1420097400, 1420099200, "min", result) == 1 );
        CHECK ( result == result_prev );
        CHECK ( samples_section.size () == size );

        result_prev = result;
        CHECK ( cm::web::calculate (samples_section, 1420097400, 1420099200, "max", result) == 1 );
        CHECK ( result == result_prev );
        CHECK ( samples_section.size () == size );

        // empty 1h
        // 2015-01-01 18:00:00  ->  2015-01-01 19:00:00 
        result_prev = result;
        CHECK ( cm::web::calculate (samples_section, 1420135200, 1420138800, "arithmetic_mean", result) == 1 );
        CHECK ( result == result_prev );
        CHECK ( samples_section.size () == size );

        result_prev = result;
        CHECK ( cm::web::calculate (samples_section, 1420135200, 1420138800, "min", result) == 1 );
        CHECK ( result == result_prev );
        CHECK ( samples_section.size () == size );

        result_prev = result;
        CHECK ( cm::web::calculate (samples_section, 1420135200, 1420138800, "max", result) == 1 );
        CHECK ( result == result_prev );
        CHECK ( samples_section.size () == size );

        // empty 8h
        // 2015-01-01 16:00:00  ->  2015-01-02 00:00:00
        result_prev = result;
        CHECK ( cm::web::calculate (samples_section, 1420128000, 1420156800, "arithmetic_mean", result) == 1 );
        CHECK ( result == result_prev );
        CHECK ( samples_section.size () == size );

        result_prev = result;
        CHECK ( cm::web::calculate (samples_section, 1420128000, 1420156800, "min", result) == 1 );
        CHECK ( result == result_prev );
        CHECK ( samples_section.size () == size );

        result_prev = result;
        CHECK ( cm::web::calculate (samples_section, 1420128000, 1420156800, "max", result) == 1 );
        CHECK ( result == result_prev );
        CHECK ( samples_section.size () == size );

        // 8h
        // 2015-01-01 00:00:00  ->  2015-01-01 08:00:00
        CHECK ( cm::web::calculate (samples_section, 1420070400, 1420099200, "arithmetic_mean", result) == 0 );
        double computed = (564 + 564*(RI-1) + 257*RI + 723*(RI/2) + 443 + 1002 + 931*228 + 321) / (double) (1 + RI-1 + RI + RI/2 + 1 + 1 + 228 + 1);
        CHECK ( result == Approx (computed) );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420070400, 1420099200, "min", result) == 0 );
        CHECK ( result == 257 );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420070400, 1420099200, "max", result) == 0 );
        CHECK ( result == 1002 );
        CHECK ( samples_section.size () == size );

        // 15m
        // 2015-01-02 00:45:00  ->  2015-01-02 01:00:00
        CHECK ( cm::web::calculate (samples_section, 1420159500, 1420160400, "arithmetic_mean", result) == 0 );
        CHECK ( result == Approx (438.016) );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420159500, 1420160400, "min", result) == 0 );
        CHECK ( result == 327 );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420159500, 1420160400, "max", result) == 0 );
        CHECK ( result == 673 );
        CHECK ( samples_section.size () == size );

        // 30m
        // 2015-01-02 00:30:00  ->  2015-01-02 01:00:00
        CHECK ( cm::web::calculate (samples_section, 1420158600, 1420160400, "arithmetic_mean", result) == 0 );
        CHECK ( result == Approx (438.016) );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420158600, 1420160400, "min", result) == 0 );
        CHECK ( result == 327 );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420158600, 1420160400, "max", result) == 0 );
        CHECK ( result == 673 );
        CHECK ( samples_section.size () == size );

        // 1h
        // 2015-01-02 00:00:00  ->  2015-01-02 01:00:00
        CHECK ( cm::web::calculate (samples_section, 1420156800, 1420160400, "arithmetic_mean", result) == 0 );
        CHECK ( result == Approx (442.608) );
        CHECK ( samples_section.size () == size );
        
        CHECK ( cm::web::calculate (samples_section, 1420156800, 1420160400, "min", result) == 0 );
        CHECK ( result == 327 );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420156800, 1420160400, "max", result) == 0 );
        CHECK ( result == 1012 );
        CHECK ( samples_section.size () == size );

        // 8h
        // 2015-01-01 17:00:00  ->  2015-01-02 01:00:00
        CHECK ( cm::web::calculate (samples_section, 1420131600, 1420160400, "arithmetic_mean", result) == 0 );
        CHECK ( result == Approx (442.608) );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420131600, 1420160400, "min", result) == 0 );
        CHECK ( result == 327 );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420131600, 1420160400, "max", result) == 0 );
        CHECK ( result == 1012 );
        CHECK ( samples_section.size () == size );

        // 24h
        // 2015-01-01 01:00:00  ->  2015-01-02 01:00:00
        CHECK ( cm::web::calculate (samples_section, 1420074000, 1420160400, "arithmetic_mean", result) == 0 );
        computed =
        (564 + 564*(RI-1) + 257*RI + 723*(RI/2) + 443 + 1002 + 931*228 + 321 + 52 + 1073 + 1012 + 327 + 437*122 + 673) /
        (double) (1 + RI-1 + RI + RI/2 + 1 + 1 + 228 + 1 + 1 + 1 + 1 + 1 + 122 + 1);
        CHECK ( result == Approx (computed) );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420074000, 1420160400, "min", result) == 0 );
        CHECK ( result == Approx (52) );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420074000, 1420160400, "max", result) == 0 );
        CHECK ( result == Approx (1073) );
        CHECK ( samples_section.size () == size );

        // Try different ending combinations
        samples_section.emplace (std::make_pair (1420329599, 5 )); // 2015-01-03 23:59:59
        size = samples_section.size ();

        // 15m
        // 2015-01-03 23:45:00  ->  2015-01-04 00:00:00
        CHECK ( cm::web::calculate (samples_section, 1420328700, 1420329600, "arithmetic_mean", result) == 0 );
        CHECK ( result == 5 );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420328700, 1420329600, "min", result) == 0 );
        CHECK ( result == 5 );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420328700, 1420329600, "max", result) == 0 );
        CHECK ( result == 5 );
        CHECK ( samples_section.size () == size );

        // 30m
        // 2015-01-03 23:30:00  ->  2015-01-04 00:00:00
        CHECK ( cm::web::calculate (samples_section, 1420327800, 1420329600, "arithmetic_mean", result) == 0 );
        CHECK ( result == 5 );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420327800, 1420329600, "min", result) == 0 );
        CHECK ( result == 5 );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420327800, 1420329600, "max", result) == 0 );
        CHECK ( result == 5 );
        CHECK ( samples_section.size () == size );


        // 1h
        // 2015-01-03 23:00:00  ->  2015-01-04 00:00:00
        CHECK ( cm::web::calculate (samples_section, 1420327800, 1420329600, "arithmetic_mean", result) == 0 );
        CHECK ( result == 5 );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420327800, 1420329600, "min", result) == 0 );
        CHECK ( result == 5 );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420327800, 1420329600, "max", result) == 0 );
        CHECK ( result == 5 );
        CHECK ( samples_section.size () == size );

        // 8h
        // 2015-01-03 16:00:00  ->  2015-01-04 00:00:00
        CHECK ( cm::web::calculate (samples_section, 1420300800, 1420329600, "arithmetic_mean", result) == 0 );
        CHECK ( result == 5 );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420300800, 1420329600, "min", result) == 0 );
        CHECK ( result == 5 );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420300800, 1420329600, "max", result) == 0 );
        CHECK ( result == 5 );
        CHECK ( samples_section.size () == size );

        // 24h
        // 2015-01-03 00:00:00  ->  2015-01-04 00:00:00
        CHECK ( cm::web::calculate (samples_section, 1420243200, 1420329600, "arithmetic_mean", result) == 0 );
        CHECK ( result == 5 );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420243200, 1420329600, "min", result) == 0 );
        CHECK ( result == 5 );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420243200, 1420329600, "max", result) == 0 );
        CHECK ( result == 5 );
        CHECK ( samples_section.size () == size );
        
        // more measurements
        samples_section.emplace (std::make_pair (1420387723 - RI - 1, 100)); 
        samples_section.emplace (std::make_pair (1420387723, 150)); // 2015-01-04 16:08:43
        samples_section.emplace (std::make_pair (1420387793, 200)); // 2015-01-04 16:09:53
        samples_section.emplace (std::make_pair (1420387897, 50)); // 2015-01-04 16:11:37
        size = samples_section.size ();

        // 15m 
        // 2015-01-04 16:00:00  ->  2015-01-04 16:15:00
        CHECK ( cm::web::calculate (samples_section, 1420387200, 1420388100, "arithmetic_mean", result) == 0 );
        CHECK ( result == Approx (178.693) );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420387200, 1420388100, "min", result) == 0 );
        CHECK ( result == 50 );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420387200, 1420388100, "max", result) == 0 );
        CHECK ( result == 200 );
        CHECK ( samples_section.size () == size );

        // 30m 
        // 2015-01-04 15:45:00  ->  2015-01-04 16:15:00
        CHECK ( cm::web::calculate (samples_section, 1420386300, 1420388100, "arithmetic_mean", result) == 0 );
        CHECK ( result == Approx (178.693) );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420386300, 1420388100, "min", result) == 0 );
        CHECK ( result == 50 );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420386300, 1420388100, "max", result) == 0 );
        CHECK ( result == 200 );
        CHECK ( samples_section.size () == size );

        // 1h 
        // 2015-01-04 15:15:00  ->  2015-01-04 16:15:00
        CHECK ( cm::web::calculate (samples_section, 1420384500, 1420388100, "arithmetic_mean", result) == 0 );
        CHECK ( result == Approx (178.693) );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420384500, 1420388100, "min", result) == 0 );
        CHECK ( result == 50 );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420384500, 1420388100, "max", result) == 0 );
        CHECK ( result == 200 );
        CHECK ( samples_section.size () == size );

        // 8h 
        // 2015-01-04 08:15:00  ->  2015-01-04 16:15:00
        CHECK ( cm::web::calculate (samples_section, 1420359300, 1420388100, "arithmetic_mean", result) == 0 );
        CHECK ( result == Approx (178.693) );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420359300, 1420388100, "min", result) == 0 );
        CHECK ( result == 50 );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420359300, 1420388100, "max", result) == 0 );
        CHECK ( result == 200 );
        CHECK ( samples_section.size () == size );

        // 24h 
        // 2015-01-03 16:15:00  ->  2015-01-04 16:15:00
        CHECK ( cm::web::calculate (samples_section, 1420301700, 1420388100, "arithmetic_mean", result) == 0 );
        CHECK ( result == Approx (177.711) );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420301700, 1420388100, "min", result) == 0 );
        CHECK ( result == 5 );
        CHECK ( samples_section.size () == size );

        CHECK ( cm::web::calculate (samples_section, 1420301700, 1420388100, "max", result) == 0 );
        CHECK ( result == 200 );
        CHECK ( samples_section.size () == size );
    }
    SECTION ("no measurement on end timestamp - measurement after, but too far away") {
        // The last measurement should therefore have weight: 1
        // and no emplace should happen

        std::map <int64_t, double> samples_section;
        // std::size_t size = 0;
        double result_prev = 0;
         
        samples_section.emplace (std::make_pair (1420156800 - RI, 327 )); // 2015-01-02 00:00:00 - RI
        samples_section.emplace (std::make_pair (1420156800 - RI, 327 )); // 2015-01-02 00:00:00 - RI
        samples_section.emplace (std::make_pair (1420156801, 456 )); // 2015-01-02 00:00:01
        // size = samples_section.size (); // result is not evaluated
/*
        CHECK ( cm::web::calculate (samples_section, , , "arithmetic_mean", result) == 0 );
        CHECK ( result == Approx (177.711) );
        CHECK ( samples_section.size () == size );
*/


       
    }
    SECTION ("no measurement on end timestamp - measurement after and in range") {
        // The last measurement should therefore have weight (end - ts)
        // emplace should happen


    }   
// TODO: below are unit test not tailored for repeat interval changed
    SECTION ("min") {
        CHECK ( cm::web::calculate (samples, 1430121600, 1430150400, "min", result) == 0 ); // 2015-04-27 08:00:00 -> 2015-04-27 16:00:00 8h
        CHECK ( result == 12.5 );

        std::size_t size = samples.size ();
        CHECK ( cm::web::calculate (samples, 1430140500, 1430141400, "min", result) == 0 ); // 2015-04-27 13:15:00 -> 2015-04-27 13:30:00 30m
        CHECK ( result == 12.5 );
        CHECK ( samples.size () == size + 1 );
        CHECK ( samples.at (1430141400) == 12.5 );

        size = samples.size ();
        CHECK ( cm::web::calculate (samples, 1430035200, 1430064000, "min", result) == 0 ); // 2015-04-26 08:00:00 -> 2015-04-26 16:00:00 8h
        CHECK ( result == 4 );
        CHECK ( samples.size () == size );

        size = samples.size ();
        CHECK ( cm::web::calculate (samples, 1430017200, 1430018100, "min", result) == 0 ); // 2015-04-26 03:00:00 -> 2015-04-26 03:15:00 15m
        CHECK ( result == 10 );
        CHECK ( samples.size () == size );

        size = samples.size ();
        CHECK ( cm::web::calculate (samples, 1430017200, 1430018100, "min", result) == 0 ); // 2015-04-26 03:00:00 -> 2015-04-26 03:15:00 15m
        CHECK ( result == 10 );
        CHECK ( samples.size () == size );       

        size = samples.size ();
        CHECK ( cm::web::calculate (samples, 1430035200, 1430064000, "min", result) == 0 ); // 2015-04-26 08:00:00 -> 2015-04-26 16:00:00 8h
        CHECK ( result == 4 );
        CHECK ( samples.size () == size );       

        size = samples.size ();
        CHECK ( cm::web::calculate (samples, 1429930800, 1429931700, "min", result) == 0 ); // 2015-04-25 03:00:00 -> 2015-04-25 03:15:00 15m
        CHECK ( result == 10 );
        CHECK ( samples.size () == size + 1 );
        CHECK ( samples.at(1429931700) == 15 );

        size = samples.size ();
        CHECK ( cm::web::calculate (samples, 1429948800, 1429977600, "min", result) == 0 ); // 2015-04-25 08:00:00 -> 2015-04-25 16:00:00 8h
        CHECK ( result == 4 );
        CHECK ( samples.size () == size + 1 );
        CHECK ( samples.at(1429977600) == 50 );

        size = samples.size ();
        CHECK ( cm::web::calculate (samples, 1429890300, 1429891200, "min", result) == 0 ); // 2015-04-24 15:45:00 -> 2015-04-24 16:00:00 15m
        CHECK ( result == 4 );
        CHECK ( samples.size () == size );       
    }
    
    SECTION ("max") {
        CHECK ( cm::web::calculate (samples, 1430121600, 1430150400, "max", result) == 0 ); // 2015-04-27 08:00:00 -> 2015-04-27 16:00:00 8h
        CHECK ( result == 12.7);

        std::size_t size = samples.size ();
        CHECK ( cm::web::calculate (samples, 1430140500, 1430141400, "max", result) == 0 ); // 2015-04-27 13:15:00 -> 2015-04-27 13:30:00 30m
        CHECK ( result == 12.5 );
        CHECK ( samples.size () == size + 1 );
        CHECK ( samples.at (1430141400) == 12.5 );

        size = samples.size ();
        CHECK ( cm::web::calculate (samples, 1430035200, 1430064000, "max", result) == 0 ); // 2015-04-26 08:00:00 -> 2015-04-26 16:00:00 8h
        CHECK ( result == 170 );
        CHECK ( samples.size () == size );

        size = samples.size ();
        CHECK ( cm::web::calculate (samples, 1430017200, 1430018100, "max", result) == 0 ); // 2015-04-26 03:00:00 -> 2015-04-26 03:15:00 15m
        CHECK ( result == 100 );
        CHECK ( samples.size () == size );

        size = samples.size ();
        CHECK ( cm::web::calculate (samples, 1430035200, 1430064000, "max", result) == 0 ); // 2015-04-26 08:00:00 -> 2015-04-26 16:00:00 8h
        CHECK ( result == 170 );
        CHECK ( samples.size () == size );

        size = samples.size ();
        CHECK ( cm::web::calculate (samples, 1429930800, 1429931700, "max", result) == 0 ); // 2015-04-25 03:00:00 -> 2015-04-25 03:15:00 15m
        CHECK ( result == 100 );
        CHECK ( samples.size () == size + 1 );
        CHECK ( samples.at(1429931700) == 15 );       

        size = samples.size ();
        CHECK ( cm::web::calculate (samples, 1429948800, 1429977600, "max", result) == 0 ); // 2015-04-25 08:00:00 -> 2015-04-25 16:00:00 8h
        CHECK ( result == 50 );
        CHECK ( samples.size () == size + 1 );
        CHECK ( samples.at(1429977600) == 50 );

        size = samples.size ();
        CHECK ( cm::web::calculate (samples, 1429890300, 1429891200, "max", result) == 0 ); // 2015-04-24 15:45:00 -> 2015-04-24 16:00:00 15m
        CHECK ( result == 50 );
        CHECK ( samples.size () == size );
    }
    
    SECTION ("empty interval") {
        CHECK ( cm::web::calculate (samples, 1430208000, 1430236800, "min", result) == 1 ); // 2015-04-28 08:00:00 -> 2015-04-28 16:00:00
        CHECK ( cm::web::calculate (samples, 1430208000, 1430236800, "max", result) == 1 ); // 2015-04-28 08:00:00 -> 2015-04-28 16:00:00
        CHECK ( cm::web::calculate (samples, 1430208000, 1430236800, "arithmetic_mean", result) == 1 ); // 2015-04-28 08:00:00 -> 2015-04-28 16:00:00

        CHECK ( cm::web::calculate (samples, 1430149500, 1430150400, "min", result) == 1 ); // 2015-04-27 15:45:00 -> 2015-04-27 16:00:00
        CHECK ( cm::web::calculate (samples, 1430149500, 1430150400, "max", result) == 1 ); // 2015-04-27 15:45:00 -> 2015-04-27 16:00:00
        CHECK ( cm::web::calculate (samples, 1430149500, 1430150400, "arithmetic_mean", result) == 1 ); // 2015-04-27 15:45:00 -> 2015-04-27 16:00:00

    }
/* TODO: Write simple isolated cases here, bigger ones in ci script
    SECTION ("arithmetic_mean") {
    }
*/

}

// check_completeness (int64_t last_container_timestamp, int64_t last_average_timestamp, int64_t end_timestamp, const char *step, int64_t& new_start)
TEST_CASE ("cm::web::check_completeness","[agent-cm][computation][average][check_completeness]")
{
    SECTION ("Test cases that can be applied to all average steps (alignment with start of day)") {
        int i = 0;
        int64_t new_start = 0;
        for (i = 0; i < AVG_STEPS_SIZE; i++) {
            //////////////////////////////////////////////////////////////
            // CASE: last container timestamp > last average timestamp
            // 1433116800 == 2015-06-01 00:00:00
            // 1435708800 == 2015-07-01 00:00:00
            INFO ("Step is " << AVG_STEPS[i]);
            CHECK ( cm::web::check_completeness (1433116800, INT64_MIN, 1435708800, AVG_STEPS[i], new_start) == -1 );
            
            // 1433059200 == 2015-05-31 08:00:00 
            CHECK ( cm::web::check_completeness (1433116800, 1433059200, 1435708800, AVG_STEPS[i], new_start) == -1 );
            CHECK ( new_start == 0 );

            CHECK ( cm::web::check_completeness (1433116800, INT64_MIN, 1433116800 + average_step_seconds (AVG_STEPS[i]), AVG_STEPS[i], new_start) == -1 );
            CHECK ( new_start == 0 );

            CHECK ( cm::web::check_completeness (1433116800, 1433059200, 1433116800 + average_step_seconds (AVG_STEPS[i]), AVG_STEPS[i], new_start) == -1 );
            CHECK ( new_start == 0 );

            // end timestamp == last container timestamp
            new_start = 0;
            CHECK ( cm::web::check_completeness (1433116800, INT64_MIN, 1433116800, AVG_STEPS[i], new_start) == -1 );
            CHECK ( new_start == 0 );
            CHECK ( cm::web::check_completeness (1433116800, 1433059200, 1433116800, AVG_STEPS[i], new_start) == -1  );
            CHECK ( new_start == 0 );

            //////////////////////////////////////////////////////////////
            // CASE: last container timestamp == last average timestamp
            // 1433116800 == 2015-06-01 00:00:00
            // 1435708800 == 2015-07-01 00:00:00
            CHECK ( cm::web::check_completeness (1433116800, 1433116800, 1435708800, AVG_STEPS[i], new_start) == 0 );
            CHECK ( new_start == 1433116800 + average_step_seconds (AVG_STEPS[i]) );

            CHECK ( cm::web::check_completeness (1433116800, 1433116800, 1433116800 + average_step_seconds (AVG_STEPS[i]), AVG_STEPS[i], new_start) == 0 );
            CHECK ( new_start == 1433116800 + average_step_seconds (AVG_STEPS[i]) );

            // end timestamp == last container timestamp
            new_start = 0;
            CHECK ( cm::web::check_completeness (1433116800, 1433116800, 1433116800, AVG_STEPS[i], new_start) == 1 );
            CHECK ( new_start == 0 );

            //////////////////////////////////////////////////////////////
            // CASE: last container timestamp < last average timestamp
            // 1433116800 == 2015-06-01 00:00:00
            // 1435708800 == 2015-07-01 00:00:00
            // 1454284800 == 2016-02-01 00:00:00
            new_start = 0;
            CHECK ( cm::web::check_completeness (1433116800, 1454284800, 1435708800, AVG_STEPS[i], new_start) == 1 );
            CHECK ( new_start == 0 );

            CHECK ( cm::web::check_completeness (1433116800, 1454284800, 1433116800 + average_step_seconds (AVG_STEPS[i]), AVG_STEPS[i], new_start) == 1 );
            CHECK ( new_start == 0 );

            // end timestamp == last container timestamp
            CHECK ( cm::web::check_completeness (1433116800, 1454284800, 1433116800, AVG_STEPS[i], new_start) == 1 );
            CHECK ( new_start == 0 );


            ///////////////////////////////////////////////////////////////////////////////
            // CASE: simulate erroneous persist::get_measurements_averages return values
            // 1433116800 == 2015-06-01 00:00:00
            new_start = 0;
            CHECK (
            cm::web::check_completeness
            (1433116800, 1433116800 + average_step_seconds (AVG_STEPS[i]), 1433116800 + average_step_seconds (AVG_STEPS[i]), AVG_STEPS[i], new_start) == -1 );
            CHECK (
            cm::web::check_completeness
            (1433116800, 1433116800 + average_step_seconds (AVG_STEPS[i]), 1433116800 + 2*average_step_seconds (AVG_STEPS[i]), AVG_STEPS[i], new_start) == -1 );
            CHECK ( new_start == 0 );
        }
        // TODO: create a few  hand tailored cases for each avg. step
    }

}


/*
        samples_section.emplace (std::make_pair (1420329599, 5 )); // 2015-01-03 23:59:59 old, delete

        samples_section.emplace (std::make_pair (1420387521, 50 )); // 2015-01-04 16:05:21
        samples_section.emplace (std::make_pair (1420387573, 63 )); // 2015-01-04 16:06:13 
        samples_section.emplace (std::make_pair (1420387614, 78 )); // 2015-01-04 16:06:54
        samples_section.emplace (std::make_pair (1420387641, 91 )); // 2015-01-04 16:07:21
        samples_section.emplace (std::make_pair (1420387674, 148 )); // 2015-01-04 16:07:54


        samples_section.emplace (std::make_pair (1420466472, 137 )); // 2015-01-05 14:01:12
        samples_section.emplace (std::make_pair (1420466472 + RI/2, 125 ));
        samples_section.emplace (std::make_pair (1420466472 + RI, 113 ));
        samples_section.emplace (std::make_pair (1420466472 + 2*RI - 1, 102 ));
        samples_section.emplace (std::make_pair (1420466472 + 3*RI - 1, 95 ));
        
        samples_section.emplace (std::make_pair (1420466472 + 4*RI,   ));        
        samples_section.emplace (std::make_pair (1420466472 + 4*RI + 104,  ));
        samples_section.emplace (std::make_pair (1420466472 + 4*RI + 104 + 70,  ));
        samples_section.emplace (std::make_pair (1420466472 + 4*RI + 104 + 70 + 132,  ));
 
*/
