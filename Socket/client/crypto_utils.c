#include "crypto_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/buffer.h>

void handleErrors() {
    ERR_print_errors_fp(stderr);
    abort();
}

char* generate_rsa_pubkey_pem() {
    EVP_PKEY_CTX *pctx = NULL;
    EVP_PKEY *pkey = NULL;
    BIO *pub_bio = NULL;
    char *pubkey_pem = NULL;
    size_t pub_len = 0;

    ERR_load_crypto_strings();

    pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (!pctx) handleErrors();

    if (EVP_PKEY_keygen_init(pctx) <= 0) handleErrors();

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(pctx, 4096) <= 0) handleErrors();

    if (EVP_PKEY_keygen(pctx, &pkey) <= 0) handleErrors();

    pub_bio = BIO_new(BIO_s_mem());
    if (!PEM_write_bio_PUBKEY(pub_bio, pkey)) handleErrors();

    pub_len = BIO_get_mem_data(pub_bio, &pubkey_pem);

    char *pubkey_copy = malloc(pub_len + 1);
    if (!pubkey_copy) {
        fprintf(stderr, "malloc failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(pubkey_copy, pubkey_pem, pub_len);
    pubkey_copy[pub_len] = '\0';

    BIO_free(pub_bio);
    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(pctx);
    ERR_free_strings();

    return pubkey_copy;
}

char* encrypt_with_pubkey(const char* pubkey_pem, const char* plaintext) {
    BIO *bio = BIO_new_mem_buf(pubkey_pem, -1);
    if (!bio) return NULL;

    EVP_PKEY *pubkey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
    BIO_free(bio);
    if (!pubkey) return NULL;

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pubkey, NULL);
    if (!ctx) {
        EVP_PKEY_free(pubkey);
        return NULL;
    }
    if (EVP_PKEY_encrypt_init(ctx) <= 0) goto err;

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) goto err;

    size_t outlen = 0;
    if (EVP_PKEY_encrypt(ctx, NULL, &outlen, (unsigned char*)plaintext, strlen(plaintext)) <= 0) goto err;

    unsigned char *outbuf = malloc(outlen);
    if (!outbuf) goto err;

    if (EVP_PKEY_encrypt(ctx, outbuf, &outlen, (unsigned char*)plaintext, strlen(plaintext)) <= 0) {
        free(outbuf);
        goto err;
    }

    BIO *b64 = BIO_new(BIO_f_base64());
    BIO *bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(b64, outbuf, outlen);
    BIO_flush(b64);

    BUF_MEM *bptr;
    BIO_get_mem_ptr(b64, &bptr);

    char *b64text = malloc(bptr->length + 1);
    if (!b64text) {
        BIO_free_all(b64);
        free(outbuf);
        goto err;
    }
    memcpy(b64text, bptr->data, bptr->length);
    b64text[bptr->length] = '\0';

    BIO_free_all(b64);
    free(outbuf);
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pubkey);

    return b64text;

err:
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pubkey);
    return NULL;
}
