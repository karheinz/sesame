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


#ifndef SESAME_UTILS_PARSE_RESULT_HPP
#define SESAME_UTILS_PARSE_RESULT_HPP

#include <ostream>
#include "types.hpp"
#include "sesame/commands/ICommand.hpp"

namespace sesame { namespace utils {

class ParseResult
{
   public:
      ParseResult();

      void invalidate();

      bool isValid() const;

      void addToken( const String& token );

      std::shared_ptr<commands::ICommand> getCommand() const;

      void setCommand( std::shared_ptr<commands::ICommand> command );

      String getCommandToken() const;

      Vector<String> getArgumentTokens() const;

      void clear();

      /** User requests completion of edit mode? */
      bool completeEditMode();

      void setCompleteEditMode();

      /** User requests completion of file? */
      bool completeFile();

      void setCompleteFile();

      /** User requests completion of space? */
      bool completeSpace();

      void setCompleteSpace();

      void setCompleteEntry();

      /** User requests completion of entry? */
      bool completeEntry();

      void setCompleteCommand();

      /** User requests completion of command? */
      bool completeCommand();

      void setCompleteSubCommand();

      /** User requests completion of sub command? */
      bool completeSubCommand();

      void setCompleteAttribute();

      /** User requests completion of attribute? */
      bool completeAttribute();

      void setCompletePasswordOrKey();

      /** User requests completion of password or key? */
      bool completePasswordOrKey();

      /**
       * Sets the entry id.
       *
       * @param entryId the entry id
       */
      void setEntryId( const String& entryId );

      /**
       * Returns the entry id.
       *
       * @return the entry id
       */
      String getEntryId();

   private:
      bool m_Valid;
      Vector<String> m_Tokens;
      std::shared_ptr<commands::ICommand> m_Command;
      bool m_CompleteEditMode;
      bool m_CompleteFile;
      bool m_CompleteSpace;
      bool m_CompleteEntry;
      bool m_CompleteCommand;
      bool m_CompleteSubCommand;
      bool m_CompleteAttribute;
      bool m_CompletePasswordOrKey;
      String m_EntryId;
};

} }

inline std::ostream& operator<<( std::ostream& s, sesame::utils::ParseResult& p )
{
   if ( ! p.isValid() )
   {
      s << "ERROR: syntax error";
   }
   else
   {
      s << p.getCommandToken();
      for ( auto& argument : p.getArgumentTokens() )
      {
         s << "\n  " << argument;
      }
   }

   return s;
}

#endif
