#include "topic_cache.h"

namespace persist {

bool TopicCache::has(const std::string& topic) const {
    if (_cache.count(topic) == 1)
        return true;

    return false;
}

void TopicCache::add(const std::string& topic) {
    if (_max <= _cache.size())
    {
        _cache.clear();
    }

    _cache.insert(topic);
}

}
