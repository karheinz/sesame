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


#include <cstring>
#include "gtest/gtest.h"
#include "types.hpp"
#include "sesame/crypto/F4.hpp"
#include "sesame/utils/filesystem.hpp"


namespace sesame { namespace test { namespace crypto {

TEST( F4Test, BasicUsage )
{
   String s1( "Hello, world!" );
   Vector<char> data( s1.begin(), s1.end() );
   data.push_back( '\0' );
   
   String in( F4_TEST_IMAGE );
   String out( sesame::utils::incrementFileName( in ) );

   sesame::crypto::F4 algo;
   algo.embed( in, out, data );

   Vector<char> result;
   algo.extract( out, result );

   std::size_t length( strnlen( result.data(), result.size() ) );
   ASSERT_EQ( s1.size(), length );
   ASSERT_TRUE( length < result.size() );
   String s2( result.data(), result.data() + length );
   ASSERT_EQ( s1, s2 );
}


} } }
