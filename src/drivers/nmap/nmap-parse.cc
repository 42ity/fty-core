#include <cxxtools/convert.h>
#include <cxxtools/string.h>
#include <cxxtools/xml/xmlreader.h>
#include <cxxtools/xml/node.h>
#include <cxxtools/xml/startelement.h>
#include <cxxtools/xml/endelement.h>

#include <cassert>

#include <czmq.h>

#include "nmap-parse.h"
#include "nmap_msg.h"
#include "log.h"

using namespace cxxtools::xml;
using namespace cxxtools;

static const char* DRIVER_NMAP_REPLY = "ipc://@/bios/driver/nmap_reply";

enum class ListState {
    START,
    HOST,
    HOSTNAME,
    ENDHOST
};

void parse_list_scan(std::istream& inp, zsock_t *socket) {
    assert (socket);
    assert (zsock_is (socket));
    log_open();
    log_set_level(LOG_DEBUG);
    log_info ("parse_list_scan() start\n");    
    XmlReader r{inp, 0};
    std::list<std::string> ls_res;
    std::string reason_v;
    std::string addr_v;
    int host_state_v;
    zhash_t *hostnames = NULL;

    enum ListState state = ListState::START;

    for (auto node_it = r.current(); node_it != r.end(); ++node_it ) {

        switch(state) {
            case ListState::START:

                if (node_it->type() != Node::StartElement) {
                    continue;
                }
                
                {
                const StartElement& el = static_cast<const StartElement&>(*node_it);

                const auto k = convert<std::string>(el.name());
                if (k != "status") {
                    continue;
                }

                static auto state_attr = convert<String>("state");
                const auto v = convert<std::string>(el.attribute(state_attr));
                // host state
                if (v == "up" || v == "unknown") {
                    state = ListState::HOST;
                    // quick & dirty - to be reworked
                    if (v.compare("up") == 0) {
                        host_state_v = 0;
                    }
                    else if (v.compare("down") == 0) {
                        host_state_v = 1;
                    }
                    else if (v.compare("unknown") == 0) {
                        host_state_v = 2;
                    }
                    else if (v.compare("skipped") == 0) {
                        host_state_v = 3;
                    }
                        
                    static auto reason = convert<String>("reason");
                    reason_v = convert<std::string>(el.attribute(reason));
                }
                else {
                    state = ListState::ENDHOST;
                }
                }
                break;

            case ListState::HOST:
                {

                    if (node_it->type() != Node::StartElement) {
                        continue;
                    }
                
                    const StartElement& el = static_cast<const StartElement&>(*node_it);

                    const auto k = convert<std::string>(el.name());
                    if (k != "address") {
                        continue;
                    }

                    static const auto addr = convert<String>("addr");
                    if (!el.hasAttribute(addr)) {
                        continue;
                    }

                    addr_v = convert<std::string>(el.attribute(addr));
                    state = ListState::HOSTNAME;
                    // create zhashmap
                    assert (hostnames == NULL);
                    hostnames = zhash_new ();
                    assert (hostnames);
                }
                break;

            case ListState::HOSTNAME:

                if (node_it->type() == Node::EndElement) {
                    const EndElement& el = static_cast<const EndElement&>(*node_it);
                    const auto name = convert<std::string>(el.name());
                    if (name == "host") {                        
                        state = ListState::START;
                        nmap_msg_t *msg = nmap_msg_new (NMAP_MSG_LIST_SCAN);
                        assert (msg);
                        nmap_msg_set_addr (msg, "%s", addr_v.c_str());
                        nmap_msg_set_host_state (msg, host_state_v);
                        nmap_msg_set_reason (msg, "%s", reason_v.c_str());                                                

                        int rv = nmap_msg_send (&msg, socket);
                        assert (rv != -1);

                        zhash_destroy (&hostnames);
                        assert (hostnames == NULL);
                        continue;
                    }
                }

                {
                if (node_it->type() != Node::StartElement) {
                    continue;
                }
                const StartElement& el = static_cast<const StartElement&>(*node_it);
                const auto k = convert<std::string>(el.name());
                if (k != "hostname") {
                    continue;
                }

                static const auto name = convert<String>("name");
                static const auto type = convert<String>("type");
                if (! el.hasAttribute(name) || ! el.hasAttribute(type)) {
                    continue;
                }

                auto v1 = el.attribute(name);
                auto v2 = el.attribute(type);
            
                assert (hostnames);
                int rv = zhash_insert (hostnames, &v1, &v2); 
                assert (rv == 0);

                }
                break;

            case ListState::ENDHOST:
                {
                if (node_it->type() != Node::EndElement) {
                    continue;
                }               
                
                const EndElement& el = static_cast<const EndElement&>(*node_it);

                auto k = convert<std::string>(el.name());
                if (k != "host") {
                    continue;
                }
                state = ListState::START;
                }
                break;
        }
    }
    log_info ("parse_list_scan() end\n");    
    
}

void parse_list_scan(const std::string& inp, zsock_t *socket) {
    std::istringstream stream{inp};
    return parse_list_scan(stream, socket);
}

void parse_device_scan(std::istream& inp) {

    XmlReader r{inp, 0};
    std::list<std::string> ls_res;

    for (auto node_it = r.current(); node_it != r.end(); ++node_it ) {

        if (node_it->type() != Node::StartElement) {
            continue;
        }
        
        const StartElement& el = static_cast<const StartElement&>(*node_it);
        auto name = convert<std::string>(el.name());

        if (name == "status") {
            static auto state = convert<String>("state");
            static auto reason = convert<String>("reason");
            static auto reason_ttl = convert<String>("reason_ttl");
            std::cout << "state: " << el.attribute(state) << \
                         " reason: " << el.attribute(reason) << \
                         " reason_ttl: " << el.attribute(reason_ttl) << std::endl;
        }
        else if (name == "address") {
            static auto addr = convert<String>("addr");
            static auto addrtype = convert<String>("addrtype");
            static auto vendor = convert<String>("vendor");
            std::cout << "addr: " << el.attribute(addr) << \
                         " addrtype: " << el.attribute(addrtype);
            if (el.hasAttribute(vendor)) {
                std::cout << " vendor: " << el.attribute(vendor);
            }
            std::cout << std::endl;
        }
        else if (name == "hostname") {
            static auto name = convert<String>("name");
            static auto type = convert<String>("type");
            std::cout << "name: " << el.attribute(name) << \
                         " type: " << el.attribute(type);
            std::cout << std::endl;
        }
        else if (name == "port") {
            static auto protocol = convert<String>("protocol");
            static auto portid = convert<String>("portid");
            std::cout << "protocol: " << el.attribute(protocol) << \
                         " portid: " << el.attribute(portid);
            std::cout << std::endl;
        }
        else if (name == "state") {
            static auto state = convert<String>("state");
            static auto reason = convert<String>("reason");
            static auto reason_ttl = convert<String>("reason_ttl");
            std::cout << "state: " << el.attribute(state) << \
                         " reason: " << el.attribute(reason) << \
                         " reason_ttl: " << el.attribute(reason_ttl);
            std::cout << std::endl;
        }
        else if (name == "service") {
            static auto name = convert<String>("name");
            static auto method = convert<String>("method");
            static auto conf = convert<String>("conf");
            std::cout << "name: " << el.attribute(name) << \
                         " method: " << el.attribute(method) << \
                         " conf: " << el.attribute(conf);
            std::cout << std::endl;
        }
    }
}

void parse_device_scan(const std::string& inp) {
    std::istringstream stream{inp};
    parse_device_scan(stream);
}
