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
 * \file test-cli.cc
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \brief Not yet documented file
 */
#include "catch.hpp"    
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "cli.h"

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

SCENARIO("Test handle_global_options","[hndl_glbl_ptns][cli]"){

    GIVEN("argc = 0     and     argv = NULL"){

        int argc0 = 0;
        const char ** argv0 = NULL;
        struct global_opts gopts0;
        copy_gl_opts(&gopts,&gopts0);

        WHEN("Handle_global_options used"){
    
            int optind = handle_global_options(argc0, argv0, &gopts0);
            
            THEN("return value is 0"){ 
                REQUIRE(optind == 0);
                REQUIRE(is_equal_gl_opts(&gopts,&gopts0));
            }
        }
    }       


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

    GIVEN("argv = NULL"){
        int argc2 = 5;
        const char ** argv2 = NULL;
        struct global_opts gopts0;
        copy_gl_opts(&gopts,&gopts0);
        
        WHEN("Handle_global_options used"){
            int optind = handle_global_options(argc2, argv2, &gopts0);
            
            THEN("return value is 0"){
                REQUIRE(optind == 0);
                REQUIRE(is_equal_gl_opts(&gopts,&gopts0));
            }
        }
    }
    
    GIVEN("gopts = NULL"){
        const char *argv3[] = {"cli", "--foo", "--bar", NULL};
        struct  global_opts *gopts3 = NULL;
            
        WHEN("Handle_global_options used"){
            int optind = handle_global_options(3, argv3, gopts3);
            
            THEN("return value is 0"){
                REQUIRE(optind == 0);
            }
        }       
    }

    GIVEN("argv[arc] != NULL"){
        const char *argv4[] = {"cli", "--foo", "--bar"};
        int argc4 = 2;
        struct global_opts gopts0;
        copy_gl_opts(&gopts,&gopts0);

        WHEN("Handle_global_options used"){
            int optind = handle_global_options(argc4, argv4, &gopts0);

            THEN("return value is 0"){
                REQUIRE(optind == 0);
                REQUIRE(is_equal_gl_opts(&gopts,&gopts0));

            }
        }
    }

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

    GIVEN("one argument --verbose"){
        const char *argv7[] = {"cli", "--verbose", NULL};
        struct global_opts gopts0;
        copy_gl_opts(&gopts,&gopts0);

        WHEN("test --verbosity cmd"){
            int optind = handle_global_options(2, argv7, &gopts0);
        
            THEN("return value is 2, verbosity become 1 , use-pager remain true"){
                REQUIRE(optind == 2);
                REQUIRE(gopts0.verbosity == 1);
                REQUIRE(gopts0.use_pager == true);
            }
        }
    }

    GIVEN(" one argument -vv"){
        const char *argv8[] = {"cli", "-vv", NULL};
        struct global_opts gopts0;
        copy_gl_opts(&gopts,&gopts0);

        WHEN(" test -vv cmd "){
            int optind = handle_global_options(2, argv8, &gopts0);

            THEN("return value is 2, verbosity become 2 , use-pager remain true"){
                REQUIRE(optind == 2);
                REQUIRE(gopts0.verbosity == 2);
                REQUIRE(gopts0.use_pager == true);
            }
        }
    }
    
    GIVEN(" two arguments -vvv --no-paper"){
        const char *argv10[] = {"cli", "-vvv", "--no-pager", NULL};
        struct global_opts gopts0;
        copy_gl_opts(&gopts,&gopts0);
    
        WHEN(" test -vv --no-pager cmd "){
            int optind = handle_global_options(3, argv10, &gopts0);
    
            THEN("return value is 3, verbosity become 3 , use-pager become false"){
                REQUIRE(optind == 3);
                REQUIRE(gopts0.verbosity == 3);
                REQUIRE(gopts0.use_pager == false);
            }
        }
    }
    
    GIVEN(" one argument --unknown"){
        const char *argv11[] = {"cli", "--unknown", NULL};
        struct global_opts gopts0;
        copy_gl_opts(&gopts,&gopts0);
        WHEN(" test unknown global argument "){
            int optind = handle_global_options(2, argv11, &gopts0);
            
            THEN("return value is -1, verbosity remain 0 , use-pager remain true"){
                REQUIRE(optind == -1);
                //  fprintf(stderr, "ERROR: unknown option %s\n", argv11[-optind]);
                REQUIRE(gopts0.verbosity == 0);
                // this is just shared state between tests, lets handle that in a framework
                REQUIRE(gopts0.use_pager == true);//  ditto 
            }
        }
    }
}

