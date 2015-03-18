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


#include <iostream>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include "sesame/crypto/ScryptAesCbcShaV1Machine.hpp"
#include "sesame/packaging.hpp"
#include "sesame/utils/string.hpp"

extern "C"
{
#include "crypto_scrypt.h"
}

namespace sesame { namespace crypto {

   const uint32_t ScryptAesCbcShaV1Machine::AES_BLOCK_SIZE( 16 );
   const uint32_t ScryptAesCbcShaV1Machine::AES_KEY_SIZE( 32 );
   const uint32_t ScryptAesCbcShaV1Machine::DIGEST_SIZE( 32 );
   const uint32_t ScryptAesCbcShaV1Machine::HMAC_DIGEST_SIZE( 32 );
   const uint32_t ScryptAesCbcShaV1Machine::HMAC_KEY_SIZE( 32 );


   ScryptAesCbcShaV1Machine::ScryptAesCbcShaV1Machine() :
      IMachine(),
      m_PRNG( std::random_device()() )
   {
      // Check used AES configuration.
      EVP_CIPHER_CTX context;
      EVP_CIPHER_CTX_init( &context );
      EVP_CipherInit_ex( &context, EVP_aes_256_cbc(), nullptr, nullptr, nullptr, 1 );

      if ( EVP_CIPHER_CTX_iv_length( &context ) != AES_BLOCK_SIZE )
      {
         throw std::runtime_error( "wrong AES block size" );
      }

      if ( EVP_CIPHER_CTX_key_length( &context ) != AES_KEY_SIZE )
      {
         throw std::runtime_error( "wrong AES key size" );
      }

      if ( EVP_CIPHER_CTX_block_size( &context ) != AES_BLOCK_SIZE )
      {
         throw std::runtime_error( "wrong AES padding size" );
      }

      if ( ( EVP_CIPHER_CTX_mode( &context ) & EVP_CIPH_CBC_MODE ) != EVP_CIPH_CBC_MODE )
      {
         throw std::runtime_error( "wrong AES mode of operation" );
      }

      if ( ! usesPkcs7Padding() )
      {
         throw std::runtime_error( "wrong AES padding mode" );
      }
   }

   bool ScryptAesCbcShaV1Machine::encrypt(
      const uint8_t* plaintext,
      const std::size_t length,
      const Vector<uint8_t>& key,
      Vector<uint8_t>& ciphertext
      )
   {
      Vector<uint8_t> ivec;
      genToken( AES_BLOCK_SIZE, ivec );

      return encryptAesCbc(
         plaintext,
         length,
         key,
         ivec,
         ciphertext
         );
   }

   bool ScryptAesCbcShaV1Machine::encrypt(
      const Vector<uint8_t>& plaintext,
      const Vector<uint8_t>& key,
      Vector<uint8_t>& ciphertext
      )
   {
      Vector<uint8_t> ivec;
      genToken( AES_BLOCK_SIZE, ivec );

      return encryptAesCbc(
         plaintext.data(),
         plaintext.size(),
         key,
         ivec,
         ciphertext
         );
   }

   bool ScryptAesCbcShaV1Machine::encryptAesCbc(
      const uint8_t* plaintext,
      const std::size_t length,
      const Vector<uint8_t>& key,
      const Vector<uint8_t>& ivec,
      Vector<uint8_t>& ciphertext
      )
   {
      if ( plaintext == nullptr ||
           length == 0 ||
           key.size() != AES_KEY_SIZE ||
           ivec.size() != AES_BLOCK_SIZE
         )
      {
         return false;
      }

      // Alloc ciphertext bytes, first block is IV. Also consider padding.
      ciphertext.resize( ( 1 + ( length / AES_BLOCK_SIZE ) + 1 ) * AES_BLOCK_SIZE );

      // Write ivec to first block.
      std::memcpy( ciphertext.data(), ivec.data(), ivec.size() );

      // Setup AES CBC 256.
      EVP_CIPHER_CTX context;
      EVP_CIPHER_CTX_init( &context );
      EVP_CipherInit_ex( &context, EVP_aes_256_cbc(), nullptr, key.data(), ivec.data(), 1 );

      int32_t written1( 0 );
      int32_t written2( 0 );

      // Update ...
      if ( ! EVP_CipherUpdate(
                &context,
                ciphertext.data() + AES_BLOCK_SIZE,
                &written1,
                plaintext,
                length
                )
         )
      {
         EVP_CIPHER_CTX_cleanup( &context );
         return false;
      }

      // ... and finalize.
      if ( ! EVP_CipherFinal_ex(
                &context,
                ciphertext.data() + AES_BLOCK_SIZE + written1,
                &written2
                )
         )
      {
         EVP_CIPHER_CTX_cleanup( &context );
         return false;
      }

      EVP_CIPHER_CTX_cleanup( &context );

      ciphertext.shrink_to_fit();
      return ( ciphertext.size() == ( AES_BLOCK_SIZE + written1 + written2 ) );
   }

   bool ScryptAesCbcShaV1Machine::encryptAesCbc(
      const Vector<uint8_t>& plaintext,
      const Vector<uint8_t>& key,
      const Vector<uint8_t>& ivec,
      Vector<uint8_t>& ciphertext
      )
   {
      return encryptAesCbc(
         plaintext.data(),
         plaintext.size(),
         key,
         ivec,
         ciphertext
         );
   }

   bool ScryptAesCbcShaV1Machine::decrypt(
      const uint8_t* ciphertext,
      const std::size_t length,
      const Vector<uint8_t>& key,
      Vector<uint8_t>& plaintext
      )
   {
      return decryptAesCbc( ciphertext, length, key, true, plaintext );
   }

   bool ScryptAesCbcShaV1Machine::decrypt(
      const Vector<uint8_t>& ciphertext,
      const Vector<uint8_t>& key,
      Vector<uint8_t>& plaintext
      )
   {
      return decryptAesCbc( ciphertext.data(), ciphertext.size(), key, true, plaintext );
   }

   bool ScryptAesCbcShaV1Machine::decryptAesCbc(
      const uint8_t* ciphertext,
      const std::size_t length,
      const Vector<uint8_t>& key,
      bool padding,
      Vector<uint8_t>& plaintext
      )
   {
      if ( ciphertext == nullptr ||
           length < ( AES_BLOCK_SIZE + ( padding ? AES_BLOCK_SIZE : 1 ) ) ||
           ( length % ( padding ? AES_BLOCK_SIZE : 1 ) ) != 0 ||
           key.size() != AES_KEY_SIZE
         )
      {
         return false;
      }

      // Alloc plaintext bytes, ignore IV.
      plaintext.resize( length - AES_BLOCK_SIZE );

      // Setup AES CBC 256.
      EVP_CIPHER_CTX context;
      EVP_CIPHER_CTX_init( &context );
      EVP_CipherInit_ex( &context, EVP_aes_256_cbc(), nullptr, key.data(), ciphertext, 0 );
      EVP_CIPHER_CTX_set_padding( &context, padding ? 1 : 0 );

      int32_t written1( 0 );
      int32_t written2( 0 );

      // Update ...
      if ( ! EVP_CipherUpdate(
                &context,
                plaintext.data(),
                &written1,
                ciphertext + AES_BLOCK_SIZE,
                length - AES_BLOCK_SIZE
                )
         )
      {
         EVP_CIPHER_CTX_cleanup( &context );
         return false;
      }

      // ... and finalize.
      if ( ! EVP_CipherFinal_ex(
                &context,
                plaintext.data() + written1,
                &written2
                )
         )
      {
         EVP_CIPHER_CTX_cleanup( &context );
         return false;
      }

      EVP_CIPHER_CTX_cleanup( &context );

      // Resize plaintext (was probably padded).
      plaintext.resize( static_cast<uint32_t>( written1 ) + written2 );

      return true;
   }

   bool ScryptAesCbcShaV1Machine::decryptAesCbc(
      const Vector<uint8_t>& ciphertext,
      const Vector<uint8_t>& key,
      bool padding,
      Vector<uint8_t>& plaintext
      )
   {
      return decryptAesCbc( ciphertext.data(), ciphertext.size(), key, padding, plaintext );
   }

   const uint32_t ScryptAesCbcShaV1Machine::getDigestLength()
   {
      return DIGEST_SIZE;
   }

   bool ScryptAesCbcShaV1Machine::calcDigest(
      const uint8_t* data,
      const std::size_t length,
      Vector<uint8_t>& digest
      )
   {
      digest.resize( DIGEST_SIZE );

      uint32_t written( 0 );
      EVP_MD_CTX context;
      EVP_MD_CTX_init( &context );
      EVP_DigestInit_ex( &context, EVP_sha256(), nullptr );
      EVP_DigestUpdate( &context, data, length );
      EVP_DigestFinal_ex( &context, digest.data(), &written );
      EVP_MD_CTX_cleanup( &context );

      return ( written == DIGEST_SIZE );
   }


   bool ScryptAesCbcShaV1Machine::calcDigest(
      const Vector<uint8_t>& data,
      Vector<uint8_t>& digest
      )
   {
      return calcDigest( data.data(), data.size(), digest );
   }

   const uint32_t ScryptAesCbcShaV1Machine::getHmacLength()
   {
      return HMAC_DIGEST_SIZE;
   }

   bool ScryptAesCbcShaV1Machine::calcHmac(
      const uint8_t* data,
      const std::size_t length,
      const Vector<uint8_t>& key,
      Vector<uint8_t>& hmac
      )
   {
      if ( key.size() != HMAC_KEY_SIZE )
      {
         return false;
      }

      hmac.resize( HMAC_DIGEST_SIZE );

      uint32_t written( 0 );
      HMAC_CTX context;
      HMAC_CTX_init( &context );
      HMAC_Init_ex( &context, key.data(), key.size(), EVP_sha256(), nullptr );
      HMAC_Update( &context, data, length );
      HMAC_Final( &context, hmac.data(), &written );
      HMAC_CTX_cleanup( &context );

      return ( written == HMAC_DIGEST_SIZE );
   }

   bool ScryptAesCbcShaV1Machine::calcHmac(
      const Vector<uint8_t>& data,
      const Vector<uint8_t>& key,
      Vector<uint8_t>& hmac
      )
   {
      return calcHmac( data.data(), data.size(), key, hmac );
   }

   bool ScryptAesCbcShaV1Machine::deriveKey(
      const String& password,
      Map<String,Vector<uint8_t>>& params,
      Vector<uint8_t>& key
      )
   {
      getKeyDerivationParams( params );

      // salt
      Vector<uint8_t> salt;
      unpackV( params[ utils::fromUtf8( u8"salt" ) ], salt );
      // Shorter than 32 Byte?
      if ( salt.size() < 32 )
      {
         return false;
      }

      // ld N
      uint32_t ldN;
      unpackV( params[ utils::fromUtf8( u8"ldN" ) ], ldN );
      // More than 2^63 Byte RAM?!
      if ( ldN > 63 )
      {
         return false;
      }

      // r
      uint32_t r;
      unpackV( params[ utils::fromUtf8( u8"r" ) ], r );

      // p
      uint32_t p;
      unpackV( params[ utils::fromUtf8( u8"p" ) ], p );

      // Derive 32 byte key using scrypt.
      key.resize( AES_KEY_SIZE );
      return ( crypto_scrypt(
                  reinterpret_cast<const uint8_t*>( password.c_str() ),
                  password.size(),
                  salt.data(),
                  salt.size(),
                  static_cast<uint64_t>( std::exp2( ldN ) ),
                  r,
                  p,
                  key.data(),
                  key.size()
                  ) == 0 );
   }

   bool ScryptAesCbcShaV1Machine::getKeyDerivationParams( Map<String,Vector<uint8_t>>& params )
   {
      // salt
      if ( params.find( utils::fromUtf8( u8"salt" ) ) == params.end() )
      {
         Vector<uint8_t> salt;
         if ( ! genToken( 32, salt ) )
         {
            return false;
         }

         Vector<uint8_t> v;
         packV( v, salt );
         params[ utils::fromUtf8( u8"salt" ) ] = v;
      }

      // ld N
      if ( params.find( utils::fromUtf8( u8"ldN" ) ) == params.end() )
      {
         uint32_t ldN;
         ldN = 20;
         Vector<uint8_t> v;
         packV( v, ldN );
         params[ utils::fromUtf8( u8"ldN" ) ] = v;
      }

      // r
      if ( params.find( utils::fromUtf8( u8"r" ) ) == params.end() )
      {
         uint32_t r;
         r = 8;
         Vector<uint8_t> v;
         packV( v, r );
         params[ utils::fromUtf8( u8"r" ) ] = v;
      }

      // p
      if ( params.find( utils::fromUtf8( u8"p" ) ) == params.end() )
      {
         uint32_t p;
         p = 1;
         Vector<uint8_t> v;
         packV( v, p );
         params[ utils::fromUtf8( u8"p" ) ] = v;
      }

      return true;
   }

   bool ScryptAesCbcShaV1Machine::genToken(
      const std::size_t length,
      Vector<uint8_t>& token
      )
   {
      token.resize( length );

      uint32_t times( length / sizeof( uint32_t ) );
      if ( length % sizeof( uint32_t ) )
      {
         ++times;
      }

      uint32_t tmp;
      for ( uint32_t i = 0; i < times; ++i )
      {
         tmp = static_cast<uint32_t>( m_PRNG() );

         std::memcpy(
            token.data() + ( i * sizeof( uint32_t ) ),
            &tmp,
            std::min( sizeof( uint32_t ), token.size() - ( i * sizeof( uint32_t ) ) )
            );
      }

      return true;
   }

   bool ScryptAesCbcShaV1Machine::usesPkcs7Padding()
   {
      static const Vector<uint8_t> key( AES_KEY_SIZE, 0 );
      static const Vector<uint8_t> ivec( AES_BLOCK_SIZE, 0 );
      static const Vector<uint8_t> plaintext = { 0x80 };
      static const Vector<uint8_t> plaintextWithPadding = {
         0x80, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
         0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f
         };
      Vector<uint8_t> ciphertext;
      Vector<uint8_t> decryptedCiphertext;

      // Encrypt with padding.
      if ( ! encryptAesCbc(
         plaintext,
         key,
         ivec,
         ciphertext
         )
      )
      {
         return false;
      }

      // Decrypt without padding.
      if ( ! decryptAesCbc(
         ciphertext,
         key,
         false,
         decryptedCiphertext
         )
      )
      {
         return false;
      }

      return ( plaintextWithPadding == decryptedCiphertext );
   }

} }
