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


#include <cerrno>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "sesame/utils/filesystem.hpp"

namespace sesame { namespace utils {

bool exists( const String& path )
{
   struct stat buf;
   return ( lstat( path.c_str(), &buf ) != -1 );
}

bool isFile( const String& path )
{
   bool yes( true );
   struct stat buf;
   if ( lstat( path.c_str(), &buf ) != -1 )
   {
      yes = ( S_ISREG( buf.st_mode ) > 0 );
   }
   else
   {
      if ( errno != ENOENT )
      {
         throw std::runtime_error( "failed to stat file" );
      }
      else
      {
         yes = false;
      }
   }

   return yes;
}

std::size_t getFileSize( const String& path )
{
   if ( isFile( path ) )
   {
      struct stat buf;
      if ( lstat( path.c_str(), &buf ) == -1 )
      {
         throw std::runtime_error( "failed to stat file" );
      }
      return buf.st_size;
   }
   else
   {
      StringStream s;
      s << path << " is no file";
      throw std::runtime_error( s.str().c_str() );
   }
}

bool removeFile( const String& path )
{
   if ( isFile( path ) )
   {
      return ( unlink( path.c_str() ) != -1 );
   }
   else
   {
      StringStream s;
      s << path << " is no file";
      throw std::runtime_error( s.str().c_str() );
   }
}

const String getExtension( const String& path, const String& delimiter )
{
   auto index( path.find_last_of( "." ) );
   auto indexDelim( path.find_last_of( delimiter ) );

   if ( index != String::npos && indexDelim != String::npos && indexDelim > index )
   {
      index = String::npos;
   }

   if ( index != String::npos && ( index + 1 ) < path.size() )
   {
      return path.substr( index + 1 );
   }
   else
   {
      return "";
   }
}

const String incrementFileName( const String& fileNameIn, const String& delimiter )
{
   auto index( fileNameIn.find_last_of( "." ) );
   auto indexDelim( fileNameIn.find_last_of( delimiter ) );

   if ( index != String::npos && indexDelim != String::npos && indexDelim > index )
   {
      index = String::npos;
   }

   String fileNameOut;
   uint32_t count( 1 );
   do
   {
      StringStream s;

      if ( index == String::npos )
      {
         s << fileNameIn << "." << count++;
      }
      else
      {
         s << fileNameIn.substr( 0, index );
         s << "." << count++;
         if ( ( index + 1 ) < fileNameIn.size() )
         {
            s << "." << fileNameIn.substr( index + 1 );
         }
      }

      fileNameOut = s.str();
   }
   while ( exists( fileNameOut ) );

   return fileNameOut;
}

} }
