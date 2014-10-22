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

static const char* HOSTSTATE_UP = "up";
static const char* HOSTSTATE_DOWN = "down";
static const char* HOSTSTATE_UNKNOWN = "unknown";
static const char* HOSTSTATE_SKIPPED = "skipped";
static const int HOSTSTATE_UP_ID = 1;
static const int HOSTSTATE_DOWN_ID = 2;
static const int HOSTSTATE_UNKNOWN_ID = 3;
static const int HOSTSTATE_SKIPPED_ID = 4;

byte hoststate_byte (const char *host_state) {
    assert (host_state);
    if (strcmp (host_state, HOSTSTATE_UP) == 0) {
        return HOSTSTATE_UP_ID;
    }
    else if (strcmp (host_state, HOSTSTATE_DOWN) == 0) {
        return HOSTSTATE_DOWN_ID;
    }
    else if (strcmp (host_state, HOSTSTATE_UNKNOWN) == 0) {
        return HOSTSTATE_UNKNOWN_ID;
    }
    else if (strcmp (host_state, HOSTSTATE_SKIPPED) == 0) {
        return HOSTSTATE_SKIPPED_ID;
    }
    else {
        return 0;
    }
}

const char* byte_hoststate (byte host_state) {
    switch (host_state) {
        case HOSTSTATE_UP_ID:
            return HOSTSTATE_UP;
        case HOSTSTATE_DOWN_ID:
            return HOSTSTATE_DOWN;
        case HOSTSTATE_UNKNOWN_ID:
            return HOSTSTATE_UNKNOWN;
        case HOSTSTATE_SKIPPED_ID:
            return HOSTSTATE_SKIPPED;
        default:
            return NULL;            
    }
}

// decide whether to go for c++ enum or export all od these ansi style, or at all
#define PORT_UNKNOWN 0
#define PORT_CLOSED 1
#define PORT_OPEN 2
#define PORT_FILTERED 3
#define PORT_TESTING 4
#define PORT_FRESH 5
#define PORT_UNFILTERED 6
#define PORT_OPENFILTERED 7
#define PORT_CLOSEDFILTERED 8


const char *state2str (int state) {
  switch (state) {
  case PORT_OPEN:
    return "open";
    break;
  case PORT_CLOSED:
    return "closed";
    break;
  case PORT_FILTERED:
    return "filtered";
    break;
  case PORT_UNFILTERED:
    return "unfiltered";
    break;
  case PORT_OPENFILTERED:
    return "open|filtered";
    break;
  case PORT_CLOSEDFILTERED:
    return "closed|filtered";
    break;
  default:
    return "unknown";
    break;
  }
  return "unknown";
}

// TODO uint8_t
int str2state (const char *port_state) {
    assert (port_state);
    if (strcmp (port_state, "open") == 0) {
        return PORT_OPEN; 
    }
    else if (strcmp (port_state, "closed") == 0) {
        return PORT_CLOSED;
    }
    else if (strcmp (port_state, "filtered") == 0) {
        return PORT_FILTERED;
    } else if (strcmp (port_state, "unfiltered") == 0) {
        return PORT_UNFILTERED;
    } else if (strcmp (port_state, "open|filtered") == 0) {
        return PORT_OPENFILTERED;
    } else if (strcmp (port_state, "closed|filtered") == 0) {
        return PORT_CLOSEDFILTERED;
    } else {
        return PORT_UNKNOWN;
    }        
    return PORT_UNKNOWN;
}

#define METHOD_NOT_SET  0
#define METHOD_TABLE    1
#define METHOD_PROBED   2
int str2method (const char * method) {
    assert (method);
    if (strcmp (method, "table") == 0) {
        return METHOD_TABLE;
    }
    else if (strcmp (method, "probed") == 0) {
        return METHOD_PROBED;
    }
    else {
        return METHOD_NOT_SET;
    }  
}

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
    log_close ();    
}

void parse_list_scan(const std::string& inp, zsock_t *socket) {
    std::istringstream stream{inp};
    return parse_list_scan(stream, socket);
}

void parse_device_scan(std::istream& inp, zsock_t *socket) {

    assert (socket);
    assert (zsock_is (socket));

    log_open();
    log_set_level(LOG_DEBUG);
    log_info ("parse_device_scan() start\n");    

    XmlReader r{inp, 0};
    std::list<std::string> ls_res;

    zhash_t *addresses = zhash_new ();
    assert (addresses);
    zhash_t *hostnames = zhash_new ();
    assert (hostnames);

    bool in_host = false;
    bool in_hostnames = false;
    bool in_ports = false;
    std::string scan_target;

    nmap_msg_t *msg = nmap_msg_new (NMAP_MSG_DEV_SCAN);
    assert (msg);
    
    nmap_msg_t *port_scan = NULL;

    // tntnet specific String constants; _term suffix
    const String addr_term("addr");
    const String address_term("address");
    const String addrtype_term("addrtype");
    const String conf_term("conf");
    const String method_term("method");
    const String name_term("name");

    const String portid_term("portid");
    const String protocol_term("protocol");


    const String reason_term("reason");
    const String reason_ttl_term("reason_ttl");
    const String reason_ip_term("reason_ip");

    const String state_term("state");
    const String vendor_term("vendor");
    const String type_term("type");    

    for (auto node_it = r.current(); node_it != r.end(); ++node_it ) {
        
        //      START ELEMENT   // 
        if (node_it->type() == Node::StartElement) {
            const StartElement& el = static_cast<const StartElement&>(*node_it);
            const String& name = el.name();

            if (name.compare ("host") == 0) {
                in_host = true;
            }
            else if (name.compare ("status") == 0) {
                assert (in_host);
                static String state = el.attribute (state_term);
                static String reason = el.attribute (reason_term);

                static byte host_state_v =
                hoststate_byte (convert<std::string>(state).c_str());
                assert (host_state_v != 0);

                nmap_msg_set_host_state (msg, host_state_v);
                nmap_msg_set_reason (msg, "%s", reason.c_str());
             }
            else if (name.compare("address") == 0) {
                assert (in_host);                
                static String vendor = el.attribute (vendor_term);
                static String addr = el.attribute (addr_term);
                // TODO vlozit do inicializovaneho zhash addresses
                // addr, vendor        
            }
            else if (name.compare("hostnames") == 0) {
                assert (in_host);
                in_hostnames = true;
            }
            else if (name.compare("hostname") == 0) {
                assert (in_host);
                assert (in_hostnames);
                static String name = el.attribute (vendor_term);
                static String type = el.attribute (addr_term);
                // TODO vlozit do inicializovaneho zhash
                
            }
            else if (name.compare("ports") == 0) {
                in_ports = true;
            }
            else if (name.compare("port") == 0) {
                assert (in_host);
                assert (in_ports);
                static String protocol = el.attribute (protocol_term);
                static String portid = el.attribute (portid_term);
                assert (port_scan == NULL);
                port_scan = nmap_msg_new (NMAP_MSG_PORT_SCAN);
                nmap_msg_set_protocol (port_scan, "%s",
                    convert<std::string>(protocol).c_str());
                nmap_msg_set_portid (port_scan,
                     std::stoi(convert<std::string>(portid)));

            }
            else if (name.compare("state") == 0) {
                assert (in_host);
                assert (in_ports);
                assert (port_scan);
                static String state = el.attribute (state_term);
                static String reason = el.attribute (reason_term);
                static String reason_ttl = el.attribute (reason_ttl_term);
                static String reason_ip = el.attribute (reason_ip_term);
                nmap_msg_set_port_state (port_scan,
                    (byte) str2state(convert<std::string>(state).c_str()));
                nmap_msg_set_reason (port_scan, "%s",
                    convert<std::string>(reason).c_str());              
                nmap_msg_set_reason_ttl (port_scan,
                    std::stoi(convert<std::string>(reason_ttl)));              
                nmap_msg_set_reason_ip (port_scan, "%s",
                    convert<std::string>(reason_ip).c_str());              
               
            } else if (name.compare("service") == 0) {
                // <!ELEMENT port (state , owner? , service?, script*) >
                assert (in_host);
                assert (in_ports);
                assert (port_scan);
                static String name = el.attribute (name_term);
                static String conf = el.attribute (conf_term);
                static String method = el.attribute (method_term);

                zmsg_t *serv_msg =
                nmap_msg_encode_service_scan (
                    convert<std::string>(name).c_str(),
                    (byte) std::stoi(convert<std::string>(conf)),
                    (byte) str2method(convert<std::string>(method).c_str()),
                    "", "", "", 0, 0, 0, 0, 0, "", "" ,"" , "", ""
                    );
/* TODO: rest
                    const char *version,
                    const char *product,
                    const char *extrainfo,
                    byte tunnel,
                    byte service_proto,
                    uint32_t rpcnum,
                    uint32_t lowver,
                    uint32_t highver,
                    const char *hostname,
                    const char *ostype,
                    const char *devicetype,
                    const char *servicefp,
                    const char *cpe);             
*/
                nmap_msg_set_service (port_scan, &serv_msg);
                assert (serv_msg = NULL);                

            // TBD
            } else if (name.compare ("scripts") == 0) {
            } else if (name.compare ("script") == 0) {
            } else if (name.compare ("os") == 0) {
            } else if (name.compare ("os_scan") == 0) {
            } else if (name.compare ("portused") == 0) {
            } else if (name.compare ("osmatch") == 0) {
            } else if (name.compare ("osclass") == 0) {
            }

        //      END ELEMENT     //
        } else if (node_it->type() == Node::EndElement) {            
            const EndElement& el = static_cast<const EndElement&>(*node_it);
            const String& name = el.name();
        
            if (name.compare ("host") == 0) {
                in_host = false;
                // case: </host> reached and msg hasn't been sent
                //       means error in our code, not invalid xml
                assert (msg == NULL);
            }
            else if (name.compare ("hostnames") == 0) {
                in_hostnames = false;
            }
            else if (name.compare ("ports") == 0) {
                in_ports = false;
            }
        } else {
            continue;
        }

    }
    nmap_msg_destroy (&msg);
    assert (msg == NULL);
    log_info ("parse_device_scan() end\n");    
    log_close ();    
}

void parse_device_scan(const std::string& inp, zsock_t *socket) {
    std::istringstream stream{inp};
    parse_device_scan(stream, socket);
}
