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


#include <chrono>
#include <iomanip>
#include <random>
#include <utility>

#include "sesame/Entry.hpp"

using std::chrono::system_clock;

namespace sesame
{
   Entry::Entry() :
      m_Id( std::random_device()() ),
      m_InstanceId( 0 ),
      m_CreatedAt( system_clock::to_time_t( system_clock::now() ) ),
      m_UpdatedAt( m_CreatedAt ),
      m_Name()
   {
   }

   Entry::Entry( const String& name ) :
      m_Id( std::random_device()() ),
      m_InstanceId( 0 ),
      m_CreatedAt( system_clock::to_time_t( system_clock::now() ) ),
      m_UpdatedAt( m_CreatedAt ),
      m_Name( name )
   {
   }

   uint32_t Entry::getId() const
   {
      return m_Id;
   }

   String Entry::getIdAsHexString()
   {
      StringStream s;
      s << std::hex << std::setw( 8 ) << std::setfill( '0' ) << m_Id;
      return s.str();
   }

   String Entry::getName() const
   {
      return m_Name;
   }

   void Entry::setName( const String& name )
   {
      m_Name = name;
   }

   Map<String,String> Entry::getAttributes() const
   {
      return m_Attributes;
   }

   bool Entry::addAttribute( const String& name, const String& value )
   {
      return m_Attributes.insert( std::make_pair<String,String>( String( name ), String( value ) ) ).second;
   }

   bool Entry::updateAttribute( const String& name, const String& value )
   {
      if ( m_Attributes.find( name ) != m_Attributes.end() )
      {
         m_Attributes[ name ] = value;
         return true;
      }
      else
      {
         return false;
      }
   }

   bool Entry::deleteAttribute( const String& name )
   {
      return ( m_Attributes.erase( name ) == 1 );
   }

   Map<String,Data> Entry::getLabeledData() const
   {
      return m_LabeledData;
   }

   bool Entry::addLabeledData( const String& label, const Data& data )
   {
      return m_LabeledData.insert( std::make_pair<String,Data>( String( label ), Data( data ) ) ).second;
   }

   bool Entry::updateLabeledData( const String& label, const Data& data )
   {
      if ( m_LabeledData.find( label ) != m_LabeledData.end() )
      {
         m_LabeledData[ label ] = data;
         return true;
      }
      else
      {
         return false;
      }
   }

   bool Entry::deleteLabeledData( const String& label )
   {
      return ( m_LabeledData.erase( label ) == 1 );
   }

   bool Entry::isPlain() const
   {
      bool result( true );

      for ( Map<String,Data>::const_iterator it = m_LabeledData.cbegin();
         it != m_LabeledData.cend();
         ++it
         )
      {
         result = ( result && it->second.isPlaintextAvailable() );
         if ( result == false )
         {
            break;
         }
      }

      return result;
   }

   Set<String> Entry::getTags() const
   {
      return m_Tags;
   }

   bool Entry::hasTag( const String& tag ) const
   {
      return ( m_Tags.find( tag ) != m_Tags.cend() );
   }

   void Entry::addTag( const String& tag )
   {
      m_Tags.insert( tag );
   }

   void Entry::deleteTag( const String& tag )
   {
      m_Tags.erase( tag );
   }

   void Entry::clear()
   {
      reconfigure( 0 );
   }

   void Entry::reconfigure( uint32_t instanceId )
   {
      m_InstanceId = instanceId;

      for ( Map<String,Data>::iterator it = m_LabeledData.begin();
         it != m_LabeledData.end();
         ++it
         )
      {
         it->second.clear();
      }
   }
}
