/* Simple buffer example for low-memory conditions (Arduino) */

#include "MyAes.hpp"

MyAes::MyAes()
{
  // Initialize mbedTLS AES context
  // mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);

  // Set the encryption key
  mbedtls_aes_setkey_enc(&aes, aesKey, 128); // 128-bit key
}

void MyAes::encrypt()
{
  // Perform encryption
  mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, plainText, cipherText);

  Serial.print("Plaintext: ");
  for (int i = 0; i < 16; i++)
  {
    Serial.printf("%02X ", plainText[i]);
  }
  Serial.println();

  Serial.print("Ciphertext: ");
  for (int i = 0; i < 16; i++)
  {
    Serial.printf("%02X ", cipherText[i]);
  }
  Serial.println();
}

void MyAes::decrypt(char *inputText)
{
  // unsigned char inputText[16] = {0x8A, 0xE0, 0x35, 0x85, 0x23, 0xFE, 0x15, 0x49, 0x76, 0xDE, 0x18, 0xFC, 0x10, 0x2E, 0xB1, 0xD4};
  for (int i = 0; i < 16; i++)
  {
    cipherText[i] = inputText[i];
  }
  // Set the decryption key
  mbedtls_aes_setkey_dec(&aes, aesKey, 128); // 128-bit key

  // Perform decryption
  mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, cipherText, decryptedText);

  if (MyLog::DEBUG)
  {
    DEBUG_PRINT("Decrypted text[16]: ");
    for (int i = 0; i < 16; i++)
    {
      Serial.printf("0x%02X, ", decryptedText[i]);
    }
    Serial.println();
  }
}

void MyAes::test()
{
  Serial.println("AES-128 ECB Encryption/Decryption Example on ESP32");

  // Perform encryption
  mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, plainText, cipherText);

  Serial.print("Plaintext: ");
  for (int i = 0; i < 16; i++)
  {
    Serial.printf("%02X ", plainText[i]);
  }
  Serial.println();

  Serial.print("Ciphertext: ");
  for (int i = 0; i < 16; i++)
  {
    Serial.printf("%02X ", cipherText[i]);
  }
  Serial.println();

  // Set the decryption key
  mbedtls_aes_setkey_dec(&aes, aesKey, 128); // 128-bit key

  // Perform decryption
  mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, cipherText, decryptedText);

  Serial.print("Decrypted Text: ");
  for (int i = 0; i < 16; i++)
  {
    Serial.printf("%02X ", decryptedText[i]);
  }
  Serial.println();

  // Free the AES context
  mbedtls_aes_free(&aes);
}

/*
void MyAes::decrypt(unsigned char *input, char *key, unsigned char *output)
{
  unsigned char iv[] = {0xff, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_enc(&aes, (const unsigned char *)key, strlen(key) * 8);
  mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, INPUT_LENGTH, iv, (const unsigned char *)input, output);
  mbedtls_aes_free(&aes);
}
*/