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


#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

#include <apg.hpp>

#include "types.hpp"
#include "sesame/commands/ApgTask.hpp"
#include "sesame/utils/lines.hpp"
#include "sesame/utils/string.hpp"

extern std::vector<std::pair<std::string,std::string>> apgCache;


namespace sesame { namespace commands {

ApgTask::ApgTask( const Vector<String>& tokens ) :
   ICommand(),
   m_Tokens( tokens )
{
}

void ApgTask::run( std::shared_ptr<Instance>& instance )
{
   if ( ! instance )
   {
      throw std::runtime_error( "no instance open" );
   }

   Vector<char*> args( m_Tokens.size() + 1, nullptr );
   std::size_t count( 0 );
   for ( auto& token : m_Tokens )
   {
      args[ count++ ] = const_cast<char*>( token.c_str() );
   }

   apgCache.clear();
   apgCache = apg( m_Tokens.size(), args.data() );

   if ( ! apgCache.empty() )
   {
      std::size_t maxLength( 0 );
      for ( auto& tuple : apgCache )
      {
         maxLength = std::max( maxLength, tuple.first.size() );
      }

      std::cout << "Generated passwords:" << std::endl;
      count = 1;
      for ( auto& tuple : apgCache )
      {
         if ( count < apgCache.size() )
         {
            std::cout << utils::branch();
         }
         else
         {
            std::cout << utils::corner();
         }

         std::cout << "[#" << count++ << "] " << std::setw( maxLength ) << tuple.first;
         if ( ! tuple.second.empty() )
         {
            std::cout << tuple.second;
         }
         std::cout << "\n";
      }

      std::cout << "\nYou can reference a password by entering ";
      std::cout << "\nits id in the password dialog." << std::endl;
   }
}

} }
