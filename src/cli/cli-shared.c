#include <stdio.h>

#include "utils.h"
#include "cli.h"

int handle_global_options(const int argc, const char **argv, struct global_opts *gopts) {
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

const struct command* get_builtin_command(const struct command *commands, const char *name) {
 
    struct command *p;

	if (!name) {
		return NULL;
	}

    for(p = (struct command*) commands; p->command != NULL; p++) {
        if (streq(p->command, name)) {
            return (const struct command*) p;
        }
    }
	return NULL;
}

