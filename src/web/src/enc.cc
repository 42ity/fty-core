#include <magic.h>
#include <stdexcept>

#include <enc.h>

// helper function to use libmagic
static std::string s_magic(const char* path, int flags) {
    magic_t magic_cookie = magic_open(flags);
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
        std::string msg = std::string{"fail to get the mime type for file "} + path + ": " + magic_error(magic_cookie);
        throw std::logic_error(msg.c_str());
    }

    std::string ret{magic};
    magic_close(magic_cookie);
    return ret;
}

std::pair<std::string, std::string> file_type_encoding(const char* path) {

    int flags = MAGIC_ERROR | MAGIC_NO_CHECK_COMPRESS | MAGIC_NO_CHECK_TAR;

    auto file_type = s_magic(path, flags);
    auto encoding = s_magic(path, MAGIC_MIME_ENCODING | flags);

    return std::make_pair(file_type, encoding);
}
