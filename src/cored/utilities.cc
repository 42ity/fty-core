/* 
Copyright (C) 2014 Eaton
 
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
Author: Karol Hrdina <karolhrdina@eaton.com>
 
Description: utilities
*/

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
\tmain\n\
\n\
DESCRIPTION\n\
\tThis program is our first shot at a real architecture (first iteration of,).\
 bla bla bla TODO finish the DESCIPTION\n\
\n\
OPTIONS\n\
\tNone at the moment\n\
\n\
VERSION\n\
  0.1.1\n\
  arch:\n\
  ~~~~~\n\
           views, modules         proxy            workers\n\
  asynchronous DEALER <---> ROUTER <-> DEALER <---> DEALER \n\
  wireproto:\tjson\n\
  ~~~~~~~~~~\n\
  modules:\tnetmon, cli\n\
  ~~~~~~~~\n\
";
  printf("%s\n", usage);
}

} // namespace utils
