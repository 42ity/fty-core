#pragma once

#include <map>
#include <string>
#include <iostream>

// TODO: Follow coding conventions
class SafeString {
    private:
        std::string data;
        // TODO: Find a better way how to lock const references
        std::mutex* lock_data;
    public:
        SafeString() { lock_data = new std::mutex; }
        SafeString(const std::string& other);
        SafeString(const SafeString& other);
        ~SafeString() {
            delete lock_data;
        }
        SafeString& operator=(const SafeString& other);
        SafeString& operator=(const std::string& other);
        operator std::string() const;
};

std::ostream& operator<< (std::ostream& out, const SafeString& str);

class KeyValueDB {

    private:
        std::map<std::string, SafeString> dict;
        std::mutex lock_dict;

    public:
        KeyValueDB();
        KeyValueDB(const std::map<std::string, SafeString>& d);
        
        /*MVY: do we need to have destructor defined?
        ~KeyValueDB() {
        }
        */


        KeyValueDB(const KeyValueDB& other);
        KeyValueDB(KeyValueDB&& other);
        
        KeyValueDB& operator= (KeyValueDB& other);
        KeyValueDB& operator= (KeyValueDB&& other);
        
        bool has_key(const std::string& key) const;
        
        SafeString& operator[] (const std::string& key);
        SafeString& operator[] (std::string&& key);
        SafeString& at(const std::string key);
        const SafeString& at( const std::string& key ) const;

};
