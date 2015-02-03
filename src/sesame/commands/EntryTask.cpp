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
#include "types.hpp"
#include "sesame/Entry.hpp"
#include "sesame/commands/EntryTask.hpp"
#include "sesame/utils/Reader.hpp"

namespace sesame { namespace commands {

namespace {
   struct EntrySorter
   {
      bool operator()( const Entry& a, const Entry& b )
      {
         return ( a.getName() < b.getName() );
      }
   };
}

EntryTask::EntryTask( const Type taskType, const String& id, const String& otherId ) :
   ICommand(),
   m_TaskType( taskType ),
   m_Id( id ),
   m_OtherId( otherId )
{
}

void EntryTask::run( std::shared_ptr<Instance>& instance )
{
   if ( ! instance )
   {
      throw std::runtime_error( "no instance open" );
   }

   switch ( m_TaskType )
   {
      case LIST:
      {
         Set<Entry> tmp( instance->getEntries() );
         Vector<Entry> entries( tmp.begin(), tmp.end() );
         std::sort( entries.begin(), entries.end(), EntrySorter() );

         for ( auto entry : entries )
         {
            std::cout << "[#" << entry.getIdAsHexString() << "] " << entry.getName() << std::endl;
         }
         break;
      }
      case SHOW:
      {
         Entry entry( instance->findEntry( m_Id ) );
         std::cout << "[#" << entry.getIdAsHexString() << "] " << entry.getName() << std::endl;
         Map<String,String> attributes( entry.getAttributes() );
         std::size_t i = 1;
         for ( auto attribute : attributes )
         {
            std::cout << "[#" << i++ << "] " << attribute.first << ": " << attribute.second << std::endl;
         }
         break;
      }
      case ADD:
      {
         utils::Reader reader( 1024 );
         String name( reader.readLine( "Name: " ) );
         Entry entry( name );
         if ( ! instance->addEntry( entry ) )
         {
            throw std::runtime_error( "failed to add entry" );
         }
         break;
      }
      case DELETE:
      {
         Entry entry( instance->findEntry( m_Id ) );
         if ( ! instance->deleteEntry( entry ) )
         {
            throw std::runtime_error( "failed to delete entry" );
         }

         std::cout << "deleted entry #" << entry.getIdAsHexString() << std::endl;

         break;
      }
      case ADD_ATTRIBUTE:
      {
         Entry entry( instance->findEntry( m_Id ) );

         utils::Reader reader( 1024 );
         String name( reader.readLine( "Name: " ) );
         String value( reader.readLine( "Value: " ) );

         if ( ! entry.addAttribute( name, value ) )
         {
            throw std::runtime_error( "failed to add attribute" );
         }
         if ( ! instance->updateEntry( entry ) )
         {
            throw std::runtime_error( "failed to update entry" );
         }

         break;
      }
      case DELETE_ATTRIBUTE:
      {
         Entry entry( instance->findEntry( m_Id ) );
         std::pair<String,String> attribute( entry.getAttribute( m_OtherId ) );

         if ( ! entry.deleteAttribute( attribute.first ) )
         {
            throw std::runtime_error( "failed to delete attribute" );
         }
         if ( ! instance->updateEntry( entry ) )
         {
            throw std::runtime_error( "failed to update entry" );
         }

         break;
      }
      default:
      {
         throw std::runtime_error( "command not implemented yet" );
      }
   }
}

} }
