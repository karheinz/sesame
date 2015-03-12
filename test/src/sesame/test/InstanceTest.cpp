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


#include <algorithm>
#include <fstream>
#include <stdexcept>
#include "gtest/gtest.h"
#include "types.hpp"
#include "sesame/definitions.hpp"
#include "sesame/packaging.hpp"
#include "sesame/Data.hpp"
#include "sesame/Entry.hpp"
#include "sesame/Instance.hpp"
#include "sesame/crypto/IMachine.hpp"
#include "sesame/crypto/MachineFactory.hpp"
#include "sesame/utils/string.hpp"


namespace sesame { namespace test {

TEST( InstanceTest, BasicUsage )
{
   utils::setLocale();

   Instance instance( PROTOCOL_SCRYPT_AES_CBC_SHA_V1 );
   ASSERT_EQ( PROTOCOL_SCRYPT_AES_CBC_SHA_V1, instance.getProtocol() );

   Entry e1( "Example Entry 1" );
   e1.addTag( "tag1" );
   e1.addTag( "tag2" );
   e1.addTag( "tag3" );
   Entry e2( "Example Entry 2" );
   e2.addTag( "tag2" );
   e2.addTag( "tag3" );
   e2.addTag( "tag4" );
   Entry e3( "Example Entry 3" );
   e3.addTag( "tag3" );
   e3.addTag( "tag4" );
   e3.addTag( "tag5" );

   ASSERT_TRUE( instance.addEntry( e1 ) );
   ASSERT_TRUE( instance.addEntry( e2 ) );
   ASSERT_TRUE( instance.addEntry( e3 ) );

   Set<Entry> entries( instance.getEntries() );
   ASSERT_EQ( 3, entries.size() );

   Vector<String> names;
   Set<Entry>::iterator it = entries.begin();
   do
   {
      names.push_back( it->getName() );
   }
   while ( ++it != entries.end() );
   std::sort( names.begin(), names.end() );

   ASSERT_EQ( "Example Entry 1", names[ 0 ] );
   ASSERT_EQ( "Example Entry 2", names[ 1 ] );
   ASSERT_EQ( "Example Entry 3", names[ 2 ] );

   Set<String> tags( instance.getTags() );
   ASSERT_EQ( 5, tags.size() );
   names.clear();
   Set<String>::iterator it2 = tags.begin();
   do
   {
      names.push_back( *it2 );
   }
   while ( ++it2 != tags.end() );
   std::sort( names.begin(), names.end() );

   ASSERT_EQ( "tag1", names[ 0 ] );
   ASSERT_EQ( "tag2", names[ 1 ] );
   ASSERT_EQ( "tag3", names[ 2 ] );
   ASSERT_EQ( "tag4", names[ 3 ] );
   ASSERT_EQ( "tag5", names[ 4 ] );
}

TEST( InstanceTest, Filter )
{
   utils::setLocale();

   Instance instance( PROTOCOL_SCRYPT_AES_CBC_SHA_V1 );
   ASSERT_EQ( PROTOCOL_SCRYPT_AES_CBC_SHA_V1, instance.getProtocol() );

   Entry e1( "Example Entry 1" );
   e1.addTag( "tag1" );
   e1.addTag( "tag2" );
   e1.addTag( "tag3" );
   Entry e2( "Example Entry 2" );
   e2.addTag( "tag2" );
   e2.addTag( "tag3" );
   e2.addTag( "tag4" );
   Entry e3( "Example Entry 3" );
   e3.addTag( "tag3" );
   e3.addTag( "tag4" );
   e3.addTag( "tag5" );

   ASSERT_TRUE( instance.addEntry( e1 ) );
   ASSERT_TRUE( instance.addEntry( e2 ) );
   ASSERT_TRUE( instance.addEntry( e3 ) );

   Set<String> filter = { "tag1", "tag5" };
   Set<Entry> entries( instance.getEntries( filter ) );
   ASSERT_EQ( 2, entries.size() );

   filter = { "tag5" };
   entries = instance.getEntries( filter );
   ASSERT_EQ( 1, entries.size() );
   Set<Entry>::iterator it = entries.begin();
   ASSERT_EQ( "Example Entry 3", it->getName() );
}

TEST( InstanceTest, Entries )
{
   utils::setLocale();

   Instance instance( PROTOCOL_SCRYPT_AES_CBC_SHA_V1 );
   ASSERT_EQ( PROTOCOL_SCRYPT_AES_CBC_SHA_V1, instance.getProtocol() );

   Entry e1( "Example Entry 1" );
   e1.addTag( "tag1" );
   e1.addTag( "tag2" );
   e1.addTag( "tag3" );
   Entry e2( "Example Entry 2" );
   e2.addTag( "tag2" );
   e2.addTag( "tag3" );
   e2.addTag( "tag4" );
   Entry e3( "Example Entry 3" );
   e3.addTag( "tag3" );
   e3.addTag( "tag4" );
   e3.addTag( "tag5" );

   ASSERT_TRUE( instance.addEntry( e1 ) );
   ASSERT_FALSE( instance.addEntry( e1 ) );
   e1.setName( "Example Entry 4" );
   ASSERT_TRUE( instance.updateEntry( e1 ) );
   Set<Entry> entries( instance.getEntries() );
   ASSERT_EQ( 1, entries.size() );
   Set<Entry>::iterator it = entries.begin();
   ASSERT_EQ( "Example Entry 4", it->getName() );
   e1.clear();
   ASSERT_FALSE( instance.updateEntry( e1 ) );
   ASSERT_FALSE( instance.deleteEntry( e1 ) );
   Entry e_del( *it );
   ASSERT_TRUE( instance.deleteEntry( e_del ) );

   entries = instance.getEntries();
   ASSERT_EQ( 0, entries.size() );
}

TEST( InstanceTest, DeSerialization )
{
   utils::setLocale();

   Map<String,Vector<uint8_t>> params1;
   {
      Vector<uint8_t> ldN;
      packV( ldN, 10U );
      params1[ utils::fromUtf8( u8"ldN" ) ] = ldN;
   }
   Map<String,Vector<uint8_t>> params2;
   {
      Vector<uint8_t> ldN;
      packV( ldN, 8U );
      params2[ utils::fromUtf8( u8"ldN" ) ] = ldN;
   }

   Instance instance( PROTOCOL_SCRYPT_AES_CBC_SHA_V1, params1, params2 );
   ASSERT_EQ( PROTOCOL_SCRYPT_AES_CBC_SHA_V1, instance.getProtocol() );

   Entry e1( "Example Entry 1" );
   e1.addTag( "tag1" );
   e1.addTag( "tag2" );
   e1.addTag( "tag3" );
   Entry e2( "Example Entry 2" );
   e2.addTag( "tag2" );
   e2.addTag( "tag3" );
   e2.addTag( "tag4" );
   Entry e3( "Example Entry 3" );
   e3.addTag( "tag3" );
   e3.addTag( "tag4" );
   e3.addTag( "tag5" );

   ASSERT_TRUE( instance.addEntry( e1 ) );
   ASSERT_TRUE( instance.addEntry( e2 ) );
   ASSERT_TRUE( instance.addEntry( e3 ) );

   std::ofstream file1( "instance.bin", std::ios_base::out | std::ios_base::trunc | std::ios_base::binary );
   ASSERT_NO_THROW( instance.write( file1, "hello world" ) );
   file1.close();

   std::ifstream file2( "instance.bin", std::ios_base::in | std::ios_base::binary );
   ASSERT_NO_THROW( Instance tmp( file2, "hello world" ) );
   file2.seekg( 0, std::ios_base::beg );
   Instance rebuild( file2, "hello world" );
   file2.close();
   ASSERT_EQ( instance.getId(), rebuild.getId() );
   ASSERT_EQ( instance.getProtocol(), rebuild.getProtocol() );
   ASSERT_EQ( instance.getEntries(), rebuild.getEntries() );

   std::ofstream file3( "instance2.bin", std::ios_base::out | std::ios_base::trunc | std::ios_base::binary );
   ASSERT_NO_THROW( instance.write( file3, "hello world" ) );
   file3.close();

   std::ifstream file4( "instance2.bin", std::ios_base::in | std::ios_base::binary );
   ASSERT_NO_THROW( Instance tmp( file4, "hello world" ) );
   file4.seekg( 0, std::ios_base::beg );
   Instance rebuild2( file4, "hello world" );
   file4.close();
   ASSERT_EQ( instance.getId(), rebuild2.getId() );
   ASSERT_EQ( instance.getProtocol(), rebuild2.getProtocol() );
   ASSERT_EQ( instance.getEntries(), rebuild2.getEntries() );

   std::ofstream file5( "instance3.bin", std::ios_base::out | std::ios_base::trunc | std::ios_base::binary );
   ASSERT_THROW( instance.write( file5, "hello world 123" ), std::runtime_error );
   file5.close();
}

TEST( InstanceTest, LabeledData )
{
   utils::setLocale();

   Map<String,Vector<uint8_t>> params1;
   {
      Vector<uint8_t> ldN;
      packV( ldN, 10U );
      params1[ utils::fromUtf8( u8"ldN" ) ] = ldN;
   }
   Map<String,Vector<uint8_t>> params2;
   {
      Vector<uint8_t> ldN;
      packV( ldN, 8U );
      params2[ utils::fromUtf8( u8"ldN" ) ] = ldN;
   }

   Instance instance( PROTOCOL_SCRYPT_AES_CBC_SHA_V1, params1, params2 );
   ASSERT_EQ( PROTOCOL_SCRYPT_AES_CBC_SHA_V1, instance.getProtocol() );

   Entry e1( "Example Entry 1" );
   e1.addTag( "tag1" );
   e1.addTag( "tag2" );
   e1.addTag( "tag3" );
   ASSERT_TRUE( instance.addEntry( e1 ) );
   ASSERT_TRUE( instance.isNew() );
   ASSERT_TRUE( instance.isDirty() );

   Data data( "password" );
   ASSERT_TRUE( data.isDirty() );
   Entry copy( instance.findEntry( e1.getIdAsHexString() ) );
   ASSERT_TRUE( copy.addLabeledData( "password", data ) );
   ASSERT_TRUE( instance.updateEntry( copy ) );
   ASSERT_TRUE( instance.isDirty() );

   std::ofstream file1( "instance.bin", std::ios_base::out | std::ios_base::trunc | std::ios_base::binary );
   ASSERT_NO_THROW( instance.write( file1, "hello world" ) );
   file1.close();
   ASSERT_FALSE( instance.isNew() );
   ASSERT_FALSE( instance.isDirty() );

   std::ifstream file2( "instance.bin", std::ios_base::in | std::ios_base::binary );
   ASSERT_NO_THROW( Instance tmp( file2, "hello world" ) );
   file2.seekg( 0, std::ios_base::beg );
   Instance rebuild( file2, "hello world" );
   file2.close();
   ASSERT_EQ( instance.getId(), rebuild.getId() );
   ASSERT_EQ( instance.getProtocol(), rebuild.getProtocol() );
   ASSERT_EQ( instance.getEntries(), rebuild.getEntries() );
   ASSERT_FALSE( rebuild.isNew() );
   ASSERT_FALSE( rebuild.isDirty() );

   copy = rebuild.findEntry( e1.getIdAsHexString() );
   ASSERT_FALSE( copy.getLabeledData().begin()->second.isPlaintextAvailable() );
   ASSERT_NO_THROW( rebuild.decryptEntry( copy, "hello world" ) );
   ASSERT_TRUE( copy.getLabeledData().begin()->second.isPlaintextAvailable() );
   ASSERT_EQ( String( "password" ), copy.getLabeledData().begin()->second.getPlaintext<String>() );
}

} }
