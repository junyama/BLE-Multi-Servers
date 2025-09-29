#ifndef MY_AES_HPP
#define MY_AES_HPP

#include <Arduino.h>
#include "MyLog.hpp"
#include "mbedtls/aes.h"
// #include "mbedtls/cipher.h"
// #include <string.h>

// #define INPUT_LENGTH 16

class MyAes
{
private:
    const char *TAG = "MyAes";
    // Define the AES-128 key (16 bytes)
    // const unsigned char aesKey[16] = {0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C};
    const unsigned char aesKey[16] = {108, 101, 97, 103, 101, 110, 100, 255, 254, 48, 49, 48, 48, 48, 48, 57};
    // Define a 16-byte plaintext block for encryption
    const unsigned char plainText[16] = {0x6B, 0xC1, 0xBE, 0xE2, 0x2E, 0x40, 0x9F, 0x96, 0xE9, 0x3D, 0x7E, 0x11, 0x73, 0x93, 0x17, 0x2A};

    mbedtls_aes_context aes;

public:
    // Buffers for ciphertext and decrypted text
    unsigned char cipherText[16];
    unsigned char decryptedText[16];

    MyAes();
    void encrypt();
    void decrypt(char *inputText);
    void test();
    // void decrypt(unsigned char *input, char *key, unsigned char *output);
};

#endif /* MY_AES_HPP */