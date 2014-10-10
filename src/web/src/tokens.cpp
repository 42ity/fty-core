#include <stdio.h>
#include <time.h>
#include <exception>
#include <mutex>
#include <cxxtools/base64codec.h>

#include "tokens.h"

tokens* tokens::get_instance() {
    static tokens* inst = NULL;
    static std::mutex mtx;
    mtx.lock();
    if(!inst) {
        inst = new tokens;
        randombytes_buf(inst->nonce, sizeof nonce);
        randombytes_buf(inst->key, sizeof key);
    }
    mtx.unlock();
    return inst;
}

std::string tokens::gen_token(int& valid) {
    unsigned char ciphertext[CIPHERTEXT_LEN];
    char buff[MESSAGE_LEN+1];
    long int tme = (time(NULL) + valid) / ROUND;
    tme *= ROUND;
    valid = (tme - time(NULL));

    for (int i = 0; i < MESSAGE_LEN; i++)
        buff[i] = 0x20;

    sprintf(buff, "%ld", tme);
    buff[strlen(buff)] = 0x20;
    buff[16] = 0;

    crypto_secretbox_easy(ciphertext, (unsigned char*)buff, MESSAGE_LEN, nonce, key);

    return cxxtools::Base64Codec::encode((char*)ciphertext, CIPHERTEXT_LEN);
}

void tokens::decode_token(char* buff, const std::string token) {
    std::string data;

    try {
        data = cxxtools::Base64Codec::decode(token);
    } catch(std::exception& e) {
        data = "";
    }

    if((crypto_secretbox_open_easy((unsigned char*)buff,
                                    (const unsigned char*)data.c_str(),
                                    data.length(), nonce, key) != 0) ||
      (data.empty())) {
        for (int i = 0; i <= MESSAGE_LEN; i++) buff[i] = 0;
    }
}

bool tokens::verify_token(const std::string token) {
    char buff[MESSAGE_LEN+1];
    long int tme=0;
    decode_token(buff, token);
    sscanf(buff, "%ld", &tme);
    return tme>time(NULL);
}
