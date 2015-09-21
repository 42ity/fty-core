#include <map>
#include <string>

/**
 * \brief Rights management structure
 *
 * Put here list of urls and the required access level. First entry is string
 * starting with letter "R" or "W". This denotes whether we are dealing with
 * read access or write access. It is directly followed by url. It doesn't have
 * to be full url - if part that you wrote matches, rule is applied. Longer the
 * string, the more preference it has. Access level is number from -1
 * (Unauthenticated) to 3 (Admin). You specify the least privileges user needs
 * to access this call. So for example if you specify, that
 * "W/api/v1/admin/time" requires level 2, users with level 2 and above can use
 * it even if "W/api/v1/admin" requires level 3.
 *
 */

std::map<std::string, int> rights_management = {
{ "R", -1 },
{ "W", 2 },
{ "W/api/v1/admin", 3 },
};
