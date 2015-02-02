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


#ifndef SESAME_CRYPTO_MACHINE_FACTORY_HPP
#define SESAME_CRYPTO_MACHINE_FACTORY_HPP

#include <memory>

#include "sesame/definitions.hpp"
#include "sesame/crypto/IMachine.hpp"


namespace sesame { namespace crypto {

class MachineFactory
{
   public:
      /**
       * Returns a crypto machine implementing the specified protocol.
       *
       * @param protocol the protocol to be implemented by the machine
       *
       * @return a crypto machine instance
       */
      static std::shared_ptr<IMachine> buildMachine( const Protocol protocol );

   private:
      /**
       * Default constructor, not allowed.
       */
      MachineFactory();

      /**
       * Copy constructor, not allowed.
       */
      MachineFactory( const MachineFactory& );

      /**
       * Assignement operator, not allowed.
       */
      MachineFactory& operator=( const MachineFactory& );
};

} }

#endif
