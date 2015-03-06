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


#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>
#include <utility>

#include <xselection.h>

#include "types.hpp"
#include "sesame/Entry.hpp"
#include "sesame/commands/EntryTask.hpp"
#include "sesame/utils/colors.hpp"
#include "sesame/utils/completion.hpp"
#include "sesame/utils/filesystem.hpp"
#include "sesame/utils/lines.hpp"
#include "sesame/utils/string.hpp"
#include "sesame/utils/xselection.hpp"
#include "sesame/utils/Reader.hpp"
#include "sesame/utils/TeclaReader.hpp"

namespace sesame { namespace commands {

namespace {
   struct EntrySorter
   {
      bool operator()( const Entry& a, const Entry& b )
      {
         return ( a.getName() < b.getName() );
      }
   };

   const Vector<Entry> toSortedVector( const Set<Entry>& s )
   {
      Vector<Entry> v( s.begin(), s.end() );
      std::sort( v.begin(), v.end(), EntrySorter() );
      return v;
   }

   template <class T>
   struct ElemSorter
   {
      bool operator()( const std::pair<String,T>& a, const std::pair<String,T>& b )
      {
         return ( a.first < b.first );
      }
   };

   template <class T>
   const Vector<std::pair<String,T>> toSortedVector( const Map<String,T>& m )
   {
      Vector<std::pair<String,T>> v( m.begin(), m.end() );
      std::sort( v.begin(), v.end(), ElemSorter<T>() );
      return v;
   }

   template <class T>
   const std::pair<String,T> getElemAtPos( const Vector<std::pair<String,T>>& v, const String& pos )
   {
      // Adjust pos.
      String s( pos );

      if ( s.find( "#" ) == 0 )
      {
         s.replace( 0, 1, "" );
      }

      StringStream ss;
      ss << s;
      std::size_t p;
      ss >> p;

      if ( p < 1 || p > v.size() )
      {
         throw std::runtime_error( "elem not found" );
      }

      return v[ p - 1 ];
   }
}

EntryTask::EntryTask( const Type taskType, const String& id, const String& pos ) :
   ICommand(),
   m_TaskType( taskType ),
   m_Id( id ),
   m_Pos( pos )
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
         const Vector<Entry> entries( toSortedVector( instance->getEntries() ) );

         if ( entries.empty() )
         {
            std::cout << "No entries yet." << std::endl;
         }
         else
         {
            std::cout << "Entries:" << std::endl;
         }

         std::size_t i( entries.size() );
         for ( auto& entry : entries )
         {
            if ( i > 1 )
            {
               std::cout << utils::branch();
            }
            else
            {
               std::cout << utils::corner();
            }

            std::cout << "[#" << entry.getIdAsHexString() << "] "
                      << utils::ESC_SEQ_BOLD
                      << entry.getName()
                      << utils::ESC_SEQ_RESET
                      << std::endl;

            --i;
         }
         break;
      }
      case SHOW:
      {
         const Entry entry( instance->findEntry( m_Id ) );
         std::cout << "[#" << entry.getIdAsHexString() << "] "
                   << utils::ESC_SEQ_BOLD
                   << entry.getName()
                   << utils::ESC_SEQ_RESET
                   << std::endl;

         const Vector<std::pair<String,String>> attributes( toSortedVector( entry.getAttributes() ) );
         const Vector<std::pair<String,Data>> data( toSortedVector( entry.getLabeledData() ) );
         String filler( utils::down() );

         std::cout << utils::branch() << "Attribute(s):" << std::endl;
         std::size_t i = 1;
         for ( auto& attribute : attributes )
         {
            if ( i < attributes.size() )
            {
               std::cout << filler << utils::branch( 1 );
            }
            else
            {
               std::cout << filler << utils::corner( 1 );
            }
            std::cout << "[#" << i++ << "] " << attribute.first << ": " << attribute.second << std::endl;
         }

         std::cout << utils::corner() << "Password(s)/Key(s):" << std::endl;
         std::size_t k = 1;
         for ( auto& date : data )
         {
            if ( k < data.size() )
            {
               std::cout << utils::empty() << utils::branch( 1 );
            }
            else
            {
               std::cout << utils::empty() << utils::corner( 1 );
            }
            std::cout << "[#" << k++ << "] " << date.first << ": ";
            if ( date.second.getType() == Data::TEXT )
            {
               if ( date.second.isPlaintextAvailable() )
               {
                  std::cout << date.second.getPlaintext<String>();
               }
               else
               {
                  std::cout << "<encrypted text>";
               }
            }
            else
            {
               if ( date.second.isPlaintextAvailable() )
               {
                  std::cout << "binary data (" <<
                     date.second.getPlaintext<Vector<uint8_t>>().size() << " bytes)";
               }
               else
               {
                  std::cout << "<encrypted binary data>";
               }
            }

            std::cout << std::endl;
         }
         break;
      }
      case DECRYPT:
      {
         Entry entry( instance->findEntry( m_Id ) );
         decryptEntry( instance, entry );
         instance->updateEntry( entry );
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
      case UPDATE:
      {
         Entry entry( instance->findEntry( m_Id ) );

         utils::Reader reader( 1024 );
         String name( reader.readLine( "Name: " ) );
         name = utils::strip( name );
         entry.setName( name );

         if ( ! instance->updateEntry( entry ) )
         {
            throw std::runtime_error( "failed to update entry" );
         }

         break;
      }
      case ADD_ATTRIBUTE:
      {
         Entry entry( instance->findEntry( m_Id ) );

         utils::Reader reader( 1024 );
         String name( reader.readLine( "Name: " ) );
         name = utils::strip( name );
         String value( reader.readLine( "Value: " ) );
         value = utils::strip( value );

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
         Vector<std::pair<String,String>> attributes( toSortedVector( entry.getAttributes() ) );
         const std::pair<String,String> attribute( getElemAtPos( attributes, m_Pos ) );

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
      case UPDATE_ATTRIBUTE:
      {
         Entry entry( instance->findEntry( m_Id ) );
         const Vector<std::pair<String,String>> attributes( toSortedVector( entry.getAttributes() ) );
         const std::pair<String,String> attribute( getElemAtPos( attributes, m_Pos ) );

         utils::Reader reader( 1024 );
         //std::cin.write( attribute.first );
         String name( reader.readLine( "Name: " ) );
         name = utils::strip( name );
         //std::cin.write( attribute.second );
         String value( reader.readLine( "Value: " ) );
         value = utils::strip( value );

         if ( ! entry.updateAttribute( attribute.first, name, value ) )
         {
            throw std::runtime_error( "failed to update attribute" );
         }
         if ( ! instance->updateEntry( entry ) )
         {
            throw std::runtime_error( "failed to update entry" );
         }

         break;
      }
      case ADD_PASSWORD:
      {
         Entry entry( instance->findEntry( m_Id ) );

         utils::Reader reader( 1024 );
         String label( reader.readLine( "Label: " ) );
         label = utils::strip( label );
         String password( reader.readLine( "Password: " ) );
         password = utils::strip( password );

         if ( ! entry.addLabeledData( label, Data( password ) ) )
         {
            throw std::runtime_error( "failed to add password" );
         }
         if ( ! instance->updateEntry( entry ) )
         {
            throw std::runtime_error( "failed to update entry" );
         }

         break;
      }
      case ADD_KEY:
      {
         Entry entry( instance->findEntry( m_Id ) );

         utils::Reader reader( 1024 );
         String label( reader.readLine( "Label: " ) );
         label = utils::strip( label );
         const Vector<uint8_t> data( askForInputFileAndRead() );

         if ( ! entry.addLabeledData( label, Data( data ) ) )
         {
            throw std::runtime_error( "failed to add key" );
         }
         if ( ! instance->updateEntry( entry ) )
         {
            throw std::runtime_error( "failed to update entry" );
         }

         break;
      }
      case DELETE_PASSWORD_OR_KEY:
      {
         Entry entry( instance->findEntry( m_Id ) );
         Vector<std::pair<String,Data>> labeledData( toSortedVector( entry.getLabeledData() ) );
         const std::pair<String,Data> labeledDate( getElemAtPos( labeledData, m_Pos ) );

         if ( ! entry.deleteLabeledData( labeledDate.first ) )
         {
            throw std::runtime_error( "failed to delete password or key" );
         }
         if ( ! instance->updateEntry( entry ) )
         {
            throw std::runtime_error( "failed to update entry" );
         }

         break;
      }
      case UPDATE_PASSWORD_OR_KEY:
      {
         Entry entry( instance->findEntry( m_Id ) );
         const Vector<std::pair<String,Data>> labeledData( toSortedVector( entry.getLabeledData() ) );
         const std::pair<String,Data> labeledDate( getElemAtPos( labeledData, m_Pos ) );

         utils::Reader reader( 1024 );
         String label( reader.readLine( "Label: " ) );
         label = utils::strip( label );

         if ( labeledDate.second.getType() == Data::TEXT )
         {
            String password( reader.readLine( "Password: " ) );
            password = utils::strip( password );

            if ( ! entry.updateLabeledData( labeledDate.first, label, Data( password ) ) )
            {
               throw std::runtime_error( "failed to update password" );
            }
         }
         else
         {
            const Vector<uint8_t> data( askForInputFileAndRead() );

            if ( ! entry.updateLabeledData( labeledDate.first, label, Data( data ) ) )
            {
               throw std::runtime_error( "failed to update key" );
            }
         }

         if ( ! instance->updateEntry( entry ) )
         {
            throw std::runtime_error( "failed to update entry" );
         }

         break;
      }
      case EXPORT_PASSWORD_OR_KEY:
      {
         Entry entry( instance->findEntry( m_Id ) );
         Vector<std::pair<String,Data>> labeledData( toSortedVector( entry.getLabeledData() ) );
         std::pair<String,Data> labeledDate( getElemAtPos( labeledData, m_Pos ) );

         if ( labeledDate.second.getType() == Data::TEXT )
         {
            decryptData( instance, labeledDate.second );
            utils::xselect( labeledDate.second.getPlaintext<String>() );
         }
         else
         {
            utils::TeclaReader reader( 1024 );
            reader.addCompletion( cpl_file_completions, nullptr );
            String path( reader.readLine( "File: " ) );
            path = utils::strip( path );

            if ( path.empty() )
            {
               throw std::runtime_error( "missing filename" );
            }

            bool alreadyExists( utils::exists( path.c_str() ) );

            if ( alreadyExists )
            {
               if ( utils::isFile( path.c_str() ) )
               {
                  utils::Reader reader( 1024 );
                  String choice( reader.readLine( "Overwrite existing file? [y/N]  " ) );
                  choice = utils::strip( utils::toUtf8( choice ) );
                  if ( choice != u8"y" && choice != u8"Y" )
                  {
                     break;
                  }
               }
               else
               {
                  StringStream s;
                  s << path << " is no file";
                  throw std::runtime_error( s.str().c_str() );
               }
            }

            std::ofstream file(
               path.c_str(),
               std::ios_base::out | std::ios_base::trunc | std::ios_base::binary
               );

            if ( ! file.good() )
            {
               throw std::runtime_error( "failed to open file" );
            }

            Vector<uint8_t> key( labeledDate.second.getPlaintext<Vector<uint8_t>>() );
            if ( ! key.empty() )
            {
               file.write( reinterpret_cast<char*>( key.data() ), key.size() );
            }
         }

         break;
      }
      default:
      {
         throw std::runtime_error( "command not implemented yet" );
      }
   }
}

const Vector<uint8_t> EntryTask::askForInputFileAndRead()
{
   utils::TeclaReader reader( 1024 );
   reader.addCompletion( cpl_file_completions, nullptr );
   String path( reader.readLine( "File: " ) );
   path = utils::strip( path );

   if ( path.empty() )
   {
      throw std::runtime_error( "missing filename" );
   }
   else if ( ! utils::isFile( path.c_str() ) )
   {
      StringStream s;
      s << path << " is no file";
      throw std::runtime_error( s.str().c_str() );
   }

   std::ifstream file( path.c_str(), std::ios_base::in | std::ios_base::binary );
   if ( ! file.good() )
   {
      throw std::runtime_error( "failed to open file" );
   }
   Vector<uint8_t> data;
   data.resize( utils::getFileSize( path ) );

   if ( data.empty() )
   {
      throw std::runtime_error( "file is empty" );
   }

   file.read( reinterpret_cast<char*>( data.data() ), data.size() );

   return data;
}

void EntryTask::decryptEntry( std::shared_ptr<Instance>& instance, Entry& entry )
{
   if ( ! entry.isPlain() )
   {
      utils::Reader reader( 1024 );
      String password( reader.readLine( "password or phrase: ", true ) );
      password = utils::strip( password );
      instance->decryptEntry( entry, password );
   }
}

void EntryTask::decryptData( std::shared_ptr<Instance>& instance, Data& data )
{
   if ( ! data.isPlaintextAvailable() )
   {
      utils::Reader reader( 1024 );
      String password( reader.readLine( "password or phrase: ", true ) );
      password = utils::strip( password );
      instance->decryptData( data, password );
   }
}

} }
