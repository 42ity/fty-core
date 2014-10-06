#include <cxxtools/convert.h>
#include <cxxtools/string.h>
#include <cxxtools/xml/xmlreader.h>
#include <cxxtools/xml/node.h>
#include <cxxtools/xml/startelement.h>
#include <cxxtools/xml/endelement.h>

#include <cassert>

#include "nmap.h"

using namespace cxxtools::xml;
using namespace cxxtools;

std::list<std::string> parse_list_scan(std::istream& inp) {
    
    XmlReader r{inp, 0};
    std::list<std::string> ls_res;

    for (auto node_it = r.current(); node_it != r.end(); ++node_it ) {

        if (node_it->type() != Node::StartElement) {
            continue;
        }
        
        const StartElement& el = static_cast<const StartElement&>(*node_it);

        auto k = convert<std::string>(el.name());
        if (k != "address") {
            continue;
        }

        auto addr = convert<String>("addr");
        if (!el.hasAttribute(addr)) {
            continue;
        }

        auto v = convert<std::string>(el.attribute(addr));
        ls_res.push_back(v);
    }

    return ls_res;
}

std::list<std::string> parse_list_scan(const std::string& inp) {
    std::istringstream stream{inp};
    return parse_list_scan(stream);
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

/*
int main() {

    JsonSerializer js{std::cout};
    js.beautify(true);
    SerializationInfo si;

    std::ifstream f{"device-scan.xml"};
    parse_device_scan(f);

    //js.serialize(si).finish();
}
*/
