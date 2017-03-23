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
#include "types.hpp"
#include "sesame/utils/grammar.h"
#include "sesame/utils/Parser.hpp"

extern void* ParseAlloc( void* ( *allocProc )( size_t ) );
extern void Parse( void*, int, char*, struct sesame::utils::ParseResult* );
extern void* ParseFree( void*, void( *freeProc )( void* ) );


namespace sesame { namespace utils {

Parser::Parser() :
   m_Lexer(),
   m_Parser( ParseAlloc( &std::malloc ) )
{
   yylex_init( &m_Lexer );
}

Parser::~Parser()
{
   yylex_destroy( m_Lexer );
   ParseFree( m_Parser, &std::free );
}

const ParseResult Parser::parse( const String& line ) const
{
   // Init lexer.
   YY_BUFFER_STATE buffer( yy_scan_string( line.c_str(), m_Lexer ) );
   yy_switch_to_buffer( buffer, m_Lexer );

   // Reset parser.
   ParseResult dummyParseResult;
   Parse( m_Parser, 0, nullptr, &dummyParseResult );

   // Start parsing.
   int token;
   Vector<String> tokens;
   tokens.reserve( 32 );
   ParseResult parseResult;

   do
   {
      token = yylex( m_Lexer );

      if ( token > 0 )
      {
         tokens.push_back( yyget_text( m_Lexer ) );
         Parse( m_Parser, token, const_cast<char*>( tokens.back().c_str() ), &parseResult );
      }
      else
      {
         break;
      }
   }
   while ( tokens.capacity() > tokens.size() );

   // Finish parsing.
   yy_delete_buffer( buffer, m_Lexer );
   Parse( m_Parser, 0, nullptr, &parseResult );

   return parseResult;
}

} }
