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


#include <iostream>
#include "sesame/Data.hpp"


namespace sesame
{
   Data::Data() :
      m_Type( TEXT ),
      m_PlaintextAvailable( false ),
      m_Dirty( true )
   {
   }

   // private
   Data::Data(
      const Type type,
      const Vector<uint8_t>& ciphertext,
      const Vector<uint8_t>& hmac
      ) :
      m_Type( type ),
      m_Ciphertext( ciphertext ),
      m_Hmac( hmac ),
      m_PlaintextAvailable( false ),
      m_Dirty( false )
   {
   }

   Data::Data( const String& plaintext ) :
      m_Type( TEXT ),
      m_Plaintext(
         reinterpret_cast<const char*>( &( plaintext.front() ) ),
         reinterpret_cast<const char*>( &( plaintext.back() ) ) + 1
         ),
      m_PlaintextAvailable( true ),
      m_Dirty( true )
   {
   }

   Data::Data( const Vector<uint8_t>& plaintext ) :
      m_Type( BINARY ),
      m_Plaintext( plaintext ),
      m_PlaintextAvailable( true ),
      m_Dirty( true )
   {
   }

   Data::Type Data::getType() const
   {
      return m_Type;
   }

   void Data::setPlaintext( const String& plaintext )
   {
      // Text is stored without trailing zero.
      m_Type = TEXT;
      m_Plaintext = Vector<uint8_t>(
         reinterpret_cast<const uint8_t*>( plaintext.c_str() ),
         reinterpret_cast<const uint8_t*>( plaintext.c_str() ) + plaintext.size()
         );
      m_Ciphertext.resize( 0 );
      m_Hmac.clear();
      m_PlaintextAvailable = true;
      m_Dirty = true;
   }

   void Data::setPlaintext( const Vector<uint8_t>& plaintext )
   {
      m_Type = BINARY;
      m_Plaintext = plaintext;
      m_Ciphertext.resize( 0 );
      m_Hmac.clear();
      m_PlaintextAvailable = true;
      m_Dirty = true;
   }

   template <>
   Vector<uint8_t> Data::getPlaintext<Vector<uint8_t> >() const
   {
      return m_Plaintext;
   }

   template <>
   String Data::getPlaintext<String>() const
   {
      // Stop at first zero byte.
      // Implementation here is not very effective but its OK
      // as only small content size is expected.

      String tmp(
         reinterpret_cast<const char*>( &( m_Plaintext.front() ) ),
         reinterpret_cast<const char*>( &( m_Plaintext.back() ) ) + 1
         );

      return String( tmp.c_str() );
   }

   bool Data::isPlaintextAvailable() const
   {
      return m_PlaintextAvailable;
   }

   bool Data::isDirty() const
   {
      return m_Dirty;
   }

   void Data::clear()
   {
      m_Ciphertext.resize( 0 );
      m_Hmac.clear();
      m_Dirty = true;
   }
}
