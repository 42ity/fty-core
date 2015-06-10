
#include "log.h"
#include "subprocess.h"
#include "nutscan.h"

#include <iostream>

int main(int argc, char** argv) {

    if (argc != 3) {
        log_error("Usage: ./bios-email name ip");
        return -1;
    }

    std::string nut_config;
    auto ret = shared::nut_scan_snmp(argv[1], argv[2], nut_config);

    if (ret != 0) {
        std::cerr << "fail!" << std::endl;
        return 1;
    }

    std::cout << nut_config << std::endl;
    return 0;
}
