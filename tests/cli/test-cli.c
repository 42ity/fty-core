#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "cli.h"

static struct global_opts gopts = {
    .show_help = false,
    .verbosity = 0,
    .use_pager = true
};

int main() {

    int optind = 0;
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
