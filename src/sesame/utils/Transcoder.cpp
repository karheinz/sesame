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
#include <cstdlib>
#include <string>
#include "sesame/utils/Transcoder.hpp"

namespace
{
   const Vector<char> BOM_UTF8 = { '\xEF', '\xBB', '\xBF' };

   const Vector<Vector<char>> BOMS_UTF16 = {
      { '\xFF', '\xFE' }, /* LE */
      { '\xFE', '\xFF' }  /* BE */
      };
   const Vector<Vector<char>> BOMS_UTF32 = {
      { '\xFF', '\xFE', '\x00', '\x00' }, /* LE */
      { '\x00', '\x00', '\xFE', '\xFF' }  /* BE */
      };
}

namespace sesame { namespace utils {

Transcoder::Transcoder( const String& from, const String& to ) :
   m_To( to + "//TRANSLIT" ),
   m_Descriptor( iconv_open( m_To.c_str(), from.c_str() ) ),
   m_Buffer( 4096 ),
   m_ChunkSize( m_Buffer.size() / 4 )
{
   if ( m_Descriptor == (iconv_t)( -1 ) )
   {
      throw std::runtime_error( "failed to init transcoder" );
   }
}

Transcoder::~Transcoder()
{
   if ( m_Descriptor != (iconv_t)( -1 ) )
   {
      iconv_close( m_Descriptor );
   }
}

String Transcoder::transcode( const char* string, const std::size_t length )
{
   String result;
   std::size_t rc;
   std::size_t in( length );
   std::size_t read;
   std::size_t out( m_Buffer.size() );

   char* i( const_cast<char*>( string ) );
   char* end( const_cast<char*>( string ) + length );
   char* o( m_Buffer.data() );

   // Reset internal state of conversion descriptor.
   iconv( m_Descriptor, nullptr, &in, nullptr, &out );

   uint8_t error( 0 );
   while ( i < end )
   {
      // Set in and out sizes.
      if ( ( i + m_ChunkSize ) <= end )
      {
         in = m_ChunkSize;
      }
      else
      {
         in = end - i;
      }
      read = in;
      out = m_Buffer.size();

      rc = iconv(
            m_Descriptor,
            &i,
            &in,
            &o,
            &out
            );

      switch ( rc )
      {
         case -1:
            // only handle illegal sequence
            if ( errno == EILSEQ )
            {
               throw std::runtime_error( "illegal multibyte sequence detected" );
            }
            ++error;
            break;
         default:
            if ( out < m_Buffer.size() )
            {
               result.insert( result.end(), m_Buffer.data(), m_Buffer.data() + m_Buffer.size() - out );
            }
            if ( error > 0 )
            {
               --error;
            }
      }

      i += ( read - in );

      if ( error > 1 )
      {
         throw std::runtime_error( "error occurred while transcoding string" );
      }
   }

   std::size_t bomSize( getBomSize( result ) );
   return result.substr( bomSize );
}

String Transcoder::transcode( const String& string )
{
   return transcode( string.data(), string.size() );
}

std::size_t Transcoder::getBomSize( const String& data )
{
   Vector<char> tmp;

   if ( data.size() > 4 )
   {
      tmp = Vector<char>( data.data(), data.data() + 4 );
      for ( auto& bom : BOMS_UTF32 )
      {
         if ( bom == tmp )
         {
            return 4;
         }
      }
   }
   if ( data.size() > 3 )
   {
      tmp = Vector<char>( data.data(), data.data() + 3 );
      if ( BOM_UTF8 == tmp )
      {
         return 3;
      }
   }
   if ( data.size() > 2 )
   {
      tmp = Vector<char>( data.data(), data.data() + 2 );
      for ( auto& bom : BOMS_UTF16 )
      {
         if ( bom == tmp )
         {
            return 2;
         }
      }
   }

   return 0;
}

} }
