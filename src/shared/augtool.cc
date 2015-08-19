#include <vector>
#include <string>
#include <cxxtools/split.h>

// Helper function to parse output of augtool
std::string real_out(const std::string in, bool key_value = true, std::string sep = "") {
    std::vector<std::string> spl;
    bool not_first = false;
    std::string out;
    cxxtools::split("\n", in, std::back_inserter(spl));
    if(spl.size() >= 3) {
        spl.erase(spl.begin());
        spl.pop_back();
    } else {
        return out;
    }
    for(auto i : spl) {
        auto pos = i.find_first_of("=");
        if(pos == std::string::npos) {
            if(key_value)
                continue;
            if(not_first)
                out += sep;
            out += i;
        } else {
            if(not_first)
                out += sep;
            out += i.substr(pos+2);
        }
        not_first = true;
    }
    return out;
}

