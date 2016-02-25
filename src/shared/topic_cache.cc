/*
 *
 * Copyright (C) 2015 Eaton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*!
 * \file topic_cache.cc
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \author GeraldGuillaume <GeraldGuillaume@Eaton.com>
 * \brief map of (topic name, topic id) keys
 */
#include "topic_cache.h"

namespace persist {

bool TopicCache::has(const std::string& topic_name) const {
    if (_cache.count(topic_name) == 1)
        return true;

    return false;
}

void TopicCache::add(const std::string& topic_name, int topic_id) {
    if (_max <= _cache.size())
    {
        _cache.clear();
    }
    _cache.insert(std::make_pair (topic_name, topic_id));
}

int TopicCache::get(const std::string& topic_name) {
    if(!has(topic_name))return 0;
    return _cache[topic_name];
}

}
