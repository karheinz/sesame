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


#ifndef SESAME_PACKAGING_HPP
#define SESAME_PACKAGING_HPP

#include <iostream>
#include <stdexcept>

#include "types.hpp"
#include "msgpack.hpp"


namespace sesame {

/**
 * For easy serialization.
 *
 * @param o the stream to write to
 * @param e the element to read from
 *
 * @return the stream
 */
template <typename T>
std::ostream& pack( std::ostream& o, const T& e )
{
   msgpack::pack( o, e );
   return o;
}

/**
 * For easy deserialization.
 *
 * @param i the stream to read from
 * @param e the element to write to
 *
 * @throw std::runtime_error if unpacking failed
 *
 * @return the stream
 */
template <typename T>
std::istream& unpack( std::istream& i, T& e )
{
   Vector<char> buffer( 128, 0 );
   Vector<char> data;
   data.reserve( buffer.size() );
   bool success( false );

   while ( i.good() )
   {
      std::size_t offset( 0 );
      msgpack::unpacked result;

      i.read( buffer.data(), buffer.size() );
      data.insert( data.end(), buffer.data(), buffer.data() + i.gcount() );

      try
      {
         msgpack::unpack( &result, data.data(), data.size(), &offset );
         msgpack::object o( result.get() );
         e = o.as<T>();

         // Some data left to process? Rewind stream buffer.
         if ( data.size() > offset )
         {
            if ( i.eof() )
            {
               i.clear();
            }
            i.seekg( static_cast<int64_t>( offset ) - data.size(), std::ios_base::cur );
         }

         success = true;
         break;
      }
      catch ( msgpack::unpack_error& )
      {
         // Mmmhh, go on and read more data from stream ...
      }
      catch ( std::bad_cast& )
      {
         // Reconstruction failed :(
         break;
      }
   }

   if ( ! success )
   {
      throw std::runtime_error( "unpacking failed" );
   }

   return i;
}

/**
 * Reads content from stream into vector.
 *
 * @param i the stream to read from
 * @param v the vector to write to
 */
inline void readIntoVector( std::istream& i, Vector<char>& v )
{
   i.seekg( 0, std::ios_base::end );
   std::streamsize size( i.tellg() );
   i.seekg( 0, std::ios_base::beg );

   v.resize( size );
   i.read( v.data(), size );
}

/**
 * Reads content from stream into vector.
 *
 * @param i the stream to read from
 * @param v the vector to write to
 */
inline void readIntoVector( std::istream& i, Vector<uint8_t>& v )
{
   i.seekg( 0, std::ios_base::end );
   std::streamsize size( i.tellg() );
   i.seekg( 0, std::ios_base::beg );

   v.resize( size );
   i.read( reinterpret_cast<char*>( v.data() ), size );
}

/**
 * For easy serialization.
 *
 * @param v the vector to write to
 * @param e the element to read from
 */
template <typename T>
inline void packV( Vector<uint8_t>& v, const T& e )
{
   StringStream stream;
   pack( stream, e );

   readIntoVector( stream, v );
}

/**
 * For easy deserialization.
 *
 * @param v the vector to read from
 * @param e the element to write to
 */
template <typename T>
inline void unpackV( const Vector<uint8_t>& v, T& e )
{
   StringStream stream;
   stream.write( reinterpret_cast<const char*>( v.data() ), v.size() );

   unpack( stream, e );
}

}

#endif
