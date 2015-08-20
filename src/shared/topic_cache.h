/*
Copyright (C) 2015 Eaton

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/*! \file   topic_cache.h
    \brief  Realisation of small cache for topic
    \author MichalVyskocil <MichalVyskocil@Eaton.com>
    \author AlenaChernikava <AlenaChernikava@Eaton.com>
 */

#ifndef SRC_SHARED_TOPIC_CACHE_H_
#define SRC_SHARED_TOPIC_CACHE_H_

#include <string>
#include <set>

namespace persist {

class TopicCache {
    public:
        explicit TopicCache(size_t max = 1024):
            _cache{},
            _max{max}
        {};

        TopicCache (const TopicCache& other) = delete;
        TopicCache (const TopicCache&& other) = delete;
        TopicCache& operator=(TopicCache& other) = delete;
        TopicCache& operator=(TopicCache&& other) = delete;

        //\brief check if value is in cache or not
        bool has(const std::string& topic) const;

         //\brief add a key to cache
        void add(const std::string& topic);

    private:
        std::set<std::string> _cache;
        size_t _max;
};

}
#endif // SRC_SHARED_TOPIC_CACHE_H_
