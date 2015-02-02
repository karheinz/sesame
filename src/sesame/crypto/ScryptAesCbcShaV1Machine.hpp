// Copyright (c) 2015, Karsten Heinze <karsten.heinze@sidenotes.de>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef SESAME_CRYPTO_SCRYPT_AES_CBC_SHA_V1_MACHINE
#define SESAME_CRYPTO_SCRYPT_AES_CBC_SHA_V1_MACHINE

#include <random>
#include <openssl/evp.h>
#include "IMachine.hpp"


namespace sesame { namespace crypto {

/**
 * Crypto machine for PROTOCOL_SCRYPT_AES_CBC_SHA_V1.
 */
class ScryptAesCbcShaV1Machine : public IMachine
{
   public:
      /** AES block size is always 16 byte. */
      static const uint32_t AES_BLOCK_SIZE;
      /** AES key size used: 32 byte == 256 bit */
      static const uint32_t AES_KEY_SIZE;
      /** AES padding size: 1 byte, means no padding */
      static const uint32_t AES_PADDING_SIZE;
      /** digest size: 32 byte */
      static const uint32_t DIGEST_SIZE;
      /** HMAC digest size: 32 byte */
      static const uint32_t HMAC_DIGEST_SIZE;
      /** HMAC key size used: 32 byte == 256 bit */
      static const uint32_t HMAC_KEY_SIZE;


      /**
       * Default constructor.
       */
      ScryptAesCbcShaV1Machine();

      /**
       * Encryptes passed <tt>plaintext</tt> and
       * writes it to <tt>ciphertext</tt> using
       * AES256 in CBC mode.
       *
       * @param plaintext pointer to the plaintext to encrypt
       * @param length length of the plaintext to encrypt
       * @param key the key to use
       * @param[out] ciphertext the ciphertext
       *
       * @return <tt>true</tt> for success, otherwise <tt>false</tt>
       */
      virtual bool encrypt(
         const uint8_t* plaintext,
         const std::size_t length,
         const Vector<uint8_t>& key,
         Vector<uint8_t>& ciphertext
         );

      /**
       * Encryptes passed <tt>plaintext</tt> and
       * writes it to <tt>ciphertext</tt> using
       * AES256 in CBC mode.
       *
       * @param plaintext the plaintext to encrypt
       * @param key the key to use
       * @param[out] ciphertext the ciphertext
       *                (first block is the used IV)
       *
       * @return <tt>true</tt> for success, otherwise <tt>false</tt>
       */
      virtual bool encrypt(
         const Vector<uint8_t>& plaintext,
         const Vector<uint8_t>& key,
         Vector<uint8_t>& ciphertext
         );

      /**
       * Encryptes passed <tt>plaintext</tt> and
       * writes it to <tt>ciphertext</tt> using
       * AES256 in CBC mode.
       *
       * Important: This method is for unit tests only!!!
       *
       * @param plaintext pointer to the plaintext to encrypt
       * @param length length of the plaintext to encrypt
       * @param key the key to use
       * @param ivec the ivec to use
       * @param[out] ciphertext the ciphertext
       *                (first block is the used IV)
       *
       * @return <tt>true</tt> for success, otherwise <tt>false</tt>
       */
      virtual bool encryptAesCbc(
         const uint8_t* plaintext,
         const std::size_t length,
         const Vector<uint8_t>& key,
         const Vector<uint8_t>& ivec,
         Vector<uint8_t>& ciphertext
         );

      /**
       * Encryptes passed <tt>plaintext</tt> and
       * writes it to <tt>ciphertext</tt> using
       * AES256 in CBC mode.
       *
       * Important: This method is for unit tests only!!!
       *
       * @param plaintext the plaintext to encrypt
       * @param key the key to use
       * @param ivec the ivec to use
       * @param[out] ciphertext the ciphertext
       *                (first block is the used IV)
       *
       * @return <tt>true</tt> for success, otherwise <tt>false</tt>
       */
      virtual bool encryptAesCbc(
         const Vector<uint8_t>& plaintext,
         const Vector<uint8_t>& key,
         const Vector<uint8_t>& ivec,
         Vector<uint8_t>& ciphertext
         );

      /**
       * Decryptes passed <tt>ciphertext</tt> and
       * writes it to <tt>plaintext</tt> using
       * AES256 in CBC mode.
       *
       * @param ciphertext pointer to ciphertext to decrypt
       * @param length length of the ciphertext
       * @param key the key to use
       * @param[out] plaintext the plaintext
       *
       * @return <tt>true</tt> for success, otherwise <tt>false</tt>
       */
      virtual bool decrypt(
         const uint8_t* ciphertext,
         const std::size_t length,
         const Vector<uint8_t>& key,
         Vector<uint8_t>& plaintext
         ) ;

      /**
       * Decryptes passed <tt>ciphertext</tt> and
       * writes it to <tt>plaintext</tt> using
       * AES256 in CBC mode.
       *
       * @param ciphertext the ciphertext to decrypt
       *           (first block is the used IV)
       * @param key the key to use
       * @param[out] plaintext the plaintext
       *
       * @return <tt>true</tt> for success, otherwise <tt>false</tt>
       */
      virtual bool decrypt(
         const Vector<uint8_t>& ciphertext,
         const Vector<uint8_t>& key,
         Vector<uint8_t>& plaintext
         );

      /**
       * Decryptes passed <tt>ciphertext</tt> and
       * writes it to <tt>plaintext</tt> using
       * AES256 in CBC mode.
       *
       * Important: This method is for unit tests only!!!
       *
       * @param ciphertext pointer to the ciphertext to decrypt
       *           (first block is the used IV)
       * @param length length of the ciphertext
       * @param key the key to use
       * @param padding <tt>true</tt> for last block padded, otherwise false
       * @param[out] plaintext the plaintext
       *
       * @return <tt>true</tt> for success, otherwise <tt>false</tt>
       */
      virtual bool decryptAesCbc(
         const uint8_t* ciphertext,
         const std::size_t length,
         const Vector<uint8_t>& key,
         bool padding,
         Vector<uint8_t>& plaintext
         );

      /**
       * Decryptes passed <tt>ciphertext</tt> and
       * writes it to <tt>plaintext</tt> using
       * AES256 in CBC mode.
       *
       * Important: This method is for unit tests only!!!
       *
       * @param ciphertext the ciphertext to decrypt
       *           (first block is the used IV)
       * @param key the key to use
       * @param padding <tt>true</tt> for last block padded, otherwise false
       * @param[out] plaintext the plaintext
       *
       * @return <tt>true</tt> for success, otherwise <tt>false</tt>
       */
      virtual bool decryptAesCbc(
         const Vector<uint8_t>& ciphertext,
         const Vector<uint8_t>& key,
         bool padding,
         Vector<uint8_t>& plaintext
         );

      /**
       * Returns the length of a SHA256 digest in bytes.
       *
       * @return length of digest
       */
      virtual const uint32_t getDigestLength();

      /**
       * Calculates a digest (hash value) for the passed
       * <tt>data</tt> using SHA256.
       *
       * @param data pointer to the data to calc digest for
       * @param length length of the data
       * @param[out] digest the calculated digest
       *
       * @return <tt>true</tt> for success, otherwise <tt>false</tt>
       */
      virtual bool calcDigest(
         const uint8_t* data,
         const std::size_t length,
         Vector<uint8_t>& digest
         );

      /**
       * Calculates a digest (hash value) for the passed
       * <tt>data</tt> using SHA256.
       *
       * @param data the data to calc digest for
       * @param[out] digest the calculated digest
       *
       * @return <tt>true</tt> for success, otherwise <tt>false</tt>
       */
      virtual bool calcDigest(
         const Vector<uint8_t>& data,
         Vector<uint8_t>& digest
         );

      /**
       * Returns the length of a HMAC with SHA256 in bytes.
       *
       * @return length of HMAC
       */
      virtual const uint32_t getHmacLength();

      /**
       * Calculates the HMAC for the passed <tt>data</tt>
       * using HMAC with SHA256.
       *
       * @param data pointer to the data to calc HMAC for
       * @param length length of the data
       * @param key the key to use
       * @param[out] hmac the calculated HMAC
       *
       * @return <tt>true</tt> for success, otherwise <tt>false</tt>
       */
      virtual bool calcHmac(
         const uint8_t* data,
         const std::size_t length,
         const Vector<uint8_t>& key,
         Vector<uint8_t>& hmac
         );

      /**
       * Calculates the HMAC for the passed <tt>data</tt>
       * using HMAC with SHA256.
       *
       * @param data the data to calc HMAC for
       * @param key the key to use
       * @param[out] hmac the calculated HMAC
       *
       * @return <tt>true</tt> for success, otherwise <tt>false</tt>
       */
      virtual bool calcHmac(
         const Vector<uint8_t>& data,
         const Vector<uint8_t>& key,
         Vector<uint8_t>& hmac
         );

      /**
       * Derives <tt>key</tt> from <tt>password</tt>
       * considering passed params. If params not complete,
       * defaults are used. Params are passed as Map,
       * keys are strings, values are packed in msgpack format.
       *
       * Supported params are:
       *    - salt
       *    - N (ld max mem, default 20)
       *    - r (bit blocks, default 8)
       *    - p (parallelism, default 1)
       *
       * @param password the password
       * @param[in,out] params params to consider
       * @param[out] the derived key
       *
       * @return <tt>true</tt> for success, otherwise <tt>false</tt>
       */
      virtual bool deriveKey(
         const String& password,
         Map<String,Vector<uint8_t>>& params,
         Vector<uint8_t>& key
         );

      /**
       * Returns params for key derivation, non existent params
       * are added and set to default values:
       *
       *    - salt (default: 32 byte random data)
       *    - ldN (default: 30)
       *    - r (default: 8)
       *    - p (default: 1)
       *
       * @param[in,out] params params to consider
       *
       * @return <tt>true</tt> for success, otherwise <tt>false</tt>
       */
      virtual bool getKeyDerivationParams( Map<String,Vector<uint8_t>>& params );

      /**
       * Generates a random token of passed <tt>length</tt>.
       *
       * @param length the lenght of the token
       * @param[out] token the generated token
       *
       * @return <tt>true</tt> for success, otherwise <tt>false</tt>
       */
      virtual bool genToken(
         const std::size_t length,
         Vector<uint8_t>& token
         );


   private:
      /** Forbidden copy constructor. */
      ScryptAesCbcShaV1Machine( const ScryptAesCbcShaV1Machine& other );

      /** Forbidden assignment operator. */
      ScryptAesCbcShaV1Machine& operator=( const ScryptAesCbcShaV1Machine& other );

      /** Make sure that padding PKCS#7 is used (see RFC5652 for details). */
      bool usesPkcs7Padding();


      /** PRNG used for token generation. */
      std::mt19937 m_PRNG;
};

} }

#endif
