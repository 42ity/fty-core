#include "db/inout.h"
#include "log.h"

#include <iostream>
int main() {

    log_set_level(LOG_ERR);
    persist::export_asset_csv(std::cout);

}
