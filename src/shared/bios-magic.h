#ifndef SRC_WEB_INCLUDE_ENC_H
#define SRC_WEB_INCLUDE_ENC_H

#include <string>
#include <utility>
#include <iostream>

namespace shared {
/*
 *  \brief return file (type, encoding)
 *
 *  \throws std::logic_error if magic cookie can't be loaded,
 *          database can't be loaded or file can't be analyzed
 */
std::pair<std::string, std::string>
file_type_encoding(const char* path);

// quick and dirty convert function, NEVER use it!!
void
convert_file(
        std::string::const_iterator begin,
        std::string::const_iterator end,
        std::string& out);

} //namespace shared

#endif //SRC_WEB_INCLUDE_ENC_H
