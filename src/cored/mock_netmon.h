#ifndef MOCK_NETMON_H_
#define MOCK_NETMON_H_

namespace mocks {

/*!
\brief Mock the netmon module behaviour.
       Send an event every (min, max) seconds.

Currently send the following events: ADD, REMOVE
A std::map keeps track of added networks in order to have realistic REMOVE
events. In case an adrress is generated that is already present, the cycle is
simply skipped.

\param[in]	min	minimum number of seconds to sleep before sending next event
\param[in]	max	maximum number of seconds to sleep before sending next event
\param[in]	connection zeromq endpoint, format [transport]://[address]
*/
void netmon(short min, short max, const char *connection);

static const unsigned int IFCOUNT = 4; //!< number of interfaces to generate
static const char *IFNAMES[IFCOUNT] =
{
  "eth0", "eth1", "enp25s", "br01"
}; //!< names of the interfaces to generate
static const char *IPVERSION_IPV4_STR = "IPV4";
static const char *IPVERSION_IPV6_STR = "IPV6";
static const char *EVENT_NETWORK_ADD_STR = "ADD";
static const char *EVENT_NETWORK_DEL_STR = "REMOVE";

} // namespace mocks

#endif // MOCK_NETMON_H_
