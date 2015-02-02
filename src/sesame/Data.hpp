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


#ifndef SESAME_DATA_HPP
#define SESAME_DATA_HPP

#include <iostream>

#include "types.hpp"

#include "msgpack.hpp"
#include "msgpack/type/String.hpp"
#include "msgpack/type/Vector.hpp"

#include "sesame/definitions.hpp"
#include "sesame/packaging.hpp"


namespace sesame
{
   // Forward declaration.
   class Instance;

   /**
    * Class for data, either binary or text.
    */
   class Data
   {
      public:
         /** The possible plaintext data types. */
         enum Type
         {
            BINARY,
            TEXT
         };

         /**
          * Default constructor, required for usage in a map.
          */
         Data();

         /**
          * Copy constructor.
          *
          * @param other the other data
          */
         Data( const Data& other ) = default;

         /**
          * Constructor to use with plaintext of text type.
          *
          * @param plaintext the plaintext
          */
         Data( const String& plaintext );

         /**
          * Constructor to use with plaintext of binary type.
          *
          * @param protocol the algorithms used for encryption
          * @param plaintext the plaintext
          */
         Data( const Vector<uint8_t>& plaintext );

         /**
          * Assignment operator.
          *
          * @param other the other data
          *
          * @return reference to self
          */
         Data& operator=( const Data& other ) = default;

         /** Destructor. */
         virtual ~Data() = default;

         /**
          * Returns the type of the data.
          *
          * @return the type of the data
          */
         Type getType() const;

         /**
          * Sets a new plaintext.
          *
          * Data instance is marked as dirty.
          *
          * @param plaintext the plaintext
          */
         void setPlaintext( const Vector<uint8_t>& plaintext );

         /**
          * Sets a new plaintext.
          *
          * Data instance is marked as dirty.
          *
          * @param plaintext the plaintext
          */
         void setPlaintext( const String& plaintext );

         /**
          * Dummy template to retrieve template.
          *
          * Use <tt>String</tt> or <tt>Vector<uint8_t></tt>
          * specialization to retrieve plaintext.
          * Might be empty if original data wasn't decrypted yet
          * and <tt>setPlaintext()</tt> wasn't called.
          *
          * @return empty instance of type <tt>T</tt>
          */
         template <typename T>
         T getPlaintext() const
         {
            return T();
         }

         /**
          * Returns <tt>true</tt> if plaintext is available
          *
          * @return <tt>true</tt> if plaintext is available
          */
         bool isPlaintextAvailable() const;

         /**
          * Returns <tt>true</tt> if data is dirty,
          * meaning no (re)encryption was done.
          *
          * @return <tt>true</tt> if data is dirty
          */
         bool isDirty() const;

         /**
          * Clears ciphertext and HMAC, sets dirty flag.
          * So data can be used in other contexts.
          */
         void clear();

      private:
         /**
          * Constructor to use if plaintext isn't available.
          *
          * @param protocol the algorithms used for encryption
          * @param type the type of the plaintext
          * @param ciphertext the ciphertext
          * @param hmac the HMAC about the ciphertext
          */
         Data(
            const Type type,
            const Vector<uint8_t>& ciphertext,
            const Vector<uint8_t>& hmac
            );


         /** the type of the plaintext, binary or text */
         Type m_Type;
         /** the plaintext */
         Vector<uint8_t> m_Plaintext;
         /** the ciphertext */
         Vector<uint8_t> m_Ciphertext;
         /** the HMAC over the ciphertext */
         Vector<uint8_t> m_Hmac;
         /** availability flag for plaintext */
         bool m_PlaintextAvailable;
         /** dirty flag */
         bool m_Dirty;

      // (de)serialization
      public:
         MSGPACK_DEFINE( m_Type, m_Ciphertext, m_Hmac )

      /**
       * Cryptograpic protocol is enforced by an <tt>Instance</tt>.
       * So instances are allowed to change internal state of <tt>Data</tt>.
       */
      friend class Instance;
   };

   /**
    * Returns the plaintext as vector of bytes.
    *
    * @return plaintext as vector of bytes
    */
   template <>
   Vector<uint8_t> Data::getPlaintext<Vector<uint8_t> >() const;

   /**
    * Returns the plaintext as string.
    *
    * @return the plaintext as string
    */
   template <>
   String Data::getPlaintext<String>() const;
}

// Introduce enum to msgpack.
MSGPACK_ADD_ENUM( sesame::Data::Type )

/**
 * For easy serialization.
 *
 * @param o the stream to write to
 * @param d the data to write
 *
 * @return the stream
 */
inline std::ostream& operator<<( std::ostream& o, const sesame::Data& d )
{
   return sesame::pack( o, d );
}

/**
 * For easy deserialization.
 *
 * @param i the stream to read from
 * @param d the data to write
 *
 * @return the stream
 */
inline std::istream& operator>>( std::istream& i, sesame::Data& d )
{
   return sesame::unpack( i, d );
}

#endif
