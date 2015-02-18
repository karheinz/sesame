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


#include "types.hpp"
#include "sesame/utils/ParseResult.hpp"

namespace sesame { namespace utils {

ParseResult::ParseResult() :
   m_Valid( true ),
   m_Tokens(),
   m_Command(),
   m_CompleteEditMode( false ),
   m_CompleteFile( false ),
   m_CompleteSpace( false ),
   m_CompleteEntry( false ),
   m_CompleteCommand( false ),
   m_CompleteSubCommand( false ),
   m_CompleteAttribute( false ),
   m_CompletePasswordOrKey( false ),
   m_EntryId( "" )
{
}

void ParseResult::invalidate()
{
   m_Valid = false;
}

bool ParseResult::isValid() const
{
   return m_Valid;
}

std::shared_ptr<commands::ICommand> ParseResult::getCommand() const
{
   return m_Command;
}

void ParseResult::setCommand( std::shared_ptr<commands::ICommand> command )
{
   m_Command = command;
}

String ParseResult::getCommandToken() const
{
   return ( m_Tokens.size() > 0 ? m_Tokens[ 0 ] : String() );
}

void ParseResult::addToken( const String& token )
{
   m_Tokens.insert( m_Tokens.begin(), token );
}

Vector<String> ParseResult::getArgumentTokens() const
{
   Vector<String> arguments;

   for ( std::size_t i = 1; i < m_Tokens.size(); ++i )
   {
      arguments.push_back( m_Tokens[ i ] );
   }

   return arguments;
}

void ParseResult::clear()
{
   m_Valid = true;
   m_Tokens.resize( 0 );
   m_Command.reset();
   m_CompleteEditMode = false;
   m_CompleteFile = false;
   m_CompleteSpace = false;
   m_CompleteEntry = false;
   m_CompleteCommand = false;
   m_CompleteSubCommand = false;
   m_CompleteAttribute = false;
   m_CompletePasswordOrKey = false;
   m_EntryId = "";
}

/** User requests completion of edit mode? */
bool ParseResult::completeEditMode()
{
   return m_CompleteEditMode;
}

void ParseResult::setCompleteEditMode()
{
   m_CompleteEditMode = true;
}

void ParseResult::setCompleteFile()
{
   m_CompleteFile = true;
}

bool ParseResult::completeFile()
{
   return m_CompleteFile;
}

void ParseResult::setCompleteSpace()
{
   m_CompleteSpace = true;
}

bool ParseResult::completeSpace()
{
   return m_CompleteSpace;
}

void ParseResult::setCompleteEntry()
{
   m_CompleteEntry = true;
}

bool ParseResult::completeEntry()
{
   return m_CompleteEntry;
}

void ParseResult::setCompleteCommand()
{
   m_CompleteCommand = true;
}

bool ParseResult::completeCommand()
{
   return m_CompleteCommand;
}

void ParseResult::setCompleteSubCommand()
{
   m_CompleteSubCommand = true;
}

bool ParseResult::completeSubCommand()
{
   return m_CompleteSubCommand;
}

void ParseResult::setCompleteAttribute()
{
   m_CompleteAttribute = true;
}

bool ParseResult::completeAttribute()
{
   return m_CompleteAttribute;
}

void ParseResult::setCompletePasswordOrKey()
{
   m_CompletePasswordOrKey = true;
}

bool ParseResult::completePasswordOrKey()
{
   return m_CompletePasswordOrKey;
}

void ParseResult::setEntryId( const String& entryId )
{
   m_EntryId = entryId;
}

String ParseResult::getEntryId()
{
   return m_EntryId;
}

} }
