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


#include <utility>
#include "gtest/gtest.h"
#include "types.hpp"
#include "sesame/utils/string.hpp"


namespace sesame { namespace test {

TEST( StringUtilitiesTest, Strip )
{
   String normalized;
   String s1( "Hello, world!" );

   normalized = utils::normalize( s1 );
   ASSERT_EQ( s1, normalized );

   String s2( "   " + s1 );
   normalized = utils::lstrip( s2 );
   ASSERT_EQ( s1, normalized );

   String s3( s1 + "   " );
   normalized = utils::rstrip( s3 );
   ASSERT_EQ( s1, normalized );

   String s4( "   " + s1 + "   " );
   normalized = utils::strip( s4 );
   ASSERT_EQ( s1, normalized );

   String s5( "   Hello,      world!   " );
   normalized = utils::normalize( s5 );
   ASSERT_EQ( s1, normalized );

   String s6( "   Hello,      world!   \n" );
   normalized = utils::normalize( s6 );
   ASSERT_EQ( s1, normalized );

   String s7( "\n" );
   normalized = utils::normalize( s7 );
   ASSERT_EQ( "", normalized );
}

TEST( StringUtilitiesTest, Split )
{
   std::pair<String,String> splitted;

   String toSplit1( "   left right" );
   splitted = utils::split( toSplit1 );
   ASSERT_EQ( String( "   left " ), splitted.first );
   ASSERT_EQ( String( "right" ), splitted.second );

   String toSplit2( "left right" );
   splitted = utils::split( toSplit2 );
   ASSERT_EQ( String( "left " ), splitted.first );
   ASSERT_EQ( String( "right" ), splitted.second );

   String toSplit3( "left right " );
   splitted = utils::split( toSplit3 );
   ASSERT_EQ( String( "left right " ), splitted.first );
   ASSERT_EQ( String(), splitted.second );
}

} }
