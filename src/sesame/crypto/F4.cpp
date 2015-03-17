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


#include <stdexcept>

#include "sesame/crypto/F4.hpp"
#include "sesame/crypto/jpegtranf4.h"
#include "sesame/utils/filesystem.hpp"


namespace sesame { namespace crypto {

void F4::embed(
   const String& fileNameIn,
   const String& fileNameOut,
   const Vector<char>& data
   )
   const
{
   if ( data.size() == 0 )
   {
      throw std::runtime_error( "nothing to embed" );
   }

   if ( f4_embed( fileNameIn.c_str(), fileNameOut.c_str(), data.data(), data.size() ) != 0 )
   {
      if ( utils::isFile( fileNameOut ) )
      {
         utils::removeFile( fileNameOut );
      }

      throw std::runtime_error( "failed to embed data" );
   }
}

void F4::extract(
   const String& fileNameIn,
   Vector<char>& data
   )
   const
{
   // We expect max 20% of the image as data.
   data.resize( utils::getFileSize( fileNameIn ) / 5 );

   if ( data.size() == 0 )
   {
      throw std::runtime_error( "failed to extract data" );
   }

   if ( f4_extract( fileNameIn.c_str(), data.data(), data.size() ) != 0 )
   {
      throw std::runtime_error( "failed to extract data" );
   }
}

const String F4::calcOutFileName(
   const String& fileNameIn,
   const String& delimiter
   )
   const
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
         s << fileNameIn << "." << count++ << ".jpeg";
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
   while ( utils::exists( fileNameOut ) );

   return fileNameOut;
}

} }
