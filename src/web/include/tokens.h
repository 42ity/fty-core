/**
 * \brief Header file for token manipulation class
 *
 */
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
    std::string gen_token(int& valid, bool do_round = true);
    /**
     * \brief Verifies whether supplied token is valid
     */
    bool verify_token(const std::string token);
    /**
     * \brief Decodes token, useful for debugging
     */
    void decode_token(char* buff, const std::string token);
};
