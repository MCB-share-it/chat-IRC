#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

char* generate_rsa_pubkey_pem();
char* encrypt_with_pubkey(const char* pubkey_pem, const char* plaintext);
void handleErrors();

#endif
