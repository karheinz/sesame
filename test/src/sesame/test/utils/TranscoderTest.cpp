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
#include <locale>
#include "gtest/gtest.h"
#include "types.hpp"
#include "sesame/utils/Transcoder.hpp"
#include "sesame/utils/Reader.hpp"


namespace sesame { namespace test {

namespace
{
   Vector<char> utf8 = {
      '\x48', '\x65', '\x6c', '\x6c', '\x6f', '\x2c', '\x20',
      '\x77', '\x6f', '\x72', '\x6c', '\x64', '\x21'
      };
   Vector<char> utf16 = {
      'H', '\0', 'e', '\0', 'l', '\0', 'l', '\0', 'o', '\0', ',', '\0', ' ', '\0',
      'w', '\0', 'o', '\0', 'r', '\0', 'l', '\0', 'd', '\0', '!', '\0'
      };

   Vector<char> utf16BE = {
      '\0', 'H', '\0', 'e', '\0', 'l', '\0', 'l', '\0', 'o', '\0', ',', '\0', ' ',
      '\0', 'w', '\0', 'o', '\0', 'r', '\0', 'l', '\0', 'd', '\0', '!'
      };
}

TEST( TranscoderTest, ToUtf8 )
{
   String empty;
   String s1( utf8.begin(), utf8.end() );
   utils::Transcoder transcoder1( "UTF-8", "UTF-8" );
   String s2( transcoder1.transcode( s1 ) );
   ASSERT_EQ( s1, s2 );
   String empty1( transcoder1.transcode( empty ) );
   ASSERT_TRUE( empty1.empty() );

   String s3( utf16.begin(), utf16.end() );
   utils::Transcoder transcoder2( "UTF-16", "UTF-8" );
   String s4( transcoder2.transcode( s3 ) );
   ASSERT_EQ( s1, s4 );
   String empty2( transcoder2.transcode( empty ) );
   ASSERT_TRUE( empty2.empty() );

   String s5( utf16BE.begin(), utf16BE.end() );
   utils::Transcoder transcoder3( "UTF-16BE", "UTF-8" );
   String s6( transcoder3.transcode( s5 ) );
   ASSERT_EQ( s1, s6 );
   String empty3( transcoder3.transcode( empty ) );
   ASSERT_TRUE( empty3.empty() );
}

TEST( TranscoderTest, FromUtf8 )
{
   String empty;
   String s1( utf8.begin(), utf8.end() );

   utils::Transcoder transcoder1( "UTF-8", "UTF-8" );
   String s2( transcoder1.transcode( s1 ) );
   ASSERT_EQ( s1, s2 );
   String empty1( transcoder1.transcode( empty ) );
   ASSERT_TRUE( empty1.empty() );

   utils::Transcoder transcoder2( "UTF-8", "UTF-16" );
   String s3( transcoder2.transcode( s1 ) );
   String s4( utf16.begin(), utf16.end() );
   ASSERT_EQ( s4, s3 );
   String empty2( transcoder2.transcode( empty ) );
   ASSERT_TRUE( empty2.empty() );

   utils::Transcoder transcoder3( "UTF-8", "UTF-16BE" );
   String s5( transcoder3.transcode( s1 ) );
   String s6( utf16BE.begin(), utf16BE.end() );
   ASSERT_EQ( s6, s5 );
   String empty3( transcoder3.transcode( empty ) );
   ASSERT_TRUE( empty3.empty() );
}

TEST( TranscoderTest, Reader )
{
   // Set locale!
   char* currentLocaleName( std::setlocale( LC_ALL, "" ) );
   ASSERT_TRUE( currentLocaleName != nullptr );
   std::locale currentLocale( currentLocaleName );
   std::cout.imbue( currentLocale );
   std::cerr.imbue( currentLocale );
   std::cin.imbue( currentLocale );

   std::istringstream line( "Hello, world!\n" );
   std::streambuf* backup( std::cin.rdbuf() );
   std::cin.rdbuf( line.rdbuf() );
   utils::Reader reader( 1024 );
   String expected( "Hello, world!" );
   String got = reader.readLine( "" );
   std::cin.rdbuf( backup );
   std::cin.clear();

   ASSERT_EQ( expected, got );
}

TEST( TranscoderTest, LargeStrings )
{
   String s1( utf16.begin(), utf16.end() );
   String s2;
   s2.reserve( s1.size() * 2000 );
   for ( uint32_t i = 0; i < 2000; ++i )
   {
      s2.insert( s2.end(), s1.begin(), s1.end() );
   }
   ASSERT_EQ( 2000 * s1.size(), s2.size() );

   utils::Transcoder transcoder1( "UTF-16", "UTF-8" );
   String s3( transcoder1.transcode( s2 ) );
   ASSERT_EQ( s2.size() / 2, s3.size() );

   utils::Transcoder transcoder2( "UTF-8", "UTF-16" );
   String s4( transcoder2.transcode( s3 ) );

   ASSERT_EQ( s2, s4 );
}

} }
