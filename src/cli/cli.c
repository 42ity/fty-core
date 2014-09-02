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
#include <assert.h>

#include "log.h"
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

int main(int argc, char **argv) {

  int index = 0,
      i = 0;
  index = handle_global_options(argc, (const char**) argv, &gopts);
#ifndef NDEBUG
  log_debug("####main()\t####\n");
  log_debug("# argc: '%d'\n# index: '%d'\n", argc, index);
  log_debug("# **argv:\n");
  for (i = 0; i < argc; i++) {
    log_debug("#\t[ %d ]\t%s\n", i, argv[i]);
  }
  log_debug("\n");
#endif
	if (index == HANDLE_GLOBAL_OPTIONS_BAD_INPUT || !argv[index]) {
		if (!gopts.show_help) {
			do_usage();
			exit(EXIT_FAILURE);
		}
		do_help();
		exit(EXIT_SUCCESS);
	}
	else if (index < 0) {
		log_error("ERROR: unknown global argument '%s'\n", argv[-index]);
		exit(EXIT_FAILURE);
	}
	else {
		const struct command *cmd = get_builtin_command(builtin_commands, argv[index]);

		if (cmd == NULL) {
				log_error("unknown command '%s'\n", argv[index]);
				exit(EXIT_FAILURE);
		}
		int ret = cmd->do_command((const int)(argc - index),
                              (const char**) argv+index,
                               &gopts);
    exit(ret);
	}
#ifndef NDEBUG
	log_critical("Can't reach here!\n");
#endif
	exit(EXIT_FAILURE);

}
