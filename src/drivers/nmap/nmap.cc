/*
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

/*! \file nmap.cc
    \brief Nmap driver - react on incomming queue and parse results
    \author Michal Vyskocil <michalvyskocil@eaton.com>
            Karol Hrdina    <karolhrdina@eaton.com>
 */

#include <set>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <exception>
#include <fstream>

#include <unistd.h>
#include <stdlib.h>

#include <czmq.h>

#include "subprocess.h"
#include "nmap_msg.h"
#include "nmap-driver.h"
#include "nmap-parse.h"
#include "log.h"
#include "defs.h"
#include "cidr.h"

typedef shared::ProcessQue ProcessQue;
typedef shared::SubProcess SubProcess;
typedef shared::Argv Argv;
typedef shared::ProcCacheMap ProcCacheMap;

static ProcessQue _que{4};
static ProcCacheMap _pcmap{};
static std::set<int> _fd_set;
static zsock_t *outgoing = NULL;

static std::map<NmapMethod, Argv> _map = {
    {NmapMethod::DefaultListScan, {NMAP_BIN, "-oX", "-", "-sL"}},
    {NmapMethod::DefaultDeviceScan, {SUDO_BIN, NMAP_BIN, "-oX", "-", "-R", "-sT", "-sU"}}
};

static void s_dump_nmap_xml(const std::string& _path, const std::string& input) {
    std::string path = _path;
    size_t pos = 0;
    while ((pos = path.find("/")) != std::string::npos) {
        pos = path.find("/");
        path = path.replace(pos, 1u, "_");
    }

    try {
        std::ofstream of{path};
        of << input;
        of.flush();
        of.close();
    } catch (const std::exception& exp) {
        log_debug("error on storing debug file '%s'", path.c_str());
        log_debug(exp.what());
    }
}

static int fd_handler (zloop_t *loop, zmq_pollitem_t *item, void *arg);

static int xzloop_registerfd(zloop_t *loop, int fd, SubProcess* proc) {
    if (_fd_set.count(fd) == 1) {
        return 0;
    }
    _fd_set.insert(fd);
    zmq_pollitem_t it = {NULL, fd, ZMQ_POLLIN, 0};
    return zloop_poller (loop, &it, fd_handler, static_cast<void*>(proc));
}

static void xzloop_unregisterfd(zloop_t *loop, int fd) {
    if (_fd_set.count(fd) == 0) {
        return;
    }
    _fd_set.erase(fd);
    zmq_pollitem_t it = {NULL, fd, ZMQ_POLLOUT, 0};
    return zloop_poller_end (loop, &it);
}

static int timer_handler(zloop_t *loop,
	    UNUSED_PARAM int timer_id,
	    UNUSED_PARAM void *arg) {

    _que.schedule();
    for (auto proc_it = _que.cbegin(); proc_it != _que.cend(); proc_it++) {
        auto proc = *proc_it;
        if (!proc) {
            continue;
        }
        xzloop_registerfd(loop, proc->getStdout(), proc);
        xzloop_registerfd(loop, proc->getStderr(), proc);
    }

    while (_que.hasDone()) {
        SubProcess *proc = _que.pop_done();
        xzloop_unregisterfd(loop, proc->getStdout());
        xzloop_unregisterfd(loop, proc->getStderr());

        _pcmap.pushStdout(proc->getPid(), shared::read_all(proc->getStdout()));
        _pcmap.pushStderr(proc->getPid(), shared::read_all(proc->getStderr()));

        //TBD: sending code!
        std::pair<std::string, std::string> p = _pcmap.pop(proc->getPid());

        if (proc->getReturnCode() != 0) {
            // scan has failed
            log_error ("scan failed! return code: %d; cmd line: '%s'",
                       proc->getReturnCode(), proc->argvString().c_str());
        }
        else {
            //\todo make it nicer! There needs to be code mapping argv to NmapMethod
            auto argv = proc->argv();
            if (std::count(argv.begin(), argv.end(), "-sL")) {
                if (::getenv("BIOS_DEBUG")) {
                    s_dump_nmap_xml("nmap-defaultlistscan-" + *(argv.cend()-1) + ".xml", p.first);
                }
                parse_list_scan(p.first, outgoing);
            }
            else {
                parse_device_scan(p.first, outgoing);
            }
        }

        delete proc;
    }

    return 0;
}

static int fd_handler (UNUSED_PARAM zloop_t *loop,
		zmq_pollitem_t *item, void *arg) {
    const SubProcess *proc = static_cast<const SubProcess*>(arg);

    if (proc->getStdout() == item->fd) {
        _pcmap.pushStdout(proc->getPid(), shared::read_all(proc->getStdout()));
    } else {
        _pcmap.pushStderr(proc->getPid(), shared::read_all(proc->getStderr()));
    }
    return 0;
}

static int command_handler (UNUSED_PARAM zloop_t *loop,
		zsock_t *reader,
		UNUSED_PARAM void *_arg) {
    log_info ("start");

    nmap_msg_t *msg = nmap_msg_recv (reader);
    assert (msg);

    int msg_id = nmap_msg_id (msg);

    switch (msg_id) {
        case NMAP_MSG_SCAN_COMMAND:
        {
            // field: type
            const char *command = nmap_msg_type (msg);
            assert (command && strlen (command) != 0);

            // field: args
            enum NmapMethod method;
            if (streq (command, SCAN_CMDTYPE_DEFAULT_LIST)) {                
                method = NmapMethod::DefaultListScan;
                log_debug ("default list scan");
            }
            else if (streq (command, SCAN_CMDTYPE_DEFAULT_DEVICE)) {
                method = NmapMethod::DefaultDeviceScan;
                log_debug ("default device scan");
            }
            else {
                log_error ("message of type 'scan_command' can not have field 'type' empty;");        
                nmap_msg_destroy (&msg);
                return 0;
            }
            Argv args = _map[method];
            zlist_t *zlargs = nmap_msg_args (msg);
            assert (zlargs);                        
            while (zlist_size (zlargs) > 0) {                
                char *next_item = NULL;
                next_item = (char *) zlist_pop (zlargs);
                assert (next_item);
                log_info ("%lu", strlen(next_item));
                log_info ("%s", next_item);
                args.push_back(next_item);                      
                free (next_item);
            }

            // check for IPV6 address 
            shared::CIDRAddress check_ipv6 (args[args.size() - 1]);
            if (check_ipv6.protocol() == 6) {
                log_warning ("IPV6 functionality is not implemented yet. Skipping.");
                break;
            }
            else if (check_ipv6.protocol() == 0) {
                // maybe assert in front of this if-construct would have sufficed
                log_error ("An invalid ip address has been supplied: '%s'", args[args.size() - 1].c_str());
                break;
            }
 
            // field: headers
            // Headers are not being used at the moment

            // Add the prepared command+arguments to the queue
            _que.add(args);                                     
            log_debug ("added to queue.");
            break;
        }
        case NMAP_MSG_LIST_SCAN:
        case NMAP_MSG_DEV_SCAN:
        case NMAP_MSG_PORT_SCAN:
        case NMAP_MSG_SCRIPT:
        case NMAP_MSG_OS_SCAN:
        case NMAP_MSG_PORTUSED:
        case NMAP_MSG_OSMATCH:
        case NMAP_MSG_OSCLASS:
        case NMAP_MSG_SCAN_ERROR:
        {
            log_warning ("UNEXPECTED message type received; Nmap scanner expects 'scan_command' not results of various scans.");
            return 0;
            break;
        }
        default:
        {
            log_warning ("UNKNOWN message type received; message id = '%d'", nmap_msg_id (msg));
            return 0;
            break;
        }
    }

    nmap_msg_destroy (&msg);
    log_info ("end");
    return 0;
}

void nmap_actor (zsock_t *pipe, UNUSED_PARAM void *args) {

    log_info ("start");

    zsock_t *cmdfd = zsock_new_router (DRIVER_NMAP_SOCK);
    assert (cmdfd);

    zpoller_t *poller = zpoller_new (cmdfd, pipe, NULL);
    assert (poller);
    zsock_signal (pipe, 0);

    zloop_t *loop = zloop_new();
    assert(loop);
    int rv = zloop_timer (loop, 2000, 10, timer_handler, NULL);
    assert (rv != -1);
    
    rv = zloop_reader(loop, cmdfd, command_handler, NULL);
    rv = zloop_start(loop);
    assert (rv == 0);

    zloop_destroy(&loop);        
    zpoller_destroy (&poller);
    zsock_destroy (&cmdfd);

    assert (loop == NULL);
    assert (poller == NULL);
    assert (cmdfd == NULL);

    log_info ("end");
}

void nmap_stdout (UNUSED_PARAM zsock_t *pipe,
		  UNUSED_PARAM void *args) {
/*
    log_info ("%s", "nmap_stdout() start");

    while (1) {
    nmap_msg_t *msg = nmap_msg_recv (router);
    assert (msg);
    int msg_id = nmap_msg_id (msg);
    //nmap_msg_print (msg);
    log_critical ("GOT IT! '%d'", msg_id);
    nmap_msg_destroy (&msg);
    assert (&msg);
    }
    zsock_destroy (&router);
    log_info ("%s", "nmap_stdout() end");
*/
}

int main (int argc, char **argv) {

    log_open();
    log_set_level(LOG_DEBUG);
    log_info ("nmap daemon start");

    bool test_mode = false;

    const char *target = getenv("NMAP_SCANNER_TARGET");
    const char *which = getenv("NMAP_SCANNER_TYPE");
    if (argc > 1 && strcmp(argv[1], "--test-mode") == 0) {
        test_mode = true;
        fprintf (stderr, "Test Mode:\n\
Nmap scanner expects environment variables '%s' and '%s' to be set. The first \
variable sets the scan target (i.e. 10.130.38.100/30), the second variable \
sets the scan type - it accepts two possible values a) '%s' OR b) '%s'.\n",
        "NMAP_SCANNER_TARGET", "NMAP_SCANNER_TYPE",
        "defaultlistscan", "defaultdevicescan");
        if (target == NULL ||
            strlen (target) == 0) {
            log_critical ("Environment variable %s not set.", "NMAP_SCANNER_TARGET");
            log_critical ("You have to set this variable when in --test-mode");
            return EXIT_FAILURE;
        }
        if (which == NULL ||
            strlen(which) == 0) {
            log_critical ("Environment variable %s not set.", "NMAP_SCANNER_TYPE");
            log_critical ("You have to set this variable when in --test-mode");
            return EXIT_FAILURE;
        }
        if (strcmp(which, "defaultlistscan") != 0 &&
            strcmp(which, "defaultdevicescan") != 0) {
            log_critical ("Environment variable '%s' must be either '%s' OR '%s'",
                          "NMAP_SCANNER_TYPE", "defaultlistscan", "defaultdevicescan");
            return EXIT_FAILURE;
        }
       
    }

    //
    zactor_t *nmap = zactor_new (nmap_actor, NULL);
    assert (nmap);
    zclock_sleep (1000);

    zsock_t *incoming = zsock_new_dealer (DRIVER_NMAP_SOCK);
    assert (incoming);
    outgoing = zsock_new_dealer (DRIVER_NMAP_REPLY);
    assert (outgoing);

    zpoller_t *poller = NULL;

    if (test_mode == true) {
        log_debug ("environment variable NMAP_SCANNER_TARGET = '%s'", target);
        log_debug ("environment variable NMAP_SCANNER_TYPE = '%s'", which);
         
        zsock_t *router = zsock_new_router (DRIVER_NMAP_REPLY);
        assert (router);
        // construct the nmap scan_command
        nmap_msg_t * msg = nmap_msg_new (NMAP_MSG_SCAN_COMMAND);
        assert (msg);
        nmap_msg_set_type (msg, "%s", which, NULL);

        zlist_t *zl = zlist_new ();
        zlist_append (zl, (void *) target);
        nmap_msg_set_args (msg, &zl);
        zlist_destroy (&zl);
       
        nmap_msg_send (&msg, incoming);

        poller = zpoller_new (router, NULL);
        assert (poller);

        while (!zpoller_terminated (poller)) {
            // don't care about which socket, only one is being polled            
            zpoller_wait (poller, -1);
            if (zpoller_terminated (poller)) {
                log_info ("SIGINT received. Quitting.");
                break;
            }
            // get the result and print it
            nmap_msg_t *msg = nmap_msg_recv (router);
            assert (msg != NULL);
	    // FIXME: JIM: Commented away as unused
            // int msg_id = nmap_msg_id (msg);
            nmap_msg_print (msg);
            nmap_msg_destroy (&msg);

        }
        zsock_destroy (&router);        
        assert (router == NULL);
    } else {
        poller = zpoller_new (nmap, NULL);
        assert (poller);

        while (!zpoller_terminated (poller)) {
            zsock_t *which = (zsock_t *) zpoller_wait (poller, -1);
	    // FIXME: JIM: Use the variable somehow to avout unused warning
	    if (which==NULL)
    		log_info ("zpoller_wait() timed out or was aborted");
            if (zpoller_terminated (poller)) {            
                log_info ("SIGINT received. Quitting.");
            }
        }
    }

    zpoller_destroy (&poller);
    zactor_destroy(&nmap);
    zsock_destroy (&incoming);
    zsock_destroy (&outgoing);

    log_info ("nmap daemon stop");
    log_close ();
    return EXIT_SUCCESS;
}
