#include <mutex>

#include "kvdb.h"

SafeString::SafeString(const std::string& other) {
    lock_data = new std::mutex;
    std::lock_guard<std::mutex> lg(*lock_data);
    data = other;
}

SafeString::SafeString(const SafeString& other) {
    lock_data = new std::mutex;
    std::lock_guard<std::mutex> lg(*lock_data);
    data = other.data;
}

SafeString& SafeString::operator=(const std::string& other) {
    std::lock_guard<std::mutex> lg(*lock_data);
    data = other;
    return *this;
}

SafeString& SafeString::operator=(const SafeString& other) {
    std::lock_guard<std::mutex> lg(*lock_data);
    std::lock_guard<std::mutex> ot_lg(*other.lock_data);
    data = other.data;
    return *this;
}

SafeString::operator std::string() const{
    std::lock_guard<std::mutex> lg(*lock_data);
    return data;
}

std::ostream& operator<< (std::ostream& out, const SafeString& str) {
    return out << std::string(str);
}

KeyValueDB::KeyValueDB():
    dict{},
    lock_dict{} {
}

KeyValueDB::KeyValueDB(const std::map<std::string, SafeString>& d):
    dict{d},
    lock_dict{} {
}

/*MVY: do we need to have destructor defined?
~KeyValueDB() {
}
*/


KeyValueDB::KeyValueDB(const KeyValueDB& other):
    dict{other.dict} {
}

KeyValueDB::KeyValueDB(KeyValueDB&& other):
    dict{other.dict} {
}

KeyValueDB& KeyValueDB::operator= (KeyValueDB& other) {
    this->dict.clear();
    this->dict = other.dict;
    return *this;
}

KeyValueDB& KeyValueDB::operator= (KeyValueDB&& other) {
    this->dict.clear();
    this->dict = other.dict;
    other.dict.clear();
    return *this;
}

bool KeyValueDB::has_key(const std::string& key) const {
    return this->dict.find(key) != this->dict.end();
}

SafeString& KeyValueDB::operator[] (const std::string& key) {
    std::lock_guard<std::mutex> lg(lock_dict);
    return this->dict[key];
}

SafeString& KeyValueDB::operator[] (std::string&& key) {
    std::lock_guard<std::mutex> lg(lock_dict);
    return this->dict[key];
}

SafeString& KeyValueDB::at(const std::string key) {
    std::lock_guard<std::mutex> lg(lock_dict);
    return this->dict.at(key);
}

const SafeString& KeyValueDB::at( const std::string& key ) const {
    return this->dict.at(key);
}
