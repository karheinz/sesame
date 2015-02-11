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
#include <stdexcept>

#include "sesame/Instance.hpp"
#include "sesame/commands/InstanceTask.hpp"
#include "sesame/utils/string.hpp"
#include "sesame/utils/Reader.hpp"

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
         throw std::runtime_error( "open sesame first" );
      }
   }
   else
   {
      if ( instance )
      {
         throw std::runtime_error( "close sesame first" );
      }
   }

   switch ( m_TaskType )
   {
      case NEW:
      {
         utils::Reader reader( 1024 );

         String choice;
         choice = reader.readLine( "Should all data be decrypted when opening sesame? [y/N]  " );
         choice = utils::strip( utils::toUtf8( choice ) );

         bool decryptAllByDefault;
         if ( choice.empty() )
         {
         }
         else if ( choice == u8"y" || choice == u8"Y" )
         {
            decryptAllByDefault = true;
         }
         else if ( choice == u8"n" || choice == u8"N" )
         {
            decryptAllByDefault = false;
         }
         else
         {
            throw std::runtime_error( "invalid choice" );
         }

         std::cout << "How much memory should be used for password based key derivation:" << std::endl;
         choice = reader.readLine( "[1] 128MiB, [2] 256MiB, [3] 512MiB or [4] 1024MiB?  " );
         choice = utils::strip( utils::toUtf8( choice ) );

         uint32_t ldN;
         if ( choice == u8"1" )
         {
            ldN = 17;
         }
         else if ( choice == u8"2" )
         {
            ldN = 18;
         }
         else if ( choice == u8"3" )
         {
            ldN = 19;
         }
         else if ( choice == u8"4" )
         {
            ldN = 20;
         }
         else
         {
            throw std::runtime_error( "invalid choice" );
         }

         Map<String,Vector<uint8_t>> params;
         {
            Vector<uint8_t> ldNVector;
            packV( ldNVector, ldN );
            params[ utils::fromUtf8( u8"ldN" ) ] = ldNVector;
         }


         instance.reset( new Instance( PROTOCOL_SCRYPT_AES_CBC_SHA_V1, params, decryptAllByDefault ) );
         break;
      }
      case OPEN:
      {
         // open and parse first
         std::ifstream file( m_Path.c_str(), std::ios_base::in | std::ios_base::binary );
         if ( ! file.good() )
         {
            throw std::runtime_error( "failed to open sesame" );
         }

         Instance::parse( file );

         // then try to construct
         utils::Reader reader( 1024 );
         String password( reader.readLine( "password or phrase: ", true ) );
         password = utils::strip( password );

         file.clear();
         file.seekg( 0, std::ios_base::beg );
         instance.reset( new Instance( file, password ) );

         break;
      }
      case WRITE:
      {
         {
            std::ifstream check( m_Path.c_str() );
            if ( check.good() )
            {
               utils::Reader reader( 1024 );
               String choice( reader.readLine( "Overwrite existing file? [y/N]  " ) );
               choice = utils::strip( utils::toUtf8( choice ) );
               if ( choice != u8"y" && choice != u8"Y" )
               {
                  break;
               }
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
               throw std::runtime_error( "confirmation failed" );
            }
         }

         password = utils::strip( password );
         instance->write( file, password );
         instance->recalcInitialDigest();
         break;
      }
      case CLOSE:
      {
         if ( instance && instance->isDirty() )
         {
            utils::Reader reader( 1024 );
            String choice( reader.readLine( "Sesame was modified, quit anyway?  [y/N]  " ) );
            choice = utils::strip( utils::toUtf8( choice ) );
            if ( choice != u8"y" && choice != u8"Y" )
            {
               break;
            }
         }
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
