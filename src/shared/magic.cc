#include <stdexcept>
#include <cerrno>
#include <unistd.h>
#include <sstream>
#include <fstream>

#include <magic.h>

#include "bios-magic.h"
#include "subprocess.h"
#include "log.h"

namespace shared {

// helper function to use libmagic
static std::string
s_magic(
    const char* path,
    int flags)
{
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

std::pair<std::string, std::string>
file_type_encoding(const char* path)
{

    int flags = MAGIC_ERROR | MAGIC_NO_CHECK_COMPRESS | MAGIC_NO_CHECK_TAR;

    auto file_type = s_magic(path, flags);
    auto encoding = s_magic(path, MAGIC_MIME_ENCODING | flags);

    return std::make_pair(file_type, encoding);
}

void
convert_file(
        std::string::const_iterator begin,
        std::string::const_iterator end,
        std::string& out_path)
{
    char path[30]     = "/tmp/convert-file-from-XXXXXX";
    char new_path[30] = "/tmp/convert-file-to-XXXXXX";
    char* path_p = path;
    LOG_START;

    int r = ::mkstemp(path);
    if (r == -1) {
        throw std::runtime_error( ::strerror(errno) );
    }
    ::close(r);

    std::ofstream temp(path);
    // store the content in temporary location
    //for (tnt::Part::const_iterator pi = it->getBodyBegin(); pi != it->getBodyEnd(); ++pi)
    for (auto pi = begin; pi != end; ++pi)
        temp << *pi;

    temp.flush();
    temp.close();

    // check the encoding
    auto magic = shared::file_type_encoding(path);
    const auto& encoding = magic.second;
    if (encoding != "utf-8" && encoding != "utf-16le" && encoding != "us-ascii") {
        std::string msg = "This file (" + magic.first + ") is not supported please try again with a different file.";
        ::unlink(path);
        throw std::invalid_argument( msg.c_str() );
    }

    if (encoding == "utf-16le") {
        // convert utf-16le using iconv
        int r2 = ::mkstemp(new_path);
        if (r2 == -1) {
            throw std::runtime_error( ::strerror(errno) );
        }
        ::close(r2);

        // iconv -f utf-16le -t utf-8 Book1.txt > Book1.utf8.txt
        shared::Argv args = {"/usr/bin/iconv", "-f", encoding, "-t", "utf-8", path, "-o", new_path};
        int res = shared::call(args);
        ::unlink(path);
        if (res != 0) {
            ::unlink(new_path);
            throw std::runtime_error("Can't convert input file to utf-8");
        }
        path_p = new_path;
    }
    out_path = path_p;
}


} //namespace shared
