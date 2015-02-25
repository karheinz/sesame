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
      return 0;
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
      return false;
   }
}

} }
