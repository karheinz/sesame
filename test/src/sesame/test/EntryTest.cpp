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
#include <stdexcept>
#include "gtest/gtest.h"

#include "types.hpp"
#include "sesame/definitions.hpp"
#include "sesame/packaging.hpp"
#include "sesame/utils/string.hpp"
#include "sesame/Data.hpp"
#include "sesame/Entry.hpp"


namespace sesame { namespace test {

std::ostream& operator<<( std::ostream& stream, const Vector<char> vector )
{
   stream << std::hex;
   for ( std::size_t i = 0; i < vector.size(); ++i )
   {
      stream << std::setw( 2 ) << std::setfill( '0' ) << ( 0x00ff & vector[ i ] );
   }
   stream << std::dec;

   return stream;
}

TEST( EntryTest, BasicUsage )
{
   Entry e1( "Example Entry" );

   ASSERT_EQ( "Example Entry", e1.getName() );
   ASSERT_TRUE( e1.getTags().empty() );

   e1.setName( "My Entry" );
   ASSERT_EQ( "My Entry", e1.getName() );

   e1.addTag( "tag1" );
   ASSERT_EQ( 1, e1.getTags().size() );
   Set<String> tags( e1.getTags() );
   ASSERT_FALSE( tags.find( "tag1" ) == tags.end() );
   e1.deleteTag( "tag1" );
   ASSERT_TRUE( e1.getTags().empty() );
}

TEST( EntryTest, DeSerialization )
{
   utils::setLocale();

   Entry e1( "Example Entry" );
   e1.addTag( "tag1" );

   // Serialize.
   StringStream s;
   s << e1;
   e1.setName( "Example Entry 2" );
   s << e1;
   e1.setName( "Example Entry 3" );
   s << e1;

   String st( s.str() );
   std::cout << st << std::endl;
   Vector<char> v( st.data(), st.data() + st.size() );
   std::cout << v << std::endl;

   // Deserialize.
   Entry e2;
   s >> e2;

   ASSERT_EQ( "Example Entry", e2.getName() );

   ASSERT_EQ( 1, e2.getTags().size() );
   Set<String> tags( e2.getTags() );
   ASSERT_FALSE( tags.find( "tag1" ) == tags.end() );
   e2.deleteTag( "tag1" );
   ASSERT_TRUE( e2.getTags().empty() );

   s >> e2;
   ASSERT_EQ( "Example Entry 2", e2.getName() );

   ASSERT_EQ( 1, e2.getTags().size() );
   tags = e2.getTags();
   ASSERT_FALSE( tags.find( "tag1" ) == tags.end() );
   e2.deleteTag( "tag1" );
   ASSERT_TRUE( e2.getTags().empty() );

   s >> e2;
   ASSERT_EQ( "Example Entry 3", e2.getName() );

   ASSERT_EQ( 1, e2.getTags().size() );
   tags = e2.getTags();
   ASSERT_FALSE( tags.find( "tag1" ) == tags.end() );
   e2.deleteTag( "tag1" );
   ASSERT_TRUE( e2.getTags().empty() );
}

TEST( EntryTest, LabeledData )
{
   Entry e1( "Example Entry" );
   ASSERT_TRUE( e1.addLabeledData( "password", Data( "123456" ) ) );
   ASSERT_FALSE( e1.addLabeledData( "password", Data( "123456" ) ) );
   std::map<String,Data> labeledData( e1.getLabeledData() );
   ASSERT_EQ( 1, labeledData.size() );
   ASSERT_FALSE( labeledData.find( "password" ) == labeledData.end() );
   ASSERT_TRUE( labeledData.find( "missing" ) == labeledData.end() );
   std::map<String,Data>::iterator data( labeledData.find( "password" ) );
   ASSERT_EQ( "password", ( *data ).first );
   ASSERT_EQ( "123456", ( *data ).second.getPlaintext<String>() );

   ASSERT_TRUE( e1.updateLabeledData( "password", "password", Data( "654321" ) ) );
   labeledData = e1.getLabeledData();
   ASSERT_EQ( 1, labeledData.size() );
   ASSERT_FALSE( labeledData.find( "password" ) == labeledData.end() );
   data = labeledData.find( "password" );
   ASSERT_EQ( "password", ( *data ).first );
   ASSERT_EQ( "654321", ( *data ).second.getPlaintext<String>() );
}

TEST( EntryTest, Equality )
{
   Entry e1( "Example Entry" );
   Entry e1_copy( e1 );
   ASSERT_TRUE( e1 == e1_copy );
   e1_copy.setName( "Changed Entry Name" );
   ASSERT_TRUE( e1 == e1_copy );

   Entry e2( "Example Entry" );
   ASSERT_FALSE( e1 == e2 );
}

} }
