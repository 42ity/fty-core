
#include "log.h"
#include "subprocess.h"
#include "nutscan.h"

#include <iostream>
#include <string>

int main(int argc, char** argv) {

    if (argc != 4) {
        log_error("Usage: ./bios-nutscan [snmp|xml] name ip");
        return -1;
    }

    std::string proto{argv[1]};

    if (proto != "snmp" && proto != "xml") {
        std::cerr << "first argument must be snmp or xml, got " << argv[1] << std::endl;
        return -1;
    }

    nutscan_init();
    for( int N = 1 ; N < 3 ; ++N ) {
        std::cout << "pass: " << N << std::endl;
        std::vector<std::string> nut_config;

        int ret = -1;
        if (proto == "snmp") {
            ret = shared::nut_scan_snmp(
                argv[2],
                shared::CIDRAddress(argv[3]),
                nut_config);
        }
        else {
            ret = shared::nut_scan_xml_http(
                argv[2],
                shared::CIDRAddress(argv[3]),
                nut_config);
        }

        if (ret != 0) {
            std::cerr << "fail!" << std::endl;
            return 1;
        }

        size_t i = 0;
        for (const auto& s: nut_config) {
            std::cout << i << ":" << std::endl;
            i++;
            std::cout << s << std::endl;
        }
        nutscan_free();
    }
    return 0;
}
