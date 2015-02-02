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


#include <iomanip>
#include <iostream>
#include <string>
#include "gtest/gtest.h"
#include "sesame/definitions.hpp"
#include "types.hpp"
#include "sesame/crypto/ScryptAesCbcShaV1Machine.hpp"


namespace sesame { namespace test { namespace crypto {

const uint32_t AES_BLOCK_SIZE( sesame::crypto::ScryptAesCbcShaV1Machine::AES_BLOCK_SIZE );
const uint32_t AES_KEY_SIZE( sesame::crypto::ScryptAesCbcShaV1Machine::AES_KEY_SIZE );
const uint32_t DIGEST_SIZE( sesame::crypto::ScryptAesCbcShaV1Machine::DIGEST_SIZE );
const uint32_t HMAC_DIGEST_SIZE( sesame::crypto::ScryptAesCbcShaV1Machine::HMAC_DIGEST_SIZE );

std::ostream& operator<<( std::ostream& stream, const Vector<uint8_t> vector )
{
   stream << std::hex;
   for ( uint32_t i = 0; i < vector.size(); ++i )
   {
      stream << std::setw( 2 ) << std::setfill( '0' ) << static_cast<uint16_t>( vector[ i ] );
   }
   stream << std::dec;

   return stream;
}

TEST( ScryptAesCbcShaV1MachineTest, Random )
{
   const uint32_t NUM_OF_TOKENS( 1000 );

   sesame::crypto::ScryptAesCbcShaV1Machine tmp;
   sesame::crypto::IMachine& machine( tmp );

   // Generate tokens of different lengths.
   Vector<uint8_t> token;
   machine.genToken( 32, token );
   std::cout << "token (32): " << token << std::endl;
   ASSERT_EQ( 32, token.size() );
   machine.genToken( 16, token );
   std::cout << "token (16): " << token << std::endl;
   ASSERT_EQ( 16, token.size() );
   machine.genToken( 4, token );
   std::cout << "token (4) : " << token << std::endl;
   ASSERT_EQ( 4, token.size() );
   machine.genToken( 3, token );
   std::cout << "token (3) : " << token << std::endl;
   ASSERT_EQ( 3, token.size() );
   machine.genToken( 0, token );
   std::cout << "token (0) : " << token << std::endl;
   ASSERT_EQ( 0, token.size() );

   // Generate NUM_OF_TOKENS different tokens.
   Vector<uint8_t> tokens[ NUM_OF_TOKENS ];
   for ( uint32_t i = 0; i < NUM_OF_TOKENS; ++i )
   {
      machine.genToken( 32, tokens[ i ] );
      ASSERT_EQ( 32, tokens[ i ].size() );
   }

   // Check.
   for ( uint32_t i = 0; i < NUM_OF_TOKENS; ++i )
   {
      for ( uint32_t k = 0; k < NUM_OF_TOKENS; ++k )
      {
         if ( i != k )
         {
            ASSERT_TRUE( tokens[ i ] != tokens[ k ] );
         }
      }
   }
}

TEST( ScryptAesCbcShaV1MachineTest, Aes )
{
   sesame::crypto::ScryptAesCbcShaV1Machine machine;

   Vector<uint8_t> key;
   machine.genToken( 32, key );
   Vector<uint8_t> plaintext( 2 * AES_BLOCK_SIZE, 0xab );
   Vector<uint8_t> ciphertext;
   ASSERT_TRUE( machine.encrypt( plaintext, key, ciphertext ) );
   ASSERT_EQ( 2 * AES_BLOCK_SIZE, ciphertext.size() - plaintext.size() );
   ASSERT_NE( plaintext, ciphertext );
   std::cout << "       key: " << key << std::endl;
   std::cout << " plaintext: " << plaintext << std::endl;
   std::cout << "ciphertext: " << ciphertext << std::endl;

   Vector<uint8_t> decryptedCiphertext;
   ASSERT_TRUE( machine.decrypt( ciphertext, key, decryptedCiphertext ) );
   ASSERT_EQ( 2 * AES_BLOCK_SIZE, ciphertext.size() - decryptedCiphertext.size() );
   ASSERT_EQ( plaintext, decryptedCiphertext );
   std::cout << " plaintext: " << decryptedCiphertext << std::endl;

   // Use no multiple of AES_BLOCK_SIZE.
   plaintext.resize( plaintext.size() - 1 );
   ASSERT_TRUE( machine.encrypt( plaintext, key, ciphertext ) );
   ASSERT_EQ( 0, ciphertext.size() % AES_BLOCK_SIZE );
   ASSERT_EQ( AES_BLOCK_SIZE + 1, ciphertext.size() - plaintext.size() );
   ASSERT_NE( plaintext, ciphertext );
   std::cout << "       key: " << key << std::endl;
   std::cout << " plaintext: " << plaintext << std::endl;
   std::cout << "ciphertext: " << ciphertext << std::endl;

   ASSERT_TRUE( machine.decrypt( ciphertext, key, decryptedCiphertext ) );
   ASSERT_EQ( AES_BLOCK_SIZE + 1, ciphertext.size() - decryptedCiphertext.size() );
   ASSERT_EQ( plaintext, decryptedCiphertext );
   std::cout << " plaintext: " << decryptedCiphertext << std::endl;

   // Only 32 byte as key length allowed.
   machine.genToken( 31, key );
   ASSERT_FALSE( machine.encrypt( plaintext, key, ciphertext ) );
   ASSERT_FALSE( machine.decrypt( ciphertext, key, plaintext ) );
}

TEST( ScryptAesCbcShaV1MachineTest, AesPadding )
{
   sesame::crypto::ScryptAesCbcShaV1Machine machine;

   Vector<uint8_t> plaintext( AES_BLOCK_SIZE, 0 );
   plaintext[ 0 ] = 0x80;
   const Vector<uint8_t> ivec( AES_BLOCK_SIZE, 0 );
   Vector<uint8_t> key( AES_KEY_SIZE, 0 );

   Vector<uint8_t> ciphertext;
   ASSERT_TRUE( machine.encryptAesCbc( plaintext, key, ivec, ciphertext ) );

   Vector<uint8_t> decryptedCiphertext;
   ASSERT_TRUE( machine.decrypt( ciphertext, key, decryptedCiphertext ) );

   ASSERT_EQ( plaintext, decryptedCiphertext );
}

TEST( ScryptAesCbcShaV1MachineTest, Digest )
{
   sesame::crypto::ScryptAesCbcShaV1Machine machine;

   Vector<uint8_t> plaintext( 32 * 1024, 0xab );
   Vector<uint8_t> digest, digest2;
   ASSERT_TRUE( machine.calcDigest( plaintext, digest ) );
   ASSERT_EQ( DIGEST_SIZE, digest.size() );
   std::cout << "digest1: " << digest << std::endl;

   // Calc digest using other data.
   plaintext[ 0 ] = 0xac;
   ASSERT_TRUE( machine.calcDigest( plaintext, digest2 ) );
   ASSERT_EQ( DIGEST_SIZE, digest2.size() );
   ASSERT_NE( digest, digest2 );
   std::cout << "digest2: " << digest2 << std::endl;
}

TEST( ScryptAesCbcShaV1MachineTest, Hmac )
{
   sesame::crypto::ScryptAesCbcShaV1Machine machine;

   Vector<uint8_t> key;
   machine.genToken( 32, key );
   Vector<uint8_t> plaintext( 32 * 1024, 0xab );
   Vector<uint8_t> hmac, hmac2;
   ASSERT_TRUE( machine.calcHmac( plaintext, key, hmac ) );
   ASSERT_EQ( HMAC_DIGEST_SIZE, hmac.size() );
   std::cout << "hmac1: " << hmac << std::endl;

   // Calc HMAC using another key.
   Vector<uint8_t> wrongKey;
   machine.genToken( 32, wrongKey );
   ASSERT_TRUE( machine.calcHmac( plaintext, wrongKey, hmac2 ) );
   ASSERT_EQ( HMAC_DIGEST_SIZE, hmac2.size() );
   ASSERT_NE( hmac, hmac2 );
   std::cout << "hmac2: " << hmac2 << std::endl;

   // Calc HMAC for other plaintext.
   plaintext = Vector<uint8_t>( 32 * 1024, 0xba );
   ASSERT_TRUE( machine.calcHmac( plaintext, key, hmac2 ) );
   ASSERT_EQ( HMAC_DIGEST_SIZE, hmac2.size() );
   ASSERT_NE( hmac, hmac2 );
   std::cout << "hmac3: " << hmac2 << std::endl;
}

} } }
