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


#ifndef SESAME_DEFINITIONS_HPP
#define SESAME_DEFINITIONS_HPP

#include "msgpack.hpp"

namespace sesame
{
   /**
    * All supported protocols.
    */
   enum Protocol
   {
      PROTOCOL_UNKNOWN,
      /**
       * - scrypt is used as PBKDF to derive a 256 bit key
       * - AES256 (CBC) is used for encryption
       * - HMAC (SHA256) is used for authentication
       */
      PROTOCOL_SCRYPT_AES_CBC_SHA_V1
   };

   /** The possible plaintext data types. */
   enum DataType
   {
      DATA_BINARY,
      DATA_TEXT
   };
}

// Introduce enums to msgpack.
MSGPACK_ADD_ENUM( sesame::Protocol )
MSGPACK_ADD_ENUM( sesame::DataType )

#endif
