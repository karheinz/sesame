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
#include <utility>
#include <vector>

#include <apg.hpp>

#include "types.hpp"
#include "sesame/commands/ApgTask.hpp"
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
   char* args[ m_Tokens.size() + 1 ];
   std::size_t count( 0 );
   for ( auto token : m_Tokens )
   {
      args[ count++ ] = const_cast<char*>( token.c_str() );
   }
   args[ count++ ] = nullptr;

   apgCache.clear();
   apgCache = apg( m_Tokens.size(), args );

   count = 1;
   for ( auto tuple : apgCache )
   {
      std::cout << "#" << count++ << " " << tuple.first;
      if ( ! tuple.second.empty() )
      {
         std::cout << " " << tuple.second;
      }
      std::cout << std::endl;
   }
}

} }
