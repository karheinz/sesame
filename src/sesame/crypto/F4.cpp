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

   int rc( f4_embed( fileNameIn.c_str(), fileNameOut.c_str(), data.data(), data.size() ) );

   if ( rc != 0 )
   {
      if ( utils::isFile( fileNameOut ) )
      {
         utils::removeFile( fileNameOut );
      }

      switch ( rc )
      {
         case 1:
         case 2:
            throw std::runtime_error( "internal libjpeg-turbo error" );
         case 3:
         {
            StringStream s;
            s << "failed to open " << fileNameIn;
            throw std::runtime_error( s.str().c_str() );
         }
         case 5:
         {
            StringStream s;
            s << "failed to open " << fileNameOut;
            throw std::runtime_error( s.str().c_str() );
         }
         default:
            throw std::runtime_error( "failed to embed data" );
      }
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

   int rc( f4_extract( fileNameIn.c_str(), data.data(), data.size() ) );

   if ( rc != 0 )
   {
      switch ( rc )
      {
         case 1:
            throw std::runtime_error( "internal libjpeg-turbo error" );
         case 2:
         {
            StringStream s;
            s << "failed to open " << fileNameIn;
            throw std::runtime_error( s.str().c_str() );
         }
         default:
            throw std::runtime_error( "failed to extract data" );
      }
   }
}

} }
