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
 * \file tokens.h
 * \author Michal Hrusecky <MichalHrusecky@Eaton.com>
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \author Jim Klimov <EvgenyKlimov@Eaton.com>
 * \brief Header file for token manipulation class
 *
 */

#ifndef SRC_WEB_INCLUDE_TOKENS_H
#define SRC_WEB_INCLUDE_TOKENS_H

#include <string>
#include <sodium.h>
#include <set>
#include <map>
#include <deque>

//! Maximum length of the message stored in the token
#define MESSAGE_LEN 32
//! Round timestamps to this many seconds
#define ROUND 60
//! Length of the ciphertext
#define CIPHERTEXT_LEN (crypto_secretbox_MACBYTES + MESSAGE_LEN)

struct cipher {
    long int valid_until;
    int used;
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    unsigned char key[crypto_secretbox_KEYBYTES];
};

//! Class to generate and verify tokens
class tokens {
private:
    std::deque<cipher> keys;
    std::set<std::string> revoked;
    std::multimap<long int, std::string> revoked_queue;
    void clean_revoked();
    void regen_keys();
public:
    //! Singleton get_instance method
    static tokens* get_instance();
    /**
     * \brief Generates new token
     *
     * @param valid How long should be token valid
     * @return Token
     */
    std::string gen_token(int& valid, const char* user, bool do_round = true);
    /**
     * \brief Verifies whether supplied token is valid
     */
    bool verify_token(const std::string token, long int* uid = NULL, long int* gid = NULL);
    //! Invalidates selected token
    void revoke(const std::string token);
    /**
     * \brief Decodes token, useful for debugging
     */
    void decode_token(char* buff, const std::string token);
};

#endif // SRC_WEB_INCLUDE_TOKENS_H