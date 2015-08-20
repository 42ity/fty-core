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
 * \author Michal Hrusecky
 * \author Alena Chernikava
 * \author Jim Klimov
 * \brief Not yet documented file
 */
/**
 * \brief Header file for token manipulation class
 *
 */

#ifndef SRC_WEB_INCLUDE_TOKENS_H
#define SRC_WEB_INCLUDE_TOKENS_H

#include <string>
#include <sodium.h>

//! Maximum length of the message stored in the token
#define MESSAGE_LEN 16
//! Round timestamps to this many seconds
#define ROUND 60
//! Length of the ciphertext
#define CIPHERTEXT_LEN (crypto_secretbox_MACBYTES + MESSAGE_LEN)

//! Class to generate and verify tokens
class tokens {
private:
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    unsigned char key[crypto_secretbox_KEYBYTES];
    void regen_keys();
public:
    //! Singleton get_instance method
    static tokens* get_instance(bool recreate = false);
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
    bool verify_token(const std::string token, long int* uid = NULL);
    /**
     * \brief Decodes token, useful for debugging
     */
    void decode_token(char* buff, const std::string token);
};

#endif // SRC_WEB_INCLUDE_TOKENS_H
