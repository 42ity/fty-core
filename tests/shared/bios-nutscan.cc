
#include "log.h"
#include "subprocess.h"
#include "nutscan.h"

#include <iostream>

int main(int argc, char** argv) {

    if (argc != 3) {
        log_error("Usage: ./bios-nutscan name ip");
        return -1;
    }
	nutscan_init();

    std::vector<std::string> nut_config;
    auto ret = shared::nut_scan_snmp(
            argv[1],
            shared::CIDRAddress(argv[2]),
            NULL,
            nut_config);

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
    return 0;
}
