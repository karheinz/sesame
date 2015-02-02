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


#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/types.h>

#include "gtest/gtest.h"

#include "sesame/definitions.hpp"
#include "types.hpp"
#include "sesame/crypto/ScryptAesCbcShaV1Machine.hpp"

namespace sesame { namespace test { namespace crypto {

// For output of binary data.
std::ostream& operator<<( std::ostream& stream, const Vector<uint8_t> vector );
// For Monte Carlo test.
Vector<uint8_t> concat( const Vector<uint8_t>& a, const Vector<uint8_t>& b );
Vector<uint8_t> xorVectors( const Vector<uint8_t>& a, const Vector<uint8_t>& b );

// Describes an AES job.
struct AesJob;

// Functions used for parsing.
std::vector<std::string> getRspFiles( const std::string& dirName );
void handleRspFile( const std::string& fileName );
void handleLine( const std::string& line );
uint32_t getCountValue( const std::string& line );
Vector<uint8_t> getVectorValue( const std::string& line );

// Some globals/constants used.
const uint32_t AES_BLOCK_SIZE( sesame::crypto::ScryptAesCbcShaV1Machine::AES_BLOCK_SIZE );
static bool encrypt( false );
static sesame::crypto::ScryptAesCbcShaV1Machine machine;


////////////////////////////////////////////////////////////////////////////////

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

Vector<uint8_t> concat( const Vector<uint8_t>& a, const Vector<uint8_t>& b )
{
   Vector<uint8_t> result;
   result.insert( result.end(), a.begin(), a.end() );
   result.insert( result.end(), b.begin(), b.end() );
   return result;
}

Vector<uint8_t> xorVectors( const Vector<uint8_t>& a, const Vector<uint8_t>& b )
{
   if ( a.size() != b.size() )
   {
      std::cout << "a.size() " << a.size() << ", b.size() " << b.size() << std::endl;
      throw std::runtime_error( "xorVectors(): vector sizes differ" );
   }

   Vector<uint8_t> result;
   for ( uint32_t i = 0; i < a.size(); ++i )
   {
      result.push_back( a[ i ] ^ b[ i ] );
   }

   return result;
}

struct AesJob
{
   static Vector<Vector<uint8_t> > monteCarloKeys;
   static Vector<Vector<uint8_t> > monteCarloIvs;
   static Vector<Vector<uint8_t> > monteCarloPlaintexts;
   static Vector<Vector<uint8_t> > monteCarloCiphertexts;


   AesJob( int32_t count, bool encrypt ) :
      m_HasRun( false ),
      m_Count( count ),
      m_Encrypt( encrypt )
   {
   }

   void run()
   {
      Vector<uint8_t> calculated;

      if ( m_Encrypt )
      {
         ASSERT_TRUE( machine.encryptAesCbc( m_Plaintext, m_Key, m_Iv, calculated ) );
         //std::cout << calculated << std::endl;
         //std::cout << m_Ciphertext << std::endl;
         ASSERT_EQ( 0, std::memcmp( m_Ciphertext.data(), calculated.data() + AES_BLOCK_SIZE, m_Ciphertext.size() ) );
      }
      else
      {
         // Prepend ciphertext with IV.
         m_Ciphertext.insert( m_Ciphertext.begin(), m_Iv.begin(), m_Iv.end() );

         // Sample data misses last padded block. Calculate and append it.
         Vector<uint8_t> tmp1 = m_Plaintext;
         Vector<uint8_t> tmp2;
         ASSERT_TRUE( machine.encryptAesCbc( tmp1, m_Key, m_Iv, tmp2 ) );
         m_Ciphertext.insert( m_Ciphertext.end(), tmp2.end() - AES_BLOCK_SIZE, tmp2.end() );

         ASSERT_TRUE( machine.decrypt( m_Ciphertext, m_Key, calculated ) );
         //std::cout << calculated << std::endl;
         //std::cout << m_Plaintext << std::endl;
         ASSERT_EQ( 0, std::memcmp( m_Plaintext.data(), calculated.data(), m_Plaintext.size() ) );
      }

      m_HasRun = true;
   }

   void runMonteCarlo()
   {
      Vector<uint8_t> key;
      Vector<uint8_t> iv;
      Vector<uint8_t> plaintext;
      Vector<uint8_t> ciphertext;
      Vector<uint8_t> tmp;
      Vector<uint8_t> calculated;

      // Init.
      uint32_t i( 0 );
      uint32_t j( 0 );
      key = m_Key;
      iv = m_Iv;
      plaintext = m_Plaintext;
      ciphertext = m_Ciphertext;

      // Loop.
      if ( m_Encrypt )
      {
         for ( i = 0; i <= 99; ++i )
         {
            monteCarloKeys[ i ] = key;
            monteCarloIvs[ i ] = iv;
            monteCarloPlaintexts[ i ] = plaintext;
            //std::cout << "key: " << key << std::endl;
            //std::cout << " iv: " << iv << std::endl;
            //std::cout << " pt: " << plaintext << std::endl;
            for ( j = 0; j <= 999; ++j )
            {
               if ( j == 0 )
               {
                  ASSERT_TRUE( machine.encryptAesCbc( plaintext, key, iv, calculated ) );
                  ciphertext = Vector<uint8_t>( calculated.begin() + AES_BLOCK_SIZE,
                        calculated.end() - AES_BLOCK_SIZE );
                  plaintext = iv;
               }
               else
               {
                  tmp = ciphertext;
                  ASSERT_TRUE( machine.encryptAesCbc( plaintext,
                        key, tmp, calculated ) );
                  ciphertext = Vector<uint8_t>( calculated.begin() + AES_BLOCK_SIZE,
                        calculated.end() - AES_BLOCK_SIZE );
                  plaintext = tmp;
               }
            }

            monteCarloCiphertexts[ i ] = ciphertext;
            //std::cout << " ct: " << ciphertext << std::endl;

            key = xorVectors( key, concat( plaintext, ciphertext ) );
            iv = ciphertext;
         }
      }
      else
      {
         for ( i = 0; i <= 99; ++i )
         {
            Vector<uint8_t> decIv;
            monteCarloKeys[ i ] = key;
            monteCarloIvs[ i ] = iv;
            monteCarloCiphertexts[ i ] = ciphertext;
            //std::cout << "key: " << key << std::endl;
            //std::cout << " iv: " << iv << std::endl;
            //std::cout << " ct: " << ciphertext << std::endl;
            for ( j = 0; j <= 999; ++j )
            {
               if ( j == 0 )
               {
                  // Prepend ciphertext with IV.
                  ciphertext.insert( ciphertext.begin(), iv.begin(), iv.end() );

                  // Remember current ciphertext as next IV.
                  decIv = Vector<uint8_t>( ciphertext.begin() + AES_BLOCK_SIZE, ciphertext.end() );

                  ASSERT_TRUE( machine.decryptAesCbc( ciphertext, key, false, plaintext ) );
                  ciphertext = iv;
               }
               else
               {
                  // Prepend ciphertext with IV.
                  ciphertext.insert( ciphertext.begin(), decIv.begin(), decIv.end() );

                  // Remember current ciphertext as next IV.
                  decIv = Vector<uint8_t>( ciphertext.begin() + AES_BLOCK_SIZE, ciphertext.end() );

                  tmp = plaintext;
                  ASSERT_TRUE( machine.decryptAesCbc( ciphertext, key, false, plaintext ) );
                  ciphertext = tmp;
               }
            }

            monteCarloPlaintexts[ i ] = plaintext;
            //std::cout << " pt: " << plaintext << std::endl;

            key = xorVectors( key, concat( ciphertext, plaintext ) );
            iv = plaintext;
         }
      }

      m_HasRun = true;
   }

   void checkMonteCarlo()
   {
      ASSERT_EQ( monteCarloKeys[ m_Count ], m_Key );
      ASSERT_EQ( monteCarloIvs[ m_Count ], m_Iv );
      ASSERT_EQ( monteCarloPlaintexts[ m_Count ], m_Plaintext );
      ASSERT_EQ( monteCarloCiphertexts[ m_Count ], m_Ciphertext );

      m_HasRun = true;
   }

   bool m_HasRun;
   int32_t m_Count;
   bool m_Encrypt;
   Vector<uint8_t> m_Key;
   Vector<uint8_t> m_Iv;
   Vector<uint8_t> m_Plaintext;
   Vector<uint8_t> m_Ciphertext;
};
Vector<Vector<uint8_t> > AesJob::monteCarloKeys( 100 );
Vector<Vector<uint8_t> > AesJob::monteCarloIvs( 100 );
Vector<Vector<uint8_t> > AesJob::monteCarloPlaintexts( 100 );
Vector<Vector<uint8_t> > AesJob::monteCarloCiphertexts( 100 );
static AesJob job( true, -1 );


std::vector<std::string> getRspFiles( const std::string& dirName )
{
   std::vector<std::string> fileNames;

   DIR* dir( opendir( dirName.c_str() ) );
   if ( ! dir )
   {
      throw std::runtime_error( std::strerror( errno ) );
   }

   struct dirent* entry;
   while ( ( entry = readdir( dir ) ) )
   {
      std::string fileName( entry->d_name );
      if ( fileName.find( ".rsp", fileName.size() - 4 ) != std::string::npos )
      {
         if ( fileName.find( "MCT" ) == std::string::npos )
         {
            fileNames.push_back( dirName + "/" + fileName );
         }
      }
   }

   return fileNames;
}

std::string getMonteCarloRspFile( const std::string& dirName )
{
   std::vector<std::string> fileNames;

   DIR* dir( opendir( dirName.c_str() ) );
   if ( ! dir )
   {
      throw std::runtime_error( std::strerror( errno ) );
   }

   struct dirent* entry;
   while ( ( entry = readdir( dir ) ) )
   {
      std::string fileName( entry->d_name );
      if ( fileName.find( ".rsp", fileName.size() - 4 ) != std::string::npos )
      {
         if ( fileName.find( "MCT" ) != std::string::npos )
         {
            return ( dirName + "/" + fileName );
         }
      }
   }

   return std::string();
}

void handleRspFile( const std::string& fileName )
{
   std::ifstream file( fileName );
   std::string line;

   do
   {
      std::getline( file, line );
      handleLine( line );

      // Check job. Run?
      if ( ! job.m_HasRun && job.m_Count >= 0 )
      {
         if ( job.m_Encrypt == true && job.m_Ciphertext.size() > 0 )
         {
            std::cout << "run encryption job " << job.m_Count << std::endl;
            job.run();
         }
         else if ( job.m_Encrypt == false && job.m_Plaintext.size() > 0 )
         {
            std::cout << "run decryption job " << job.m_Count << std::endl;
            job.run();
         }
      }
   }
   while ( file.good() && ! file.eof() );
}

void handleMonteCarloRspFile( const std::string& fileName )
{
   std::ifstream file( fileName );
   std::string line;

   do
   {
      std::getline( file, line );
      handleLine( line );

      // Check job. Run?
      if ( ! job.m_HasRun && job.m_Count >= 0 )
      {
         if ( job.m_Encrypt == true && job.m_Ciphertext.size() > 0 )
         {
            if ( job.m_Count == 0 )
            {
               std::cout << "run monte carlo encryption job " << job.m_Count << std::endl;
               job.runMonteCarlo();
            }
            std::cout << "check monte carlo encryption result " << job.m_Count << std::endl;
            job.checkMonteCarlo();
         }
         else if ( job.m_Encrypt == false && job.m_Plaintext.size() > 0 )
         {
            if ( job.m_Count == 0 )
            {
               std::cout << "run monte carlo decryption job " << job.m_Count << std::endl;
               job.runMonteCarlo();
            }
            std::cout << "check monte carlo decryption result " << job.m_Count << std::endl;
            job.checkMonteCarlo();
         }

      }
   }
   while ( file.good() && ! file.eof() );
}

void handleLine( const std::string& line )
{
   if ( line.find( "[ENCRYPT]" ) != std::string::npos )
   {
      encrypt = true;
   }
   else if ( line.find( "[DECRYPT]" ) != std::string::npos )
   {
      encrypt = false;
   }
   else if ( line.find( "COUNT" ) != std::string::npos )
   {
      job = AesJob( getCountValue( line ), encrypt );
   }
   else if ( line.find( "KEY" ) != std::string::npos )
   {
      job.m_Key = getVectorValue( line );
   }
   else if ( line.find( "IV" ) != std::string::npos )
   {
      job.m_Iv = getVectorValue( line );
   }
   else if ( line.find( "PLAINTEXT" ) != std::string::npos )
   {
      job.m_Plaintext = getVectorValue( line );
   }
   else if ( line.find( "CIPHERTEXT" ) != std::string::npos )
   {
      job.m_Ciphertext = getVectorValue( line );
   }
}

uint32_t getCountValue( const std::string& line )
{
   uint32_t pos = line.find( "=" );
   if ( pos == std::string::npos )
   {
      return 0;
   }

   uint32_t v;
   std::stringstream s;
   s << line.substr( pos + 2 );
   s >> v;
   return v;
}

Vector<uint8_t> getVectorValue( const std::string& line )
{
   Vector<uint8_t> result;

   uint32_t pos = line.find( "=" );
   if ( pos == std::string::npos )
   {
      return result;
   }

   uint32_t tmp;
   pos += 2;
   while ( ( pos + 1 ) < line.size() )
   {
      std::sscanf( line.substr( pos, 2 ).c_str(), "%x", &tmp );
      result.push_back( tmp );
      pos += 2;
   }
   return result;
}


//////////////////////////////////////////
//                                      //
//  Parse .rsp files and run AES jobs.  //
//                                      //
//////////////////////////////////////////

TEST( ScryptAesCbcShaV1MachineTest, NistAesAvs )
{
   std::vector<std::string> fileNames( getRspFiles( AESAVS_DIRECTORY ) );

   for ( const auto fileName : fileNames )
   {
      std::cout << "handle file: " << fileName << std::endl;
      handleRspFile( fileName );
   }
}

TEST( ScryptAesCbcShaV1MachineTest, NistAesAvsMonteCarlo )
{
   std::string fileName( getMonteCarloRspFile( AESAVS_DIRECTORY ) );

   std::cout << "handle file: " << fileName << std::endl;
   handleMonteCarloRspFile( fileName );
}

} } }
