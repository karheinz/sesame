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
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "sesame/Instance.hpp"
#include "sesame/commands/InstanceTask.hpp"
#include "sesame/crypto/F4.hpp"
#include "sesame/utils/filesystem.hpp"
#include "sesame/utils/string.hpp"
#include "sesame/utils/Reader.hpp"

extern std::vector<std::pair<std::string,std::string>> apgCache;


namespace sesame { namespace commands {

InstanceTask::InstanceTask( const Type taskType, const String& path ) :
   ICommand(),
   m_TaskType( taskType ),
   m_Path( path )
{
}

void InstanceTask::run( std::shared_ptr<Instance>& instance )
{
   if ( m_TaskType == CLOSE || m_TaskType == WRITE )
   {
      if ( ! instance )
      {
         throw std::runtime_error( "open container first" );
      }
   }
   else
   {
      if ( instance )
      {
         throw std::runtime_error( "close container first" );
      }
   }

   switch ( m_TaskType )
   {
      case NEW:
      {
         utils::Reader reader( 1024 );
         String choice;
         uint32_t ldN;

         std::cout << "First you have to specify how much memory should be used for\n";
         std::cout << "derivation of the key used for encryption of the container:" << std::endl;
         choice = reader.readLine( "[1] 512MiB, [2] 1024MiB or [3] 2048MiB?  " );
         choice = utils::strip( utils::toUtf8( choice ) );
         if ( choice == u8"1" ) { ldN = 19; }
         else if ( choice == u8"2" ) { ldN = 20; }
         else if ( choice == u8"3" ) { ldN = 21; }
         else { throw std::runtime_error( "invalid choice" ); }
         Map<String,Vector<uint8_t>> params1;
         {
            Vector<uint8_t> ldNVector;
            packV( ldNVector, ldN );
            params1[ utils::fromUtf8( u8"ldN" ) ] = ldNVector;
         }

         std::cout << "Second you have to specify how much memory should be used for\n";
         std::cout << "derivation of the key used for encryption of the embedded secrets:" << std::endl;
         choice = reader.readLine( "[1] 64MiB, [2] 128MiB or [3] 256MiB?  " );
         choice = utils::strip( utils::toUtf8( choice ) );
         if ( choice == u8"1" ) { ldN = 16; }
         else if ( choice == u8"2" ) { ldN = 17; }
         else if ( choice == u8"3" ) { ldN = 18; }
         else { throw std::runtime_error( "invalid choice" ); }
         Map<String,Vector<uint8_t>> params2;
         {
            Vector<uint8_t> ldNVector;
            packV( ldNVector, ldN );
            params2[ utils::fromUtf8( u8"ldN" ) ] = ldNVector;
         }

         instance.reset( new Instance( PROTOCOL_SCRYPT_AES_CBC_SHA_V1, params1, params2 ) );
         std::cout << "Created new container #" <<
            instance->getIdAsHexString() << "." << std::endl;
         break;
      }
      case OPEN:
      {
         // open and parse first
         if ( ! utils::exists( m_Path ) )
         {
            throw std::runtime_error( "file not found" );
         }
         else if ( ! utils::isFile( m_Path ) )
         {
            StringStream s;
            s << m_Path << " is no file";
            throw std::runtime_error( s.str().c_str() );
         }

         String ext( utils::getExtension( m_Path ) );
         std::transform( ext.begin(), ext.end(), ext.begin(), ::toupper );

         // No jpeg.
         if ( "JPEG" != ext && "JPG" != ext )
         {
            std::ifstream file( m_Path.c_str(), std::ios_base::in | std::ios_base::binary );
            if ( ! file.good() )
            {
               throw std::runtime_error( "failed to open container" );
            }

            Instance::parse( file );

            // then try to construct
            utils::Reader reader( 1024 );
            String password( reader.readLine( "password or phrase: ", true ) );
            password = utils::strip( password );

            if ( password.empty() )
            {
               throw std::runtime_error( "empty password or phrase" );
            }

            file.clear();
            file.seekg( 0, std::ios_base::beg );
            instance.reset( new Instance( file, password ) );
         }
         // Jpeg.
         else
         {
            StringStream stream;
            Vector<char> data;
            crypto::F4 algorithm;
            algorithm.extract( m_Path, data );
            String buf( data.data(), data.size() );
            stream.str( buf );

            Instance::parse( stream );

            // then try to construct
            utils::Reader reader( 1024 );
            String password( reader.readLine( "password or phrase: ", true ) );
            password = utils::strip( password );

            if ( password.empty() )
            {
               throw std::runtime_error( "empty password or phrase" );
            }

            stream.clear();
            stream.seekg( 0, std::ios_base::beg );

            instance.reset( new Instance( stream, password ) );
         }
         std::cout << "Opened container #" << instance->getIdAsHexString() << "." << std::endl;
         break;
      }
      case WRITE:
      {
         String ext( utils::getExtension( m_Path ) );
         std::transform( ext.begin(), ext.end(), ext.begin(), ::toupper );

         // No jpeg.
         if ( "JPEG" != ext && "JPG" != ext )
         {
            bool alreadyExists( utils::exists( m_Path.c_str() ) );
            if ( alreadyExists )
            {
               if ( utils::isFile( m_Path.c_str() ) )
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
                  s << m_Path << " is no file";
                  throw std::runtime_error( s.str().c_str() );
               }
            }

            std::ofstream file(
               m_Path.c_str(),
               std::ios_base::out | std::ios_base::trunc | std::ios_base::binary
               );

            if ( ! file.good() )
            {
               throw std::runtime_error( "failed to open file" );
            }

            utils::Reader reader( 1024 );
            String password( reader.readLine( "password or phrase: ", true ) );
            if ( instance->isNew() )
            {
               String confirmation( reader.readLine( "please confirm: ", true ) );
               if ( password != confirmation )
               {
                  if ( ! alreadyExists )
                  {
                     file.close();
                     utils::removeFile( m_Path );
                  }
                  throw std::runtime_error( "confirmation failed" );
               }
            }

            password = utils::strip( password );
            try
            {
               instance->write( file, password );
               instance->recalcInitialDigest();
               std::cout << "Wrote container #" << instance->getIdAsHexString() <<
                  " to " << m_Path << std::endl;
            }
            catch ( std::runtime_error& )
            {
               if ( ! alreadyExists )
               {
                  file.close();
                  utils::removeFile( m_Path );
               }
               throw;
            }
         }
         // Jpeg.
         else
         {
            if ( ! utils::exists( m_Path.c_str() ) )
            {
               throw std::runtime_error( "file not found" );
            }

            utils::Reader reader( 1024 );
            String password( reader.readLine( "password or phrase: ", true ) );
            if ( instance->isNew() )
            {
               String confirmation( reader.readLine( "please confirm: ", true ) );
               if ( password != confirmation )
               {
                  throw std::runtime_error( "confirmation failed" );
               }
            }

            password = utils::strip( password );
            Vector<char> dump;
            {
               StringStream s;
               instance->write( s, password );
               readIntoVector( s, dump );
            }

            const String fileOut( utils::incrementFileName( m_Path ) );
            crypto::F4 algorithm;
            algorithm.embed( m_Path, fileOut, dump );
            instance->recalcInitialDigest();
            std::cout << "Wrote container #" << instance->getIdAsHexString() <<
               " to " << fileOut << std::endl;
         }
         break;
      }
      case CLOSE:
      {
         apgCache.resize( 0 );

         if ( instance && instance->isDirty() )
         {
            utils::Reader reader( 1024 );
            String choice( reader.readLine( "Container was modified, quit anyway?  [y/N]  " ) );
            choice = utils::strip( utils::toUtf8( choice ) );
            if ( choice != u8"y" && choice != u8"Y" )
            {
               break;
            }
         }
         std::cout << "Closed container #" << instance->getIdAsHexString() << "." << std::endl;
         instance.reset();
         break;
      }
      default:
      {
         throw std::runtime_error( "command not implemented yet" );
      }
   }
}

} }
