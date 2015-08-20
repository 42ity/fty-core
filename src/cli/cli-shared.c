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
 * \file cli-shared.c
 * \author Karol Hrdina
 * \author Michal Vyskocil
 * \author Michal Hrusecky
 * \author Alena Chernikava
 * \brief Not yet documented file
 */
// TODO MVY (?): Missing license etc..
#include <stdio.h>
#include <assert.h>

#include "utils.h"
#include "cli.h"

int
handle_global_options(
const int argc, const char **argv, struct global_opts *gopts) {
  
  int i = 1;

  if (argc < 1 || argv == NULL || gopts == NULL || argv[argc] != NULL) {
    // TODO ? (?): log
    fprintf(stderr,
            "ERROR: bad arguments. func:'%s'. file:'%s', line:'%d'.\n",
            __func__, __FILE__, __LINE__);
    return HANDLE_GLOBAL_OPTIONS_BAD_INPUT;
  }

  const char *cmd = NULL;
  while (i != argc) {
    cmd = argv[i];
    if (cmd[0] != '-') {
      break;
    }
    
    if (streq("--help", cmd) || streq("-h", cmd)) {
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
      fprintf(stderr, "%s\n", MSG_E_USER_PAGE_NOT_IMPLEMENTED);

      gopts->use_pager = false;
    }
    else {
      return -i;
    }

    i += 1;
  }
  return i;
}

const struct command*
get_builtin_command(const struct command *commands, const char *name) {
  
  assert(name != NULL);
	if (name == NULL) {
    // TODO ? (?): log
    fprintf(stderr,
            "ERROR: argument 'name' is NULL. func:'%s', file:'%s', line:'%d'.\n",
            __func__, __FILE__, __LINE__);
    // TODO MVY (?): From code review upon request:
    //               Maybe it's better to fail than to return NULL
		return NULL;
	}
  assert(commands != NULL);
  if (commands == NULL) {
    // TODO ? (?): log
    fprintf(stderr,
            "ERROR: argument 'commands' is NULL. func:'%s', file:'%s', line:'%d'.\n",
            __func__, __FILE__, __LINE__);
    return NULL;
  }
  struct command *p = NULL;

  for (p = (struct command*) commands; p->command != NULL; p++) {
    if (streq(p->command, name)) {
      return (const struct command*) p;
    }
  }
	return NULL;
}

