#include <iostream>
#include <future>
#include <list>
#include <gtest/gtest.h>

#include "kvdb.h"

#define THREADS 2000

TEST(KeyValueDB, SimpleInsert) {
    KeyValueDB db = KeyValueDB();
    db["42"] = "Hello";
    EXPECT_STREQ("Hello", std::string(db["42"]).c_str());    
}

TEST(KeyValueDB, SimpleOverwrite) {
    KeyValueDB db = KeyValueDB();
    db["42"] = "Hello";
    db["42"] = "World";
    EXPECT_STREQ("World", std::string(db["42"]).c_str());    
}

TEST(KeyValueDB, MultithreadOverwrite) {
    KeyValueDB db = KeyValueDB();

    std::list<std::future<void>> futes;
    for (int i = 0; i != THREADS; i++) {
        futes.push_back(
            std::async(
                std::launch::async,
                [&] {
                    db["42"] = "Universe";
                })
            );
    }
    for (auto i = futes.begin(); i != futes.end(); i++ ) {
        i->wait();
    }
    EXPECT_STREQ("Universe", std::string(db["42"]).c_str());    
}

TEST(KeyValueDB, MultithreadRW) {
    KeyValueDB db = KeyValueDB();

    std::list<std::future<void>> futes;
    for (int i = 0; i != THREADS; i++) {
        futes.push_back(
            std::async(
                std::launch::async,
                [&] {
                    db["42"] = std::string(db["42"]) + "i";
                })
            );
    }

    for (auto i = futes.begin(); i != futes.end(); i++ ) {
        i->wait();
    }
    EXPECT_GT(std::string(db["42"]).length(), 0);
}
