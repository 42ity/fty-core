#include <string>
#include "utilities.h"

namespace utils
{

void print_usage(void) {
  const char *usage = "\
NAME\n\
\tmain - Le controleur du Fromage a l'odeur etrange\n\
\n\
USAGE\n\
\tmain [<sleep_min> <sleep_max>]\n\
\n\
DESCRIPTION\n\
\tThis program is our first shot at a real architecture (first iteration of,).\
 bla bla bla TODO finish the DESCIPTION\n\
\n\
OPTIONS\n\
\t<sleep_min> <sleep_max>\tminimum and maximum number of seconds to sleep\
before having the netmon module send next event. These two arguments are\
not mandatory, if omitted, default values of (1, 3) are used.\n\
\n\
VERSION\n\
  0.1.1\n\
  arch:\n\
  ~~~~~\n\
           views, modules         proxy            workers\n\
  asynchronous DEALER <---> ROUTER <-> DEALER <---> DEALER \n\
  wireproto:\tjson\n\
  ~~~~~~~~~~\n\
  modules:\tmock_netmon, mock_cli\n\
  ~~~~~~~~\n\
";
  printf("%s\n", usage);
}

} // namespace utils
