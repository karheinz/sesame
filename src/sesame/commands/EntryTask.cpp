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

extern std::vector<std::pair<std::string,std::string>> apgCache;


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

   const Vector<String> setToSortedVector( const Set<String>& s )
   {
      Vector<String> v( s.begin(), s.end() );
      std::sort( v.begin(), v.end() );
      return v;
   }

   const String getTagAtPos( const Vector<String>& v, const String& pos )
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
         throw std::runtime_error( "tag not found" );
      }

      return v[ p - 1 ];
   }

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

   String preProcessPassword( const String& password )
   {
      String result( password );

      // Reference to apgCache?
      if ( password.size() > 1 &&
           password.find( "#" ) == 0 &&
           password.substr( 1 ).find_first_not_of( "0123456789" ) == String::npos
         )
      {
         std::size_t index;
         StringStream num;
         num << password.substr( 1 );
         num >> index;

         if ( index < 1 || index > apgCache.size() )
         {
            throw std::runtime_error( "generated password does not exist" );
         }

         result = apgCache[ index - 1 ].first.c_str();
      }

      return result;
   }

   String preProcessTag( std::shared_ptr<Instance>& instance, const String& tag )
   {
      String result( tag );

      // Reference to existing tag?
      if ( tag.size() > 1 &&
           tag.find( "#" ) == 0 &&
           tag.substr( 1 ).find_first_not_of( "0123456789" ) == String::npos
         )
      {
         std::size_t index;
         StringStream num;
         num << tag.substr( 1 );
         num >> index;

         Vector<String> allTags( setToSortedVector( instance->getTags() ) );
         if ( index < 1 || index > allTags.size() )
         {
            throw std::runtime_error( "tag does not exist" );
         }

         result = allTags[ index - 1 ].c_str();
      }

      return result;
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
         Vector<Entry> entries;

         if ( m_Id.empty() )
         {
            entries = toSortedVector( instance->getEntries() );
         }
         else
         {
            Vector<String> tags( setToSortedVector( instance->getTags() ) );
            String tag( getTagAtPos( tags, m_Id ) );
            Set<String> filter;
            filter.insert( tag );

            entries = toSortedVector( instance->getEntries( filter ) );
         }

         if ( ! m_Id.empty() && entries.empty() )
         {
            throw std::runtime_error( "tag not found" );
         }

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
      case TAGS:
      {
         const Vector<String> tags( setToSortedVector( instance->getTags() ) );

         if ( tags.empty() )
         {
            std::cout << "No tags yet." << std::endl;
         }
         else
         {
            std::cout << "Tags:" << std::endl;
         }

         std::size_t i( tags.size() );
         std::size_t k( 1 );
         for ( auto& tag : tags )
         {
            if ( i-- > 1 )
            {
               std::cout << utils::branch();
            }
            else
            {
               std::cout << utils::corner();
            }

            std::cout << "[#" << k++ << "] "
                      << tag
                      << std::endl;
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

         const Vector<String> allTags( setToSortedVector( instance->getTags() ) );
         const Set<String> tags( entry.getTags() );
         const Vector<std::pair<String,String>> attributes( toSortedVector( entry.getAttributes() ) );
         const Vector<std::pair<String,Data>> data( toSortedVector( entry.getLabeledData() ) );
         String filler( utils::down() );

         std::cout << utils::branch() << "Tag(s):" << std::endl;
         std::size_t h = 0;
         for ( auto& tag : allTags )
         {
            ++h;

            if ( tags.find( tag ) != tags.end() )
            {
               if ( h < tags.size() )
               {
                  std::cout << filler << utils::branch( 1 );
               }
               else
               {
                  std::cout << filler << utils::corner( 1 );
               }

               std::cout << "[#" << h << "] " << tag << std::endl;
            }
         }

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
            if ( date.second.getType() == DATA_TEXT )
            {
               std::cout << "[#" << k++ << "][P] " << date.first << ": ";
               if ( date.second.isPlaintextAvailable() )
               {
                  std::cout << date.second.getPlaintext<String>();
               }
               else
               {
                  std::cout << "***";
               }
            }
            else
            {
               std::cout << "[#" << k++ << "][K] " << date.first << ": ";
               if ( date.second.isPlaintextAvailable() )
               {
                  std::cout << date.second.getPlaintext<Vector<uint8_t>>().size() << "B";
               }
               else
               {
                  std::cout << "***";
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
         std::cout << "Decrypted entry #" << entry.getIdAsHexString() << "." << std::endl;
         break;
      }
      case ADD:
      {
         utils::Reader reader( 1024 );
         String name( reader.readLine( "Name: " ) );
         name = utils::strip( name );
         checkInput( name, "empty name" );
         Entry entry( name );
         if ( ! instance->addEntry( entry ) )
         {
            throw std::runtime_error( "failed to add entry" );
         }
         std::cout << "Added entry #" << entry.getIdAsHexString() << "." << std::endl;
         break;
      }
      case DELETE:
      {
         Entry entry( instance->findEntry( m_Id ) );
         if ( ! instance->deleteEntry( entry ) )
         {
            throw std::runtime_error( "failed to delete entry" );
         }
         std::cout << "Deleted entry #" << entry.getIdAsHexString() << "." << std::endl;
         break;
      }
      case UPDATE:
      {
         Entry entry( instance->findEntry( m_Id ) );

         utils::Reader reader( 1024 );
         StringStream namePrompt;
         namePrompt << "Name (" << entry.getName() << "): ";
         String name( reader.readLine( namePrompt.str() ) );
         name = utils::strip( name );

         if ( name.empty() || name == entry.getName() )
         {
            std::cout << "No changes." << std::endl;
            break;
         }

         entry.setName( name );
         if ( ! instance->updateEntry( entry ) )
         {
            throw std::runtime_error( "failed to update entry" );
         }
         std::cout << "Updated entry #" << entry.getIdAsHexString() << "." << std::endl;

         break;
      }
      case ADD_ATTRIBUTE:
      {
         Entry entry( instance->findEntry( m_Id ) );

         utils::Reader reader( 1024 );
         String name( reader.readLine( "Name: " ) );
         name = utils::strip( name );
         checkInput( name, "empty name" );
         String value( reader.readLine( "Value: " ) );
         value = utils::strip( value );
         checkInput( value, "empty value" );

         if ( ! entry.addAttribute( name, value ) )
         {
            throw std::runtime_error( "failed to add attribute" );
         }
         if ( ! instance->updateEntry( entry ) )
         {
            throw std::runtime_error( "failed to update entry" );
         }
         std::cout << "Added attribute to entry #" << entry.getIdAsHexString() << "." << std::endl;
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
         std::cout << "Deleted attribute " << m_Pos << " from entry #" <<
            entry.getIdAsHexString() << "." << std::endl;
         break;
      }
      case UPDATE_ATTRIBUTE:
      {
         Entry entry( instance->findEntry( m_Id ) );
         const Vector<std::pair<String,String>> attributes( toSortedVector( entry.getAttributes() ) );
         const std::pair<String,String> attribute( getElemAtPos( attributes, m_Pos ) );

         utils::Reader reader( 1024 );
         StringStream namePrompt;
         namePrompt << "Name (" << attribute.first << "): ";
         String name( reader.readLine( namePrompt.str() ) );
         name = utils::strip( name );
         StringStream valuePrompt;
         valuePrompt << "Value (" << attribute.second << "): ";
         String value( reader.readLine( valuePrompt.str() ) );
         value = utils::strip( value );

         if ( ( name.empty() || name == attribute.first ) &&
              ( value.empty() || value == attribute.second )
            )
         {
            std::cout << "No changes." << std::endl;
            break;
         }

         if ( ! entry.updateAttribute(
                  attribute.first,
                  name.empty() ? attribute.first : name,
                  value.empty() ? attribute.second : value
                  )
            )
         {
            throw std::runtime_error( "failed to update attribute" );
         }
         if ( ! instance->updateEntry( entry ) )
         {
            throw std::runtime_error( "failed to update entry" );
         }
         std::cout << "Updated attribute " << m_Pos << " of entry #" <<
            entry.getIdAsHexString() << "." << std::endl;
         break;
      }
      case ADD_PASSWORD:
      {
         Entry entry( instance->findEntry( m_Id ) );

         utils::Reader reader( 1024 );
         String label( reader.readLine( "Label: " ) );
         label = utils::strip( label );
         checkInput( label, "empty label" );
         String password( reader.readLine( "Password: " ) );
         password = utils::strip( password );
         checkInput( password, "empty password or phrase" );
         password = preProcessPassword( password );

         if ( ! entry.addLabeledData( label, Data( password ) ) )
         {
            throw std::runtime_error( "failed to add password" );
         }
         if ( ! instance->updateEntry( entry ) )
         {
            throw std::runtime_error( "failed to update entry" );
         }
         std::cout << "Added password to entry #" <<
            entry.getIdAsHexString() << "." << std::endl;
         break;
      }
      case ADD_KEY:
      {
         Entry entry( instance->findEntry( m_Id ) );

         utils::Reader reader( 1024 );
         String label( reader.readLine( "Label: " ) );
         label = utils::strip( label );
         checkInput( label, "empty label" );
         const Vector<uint8_t> data( askForInputFileAndRead() );

         if ( ! entry.addLabeledData( label, Data( data ) ) )
         {
            throw std::runtime_error( "failed to add key" );
         }
         if ( ! instance->updateEntry( entry ) )
         {
            throw std::runtime_error( "failed to update entry" );
         }
         std::cout << "Added key to entry #" <<
            entry.getIdAsHexString() << "." << std::endl;
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
         if ( labeledDate.second.getType() == DATA_TEXT )
         {
            std::cout << "Deleted password " << m_Pos << " from entry #"
               << entry.getIdAsHexString() << "." << std::endl;
         }
         else
         {
            std::cout << "Deleted key " << m_Pos << " from entry #" <<
               entry.getIdAsHexString() << "." << std::endl;
         }
         break;
      }
      case UPDATE_PASSWORD_OR_KEY:
      {
         Entry entry( instance->findEntry( m_Id ) );
         const Vector<std::pair<String,Data>> labeledData( toSortedVector( entry.getLabeledData() ) );
         const std::pair<String,Data> labeledDate( getElemAtPos( labeledData, m_Pos ) );

         utils::Reader reader( 1024 );
         StringStream labelPrompt;
         labelPrompt << "Label (" << labeledDate.first << "): ";
         String label( reader.readLine( labelPrompt.str() ) );
         label = utils::strip( label );

         if ( labeledDate.second.getType() == DATA_TEXT )
         {
            StringStream passwordPrompt;
            if ( labeledDate.second.isPlaintextAvailable() )
            {
               passwordPrompt << "Password (" << labeledDate.second.getPlaintext<String>() << "): ";
            }
            else
            {
               passwordPrompt << "Password (***): ";
            }
            String password( reader.readLine( passwordPrompt.str() ) );
            password = utils::strip( password );
            password = preProcessPassword( password );

            if ( ( label.empty() || label == labeledDate.first ) &&
                 ( password.empty() || password == labeledDate.second.getPlaintext<String>() )
               )
            {
               std::cout << "No changes." << std::endl;
               break;
            }

            if ( ! entry.updateLabeledData(
                     labeledDate.first,
                     label.empty() ? labeledDate.first : label,
                     password.empty() ? labeledDate.second : Data( password )
                     )
               )
            {
               throw std::runtime_error( "failed to update password" );
            }
         }
         else
         {
            const Vector<uint8_t> data( askForInputFileAndRead( &( labeledDate.second ) ) );

            if ( ( label.empty() || label == labeledDate.first ) &&
                 ( data.empty() || data == labeledDate.second.getPlaintext<Vector<uint8_t>>() )
               )
            {
               std::cout << "No changes." << std::endl;
               break;
            }

            if ( ! entry.updateLabeledData(
                     labeledDate.first,
                     label.empty() ? labeledDate.first : label,
                     data.empty() ? labeledDate.second : Data( data )
                     )
               )
            {
               throw std::runtime_error( "failed to update key" );
            }
         }

         if ( ! instance->updateEntry( entry ) )
         {
            throw std::runtime_error( "failed to update entry" );
         }

         if ( labeledDate.second.getType() == DATA_TEXT )
         {
            std::cout << "Updated password " << m_Pos << " of entry #" <<
               entry.getIdAsHexString() << "." << std::endl;
         }
         else
         {
            std::cout << "Updated key " << m_Pos << " of entry #" <<
               entry.getIdAsHexString() << "." << std::endl;
         }

         break;
      }
      case EXPORT_PASSWORD_OR_KEY:
      {
         Entry entry( instance->findEntry( m_Id ) );
         Vector<std::pair<String,Data>> labeledData( toSortedVector( entry.getLabeledData() ) );
         std::pair<String,Data> labeledDate( getElemAtPos( labeledData, m_Pos ) );

         if ( labeledDate.second.getType() == DATA_TEXT )
         {
            decryptData( instance, labeledDate.second );
            utils::xselect( labeledDate.second.getPlaintext<String>() );
            std::cout << "Exported password " << m_Pos << " of entry #" <<
               entry.getIdAsHexString() << "." << std::endl;
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

            decryptData( instance, labeledDate.second );
            Vector<uint8_t> key( labeledDate.second.getPlaintext<Vector<uint8_t>>() );
            if ( ! key.empty() )
            {
               file.write( reinterpret_cast<char*>( key.data() ), key.size() );
            }
            std::cout << "Exported key " << m_Pos << " of entry #" <<
               entry.getIdAsHexString() << "." << std::endl;
         }

         break;
      }
      case ADD_TAG:
      {
         Entry entry( instance->findEntry( m_Id ) );

         utils::Reader reader( 1024 );
         String tag( reader.readLine( "Tag: " ) );
         tag = utils::strip( tag );
         checkInput( tag, "empty tag" );
         tag = preProcessTag( instance, tag );
         if ( ! entry.addTag( tag ) )
         {
            throw std::runtime_error( "tag already assigned" );
         }
         if ( ! instance->updateEntry( entry ) )
         {
            throw std::runtime_error( "failed to update entry" );
         }
         std::cout << "Added tag to entry #" <<
            entry.getIdAsHexString() << "." << std::endl;
         break;
      }
      case UPDATE_TAG:
      {
         Entry entry( instance->findEntry( m_Id ) );

         String tag( getTagAtPos( setToSortedVector( instance->getTags() ), m_Pos ) );

         if ( ! entry.hasTag( tag ) )
         {
            throw std::runtime_error( "tag not assigned to entry" );
         }

         utils::Reader reader( 1024 );
         StringStream s;
         s << "Tag (" << tag << "): ";
         String newTag( reader.readLine( s.str() ) );
         newTag = utils::strip( newTag );
         newTag = preProcessTag( instance, newTag );

         // Change tag of all affected entries!!!
         if ( newTag.empty() || newTag == tag )
         {
            std::cout << "No changes." << std::endl;
            break;
         }

         Set<String> filter;
         filter.insert( tag );
         Set<Entry> entries( instance->getEntries( filter ) );
         for ( auto& entry : entries )
         {
            if ( ! const_cast<Entry&>( entry ).deleteTag( tag ) )
            {
               throw std::runtime_error( "failed to delete old tag" );
            }
            if ( ! entry.hasTag( newTag ) )
            {
               if ( ! const_cast<Entry&>( entry ).addTag( newTag ) )
               {
                  throw std::runtime_error( "failed to add new tag" );
               }
            }
            if ( ! instance->updateEntry( entry ) )
            {
               throw std::runtime_error( "failed to update entry" );
            }
         }
         std::cout << "Updated tag of entry #" <<
            entry.getIdAsHexString() << " and " << ( entries.size() - 1 ) << " others." << std::endl;

         break;
      }
      case DELETE_TAG:
      {
         Entry entry( instance->findEntry( m_Id ) );

         String tag( getTagAtPos( setToSortedVector( instance->getTags() ), m_Pos ) );

         if ( ! entry.hasTag( tag ) )
         {
            throw std::runtime_error( "tag not assigned to entry" );
         }
         if ( ! entry.deleteTag( tag ) )
         {
            throw std::runtime_error( "failed to delete tag" );
         }
         if ( ! instance->updateEntry( entry ) )
         {
            throw std::runtime_error( "failed to update entry" );
         }
         std::cout << "Deleted tag from entry #" <<
            entry.getIdAsHexString() << "." << std::endl;
         break;
      }
      default:
      {
         throw std::runtime_error( "command not implemented yet" );
      }
   }
}

const Vector<uint8_t> EntryTask::askForInputFileAndRead( const Data* date )
{
   Vector<uint8_t> data;
   utils::TeclaReader reader( 1024 );
   reader.addCompletion( cpl_file_completions, nullptr );

   StringStream filePrompt;
   filePrompt << "File";
   if ( date )
   {
      if ( date->isPlaintextAvailable() )
      {
         filePrompt << " (" << date->getPlaintext<Vector<uint8_t>>().size() << "B)";
      }
      else
      {
         filePrompt << " (***)";
      }
   }
   filePrompt << ": ";

   String path( reader.readLine( filePrompt.str() ) );
   path = utils::strip( path );

   if ( path.empty() )
   {
      if ( ! date )
      {
         throw std::runtime_error( "missing filename" );
      }
      else
      {
         return data;
      }
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
      checkInput( password, "empty password or phrase" );
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
      checkInput( password, "empty password or phrase" );
      instance->decryptData( data, password );
   }
}

void EntryTask::checkInput( const String& input, const String& message )
{
   if ( input.empty() )
   {
      throw std::runtime_error( message.c_str() );
   }
}

} }
