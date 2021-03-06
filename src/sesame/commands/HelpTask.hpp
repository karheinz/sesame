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


#ifndef SESAME_COMMANDS_HELP_HPP
#define SESAME_COMMANDS_HELP_HPP

#include "types.hpp"
#include "sesame/version.hpp"
#include "sesame/commands/ICommand.hpp"


namespace sesame { namespace commands {

class HelpTask : public ICommand
{
   public:
      /** Available task types. */
      enum Type
      {
         HELP,
         USAGE
      };

      /**
       * Ctor for help task.
       *
       * @param taskType the task type
       * @param program the name of the program
       */
      HelpTask( const Type type, const String& program = "sesame" );

      /**
       * Dtor.
       */
      virtual ~HelpTask() = default;

      /**
       * Runs the command.
       *
       * @param instance the instance to run command on (can be <tt>nullptr</tt>)
       *
       * @throw std::runtime_error on failure
       */
      virtual void run( std::shared_ptr<Instance>& instance );

   private:
      /** the type of the task */
      const Type m_Type;
      /** the name of the program, default sesame */
      const String m_Program;
};

} }

#endif
