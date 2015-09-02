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
 * \file tokens.cc
 * \author Michal Hrusecky <MichalHrusecky@Eaton.com>
 * \author Jim Klimov <EvgenyKlimov@Eaton.com>
 * \brief Not yet documented file
 */
#include <stdio.h>
#include <time.h>
#include <exception>
#include <mutex>
#include <cxxtools/base64codec.h>
#include <sys/types.h>
#include <pwd.h>
#include <mutex>

#include "tokens.h"

//! Max time key is alive
#define MAX_LIVE 24*3600
//! Maximum tokens per key
#define MAX_USE 256

void tokens::regen_keys() {
    while(!keys.empty() && keys.front().valid_until < time(NULL))
        keys.pop_front();
    if(keys.empty() || keys.back().used > MAX_USE ||
                       keys.back().valid_until < time(NULL) - MAX_LIVE) {
        cipher new_cipher;
        randombytes_buf(new_cipher.nonce, sizeof(new_cipher.nonce));
        randombytes_buf(new_cipher.key, sizeof(new_cipher.key));
        new_cipher.valid_until = time(NULL);
        new_cipher.valid_until += 2*MAX_LIVE;
        new_cipher.used = 0;
        keys.push_back(new_cipher);
    }
}

tokens *tokens::get_instance() {
    static tokens *inst = NULL;
    static std::mutex mtx;
    mtx.lock();
    if(!inst) {
        inst = new tokens;
    }
    mtx.unlock();
    return inst;
}

std::string tokens::gen_token(int& valid, const char* user, bool do_round) {
    unsigned char ciphertext[CIPHERTEXT_LEN];
    char buff[MESSAGE_LEN + 1];
    long int tme = ((long int)time(NULL) + std::min((long int)valid, (long int)MAX_LIVE));
    static int number = random() % MAX_USE;
    int my_number;
    long int uid = -1;
    if (do_round) {
        tme /= ROUND;
        tme *= ROUND;
        valid = (tme - time(NULL));
    }

    if(user != NULL) {
        static std::mutex pwnam_lock;
        pwnam_lock.lock();
        struct passwd *pwd = getpwnam(user);
        if(pwd != NULL) {
            uid = pwd->pw_uid;
        }
        pwnam_lock.unlock();
    }

    static std::mutex mtx;
    mtx.lock();
    regen_keys();
    cipher tmp = keys.back();
    keys.back().used++;
    my_number = number;
    number = (number + 1) % MAX_USE;
    mtx.unlock();

    snprintf(buff, MESSAGE_LEN, "%ld %ld %d", tme, uid, my_number);

    crypto_secretbox_easy(ciphertext, (unsigned char *)buff, strlen(buff),
                          tmp.nonce, tmp.key);
    ciphertext[crypto_secretbox_MACBYTES + strlen(buff)] = 0;
    std::string ret = cxxtools::Base64Codec::encode((char *)ciphertext,
                                    crypto_secretbox_MACBYTES + strlen(buff));
    for(auto &i: ret) {
        if(i == '+')
            i = '_';
        if(i == '/')
            i = '-';
    }
    return ret;
}

void tokens::decode_token(char *buff, std::string token) {
    std::string data;

    for(auto &i: token) {
        if(i == '_')
            i = '+';
        if(i == '-')
            i = '/';
    }

    try {
        data = cxxtools::Base64Codec::decode(token);
    } catch (std::exception& e) {
        data = "";
    }

    for(auto i: keys) {
        if(crypto_secretbox_open_easy((unsigned char *)buff,
                                        (const unsigned char *)data.c_str(),
                                        data.length(), i.nonce, i.key) == 0) {
            return;
        }
    }
    for(int i = 0; i <= MESSAGE_LEN; i++)
        buff[i] = 0;
}

void tokens::clean_revoked() {
    std::multimap<long int, std::string>::iterator it;
    while(!revoked_queue.empty() &&
          (it = revoked_queue.begin())->first < time(NULL)) {
        revoked.erase(it->second);
        revoked_queue.erase(it);
    }
}

void tokens::revoke(const std::string token) {
    char buff[MESSAGE_LEN + 1];
    long int tme = 0;
    decode_token(buff, token);
    sscanf(buff, "%ld", &tme);
    if(tme <= time(NULL))
        return;
    revoked.insert(token);
    revoked_queue.insert(std::make_pair(tme, token));
}

bool tokens::verify_token(const std::string token, long int* uid) {
    char buff[MESSAGE_LEN + 1];
    long int tme = 0;
    clean_revoked();
    if(revoked.find(token) != revoked.end())
        return false;
    decode_token(buff, token);
    if(uid != NULL) {
        sscanf(buff, "%ld %ld", &tme, uid);
    } else {
        sscanf(buff, "%ld", &tme);
    }
    return tme > time(NULL);
}
