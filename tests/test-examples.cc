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
 * \file test-examples.cc
 * \author Alena Chernikava
 * \brief Not yet documented file
 */
#include "catch.hpp"    //include catch as a first line
//#include <stdio.h>
//#include <stdbool.h>
//#include <stdlib.h>

#include "cli.h"
/* Examples where taken from https://github.com/philsquared/Catch */

//BE SURE YOU DON'T HAVE HERE(or in any included file) ANY main FUNCTION

//If you have some strange error started with "In function ‘void ____C_A_T_C_H____T_E_S_T .... ", probably you have some syntax error. For example missing ";" or "{".

//Try to avoid global variables, while tests in future could be done concurrently

//This global variable is used only in comparison
static struct global_opts gopts = {
    .show_help = false,
    .verbosity = 0,
    .use_pager = true
};

bool is_equal_gl_opts(struct global_opts *opts_before, struct global_opts *opts_after){
    if (opts_before->show_help != opts_after->show_help)
        return false;
    else if (opts_before->verbosity != opts_after->verbosity)
        return false;
    else if (opts_before->use_pager != opts_after->use_pager)
        return false;
    else 
       return true;
}

void copy_gl_opts(struct global_opts *from, struct global_opts *to){
    to->show_help = from->show_help;
    to->verbosity = from->verbosity;
    to->use_pager = from->use_pager;
}
//Every word from the test framework must be written in capitals (REQUIRE,CHECK,GIVEN, ....)
//In one file could be more than one SCENARIO and TEST_CASE
//If you name your test-cases and sections appropriately you can achieve a BDD-style specification structure
//TEST_CASE/SCENARIO    name    - name of the test
//GIVEN                 name    - name of the initial conditions
//WHEN                  name    - name of the action to control
//THEN                  name    - name of the expected outcome 

//As a basic assertion you could use REQUIRE/REQUIRE_FALSE/CHECK/CHECK_FALSE
//More example are provided in the end of file
// 1. Nested AND_TEST
// 2. Double comparison
// 3. FAIL/SUCCEED macro
// 4. CAPTURE/WARN/INFO
// 5. REQUIRE_THROWS/REQUIRE_THROWS_AS/REQUIRE_NOTHROW/CHECK_THROWS/CHECK_THROWS_AS/CHECK_NOTHROW
// 6. Using loops


//The TEST_CASE (or SCENARIO) name must be unique. You can run sets of tests by specifying a wildcarded test name or a tag expression
//tag [.] means that this test is hidden. It isn't executed by default.
//              name                            tags(every tag in [])
SCENARIO("Test handle_global_options","[hndl_glbl_ptns][cli]"){

    //don't write any code between SCENARIO-section and GIVEN-section
    //fprinf prints a message somewhere after every GIVEN-section
    
    //in one scenario it is possible to have more that one GIVEN-section
    GIVEN("argc = 0     and     argv = NULL"){

        //define initial conditions(they are common for the all following WHEN-sections in this GIVEN-section)
        int argc0 = 0;
        const char ** argv0 = NULL;
        struct global_opts gopts0;
        copy_gl_opts(&gopts,&gopts0);
        //if you use some other functions for the initialization it is possible in the GIVEN-section to control some conditions to be sure they match
        //REQUIRE(argc0 == 0);
        //REQUIRE(argv0 == NULL);

        //it is possible to have more independent WHEN-sections in one GIVEN-section
        WHEN("Handle_global_options used"){
    
            //some actions
            //It is a bad idea to write here(in WHEN-section) some REQUIRE or CHECK statements
            int optind = handle_global_options(argc0, argv0, &gopts0);
            
            //it is possible to have more than one THEN-section, but it is not recommended. 
            //If you have to check more then one outcome, than you can 
            //  1. Use more REQUIRE(or CHECK) in one THEN-section
            //  2. Use nested AND_THEN("somename"){ ...} for the 2+ outcomes. But for the first outcome use THEN-section. (you can find example somewhere here downwards)
            //
            //Also it is possible to use the nested AND_WHEN-section in the WHEN-,THEN-,AND_THEN-sections. (you can find example somewhere here downwards)
            THEN("return value is 0"){ 
                //control expected outcome
                REQUIRE(optind == 0);
                REQUIRE(is_equal_gl_opts(&gopts,&gopts0));
            }
        }
    }       


    // use the following code as a simple example of GIVEN-section
    GIVEN("argc < 0 and     argv = NULL"){

        int argc1 = -42;
        const char ** argv1 = NULL;
        struct global_opts gopts0;
        copy_gl_opts(&gopts,&gopts0);

        WHEN("Handle_global_options used"){
            int optind = handle_global_options(argc1, argv1, &gopts0);

            THEN("return value is 0"){
                REQUIRE(optind == 0);
                REQUIRE(is_equal_gl_opts(&gopts,&gopts0));
            }
        }
    }

    //example for the nested AND_THEN-section
    GIVEN("no arguments to proceed at all"){
        const char *argv5[] = {"cli", NULL};
        struct global_opts gopts0;
        copy_gl_opts(&gopts,&gopts0);

        WHEN("Handle_global_options used"){
            int optind = handle_global_options(1, argv5, &gopts0);
        
            THEN("return value is 1"){
                REQUIRE(optind == 1);
                
                AND_THEN("verbosity remain 0"){
                    REQUIRE(gopts0.verbosity == 0);
                
                    AND_THEN("use-pager remain true"){
                        REQUIRE(gopts0.use_pager == true);
                    }
                }
            }
        }
    }

    //example for not-nested THEN
    GIVEN("test one cmd"){
        const char *argv6[] = {"cli", "cmd", NULL};
        struct global_opts gopts0;
        copy_gl_opts(&gopts,&gopts0);
 
        WHEN("Handle_global_options used"){
            int optind = handle_global_options(2, argv6, &gopts0);
        
            THEN("return value is 1, verbosity remain 0 , use-pager remain true"){
                REQUIRE(optind == 1);
                REQUIRE(gopts0.verbosity == 0);
                REQUIRE(gopts0.use_pager == true);
            }
        }
    }

}


//Example nested AND_WHEN. It is possible to use the nested AND_WHEN-section in the WHEN-,THEN-,AND_THEN-sections. 
/*
    GIVEN("initial conditions") {
        // set_initial_conditions(); 
        
        WHEN("smth is done") {
            //  do_some_action(); 
        
            THEN( "smth must hold" ) {
                REQUIRE( //some condition );
                
                AND_WHEN( "and if smth1 done again" ) {
                    // do_some_action1();
                    
                    THEN( "smth must hold" ) {
                        REQUIRE( // some condition );
                    }
                }
            }
        }
    }
*/


//Example double comparison
/* CATCH framework provide a special class Aprox for the comparison of the numbers(float and double)
   Examples are copied from the https://github.com/philsquared/Catch
   default epsilon is std::numeric_limits<float>::epsilon()*100 */
  
TEST_CASE("TC: Approximate comparisons with mixed numeric types","[Approx][.][template]"){
    const double dZero = 0;
    const double dSmall = 0.00001;
    const double dMedium = 1.234;
    REQUIRE( 1.0f == Approx( 1 ) );
    REQUIRE( 0 == Approx( dZero) );
    REQUIRE( 0 == Approx( dSmall ).epsilon( 0.001 ) );
    REQUIRE( 1.234f == Approx( dMedium ) );
    REQUIRE( dMedium == Approx( 1.234f ) );
}

TEST_CASE("TC: Use a custom approx","[Approx][custom][.][template]"){
    double d = 1.23;
    Approx approx = Approx::custom().epsilon( 0.005 );
    REQUIRE( d == approx( 1.23 ) );
    REQUIRE( d == approx( 1.22 ) );
    REQUIRE( d == approx( 1.24 ) );
    REQUIRE( d != approx( 1.25 ) );
    REQUIRE( approx( d ) == 1.23 );
    REQUIRE( approx( d ) == 1.22 );
    REQUIRE( approx( d ) == 1.24 );
    REQUIRE( approx( d ) != 1.25 );
}

/* if you need to abort the test_case in any case you could use FAIL */
TEST_CASE("TC: fail","[.][fail][template]"){
    /* do smth */
    FAIL("If you see this than smth unbelievable had happen"); //fail and aborts the test. FAIL() without arguments works also.
}

/* if the test_case must not fail, than you can use SUCCEED*/
TEST_CASE("TC: succeed","[.][succeed][template]"){
    /* do smth */
    int i =9;
    CAPTURE( i);                //CAPTURE behaves as INFO (if test passed, then message isn't displayed), but it is more comfortable way of displaying variables value.
    SUCCEED("this must always be ok");  //succeeds the test. SUCCEED() without arguments works also
}

/* there is two types of messages you can use INFO (or special case for variables CAPTURE), WARN*/
TEST_CASE("TC: INFO and WARN do not abort tests", "[messages][.][template]" ){
    INFO( "this is a " << "message" ); // This should output the message if a failure occurs. But this message you could see also if you run tests with -s parameter in command line. (show all PASSED tests)
    WARN( "this is a " << "warning" ); // This should always output the message but then continue
}

/* Shows how the INFO works*/
TEST_CASE("TC: INFO gets logged on failure, even if captured before successful assertions", "[failing][messages][.][template]" ){
    INFO( "this message may be logged later" ); /* while the following CHECK passes */
    int a = 2;
    CHECK( a == 2 );
    INFO( "this message should be logged" );    /* while the following CHECK fails */
    CHECK( a == 1 );
    INFO( "and this, but later" );
    CHECK( a == 0 );
    INFO( "but not this" );
    CHECK( a == 2 );
}


/* Some note on pointers*/
TEST_CASE("TC: Pointers can be converted to strings", "[messages][.][template]" )
{
    int p;
    WARN( "actual address of p: " << &p );
    WARN( "toString(p): " << Catch::toString( &p ) );
}


/* Using exceptions*/
namespace
{
    inline int thisThrows(){
        if( Catch::alwaysTrue() )
            throw std::domain_error( "expected exception" );
        return 1;
    }
    int thisDoesntThrow(){
        return 0;
    }
}

TEST_CASE("TC: When checked exceptions are thrown they can be expected or unexpected", "[.][template]" ){
    REQUIRE_THROWS_AS( thisThrows(), std::domain_error );
    REQUIRE_NOTHROW( thisDoesntThrow() );
    REQUIRE_THROWS( thisThrows() );
}

TEST_CASE("TC: Expected exceptions that don't throw or unexpected exceptions fail the test", "[.][failing][template]" ){
    CHECK_THROWS_AS( thisThrows(), std::string );
    CHECK_THROWS_AS( thisDoesntThrow(), std::domain_error );
    CHECK_NOTHROW( thisThrows() );
}

TEST_CASE("TC: When unchecked exceptions are thrown, but caught, they do not affect the test", "[.][template][exc]" ){
    try{
        throw std::domain_error( "unexpected exception" );
    }
    catch(...){
    }
}

/* using loops. Inside the loop it is forbidden to use SECTION/GIVEN/WHEN/THEN.*/
TEST_CASE("TC: looped SECTION tests", "[.][failing][sections][template]" ){
    int a = 1;
    for( int b = 0; b < 10; ++b ){
        INFO( "check #" << b);
        CHECK( b < a );
    }
}
