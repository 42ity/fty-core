#include <cxxtools/convert.h>
#include <cxxtools/string.h>
#include <cxxtools/xml/xmlreader.h>
#include <cxxtools/xml/node.h>
#include <cxxtools/xml/startelement.h>
#include <cxxtools/xml/endelement.h>

#include <cxxtools/jsonserializer.h>
#include <cxxtools/serializationinfo.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

#include <cassert>

// parse nmap xml output and flush JSON

using namespace cxxtools::xml;
using namespace cxxtools;

//! \brief parse results of nmap list scan -sL and return a list of ip addresses
//
// \param inp - input stream
// \return list of strings with ip addresses
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

int main() {

    JsonSerializer js{std::cout};
    js.beautify(true);
    SerializationInfo si;

    std::ifstream f{"list-scan.xml"};
    std::list<std::string> ls_res = parse_list_scan(f);

    si <<= ls_res;
    //for (auto s: ls_res) {
        //std::cout << s << std::endl;
    //}
    js.serialize(si).finish();
}
