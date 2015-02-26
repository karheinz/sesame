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


#include <algorithm>
#include <iostream>
#include <memory>
#include "types.hpp"
#include "sesame/Data.hpp"
#include "sesame/Entry.hpp"
#include "sesame/Instance.hpp"
#include "sesame/utils/completion.hpp"
#include "sesame/utils/string.hpp"
#include "sesame/utils/Parser.hpp"
#include "sesame/utils/ParseResult.hpp"

using sesame::Data;
using sesame::Entry;
using sesame::Instance;
using sesame::utils::lstrip;
using sesame::utils::split;
using sesame::utils::Parser;
using sesame::utils::ParseResult;

namespace
{
   Parser parser;
   ParseResult parseResult;
   char* emptyCString( 0 );

   const Vector<String> editModes = { "emacs", "vi" };
   const Vector<String> baseCommands = { "help", "clear", "quit", "edit-mode " };
   const Vector<String> noInstanceCommands = { "new", "open " };
   const Vector<String> instanceCommands = { "close", "write ", "list", "show ", "decrypt ",
                                             "add ", "delete ", "update ", "select " };
   const Vector<String> updateCommands = { "add_attribute", "update_attribute ", "delete_attribute ",
                                           "add_password", "add_key", "update_password_or_key ", "delete_password_or_key " };
   const Vector<String> selectCommands = { "export_key " };

   int cpl_add_completions(
      WordCompletion* cpl,
      const char* line,
      int word_end,
      Vector<String> choices,
      String part
      )
   {
      int success( 0 );
      String::size_type hit;
      for ( auto& choice : choices )
      {
         hit = String::npos;
         hit = choice.find( part );
         if ( hit != 0 )
         {
            continue;
         }
         success += cpl_add_completion(
               cpl,
               line,
               ( word_end - part.size() ),
               word_end,
               ( choice.substr( part.size() ) ).c_str(),
               emptyCString,
               emptyCString
               );
      }

      return ( success == 0 );
   }

   template <class T>
   struct ElemSorter
   {
      bool operator()( const std::pair<String,T>& a, const std::pair<String,T>& b )
      {
         return ( a.first < b.first );
      }
   };

   template <class T>
   const Vector<std::pair<String,T>> toSortedVector( const Map<String,T>& m )
   {
      Vector<std::pair<String,T>> v( m.begin(), m.end() );
      std::sort( v.begin(), v.end(), ElemSorter<T>() );
      return v;
   }
}

CPL_MATCH_FN(cpl_complete_sesame)
{
   Instance* instance( 0 );

   if ( data )
   {
      instance = reinterpret_cast<std::shared_ptr<Instance>* >( data )->get();
   }

   std::pair<String,String> splitted( split( line ) );
   String left( splitted.first );
   left = lstrip( left );
   String right( splitted.second );

   // Look for completion.
   parseResult = parser.parse( left );
   // Reset lexer for next completion.
   parser.parse( "\n" );

   // Invalid input, abort.
   if ( ! left.empty() && ! parseResult.isValid() )
   {
      return 0;
   }

   // Evaluate parse result.
   if ( left.empty() || parseResult.completeCommand() )
   {
      Vector<String> commands( baseCommands );
      if ( instance )
      {
         commands.insert( commands.end(), instanceCommands.begin(), instanceCommands.end() );
      }
      else
      {
         commands.insert( commands.end(), noInstanceCommands.begin(), noInstanceCommands.end() );
      }

      cpl_add_completions( cpl, line, word_end, commands, left );
   }
   else if ( parseResult.completeEditMode() )
   {
      cpl_add_completions( cpl, line, word_end, editModes, right );
   }
   else if ( parseResult.completeSpace() )
   {
      cpl_add_completion( cpl, line, word_end, word_end, " ", emptyCString, emptyCString );
   }
   else if ( parseResult.completeEntry() && instance )
   {
      Set<Entry> entries( instance->findEntries( right ) );
      Vector<String> choices;
      for ( auto& entry : entries )
      {
         choices.push_back( String( "#" ) + entry.getIdAsHexString() + " " );
      }

      cpl_add_completions( cpl, line, word_end, choices, right );
   }
   else if ( parseResult.completeUpdateCommand() && instance )
   {
      cpl_add_completions( cpl, line, word_end, updateCommands, right );
   }
   else if ( parseResult.completeSelectCommand() && instance )
   {
      cpl_add_completions( cpl, line, word_end, selectCommands, right );
   }
   else if ( parseResult.completeAttribute() && instance )
   {
      try
      {
         Entry entry( instance->findEntry( parseResult.getEntryId() ) );
         Map<String,String> attributes( entry.getAttributes() );
         Vector<String> choices;
         for ( std::size_t i = 1; i <= attributes.size(); ++i )
         {
            StringStream s;
            s << "#" << i;
            choices.push_back( s.str() );
         }
         cpl_add_completions( cpl, line, word_end, choices, right );
      }
      catch ( std::runtime_error& )
      {
         // entry not found
      }
   }
   else if ( parseResult.completePasswordOrKey() && instance )
   {
      try
      {
         Entry entry( instance->findEntry( parseResult.getEntryId() ) );
         Map<String,Data> labeledData( entry.getLabeledData() );
         Vector<String> choices;
         for ( std::size_t i = 1; i <= labeledData.size(); ++i )
         {
            StringStream s;
            s << "#" << i;
            choices.push_back( s.str() );
         }
         cpl_add_completions( cpl, line, word_end, choices, right );
      }
      catch ( std::runtime_error& )
      {
         // entry not found
      }
   }
   else if ( parseResult.completeKey() && instance )
   {
      try
      {
         Entry entry( instance->findEntry( parseResult.getEntryId() ) );
         const Vector<std::pair<String,Data>> labeledData( toSortedVector( entry.getLabeledData() ) );
         Vector<String> choices;
         for ( std::size_t i = 1; i <= labeledData.size(); ++i )
         {
            if ( labeledData[ i - 1 ].second.getType() == sesame::Data::BINARY )
            {
               StringStream s;
               s << "#" << i;
               choices.push_back( s.str() );
            }
         }
         cpl_add_completions( cpl, line, word_end, choices, right );
      }
      catch ( std::runtime_error& )
      {
         // entry not found
      }
   }
   else if ( parseResult.completeFile() )
   {
      cpl_file_completions( cpl, nullptr, right.c_str(), right.size() );
   }

   return 0;
}
