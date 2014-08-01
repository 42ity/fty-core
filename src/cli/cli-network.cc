/* cli-network.c: command line interface - network command
 
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
 
Description: network CLI command
References: BIOS-245, BIOS-126
*/

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <getopt.h>

#include <jsoncpp/json/json.h>
#include "utils.h"
#include "cli.h"

struct network_opts {
    bool show_details;
};

const char *msg = 
    "{\"<FAKE-RC-ID>\" : ["
    "{\"type\":\"AUTOMATIC\",\"ipaddr\":\"10.130.38.0\",\"mac\":\"a0:1d:48:b7:e2:4e\",\"name\":\"enp0s25\",\"prefixlen\":24},"
    "{\"type\":\"MANUAL\",\"ipaddr\":\"10.0.0.0\",\"mac\":\"---\",\"name\":\"---\",\"prefixlen\":8},"
    "{\"type\":\"DELETED\",\"ipaddr\":\"fe80::a21d:48ff:feb7:e24e\",\"mac\":\"de:ad:be:ef:e2:42\",\"name\":\"wlo1\",\"prefixlen\":64}]}";

static struct network_opts opts = {
    .show_details = false
};

static struct option long_options[] = {
    /* These options set a flag. */
    {"details", no_argument,       0, 'd'},
    {0, 0, 0, 0}
};

static const char* str_enum_ValueType(Json::ValueType vt) {
    switch (vt) {
        case Json::ValueType::nullValue:
            return "'null' value"; break;
        case Json::ValueType::intValue:
            return "signed integer value"; break;
        case Json::ValueType::uintValue:
            return "unsigned integer value"; break;
        case Json::ValueType::realValue:
            return "double value"; break;
        case Json::ValueType::stringValue:
            return "UTF-8 string value"; break;
        case Json::ValueType::booleanValue:
            return "bool value"; break;
        case Json::ValueType::arrayValue:
            return "array value (ordered list)"; break;
        case Json::ValueType::objectValue:
            return "object value (collection of name/value pairs)"; break;
    }
}

static char char_type(Json::Value &v) {

    if (v == "AUTOMATIC") {
        return 'A';
    }
    else if (v == "DELETED") {
        return 'D';
    }
    else if (v == "MANUAL") {
        return 'M';
    }
    return '?';
}

static int do_network_list(FILE *out, const char* json_msg, const struct global_opts *gopts) {
	assert(out);

    bool res;
    Json::Reader rd{};
    Json::Value  root{Json::ValueType::arrayValue};

    res = rd.parse(json_msg, root, false);
    if (!res) {
        fprintf(stderr, "ERROR: invalid JSON message!\n%s\n\n", msg);
        exit(EXIT_FAILURE);
    }

    assert(rd.getFormattedErrorMessages().length() == 0); // no error messages during parsing
    assert(root.isObject());

    for (auto rcid : root.getMemberNames()) {

        fputs(rcid.c_str(), stdout);
        fputs("\n", stdout);

        for (auto entry : root[rcid]) {

            assert(entry.isObject());

            fprintf(stdout, "    %c %s/%d %s %s\n",
                    char_type(entry["type"]),
                    entry["ipaddr"].asCString(),
                    entry["prefixlen"].asInt(),
                    entry["name"].asCString(),
                    entry["mac"].asCString()
            );
        }
    }
    return 0;
}


int do_network(const int argc, const char **gargv, const struct global_opts *gopts) {
    int option_index = 0;
    int c = 0;
    char *const *argv = (char *const*) gargv;
	
    fprintf(stderr, "DEBUG: running command '%s', argc = %d\n", argv[0], argc);

    if (argc == 1) {
        do_network_list(stdout, msg, gopts);
        return 0;
    }

    // command line argument parsing
    while(c != -1) {

        c = getopt_long(argc, argv, "d", long_options, &option_index);
        switch(c) {
            case -1:
                continue;
                break;
            case 'd':
                opts.show_details = true;
                break;
            case '?':
               /* getopt_long already printed an error message. */
               break;
             default:
               return -1;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "[NOT-YET-IMPLEMENTED]: do_network_help();\n");
        exit(EXIT_FAILURE);
    }

    if (streq(argv[optind], "list")) {
        return do_network_list(stdout, msg, gopts);
    }
    else if (streq(argv[optind], "add")) {
        fprintf(stderr, "[NOT-YET-IMPLEMENTED]: add;\n");
        exit(EXIT_FAILURE);
    }
    else if (streq(argv[optind], "remove")) {
        fprintf(stderr, "[NOT-YET-IMPLEMENTED]: add;\n");
        exit(EXIT_FAILURE);
    }
    else {
        fprintf(stderr, "ERROR: unknown sub-command '%s'\n", argv[optind]);
        exit(EXIT_FAILURE);
    }
    /*
         {
           fprintf (stderr, "non-option ARGV-elements: ");
           while (optind < argc)
             fprintf (stderr, "%s ", argv[optind++]);
           fputc ('\n', stderr);
         }
   */
}
