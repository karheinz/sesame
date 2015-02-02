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


#ifndef ALLOCATOR_HPP
#define ALLOCATOR_HPP

#include <cstdint>
#include <cstring>
//#include <iostream>
#include <memory>


template <typename T>
struct Allocator {
   template <class U>
   struct rebind
   {
      typedef Allocator<U> other;
   };

   typedef std::size_t size_type;
   typedef T value_type;
   typedef value_type* pointer;
   typedef const value_type* const_pointer;
   typedef std::ptrdiff_t difference_type;
   typedef value_type& reference;
   typedef const value_type& const_reference;

   Allocator() = default;

   template <class U>
   Allocator( const Allocator<U>& )
   {
   }

   virtual ~Allocator() = default;

   pointer address( reference r ) const
   {
      return std::addressof( r );
   }

   const_pointer address( const_reference r ) const
   {
      return std::addressof( r );
   }

   pointer allocate( std::size_t n )
   {
      //std::cout << "allocate " << std::dec << n * sizeof( value_type ) <<
      //   " bytes" << std::endl;

      return reinterpret_cast<pointer>( new uint8_t[ n * sizeof( value_type ) ] );
   }

   void deallocate( pointer p, std::size_t n )
   {
      //std::cout << "deallocate " << std::dec << n * sizeof( value_type ) <<
      //   " bytes" << std::endl;

      std::size_t bytes( n * sizeof( value_type ) );
      volatile uint8_t* _p( reinterpret_cast<uint8_t*>( p ) );
      while ( bytes-- )
      {
         *( _p++ ) = 0;
      }

      delete[] reinterpret_cast<uint8_t*>( p );
   }
};

template <typename T, typename U>
bool operator == (const Allocator<T>&, const Allocator<U>&)
{
  return true;
}

template <typename T, typename U>
bool operator != (const Allocator<T>& a, const Allocator<U>& b)
{
  return ! (a == b);
}

#endif
