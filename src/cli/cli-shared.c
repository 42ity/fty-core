// TODO MVY (?): Missing license etc..
#include <stdio.h>
#include <assert.h>

#include "utils.h"
#include "cli.h"

int
handle_global_options(
const int argc, const char **argv, struct global_opts *gopts) {
  
  int i = 1;
  const char *cmd = NULL;

  if (argc < 1 || argv == NULL || gopts == NULL || argv[argc] != NULL) {
    // TODO ? (?): log
    fprintf(stderr,
            "ERROR: bad arguments. func:'%s'. file:'%s', line:'%d'.\n",
            __func__, __FILE__, __LINE__);
    return HANDLE_GLOBAL_OPTIONS_BAD_INPUT;
  }

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

