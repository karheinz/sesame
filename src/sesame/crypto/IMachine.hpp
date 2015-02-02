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


#ifndef SESAME_CRYPTO_IMACHINE
#define SESAME_CRYPTO_IMACHINE

#include "types.hpp"
#include "sesame/packaging.hpp"


namespace sesame { namespace crypto {

/**
 * Interface for crypto machines.
 */
class IMachine
{
   public:
      /**
       * Encryptes passed <tt>plaintext</tt> and
       * writes it to <tt>ciphertext</tt>.
       *
       * @param plaintext the plaintext to encrypt
       * @param key the key to use
       * @param[out] ciphertext the ciphertext
       *
       * @return <tt>true</tt> for success, otherwise <tt>false</tt>
       */
      virtual bool encrypt(
         const Vector<uint8_t>& plaintext,
         const Vector<uint8_t>& key,
         Vector<uint8_t>& ciphertext
         ) = 0;

      /**
       * Encryptes passed <tt>plaintext</tt> and
       * writes it to <tt>ciphertext</tt>.
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
         ) = 0;

      /**
       * Decryptes passed <tt>ciphertext</tt> and
       * writes it to <tt>plaintext</tt>.
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
         ) = 0;

      /**
       * Decryptes passed <tt>ciphertext</tt> and
       * writes it to <tt>plaintext</tt>.
       *
       * @param ciphertext the ciphertext to decrypt
       * @param key the key to use
       * @param[out] plaintext the plaintext
       *
       * @return <tt>true</tt> for success, otherwise <tt>false</tt>
       */
      virtual bool decrypt(
         const Vector<uint8_t>& ciphertext,
         const Vector<uint8_t>& key,
         Vector<uint8_t>& plaintext
         ) = 0;

      /**
       * Calculates the HMAC for the passed <tt>data</tt>.
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
         ) = 0;

      /**
       * Calculates the HMAC for the passed <tt>data</tt>.
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
         ) = 0;

      /**
       * Calculates a digest (hash value) for the passed <tt>data</tt>.
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
         ) = 0;

      /**
       * Calculates a digest (hash value) for the passed <tt>data</tt>.
       *
       * @param data the data to calc digest for
       * @param[out] digest the calculated digest
       *
       * @return <tt>true</tt> for success, otherwise <tt>false</tt>
       */
      virtual bool calcDigest(
         const Vector<uint8_t>& data,
         Vector<uint8_t>& digest
         ) = 0;

      /**
       * Derives <tt>key</tt> from <tt>password</tt>
       * considering passed params. If params not complete,
       * defaults are used. Params are passed as Map,
       * keys are strings, values are packed in msgpack format.
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
         ) = 0;

      /**
       * Returns params for key derivation, non existent params
       * are added and set to default values.
       *
       * @param[in,out] params params to consider
       *
       * @return <tt>true</tt> for success, otherwise <tt>false</tt>
       */
      virtual bool getKeyDerivationParams( Map<String,Vector<uint8_t>>& params ) = 0;

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
         ) = 0;
};

} }

#endif
