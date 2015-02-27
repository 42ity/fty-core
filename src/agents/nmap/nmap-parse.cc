#include <cxxtools/convert.h>
#include <cxxtools/string.h>
#include <cxxtools/xml/xmlreader.h>
#include <cxxtools/xml/node.h>
#include <cxxtools/xml/startelement.h>
#include <cxxtools/xml/endelement.h>
#include <cxxtools/xml/characters.h>

#include <cassert>

#include <czmq.h>

#include "nmap-parse.h"
#include "nmap_msg.h"
#include "log.h"
#include "defs.h"

using namespace cxxtools::xml;
using namespace cxxtools;

enum class ListState {
    START,
    HOST,
    HOSTNAME,
    ENDHOST
};


static const char* HOSTSTATE_UNKNOWN = "unknown";
static const char* HOSTSTATE_UP      = "up";
static const char* HOSTSTATE_DOWN    = "down";
static const char* HOSTSTATE_SKIPPED = "skipped";

static const uint8_t    HOSTSTATE_UNKNOWN_ID = 0;
static const uint8_t    HOSTSTATE_UP_ID      = 1;
static const uint8_t    HOSTSTATE_DOWN_ID    = 2;
static const uint8_t    HOSTSTATE_SKIPPED_ID = 3;

// TODO Redo constants (HOSTSTATE_UP, ...) to std::string
//      and check as case insensitive
uint8_t hoststate_byte (const char *host_state) {
    if (host_state == NULL ||
        strlen(host_state) == 0) {
        return HOSTSTATE_UNKNOWN_ID;
    }
    
    assert (host_state);    
    if (strcmp (host_state, HOSTSTATE_UP) == 0) {
        return HOSTSTATE_UP_ID;
    }
    else if (strcmp (host_state, HOSTSTATE_DOWN) == 0) {
        return HOSTSTATE_DOWN_ID;
    }
    else if (strcmp (host_state, HOSTSTATE_SKIPPED) == 0) {
        return HOSTSTATE_SKIPPED_ID;
    }
    return HOSTSTATE_UNKNOWN_ID;
}

const char* byte_hoststate (uint8_t host_state) {
    switch (host_state) {
        case HOSTSTATE_UP_ID:
            return HOSTSTATE_UP;

        case HOSTSTATE_DOWN_ID:
            return HOSTSTATE_DOWN;

        case HOSTSTATE_SKIPPED_ID:
            return HOSTSTATE_SKIPPED;

        case HOSTSTATE_UNKNOWN_ID:
        default:
            return HOSTSTATE_UNKNOWN;
    }
}

// TODO 
#define PORT_UNKNOWN 0
#define PORT_CLOSED 1
#define PORT_OPEN 2
#define PORT_FILTERED 3
#define PORT_TESTING 4
#define PORT_FRESH 5
#define PORT_UNFILTERED 6
#define PORT_OPENFILTERED 7
#define PORT_CLOSEDFILTERED 8

// static const char* PORT_UNKNOWN_STR         "unknown"
#define PORT_UNKNOWN_STR         "unknown"
#define PORT_CLOSED_STR          "closed"
#define PORT_OPEN_STR            "open"
#define PORT_FILTERED_STR        "filtered"
#define PORT_UNFILTERED_STR      "unfiltered"
#define PORT_OPENFILTERED_STR    "open|filtered"
#define PORT_CLOSEDFILTERED_STR  "closed|filtered"


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
    }
    else if (strcmp (port_state, "unfiltered") == 0) {
        return PORT_UNFILTERED;
    }
    else if (strcmp (port_state, "open|filtered") == 0) {
        return PORT_OPENFILTERED;
    }
    else if (strcmp (port_state, "closed|filtered") == 0) {
        return PORT_CLOSEDFILTERED;
    }
    else {
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

int matryoshka2frame (zmsg_t **matryoshka, zframe_t **frame ) {
    assert (matryoshka);
    if (*matryoshka) {
        byte *buffer;
        size_t zmsg_size = zmsg_encode (*matryoshka, &buffer);

        // double check size
        // TODO after some time, remove the redundant check
        size_t check_size = 0;
        zframe_t *tmp_frame = zmsg_first (*matryoshka);
        while (tmp_frame) {
            size_t tmp_frame_size = zframe_size (tmp_frame);
            if (tmp_frame_size < 255)
                check_size += tmp_frame_size + 1;
            else
                check_size += tmp_frame_size + 1 + 4;
            tmp_frame = zmsg_next (*matryoshka);
        }
        assert (check_size == zmsg_size);

        zframe_t *ret_frame = zframe_new (buffer, zmsg_size);
        assert (ret_frame);

        zmsg_destroy (matryoshka);
        assert (*matryoshka == NULL);

        *frame = ret_frame;
        return 0;
    }
    else {
        return -2;
    }
}

void parse_list_scan(std::istream& inp, zsock_t *socket) {
    assert (socket);
    assert (zsock_is (socket));

    log_info ("start");
        
    XmlReader r{inp, 0};
    std::list<std::string> ls_res;
    std::string reason_v;
    std::string addr_v;
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
                        // don't send <host> entries that don't have
                        // any <hostname> elements inside <hostnames></...> specified
                        if (hostnames != NULL && zhash_size (hostnames) != 0) {
                            nmap_msg_t *msg = nmap_msg_new (NMAP_MSG_LIST_SCAN);
                            assert (msg);
                            nmap_msg_set_addr (msg, "%s", addr_v.c_str());
                            nmap_msg_set_reason (msg, "%s", reason_v.c_str());                                                
                            nmap_msg_set_hostnames(msg, &hostnames);

                            int rv = nmap_msg_send (&msg, socket);
                            assert (rv != -1);
                            log_debug ("'%s' sent\n,", addr_v.c_str());
                        } else {
                            log_debug ("'%s' doesn't have <hostnames>", addr_v.c_str());
                        }


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

                const char* v1 = (const char*)el.attribute(name).c_str();
                char* v2 =       (      char*)el.attribute(type).c_str();
            
                assert (hostnames);
                int rv = zhash_insert (hostnames, v1, v2); 
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
    log_info ("end");    
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
    log_info ("start");    

    XmlReader r{inp, 0};
    std::list<std::string> ls_res;

    zhash_t *addresses = zhash_new ();
    assert (addresses);
    zhash_t *hostnames = zhash_new ();
    assert (hostnames);

    int rv = -1;
    
    bool in_host = false;
    bool in_hostnames = false;

    bool in_ports = false;
    bool in_port = false;
    bool in_service = false;   

    bool in_os = false;    
    bool in_osmatch = false;    

    // multi
    bool in_cpe = false;
    std::string scan_target;

    nmap_msg_t *devscan = NULL;
    
    zmsg_t *devscan_ports = NULL;       // MATRYOSHKA ("port_scan")
    zmsg_t *devscan_os = NULL;          // MATRYOSHKA ("os_scan")
    zmsg_t *devscan_scripts = NULL;     // MATRYOSHKA ("script")

    nmap_msg_t *portscan = NULL;    
    zmsg_t *portscan_scripts = NULL;    // MATRYOSHKA ("script")

    nmap_msg_t *osscan = NULL;
    zmsg_t *osscan_portused = NULL;     // MATRYOSHKA ("os_scan")
    zmsg_t *osscan_osmatch = NULL;      // MATRYOSHKA ("portused")

    nmap_msg_t *osmatch = NULL;
    zmsg_t *osmatch_osclass = NULL;     // MATRYOSHKA ("osclass")

    // tntnet specific String constants; _term suffix
    const String accuracy_term("accuracy");
    const String addr_term("addr");
    const String address_term("address");
    const String addrtype_term("addrtype");
    const String conf_term("conf");
    const String devicetype_term("devicetype");    
    const String extrainfo_term("extrainfo");
    const String fingerprint_term("fingerprint");    
    const String highver_term("highver");    
    const String hostname_term("hostname");
    const String id_term("id");
    const String line_term("line");    
    const String lowver_term("lowver");    
    const String method_term("method");
    const String name_term("name");
    const String os_term("os");
    const String osclass_term("osclass");
    const String osmatch_term("osmatch");
    const String ostype_term("ostype");    
    const String osfingerprint_term("osfingerprint");    
    const String output_term("output");    
    const String portid_term("portid");
    const String portused_term("portused");
    const String product_term("product");
    const String proto_term("proto");
    const String protocol_term("protocol");
    const String reason_term("reason");
    const String reason_ttl_term("reason_ttl");
    const String reason_ip_term("reason_ip");
    const String rpcnum_term("rpcnum");
    const String servicefp_term("servicefp");    
    const String state_term("state");
    const String tunnel_term("tunnel");
    const String vendor_term("vendor");
    const String version_term("version");
    const String type_term("type");    

    for (auto node_it = r.current(); node_it != r.end(); ++node_it ) {
        
        //      START ELEMENT   // 
        if (node_it->type() == Node::StartElement) {
            const StartElement& el = static_cast<const StartElement&>(*node_it);
            const String& name = el.name();

            if (name.compare ("host") == 0) {
                in_host = true;

                assert (devscan == NULL);
                devscan = nmap_msg_new (NMAP_MSG_DEV_SCAN);
                assert (devscan);
            }
            else if (name.compare ("status") == 0) {
                assert (in_host);
                assert (devscan != NULL);

                // state
                nmap_msg_set_host_state (devscan,
                    (byte) hoststate_byte (
                    convert<std::string>(
                    el.attribute (state_term)).c_str()));
                // reason
                nmap_msg_set_reason (devscan, "%s",
                    convert<std::string>(
                    el.attribute (reason_term)).c_str());
            }                 
            else if (name.compare("address") == 0) {
                assert (in_host);
                assert (devscan != NULL);
                
                if (el.attribute (addrtype_term).compare ("mac") == 0) {
                    // don't get a copy, transfer ownership
                    zhash_t *hash = nmap_msg_get_addresses (devscan);
                    if (hash == NULL) {
                        hash = zhash_new ();
                        assert (hash);
                        // NOTE: By default
                        //  keys are duplicated usign strdup - check
                        //  keys are freed free() - check
                        zhash_autofree (hash);
                    }
                    // TODO: rewrite C-style cast
                    zhash_insert (hash,
                        convert<std::string>
                        (el.attribute (addr_term)).c_str(),
                        (void *) convert<std::string>
                        (el.attribute (vendor_term)).c_str());
                    // transfer ownership back
                    nmap_msg_set_addresses (devscan, &hash);
                    assert (hash == NULL);
                }
            }
            else if (name.compare("hostnames") == 0) {
                assert (in_host);
                in_hostnames = true;
            }
            else if (name.compare("hostname") == 0) {
                assert (in_host);
                assert (in_hostnames);

                assert (devscan != NULL);

                // don't get a copy, transfer ownership
                zhash_t *hash = nmap_msg_get_hostnames (devscan);
                if (hash == NULL) {
                    hash = zhash_new ();
                    assert (hash);
                    // NOTE: By default
                    //  keys are duplicated usign strdup - check
                    //  keys are freed free() - check
                    zhash_autofree (hash);
                }
                // TODO: rewrite C-style cast
                zhash_insert (hash, 
                    convert<std::string>
                    (el.attribute (name_term)).c_str(),
                    (void *) convert<std::string>
                    (el.attribute (type_term)).c_str());
                // transfer ownership back
                nmap_msg_set_hostnames (devscan, &hash);
                assert (hash == NULL);
            }
            else if (name.compare("ports") == 0) {
                in_ports = true;
            }
            else if (name.compare("port") == 0) {
                assert (in_host);
                assert (in_ports);

                in_port = true;

                assert (portscan == NULL);
                assert (portscan_scripts == NULL);

                portscan = nmap_msg_new (NMAP_MSG_PORT_SCAN);
                assert (portscan);

                // protocol
                nmap_msg_set_protocol (portscan, "%s",
                    convert<std::string>
                    (el.attribute (protocol_term)).c_str());
                // portid
                nmap_msg_set_portid (portscan,
                    std::stoi(
                    convert<std::string>
                    (el.attribute (portid_term))));
            }
            else if (name.compare("state") == 0) {
                assert (in_host);
                assert (in_ports);
                assert (in_port);

                assert (portscan != NULL);

                // state
                nmap_msg_set_port_state (portscan,
                    (byte) str2state(
                    convert<std::string>
                    (el.attribute (state_term)).c_str()));
                // reason
                nmap_msg_set_reason (portscan, "%s",
                    convert<std::string>
                    (el.attribute (reason_term)).c_str());              
                // reason_ttl
                if (el.hasAttribute (reason_ttl_term)) {
                    assert (el.attribute (reason_ttl_term).empty() == false);
                    nmap_msg_set_reason_ttl (portscan,
                        (byte) std::stoi(
                        convert<std::string>
                        (el.attribute (reason_ttl_term))));              
                }
                // reason_ip
                nmap_msg_set_reason_ip (portscan, "%s",
                    convert<std::string>
                    (el.attribute (reason_ip_term)).c_str());              
               
            } else if (name.compare("service") == 0) {
                assert (in_host);
                assert (in_ports);
                assert (in_port);
                in_service = true;

                assert (portscan != NULL);

                // name     REQUIRED
                nmap_msg_set_service_name (portscan, "%s",
                    convert<std::string>(el.attribute (name_term)).c_str());
                // conf     REQUIRED
                nmap_msg_set_service_conf (portscan,
                    (byte) std::stoi(
                    convert<std::string>(el.attribute (conf_term))));
                // method   REQUIRED
                nmap_msg_set_service_method (portscan,
                    (byte) str2method(
                    convert<std::string>(el.attribute (method_term)).c_str()));
                // Following attributes are not mandatory:
                // version
                if (el.hasAttribute (version_term)) {
                    nmap_msg_set_service_version (portscan, "%s",
                        convert<std::string>
                        (el.attribute (version_term)).c_str());
                }
                // product                
                if (el.hasAttribute (product_term)) {
                    nmap_msg_set_service_product (portscan, "%s", 
                        convert<std::string>
                        (el.attribute (product_term)).c_str());
                }
                // extrainfo
                if (el.hasAttribute (extrainfo_term)) {
                    nmap_msg_set_service_extrainfo (portscan, "%s", 
                        convert<std::string>
                        (el.attribute (extrainfo_term)).c_str());
                }
                // tunnel
                if (el.hasAttribute (tunnel_term)) {
                    assert (el.attribute (tunnel_term).empty() == false);
                    nmap_msg_set_service_tunnel (portscan,
                        (byte) std::stoi(
                        convert<std::string>
                        (el.attribute (tunnel_term))));
                }
                // proto
                if (el.hasAttribute (proto_term)) {
                    assert (el.attribute (proto_term).empty() == false);
                    nmap_msg_set_service_proto (portscan,
                        (byte) std::stoi(
                        convert<std::string>
                        (el.attribute (proto_term))));
                }
                // rpcnum
                if (el.hasAttribute (rpcnum_term)) {
                    assert (el.attribute (rpcnum_term).empty() == false);
                    nmap_msg_set_service_rpcnum (portscan,
                        (byte) std::stoi(
                        convert<std::string>
                        (el.attribute (rpcnum_term))));
                }
                // lowver
                if (el.hasAttribute (lowver_term)) {
                    assert (el.attribute (lowver_term).empty() == false);
                    nmap_msg_set_service_lowver (portscan,
                        (byte) std::stoi(
                        convert<std::string>
                        (el.attribute (lowver_term))));
                }
                // highver
                if (el.hasAttribute (highver_term)) {
                    assert (el.attribute (highver_term).empty() == false);
                    nmap_msg_set_service_highver (portscan,
                        (byte) std::stoi(
                        convert<std::string>
                        (el.attribute (highver_term))));
                }
                // hostname
                nmap_msg_set_service_hostname (portscan,
                    convert<std::string>
                    (el.attribute (hostname_term)).c_str());
                // ostype
                nmap_msg_set_service_ostype (portscan,
                    convert<std::string>
                    (el.attribute (ostype_term)).c_str());
                // devicetype
                nmap_msg_set_service_devicetype (portscan,
                    convert<std::string>
                    (el.attribute (devicetype_term)).c_str());
                // servicefp
                nmap_msg_set_service_servicefp (portscan,
                    convert<std::string>
                    (el.attribute (servicefp_term)).c_str());

            // TBD
            } else if (name.compare ("cpe") == 0) {
                in_cpe = true;
            } else if (name.compare ("scripts") == 0) {
            } else if (name.compare ("script") == 0) {
                assert (in_host);
                if (in_ports && in_port) {
                    // create encapsulating matryoshka if id doesn't exist
                    if (portscan_scripts == NULL) {
                        portscan_scripts = zmsg_new ();
                    }
                    assert (portscan_scripts);                        

                    const std::string& tmp = 
                    convert<std::string>(el.attribute (output_term));
                    zchunk_t *chunk = zchunk_new (
                        (const void *) tmp.data(),
                        tmp.size());

                    zmsg_t *to_be_ins = 
                    nmap_msg_encode_script (
                        convert<std::string>
                        (el.attribute(id_term)).c_str(),
                        chunk); // chunk is NOT released
                    assert (to_be_ins);
                    
                    zchunk_destroy (&chunk);

                    rv = zmsg_addmsg (
                        portscan_scripts,
                        (zmsg_t **) &to_be_ins); // to_be_ins ownership transfered
                    assert (rv != -1);
                    assert (chunk == NULL);
                }
            } else if (name.compare ("os") == 0) {
                in_os = true;
            } else if (name.compare ("portused") == 0) {
                assert (in_host);
                assert (in_os);
                
                // create encapsulating matryoshka if it doesn't exist
                if (osscan_portused == NULL) {
                    osscan_portused = zmsg_new ();
                }
                assert (osscan_portused != NULL);

                // std::stoi throws on empty string
                assert (el.attribute (portid_term).empty() == false);
                zmsg_t *to_be_ins =
                nmap_msg_encode_portused (
                    (byte) str2state(
                    convert<std::string>
                    (el.attribute (state_term)).c_str()),
                    convert<std::string>
                    (proto_term).c_str(),
                    (uint16_t) std::stoi(
                    convert<std::string>
                    (el.attribute(portid_term))));
                assert (to_be_ins != NULL);
                rv = zmsg_addmsg (
                    osscan_portused,
                    (zmsg_t **) &to_be_ins); // to_be_ins ownership transfered
                assert (rv != -1);

            } else if (name.compare ("osmatch") == 0) {
                assert (in_host);
                assert (in_os);

                in_osmatch = true;

		// FIXME: Access just to shut up -Werror=unused-variable
		assert(in_osmatch);

                assert (osmatch == NULL);
                osmatch = nmap_msg_new (NMAP_MSG_OSMATCH);
                assert (osmatch != NULL);
    
                // std::stoi throws on empty string
                assert (el.attribute (accuracy_term).empty() == false);
                assert (el.attribute (line_term).empty() == false);
                nmap_msg_set_name (osmatch, "%s",
                    convert<std::string>
                    (el.attribute (name_term)).c_str());
                nmap_msg_set_accuracy (osmatch,
                    (byte) std::stoi(
                    convert<std::string>
                    (el.attribute (accuracy_term))));
                nmap_msg_set_line (osmatch,
                    (byte) std::stoi(
                    convert<std::string>
                    (el.attribute (line_term))));

            } else if (name.compare ("osfingerprints") == 0) {
                assert (in_host);
                assert (in_os);
                
                if (osscan == NULL) {
                    osscan = nmap_msg_new (NMAP_MSG_OS_SCAN);
                }
                assert (osscan != NULL);

                // don't get a copy, transfer ownership
                zlist_t *list = nmap_msg_get_osfingerprints (osscan);
                if (list == NULL) {
                    list = zlist_new ();
                    assert (list);
                    zlist_autofree (list);
                }
                // TODO: rewrite C-style cast
                rv = zlist_append (list,
                    (void *) convert<std::string>
                    (el.attribute (fingerprint_term)).c_str());
                assert (rv != -1);
                // transfer ownership back
                nmap_msg_set_osfingerprints (osscan, &list);
                assert (list == NULL);
            } else if (name.compare ("osclass") == 0) {
                // TODO
                /* Note: Not being implemented right now
                assert (in_host);
                assert (in_os);
                assert (in_osmatch);

                assert (osmatch != NULL);
                */             
            }
        //      END ELEMENT     //
        } else if (node_it->type() == Node::EndElement) {            
            const EndElement& el = static_cast<const EndElement&>(*node_it);
            const String& name = el.name();
        
            if (name.compare ("host") == 0) {
                in_host = false;

                rv = nmap_msg_send (&devscan, socket);
                assert (rv != -1); 

                assert (devscan == NULL);
                assert (devscan_ports == NULL);
                assert (devscan_os == NULL);
                assert (devscan_scripts == NULL);
                assert (portscan == NULL);
                assert (portscan_scripts == NULL);
                assert (osscan == NULL);
                assert (osscan_portused == NULL);
                assert (osscan_osmatch == NULL);
                assert (osmatch == NULL);
                assert (osmatch_osclass == NULL);

            }
            else if (name.compare ("hostnames") == 0) {
                in_hostnames = false;
            }
            else if (name.compare ("ports") == 0) {
                assert (in_host);
                in_ports = false;

                // there might not have been <port>...</port> elements
                // indside <ports>...</ports>, but if there were, then
                // there exists zmsg_t* devscan_ports != NULL atd... 
                if (devscan_ports != NULL) {
                    zframe_t *devscan_ports_frame = NULL;
                    rv = matryoshka2frame (                        
                        &devscan_ports,
                        &devscan_ports_frame); // devscan_ports freed
                    assert (rv == 0);
                    assert (devscan_ports == NULL);
                    assert (devscan_ports_frame != NULL);
                    nmap_msg_set_ports (
                        devscan,
                        &devscan_ports_frame); // devscan_ports_frame freed
                    assert (devscan_ports_frame == NULL);
                }
                assert (devscan_ports == NULL);
                assert (devscan_os == NULL);
                assert (devscan_scripts == NULL);
            }
            else if (name.compare ("port") == 0) {
                assert (in_host);
                assert (in_ports);

                in_port = false;                
                assert (portscan != NULL);

                // if there were <script id='' output='' /> elements
                // then there exists zmsg_t* portscan_scripts != NULL
                // that is a matryoshka of 'script' messages
                if (portscan_scripts != NULL) {
                    zframe_t *portscan_scripts_frame = NULL;
                    rv = matryoshka2frame (                        
                        &portscan_scripts,
                        &portscan_scripts_frame); // port_scan_scripts freed
                    assert (rv == 0);
                    assert (portscan_scripts == NULL);
                    assert (portscan_scripts_frame != NULL);
                    nmap_msg_set_scripts (
                        portscan,
                        &portscan_scripts_frame); // port_scan_scripts_frame freed
                    assert (portscan_scripts_frame == NULL);                    
                }

                if (devscan_ports == NULL) {
                    devscan_ports = zmsg_new ();
                }
                assert (devscan_ports != NULL);

                zmsg_t *portscan_msg = nmap_msg_encode (&portscan); // portscan freed
                assert (portscan_msg);

                rv = zmsg_addmsg (
                    devscan_ports,
                    (zmsg_t **) &portscan_msg); // to_be_ins ownership transfered
                assert (rv != -1);

                assert (portscan == NULL);
                assert (portscan_scripts == NULL);
            }
            else if (name.compare ("service") == 0) {                
                assert (in_service);
                in_service = false;
            }
            else if (name.compare ("cpe") == 0) {
                in_cpe = false;
            }
            else if (name.compare ("os") == 0) {
                assert (in_host);
                in_os = false;

                if (osscan == NULL &&
                    osscan_portused == NULL &&
                    osscan_osmatch == NULL) {
                    continue;
                } else if (osscan == NULL) {
                    osscan = nmap_msg_new (NMAP_MSG_OS_SCAN);
                }
                assert (osscan != NULL);

                // there might not have been <portused>...</portused> elements
                // indside <os></os>, but if there were, then
                // there exists zmsg_t* osscan_portused != NULL 
                // that is a matryoshka of 'portused' messages
                if (osscan_portused != NULL) {
                    zframe_t *osscan_portused_frame = NULL;
                    rv = matryoshka2frame (                        
                        &osscan_portused,
                        &osscan_portused_frame); // osscan_portused_scripts freed

                    assert (rv == 0);
                    assert (osscan_portused == NULL);
                    assert (osscan_portused_frame != NULL);

                    nmap_msg_set_portused (
                        osscan,
                        &osscan_portused_frame); // osscan_portused_frame freed

                    assert (osscan_portused_frame == NULL);                    
                }

                // there might not have been <osmatch>...</osmatch> elements
                // indside <os></os>, but if there were, then
                // there exists zmsg_t* osscan_osmatch != NULL 
                // that is a matryoshka of 'osmatch' messages
                if (osscan_osmatch != NULL) {
                    zframe_t *osscan_osmatch_frame = NULL;
                    rv = matryoshka2frame (                        
                        &osscan_osmatch,
                        &osscan_osmatch_frame); // osscan_portused_scripts freed

                    assert (rv == 0);
                    assert (osscan_osmatch == NULL);
                    assert (osscan_osmatch_frame != NULL);

                    nmap_msg_set_osmatch (
                        osscan,
                        &osscan_osmatch_frame); // osscan_osmatch_frame freed

                    assert (osscan_osmatch_frame == NULL);                    
                }

                if (devscan_os == NULL) {
                    devscan_os = zmsg_new ();
                }
                assert (devscan_os != NULL);

                zmsg_t *osscan_msg = nmap_msg_encode (&osscan); // osscan freed
                assert (osscan_msg);

                rv = zmsg_addmsg (
                    devscan_os,
                    (zmsg_t **) &osscan_msg); // osscan_msg ownership transfered
                assert (rv != -1);

                assert (osscan == NULL);
                assert (osscan_portused == NULL);
                assert (osscan_osmatch == NULL);                
            }
            else if (name.compare ("osmatch") == 0) {
                in_osmatch = false;

                assert (osmatch != NULL);

                // there might not have been <osclass>...</osclass> elements
                // indside <osmatch></osmatch>, but if there were, then
                // there exists zmsg_t* osmatch_osclass != NULL 
                // that is a matryoshka of 'osclass' messages
                if (osmatch_osclass != NULL) {
                    zframe_t *osmatch_osclass_frame = NULL;
                    rv = matryoshka2frame (                        
                        &osmatch_osclass,
                        &osmatch_osclass_frame); // osmatch_osclass freed

                    assert (rv == 0);
                    assert (osmatch_osclass == NULL);
                    assert (osmatch_osclass_frame != NULL);

                    nmap_msg_set_osclass (
                        osmatch,
                        &osmatch_osclass_frame); // osmatch_osclass_frame freed

                    assert (osmatch_osclass_frame == NULL);                    
                }

                
            }
        //      CHARACTERS      //
        } else if (node_it->type() == Node::Characters) {
            if (in_host == false) {            
                continue;
            }
            const Characters& ch = static_cast<const Characters&>(*node_it);
            const String& content = ch.content();

            // <cpe>$content$</cpe> in <service></service>
            if (in_ports && in_port && in_service && in_cpe) {
                assert (portscan != NULL);

                // don't get a copy, transfer ownership
                zlist_t *list = nmap_msg_get_service_cpes (portscan);
                if (list == NULL) {
                    list = zlist_new ();
                    assert (list);
                    zlist_autofree (list);
                }
                // TODO: rewrite C-style cast
                rv = zlist_append (list,
                    (void *) convert<std::string>
                    (content).c_str());
                assert (rv != -1);
                // transfer ownership back
                nmap_msg_set_service_cpes (portscan, &list);
                assert (list == NULL);
            }

        } else {
            continue;
        }

    }

    assert (devscan == NULL);
    log_info ("end");    
    log_close ();    
}

void parse_device_scan(const std::string& inp, zsock_t *socket) {
    std::istringstream stream{inp};
    parse_device_scan(stream, socket);
}
