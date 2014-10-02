//cli

#include "defs.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

const char *safe_str(const char *s) {
    return s == NULL ? "(null)" : s;
}

int main(int argc, char** argv) {

    if (argc == 1) {
        fprintf(stderr, "ERROR: at least one command needed {start|stop|kill}\n");
        exit(1);
    }

    const char* cmd = argv[1];

    zsock_t *cli_sock = zsock_new_req(CLI_SOCK);
    assert(cli_sock);

    if (streq(cmd, "start")) {
        zstr_send(cli_sock, "START");
    } else if (streq(cmd, "stop")) {
        zstr_send(cli_sock, "STOP");
    } else if (streq(cmd, "kill")) {
        zstr_send(cli_sock, "$TERM");
    }
    else {
        zstr_send(cli_sock, cmd);
    }
    char* resp = zstr_recv(cli_sock);
    printf("CLI: for reply '%s'\n", safe_str(resp));
    free(resp);

    zsock_destroy(&cli_sock);
    return 0;

}
