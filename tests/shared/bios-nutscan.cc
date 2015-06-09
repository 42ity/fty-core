
#include "log.h"
#include "subprocess.h"
#include "nutscan.h"

#include <iostream>

int main(int argc, char** argv) {

    if (argc != 3) {
        log_error("Usage: ./bios-email name ip");
        return -1;
    }

    auto res = shared::nut_scan_snmp(argv[1], argv[2]);
    std::cout << res << std::endl;
    return 0;
}
