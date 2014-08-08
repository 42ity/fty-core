#ifndef UTILITIES_ZEROMQ_H_
#define UTILITIES_ZEROMQ_H_

#include <string>
#include <zmq.hpp>

namespace utils
{

namespace zeromq
{

/*!
\brief Convert zmq::message_t to std::string
\param[out] string
\param[in] message
*/
void zmsg_to_str(std::string& string, const zmq::message_t& message);

/*!
\brief Receive zeromq string from socket and convert it to a std::string
\param[in] sock zeromq socket from which to receive
\param[out] string received zmq::message_t converted to string
*/
void str_recv(zmq::socket_t& socket, std::string& string);

/*!
\brief Convert std::string to zmq::message_t and send it to a socket
\param[in] socket socket to which to send the string
\param[in] string string to be sent
\return true if successfull, false otherwise
*/
bool str_send(zmq::socket_t& socket, const std::string& string);

/*!
\brief Convert std::string to zmq::message_t and send it to a socket
        as multipart, non-terminal

A sequence of these calls followed by str_send() allows to send one
multipart message.

\param[in] socket socket to which to send the string
\param[in] string string to be sent
\return true if successfull, false otherwise
*/
bool str_sendmore(zmq::socket_t& socket, const std::string& string);

} // namespace zmq

} // namespace utils

#endif // UTILITIES_ZEROMQ_H_

