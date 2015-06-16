#include <dirent.h>
#include <string.h>

#include "filesystem.h"


namespace shared {

const char *path_separator() {
    static const char * sep = "/";
    return sep;
}

mode_t file_mode( const char *path ) {
    struct stat st;

    if( stat( path, &st ) == -1 ) return 0;
    return st.st_mode;
}

inline bool is_file( const char  *path ) {
    return S_ISREG(file_mode( path ));
}

inline bool is_dir( const char  *path ) {
    return S_ISDIR( file_mode( path ) );
}

std::vector<std::string> items_in_directory( const char *path ) {
    std::vector<std::string> result;
    struct dirent* entry;
    
    DIR * dir = opendir( path );
    if(dir) {
        while( ( entry = readdir(dir) ) != NULL ) {
            result.push_back(entry->d_name);
        }
        closedir(dir);
    }
    return result;
}

std::vector<std::string> files_in_directory( const char *path ) {
    std::vector<std::string> result;
    std::string spath = path; spath += path_separator();
    
    for( auto it : items_in_directory( path ) ) {
        if( is_file( (spath + it).c_str() ) ) result.push_back(it);
    }
    return result;
}

bool mkdir_if_needed(const char *path, mode_t mode, bool create_parent ) {
    if( ! path || strlen(path) == 0 ) return false;
    if( is_dir( path ) ) return true;

    if( create_parent ) {
        std::string parent = path;
        size_t i = parent.find_last_of( path_separator() );
        if( i != std::string::npos ) {
            parent = parent.substr(0,i);
            mkdir_if_needed( parent.c_str(), mode, create_parent );
        }
    }
    mkdir(path,mode);
    return false;
}


} // namespace shared
