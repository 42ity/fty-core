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

tokens *tokens::get_instance(bool recreate) {
    static tokens *inst = NULL;
    static std::mutex mtx;
    mtx.lock();
    if (recreate && inst!=NULL) {
        delete inst;
        inst=NULL;
    }
    if (!inst) {
        inst = new tokens;
        randombytes_buf(inst->nonce, sizeof nonce);
        randombytes_buf(inst->key, sizeof key);
    }
    mtx.unlock();
    return inst;
}

std::string tokens::gen_token(int& valid, const char* user, bool do_round) {
    unsigned char ciphertext[CIPHERTEXT_LEN];
    char buff[MESSAGE_LEN + 1];
    long int tme = (time(NULL) + valid);
    long int uid = -1;
    if (do_round) {
        tme /= ROUND;
        tme *= ROUND;
        valid = (tme - time(NULL));
    }

    for (int i = 0; i < MESSAGE_LEN; i++) {
        buff[i] = 0x20;
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

    snprintf(buff, MESSAGE_LEN, "%ld %ld", tme, uid);
    buff[strlen(buff)] = 0x20;
    buff[MESSAGE_LEN] = 0;

    crypto_secretbox_easy(ciphertext, (unsigned char *)buff, MESSAGE_LEN, nonce, key);

    return cxxtools::Base64Codec::encode((char *)ciphertext, CIPHERTEXT_LEN);
}

void tokens::decode_token(char *buff, const std::string token) {
    std::string data;

    try {
        data = cxxtools::Base64Codec::decode(token);
    } catch (std::exception& e) {
        data = "";
    }

    if ((crypto_secretbox_open_easy((unsigned char *)buff,
                                    (const unsigned char *)data.c_str(),
                                    data.length(), nonce, key) != 0) ||
        (data.empty())) {
        for (int i = 0; i <= MESSAGE_LEN; i++) {
            buff[i] = 0;
        }
    }
}

bool tokens::verify_token(const std::string token, long int* uid) {
    char buff[MESSAGE_LEN + 1];
    long int tme = 0;
    decode_token(buff, token);
    if(uid != NULL) {
        sscanf(buff, "%ld %ld", &tme, uid);
    } else {
        sscanf(buff, "%ld", &tme);
    }
    return tme > time(NULL);
}
