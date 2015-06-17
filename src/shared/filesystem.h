/* 
Copyright (C) 2014 - 2015 Eaton
 
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
Author(s): Tomas Halman <tomashalman@eaton.com>
 
Description: various random C and project wide helpers
*/

#ifndef SRC_SHARED_FILESYSTEM_H
#define SRC_SHARED_FILESYSTEM_H

// #include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include <string>

namespace shared {

/**
 * \brief returns "/"
 */
const char *path_separator();

/**
 * \brief get the file mode
 * \param path to the file
 */
mode_t file_mode( const char *path );

/**
 * \brief return true if path exists and it is a regular file
 * \param path to the file
 */
bool is_file( const char  *path );

/**
 * \brief return true if path exists and it is a directory
 * \param path to the directory
 */
bool is_dir( const char  *path );

/**
 * \brief get list of all items in directory
 * \param path to the directory
 */
std::vector<std::string> items_in_directory( const char *path );

/**
 * \brief get list of all regular files in directory
 * \param path to the directory
 */
std::vector<std::string> files_in_directory( const char *path );

/**
 * \brief create directory (if not exists
 * \param path to the newly created directory
 * \param mode (rights)
 * \param create parent directories if needed
 *
 * In case of failure also errno is set, see "man 3 mkdir" for details. 
 */
bool mkdir_if_needed(const char *path, mode_t mode = 0x755, bool create_parent=true );

} // namespace shared

#endif
