/* cli.c: command line interface - main part
 
Copyright (C) 2014 Eaton
 
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
Author(s): Michal Vyskocil <michalvyskocil@eaton.com>
 
Description: command line interface, global argument handling, subcommand execution
References: BIOS-245
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <assert.h>

#include "utils.h"
#include "cli.h"

/*! \brief dump struct global_opts */
static void dump_global_opts(const struct global_opts *g) {
    fprintf(stderr, "struct global_opts {\n");
    fprintf(stderr, "    .show_help = %s,\n", str_bool(g->show_help));
    fprintf(stderr, "    .verbosity = %d,\n", g->verbosity);
    fprintf(stderr, "    .use_pager = %s,\n", str_bool(g->use_pager));
    fprintf(stderr, "}\n");
}

static struct global_opts gopts = {
    .show_help = false,
    .verbosity = 0,
    .use_pager = true
};

static const struct command builtin_commands[] = {
    { "network", "Get, Add, Remove networks to scan", do_network },
    {NULL, NULL, NULL},
};

const char cli_usage_string[] = 
    "cli [-h|--help] command [<args>]";

static void do_usage() {
    fputs(cli_usage_string, stderr);
    fputc('\n', stderr);
}

static void do_help() {
    fputs(cli_usage_string, stderr);

	fputs("\n\nList of available commands:\n", stderr);
	struct command *c = (struct command*) builtin_commands;
	while (c->command) {
		fprintf(stderr, "    %s\t%s\n", c->command, c->description);
		c++;
	}
}

/*! \brief Parse global options
 *         ie those up first string without initial '-'
 *
 *  \param argc argument count
 *  \param argv and array of strings with arguments
 *  \param gopts pointer to structure containing the global options - things will be handled there
 *
 *  \return negative, zero or positive. Where negative is an index to first
 *   unknown argument, zero means bad input arguments or --help
 *   (gopts->show_help will be true) and positive is an index to first command
 *   in argv.
 */

static int handle_global_options(const int argc, const char **argv, struct global_opts *gopts) {
    int i = 1;
    const char *cmd = NULL;

    if (argc < 1 || argv == NULL || gopts == NULL || argv[argc] != NULL) {
        return 0;
    }

    while (i != argc) {
        cmd = argv[i];
        if (cmd[0] != '-') {
            break;
        }

        if (streq("--help", cmd)) {
            gopts->show_help =true;
            return 0;
        }
        else if (streq("--verbose", cmd)) {
            gopts->verbosity = 1;
        }
        else if (streq("-v", cmd)) {
            gopts->verbosity = 1;
        }
        else if (streq("-vv", cmd)) {
            gopts->verbosity = 2;
        }
        else if (streq("-vvv", cmd)) {
            gopts->verbosity = 3;
        }
        else if (streq("--no-pager", cmd)) {
            fprintf(stderr, "WARNING: use_pager is not implemented!\n");
            gopts->use_pager = false;
        }
        else {
            return -i;
        }

        i += 1;
    }
    return i;
}

/*! Get builtin command
 * 
 * \param name name of command to find in global builtin_commands array
 * \return pointer to struct command or NULL
 */
static const struct command* get_builtin_command(const char *name) {
 
    struct command *p;

	if (!name) {
		return NULL;
	}

    for(p = (struct command*) builtin_commands; p->command != NULL; p++) {
        if (streq(p->command, name)) {
            return (const struct command*) p;
        }
    }
	return NULL;
}

int main (int argc, char **argv) {
    int optind;
	
    optind = handle_global_options(argc, (const char**) argv, &gopts);

	if (optind == 0 || !argv[optind]) {
		if (!gopts.show_help) {
			do_usage();
			exit(EXIT_FAILURE);
		}
		do_help();
		exit(EXIT_SUCCESS);
	}
	else if (optind < 0) {
		fprintf(stderr, "ERROR: unknown global argument '%s'\n", argv[-optind]);
		exit(EXIT_FAILURE);
	}
	else {
		const struct command *cmd = get_builtin_command(argv[optind]);

		if (cmd == NULL) {
				fprintf(stderr, "ERROR: unknown command '%s'\n", argv[optind]);
				exit(EXIT_FAILURE);
		}
		return cmd->do_command((const int)(argc - optind), (const char**) argv+optind, &gopts);
	}

	fprintf(stderr, "ERROR: Can't reach here!\n");
	exit(EXIT_FAILURE);

}

void test() {

    /* unit tests */
    
    /* test invariants: argc > 0 */
    optind = handle_global_options(0, NULL, &gopts);
    assert(optind == 0);
    optind = handle_global_options(-42, NULL, &gopts);
    assert(optind == 0);
    
    /* test invariants: argv != NULL */
    optind = handle_global_options(5, NULL, &gopts);
    assert(optind == 0);
    
    /* test invariants: gopts != NULL */
    const char *argv1[] = {"cli", "--foo", "--bar", NULL};
    optind = handle_global_options(3, argv1, NULL);
    assert(optind == 0);
    
    /* test invariants: argv[argc] == NULL */
    const char *argv2[] = {"cli", "--foo", "--bar"};
    optind = handle_global_options(3, argv2, &gopts);
    assert(optind == 0);

    /* test no arguments */
    const char *argv3[] = {"cli", NULL};
    optind = handle_global_options(1, argv3, &gopts);
    assert(optind == 1);
    assert(gopts.verbosity == 0);
    assert(gopts.use_pager == true);
    
    /* test one cmd */
    const char *argv4[] = {"cli", "cmd", NULL};
    optind = handle_global_options(2, argv4, &gopts);
    assert(optind == 1);
    assert(gopts.verbosity == 0);
    assert(gopts.use_pager == true);
    
    /* test --verbosity cmd */
    const char *argv5[] = {"cli", "--verbose", NULL};
    optind = handle_global_options(2, argv5, &gopts);
    assert(optind == 2);
    assert(gopts.verbosity == 1);
    assert(gopts.use_pager == true);
    
    /* test -vv cmd */
    const char *argv6[] = {"cli", "-vv", NULL};
    optind = handle_global_options(2, argv6, &gopts);
    assert(optind == 2);
    assert(gopts.verbosity == 2);
    assert(gopts.use_pager == true);
    
    /* test -vv --no-pager cmd */
    const char *argv7[] = {"cli", "-vvv", "--no-pager", NULL};
    optind = handle_global_options(3, argv7, &gopts);
    assert(optind == 3);
    assert(gopts.verbosity == 3);
    assert(gopts.use_pager == false);

    /* test unknown global argument */
    const char *argv8[] = {"cli", "--unknown", NULL};
    optind = handle_global_options(2, argv8, &gopts);
    assert(optind == -1);
    fprintf(stderr, "ERROR: unknown option %s\n", argv8[-optind]);
    assert(gopts.verbosity == 3); /* this is just shared state between tests, lets handle that in a framework*/
    assert(gopts.use_pager == false); /* ditto */


    exit (0);
}
