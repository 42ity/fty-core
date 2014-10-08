#include <czmq.h>
#include "netdisc_msg.h"

namespace utils{

bool
process_message(std::string url, netdisc_msg_t *msg);
}
