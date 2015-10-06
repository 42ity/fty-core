#include "dbpath.h"

#include <stdlib.h>

std::string url = std::string("mysql:db=box_utf8;user=") +
                  ((getenv("DB_USER")   == NULL) ? "root" : getenv("DB_USER")) +
                  ((getenv("DB_PASSWD") == NULL) ? ""     : 
                      std::string(";password=") + getenv("DB_PASSWD"));
