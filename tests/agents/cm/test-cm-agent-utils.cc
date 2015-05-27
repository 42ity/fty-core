#include <catch.hpp>
#include <stdio.h>

#include "defs.h"
#include "cm-agent-utils.h"

TEST_CASE ("process_db_measurement_solve_left_margin","[agent-cm][computation][average][db]")
{

    SECTION ("bad args") {
        std::map <int64_t, double> samples;
        CHECK (samples.empty ());
        process_db_measurement_solve_left_margin (samples, 1426204882);
        CHECK (samples.empty ());

        samples.emplace (std::make_pair (1430173900, 12.4)); // 2015-04-28 00:31:40
        CHECK (samples.size () == 1);
        process_db_measurement_solve_left_margin (samples, -1);
        CHECK (samples.size () == 1);
    }

    SECTION ("correct 1") {
        // extended_start                start
        //       <                         <
        //                                      x
        std::map <int64_t, double> samples;
        samples.emplace (std::make_pair (1430173900, 12.4)); // 2015-04-28 00:31:40
        std::size_t size = samples.size ();

        process_db_measurement_solve_left_margin (samples, 1430173599); // 2015-04-28 00:26:39 
        CHECK ( size == samples.size () );
    }

    SECTION ("correct 2") {
        //  extended_start                start
        //        <                         <
        //   x                                    x
        std::map <int64_t, double> samples;
        samples.emplace (std::make_pair (1430172072, 1.13)); // 2015-04-28 00:01:12
        samples.emplace (std::make_pair (1430173530, 11.3)); // 2015-04-28 00:25:30
        samples.emplace (std::make_pair (1430173900, 12.4)); // 2015-04-28 00:31:40
        std::size_t size = samples.size ();

        process_db_measurement_solve_left_margin (samples, 1430173599); // 2015-04-28 00:26:39 
        CHECK ( samples.size () == 1 );
        CHECK ( samples.begin ()->first == 1430173900 );
        CHECK ( samples.begin ()->second == 12.4 );
    }

    SECTION ("correct 3") {
        //  extended_start                start
        //        <                         <
        //   x                              x
        std::map <int64_t, double> samples;
        samples.emplace (std::make_pair (1430172072, 1.13)); // 2015-04-28 00:01:12
        samples.emplace (std::make_pair (1430173530, 11.3)); // 2015-04-28 00:25:30
        samples.emplace (std::make_pair (1430173900, 12.4)); // 2015-04-28 00:31:40

        process_db_measurement_solve_left_margin (samples, 1430173600); // 2015-04-28 00:26:40
        CHECK ( samples.size () == 1 );
        CHECK ( samples.begin ()->first == 1430173900 );
        CHECK ( samples.begin ()->second == 12.4 );
      
    }

    SECTION ("correct 4") {
        //  extended_start                start
        //        <                         <
        //   x    x        x    x           x  x
        std::map <int64_t, double> samples;
        samples.emplace (std::make_pair (1430172072, 1.13)); // 2015-04-28 00:01:12
        samples.emplace (std::make_pair (1430174105, 12.4)); // 2015-04-28 00:35:05
        samples.emplace (std::make_pair (1430174117, 10.21)); // 2015-04-28 00:35:17
        samples.emplace (std::make_pair (1430174188, 119.03)); // 2015-04-28 00:36:28
        samples.emplace (std::make_pair (1430174405, 3.0482)); // 2015-04-28 00:40:05
        samples.emplace (std::make_pair (1430174410, 7.02)); // 2015-04-28 00:40:10

        process_db_measurement_solve_left_margin (samples, 1430174105); // 2015-04-28 00:35:05
        CHECK ( samples.size () == 2 );
        auto it = samples.begin ();
        CHECK ( it->first == 1430174405 );
        CHECK ( it->second ==  3.0482 );
        ++it;
        CHECK ( it->first == 1430174410 );
        CHECK ( it->second ==  7.02 );
    }

    SECTION ("correct 5") {
        //  extended_start                start
        //        <                         <
        //   x    x        x    x      x        x
        std::map <int64_t, double> samples;
        samples.emplace (std::make_pair (1430172072, 1.13)); // 2015-04-28 00:01:12
        samples.emplace (std::make_pair (1430174105, 12.4)); // 2015-04-28 00:35:05
        samples.emplace (std::make_pair (1430174117, 10.21)); // 2015-04-28 00:35:17
        samples.emplace (std::make_pair (1430174188, 119.03)); // 2015-04-28 00:36:28
        samples.emplace (std::make_pair (1430174399, 12.12)); // 2015-04-28 00:39:59
        samples.emplace (std::make_pair (1430174401, 3.0482)); // 2015-04-28 00:40:01
        samples.emplace (std::make_pair (1430174410, 7.02)); // 2015-04-28 00:40:10
        samples.emplace (std::make_pair (1430174422, 17.1)); // 2015-04-28 00:40:22

        process_db_measurement_solve_left_margin (samples, 1430174105); // 2015-04-28 00:35:05
        CHECK ( samples.size () == 3 );
        auto it = samples.begin ();
        CHECK ( it->first == 1430174405 );
        CHECK ( it->second ==  3.0482 );
        ++it;
        CHECK ( it->first == 1430174410 );
        CHECK ( it->second ==  7.02 );       
        ++it;
        CHECK ( it->first == 1430174422 );
        CHECK ( it->second ==  17.1 );       
    }

    SECTION ("correct 6") {
        //  extended_start                start
        //        <                         <
        //   x            x    x      x        x
        std::map <int64_t, double> samples;
        samples.emplace (std::make_pair (1430172072, 1.13)); // 2015-04-28 00:01:12
        samples.emplace (std::make_pair (1430174117, 10.21)); // 2015-04-28 00:35:17
        samples.emplace (std::make_pair (1430174188, 119.03)); // 2015-04-28 00:36:28
        samples.emplace (std::make_pair (1430174399, 12.12)); // 2015-04-28 00:39:59
        samples.emplace (std::make_pair (1430174404, 3.12)); // 2015-04-28 00:40:04
        samples.emplace (std::make_pair (1430174410, 7.02)); // 2015-04-28 00:40:10
        samples.emplace (std::make_pair (1430174422, 17.1)); // 2015-04-28 00:40:22

        process_db_measurement_solve_left_margin (samples, 1430174105); // 2015-04-28 00:35:05
        CHECK ( samples.size () == 3 );
        auto it = samples.begin ();
        CHECK ( it->first == 1430174405 );
        CHECK ( it->second ==  3.12 );
        ++it;
        CHECK ( it->first == 1430174410 );
        CHECK ( it->second ==  7.02 );       
        ++it;
        CHECK ( it->first == 1430174422 );
        CHECK ( it->second ==  17.1 );       
    }
}

TEST_CASE ("process_db_measurement_calculate","[agent-cm][computation][average][db]")
{
    std::map <int64_t, double> samples;
    double result;

    samples.emplace (std::make_pair (1430240580, 12.3 )); // 2015-04-28 17:03:00
    samples.emplace (std::make_pair (1430150400, 12.4 )); // 2015-04-27 16:00:00
    samples.emplace (std::make_pair (1430141295, 12.5 )); // 2015-04-27 13:28:15
    samples.emplace (std::make_pair (1430141537, 12.7 )); // 2015-04-27 13:32:17
    samples.emplace (std::make_pair (1430063833, 12.6 )); // 2015-04-26 15:57:13

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
        samples. clear ();
        CHECK ( process_db_measurement_calculate (samples, 1, 2, "min", result) == -1 );
        samples.emplace (std::make_pair (1000, 12.231));
        CHECK ( process_db_measurement_calculate (samples, 3, 2, "min", result) == -1 );
        CHECK ( process_db_measurement_calculate (samples, 1430174410, 1430174410, "min", result) == -1 );
        CHECK ( process_db_measurement_calculate (samples, 1430174410, 1430174409, "min", result) == -1 );
        CHECK ( process_db_measurement_calculate (samples, 500, 1500, "mix", result) == -1 );
        CHECK ( process_db_measurement_calculate (samples, 500, 1500, "man", result) == -1 );
        CHECK ( process_db_measurement_calculate (samples, 500, 1500, "arithmetic mean", result) == -1 );
        CHECK ( process_db_measurement_calculate (samples, -1, 2, "min", result) == -1 );
        CHECK ( process_db_measurement_calculate (samples, 1, -2, "min", result) == -1 );
        CHECK ( process_db_measurement_calculate (samples, -5, -2, "min", result) == -1 );
    }

    SECTION ("min") {
        CHECK ( process_db_measurement_calculate (samples, 1430121600, 1430150400, "min", result) == 0 ); // 2015-04-27 08:00:00 -> 2015-04-27 16:00:00 8h
        CHECK ( result == 12.5 );

        std::size_t size = samples.size ();
        CHECK ( process_db_measurement_calculate (samples, 1430140500, 1430141400, "min", result) == 0 ); // 2015-04-27 13:15:00 -> 2015-04-27 13:30:00 30m
        CHECK ( result == 12.5 );
        CHECK ( samples.size () == size + 1 );
        CHECK ( samples.at (1430141400) == 12.5 );

        size = samples.size ();
        CHECK ( process_db_measurement_calculate (samples, 1430035200, 1430064000, "min", result) == 0 ); // 2015-04-26 08:00:00 -> 2015-04-26 16:00:00 8h
        CHECK ( result == 4 );
        CHECK ( samples.size () == size );

        size = samples.size ();
        CHECK ( process_db_measurement_calculate (samples, 1430017200, 1430018100, "min", result) == 0 ); // 2015-04-26 03:00:00 -> 2015-04-26 03:15:00 15m
        CHECK ( result == 10 );
        CHECK ( samples.size () == size );

        size = samples.size ();
        CHECK ( process_db_measurement_calculate (samples, 1430017200, 1430018100, "min", result) == 0 ); // 2015-04-26 03:00:00 -> 2015-04-26 03:15:00 15m
        CHECK ( result == 10 );
        CHECK ( samples.size () == size );       

        size = samples.size ();
        CHECK ( process_db_measurement_calculate (samples, 1430035200, 1430064000, "min", result) == 0 ); // 2015-04-26 08:00:00 -> 2015-04-26 16:00:00 8h
        CHECK ( result == 4 );
        CHECK ( samples.size () == size );       

        size = samples.size ();
        CHECK ( process_db_measurement_calculate (samples, 1429930800, 1429931700, "min", result) == 0 ); // 2015-04-25 03:00:00 -> 2015-04-25 03:15:00 15m
        CHECK ( result == 10 );
        CHECK ( samples.size () == size + 1 );
        CHECK ( samples.at(1429931700) == 15 );

        size = samples.size ();
        CHECK ( process_db_measurement_calculate (samples, 1429948800, 1429977600, "min", result) == 0 ); // 2015-04-25 08:00:00 -> 2015-04-25 16:00:00 8h
        CHECK ( result == 4 );
        CHECK ( samples.size () == size + 1 );
        CHECK ( samples.at(1429977600) == 50 );

        size = samples.size ();
        CHECK ( process_db_measurement_calculate (samples, 1429890300, 1429891200, "min", result) == 0 ); // 2015-04-24 15:45:00 -> 2015-04-24 16:00:00 15m
        CHECK ( result == 4 );
        CHECK ( samples.size () == size );       
    }
    
    SECTION ("max") {
        CHECK ( process_db_measurement_calculate (samples, 1430121600, 1430150400, "max", result) == 0 ); // 2015-04-27 08:00:00 -> 2015-04-27 16:00:00 8h
        CHECK ( result == 12.7);

        std::size_t size = samples.size ();
        CHECK ( process_db_measurement_calculate (samples, 1430140500, 1430141400, "max", result) == 0 ); // 2015-04-27 13:15:00 -> 2015-04-27 13:30:00 30m
        CHECK ( result == 12.5 );
        CHECK ( samples.size () == size + 1 );
        CHECK ( samples.at (1430141400) == 12.5 );

        size = samples.size ();
        CHECK ( process_db_measurement_calculate (samples, 1430035200, 1430064000, "max", result) == 0 ); // 2015-04-26 08:00:00 -> 2015-04-26 16:00:00 8h
        CHECK ( result == 170 );
        CHECK ( samples.size () == size );

        size = samples.size ();
        CHECK ( process_db_measurement_calculate (samples, 1430017200, 1430018100, "max", result) == 0 ); // 2015-04-26 03:00:00 -> 2015-04-26 03:15:00 15m
        CHECK ( result == 100 );
        CHECK ( samples.size () == size );

        size = samples.size ();
        CHECK ( process_db_measurement_calculate (samples, 1430035200, 1430064000, "max", result) == 0 ); // 2015-04-26 08:00:00 -> 2015-04-26 16:00:00 8h
        CHECK ( result == 170 );
        CHECK ( samples.size () == size );

        size = samples.size ();
        CHECK ( process_db_measurement_calculate (samples, 1429930800, 1429931700, "max", result) == 0 ); // 2015-04-25 03:00:00 -> 2015-04-25 03:15:00 15m
        CHECK ( result == 100 );
        CHECK ( samples.size () == size + 1 );
        CHECK ( samples.at(1429931700) == 15 );       

        size = samples.size ();
        CHECK ( process_db_measurement_calculate (samples, 1429948800, 1429977600, "max", result) == 0 ); // 2015-04-25 08:00:00 -> 2015-04-25 16:00:00 8h
        CHECK ( result == 50 );
        CHECK ( samples.size () == size + 1 );
        CHECK ( samples.at(1429977600) == 50 );

        size = samples.size ();
        CHECK ( process_db_measurement_calculate (samples, 1429890300, 1429891200, "max", result) == 0 ); // 2015-04-24 15:45:00 -> 2015-04-24 16:00:00 15m
        CHECK ( result == 50 );
        CHECK ( samples.size () == size );
    }
    
    SECTION ("empty interval") {
        CHECK ( process_db_measurement_calculate (samples, 1430208000, 1430236800, "min", result) == 1 ); // 2015-04-28 08:00:00 -> 2015-04-28 16:00:00
        CHECK ( process_db_measurement_calculate (samples, 1430208000, 1430236800, "max", result) == 1 ); // 2015-04-28 08:00:00 -> 2015-04-28 16:00:00
        CHECK ( process_db_measurement_calculate (samples, 1430208000, 1430236800, "arithmetic_mean", result) == 1 ); // 2015-04-28 08:00:00 -> 2015-04-28 16:00:00

        CHECK ( process_db_measurement_calculate (samples, 1430149500, 1430150400, "min", result) == 1 ); // 2015-04-27 15:45:00 -> 2015-04-27 16:00:00
        CHECK ( process_db_measurement_calculate (samples, 1430149500, 1430150400, "max", result) == 1 ); // 2015-04-27 15:45:00 -> 2015-04-27 16:00:00
        CHECK ( process_db_measurement_calculate (samples, 1430149500, 1430150400, "arithmetic_mean", result) == 1 ); // 2015-04-27 15:45:00 -> 2015-04-27 16:00:00

    }
/* TODO: Write simple isolated cases here, bigger ones in ci script
    SECTION ("arithmetic_mean") {
    }
*/

}


