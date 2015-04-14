#ifndef SRC_WEB_INCLUDE_ENC_H
#define SRC_WEB_INCLUDE_ENC_H

#include <string>

/*
 *  return file encoding
 *
 *  \throws std::logic_error if magic cookie can't be loaded,
 *          database can't be loaded or file can't be analyzed
 */
std::string file_encoding(const char* path);

#endif //SRC_WEB_INCLUDE_ENC_H
