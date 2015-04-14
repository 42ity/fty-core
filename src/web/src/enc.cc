#include <magic.h>
#include <stdexcept>

#include "enc.h"

std::string file_encoding(const char* path) {
    magic_t magic_cookie = magic_open(MAGIC_MIME_ENCODING | MAGIC_ERROR | MAGIC_NO_CHECK_COMPRESS | MAGIC_NO_CHECK_TAR);
    if (! magic_cookie) {
        magic_close(magic_cookie);
        throw std::logic_error("fail to open magic_cookie");
    }

    //int r = magic_load(magic_cookie, magic_getpath(NULL, 0));
    int r = magic_load(magic_cookie, NULL);
    if (r == -1) {
        magic_close(magic_cookie);
        throw std::logic_error("fail to load the magic database");
    }

    const char* magic = magic_file(magic_cookie, path);
    if (! magic) {
        magic_close(magic_cookie);
        std::string msg{"fail to get the mime for file "};
        msg += path;
        msg += ": ";
        msg += magic_error(magic_cookie);
        throw std::logic_error(msg.c_str());
    }

    std::string ret{magic};
    magic_close(magic_cookie);
    return ret;
}
