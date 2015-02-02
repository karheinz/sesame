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


#include <stdexcept>
#include <string>
#include "gtest/gtest.h"
#include "sesame/definitions.hpp"
#include "types.hpp"
#include "sesame/Data.hpp"
#include "msgpack.hpp"


namespace sesame { namespace test {

TEST( DataTest, BasicUsage )
{
   Data d( "Some data!" );
   ASSERT_EQ( Data::TEXT, d.getType() );
   ASSERT_TRUE( d.isDirty() );
   ASSERT_TRUE( d.isPlaintextAvailable() );

   String plaintext( d.getPlaintext<String>() );
   ASSERT_EQ( "Some data!", plaintext );
   ASSERT_EQ( 10, plaintext.size() );

   Vector<uint8_t> b1 = { 72U, 101U, 108U, 108U, 111U, 33U };
   d.setPlaintext( b1 );
   ASSERT_EQ( Data::BINARY, d.getType() );
   ASSERT_TRUE( d.isDirty() );
   ASSERT_TRUE( d.isPlaintextAvailable() );
   Vector<uint8_t> b2( d.getPlaintext<Vector<uint8_t> >() );
   ASSERT_EQ( b1, b2 );
   ASSERT_EQ( 6, b2.size() );

   // Types other than Vector<uint8_t> and String have no effect.
   std::string dummy( d.getPlaintext<std::string>() );
   ASSERT_EQ( 0, dummy.size() );

   // String works.
   String hello( d.getPlaintext<String>() );
   ASSERT_EQ( "Hello!", hello );
   ASSERT_EQ( 6, hello.size() );

   // String plaintext ends with first zero byte.
   Vector<uint8_t> b3 = { 72U, 101U, 108U, 108U, 0U, 111U, 33U };
   d.setPlaintext( b3 );
   ASSERT_EQ( 7, d.getPlaintext<Vector<uint8_t> >().size() );
   ASSERT_EQ( 4, d.getPlaintext<String>().size() );
}

TEST( DataTest, DeSerialization )
{
   Data d( "Some data!" );
   ASSERT_EQ( Data::TEXT, d.getType() );
   ASSERT_TRUE( d.isPlaintextAvailable() );
   d.setPlaintext( Vector<uint8_t>( 16, 0xff ) );
   ASSERT_EQ( Data::BINARY, d.getType() );

   // Serialize.
   StringStream s;
   msgpack::pack( s, d );

   // Deserialize.
   msgpack::unpacked r;
   msgpack::unpack( &r, s.str().data(), s.str().size() );
   msgpack::object o( r.get() );
   Data d2( o.as<Data>() );
   ASSERT_EQ( Data::BINARY, d2.getType() );
   ASSERT_FALSE( d2.isPlaintextAvailable() );
}

} }
