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


#ifndef SESAME_COMMANDS_ENTRY_TASK_HPP
#define SESAME_COMMANDS_ENTRY_TASK_HPP

#include "types.hpp"
#include "sesame/commands/ICommand.hpp"


namespace sesame { namespace commands {

class EntryTask : public ICommand
{
   public:
      /** Available task types. */
      enum Type
      {
         LIST,
         SEARCH,
         SHOW,
         DECRYPT,
         ADD,
         UPDATE,
         DELETE,
         ADD_ATTRIBUTE,
         DELETE_ATTRIBUTE,
         UPDATE_ATTRIBUTE,
         ADD_PASSWORD,
         ADD_KEY,
         DELETE_PASSWORD_OR_KEY,
         UPDATE_PASSWORD_OR_KEY,
         EXPORT_KEY
      };

      /**
       * Ctor for entry task.
       *
       * @param taskType the task type
       * @param id the id of the entry
       * @param pos the position of the attribute or labeled data (starting with 1)
       */
      EntryTask( const Type taskType, const String& id = "", const String& pos = "" );

      /**
       * Dtor.
       */
      virtual ~EntryTask() = default;

      /**
       * Runs the command.
       *
       * @param instance the instance to run command on (can be <tt>nullptr</tt>)
       *
       * @throw std::runtime_error on failure
       */
      virtual void run( std::shared_ptr<Instance>& instance );

   private:
      /**
       * Decrypts the passed entry.
       *
       * @param instance the instance of the entry
       * @param entry the entry to decrypt
       */
      void decryptEntry( std::shared_ptr<Instance>& instance, Entry& entry );

      /**
       * Asks for input file and reads binary content.
       *
       * @return path to input file
       */
      const Vector<uint8_t> askForInputFileAndRead();


      /** task type */
      const Type m_TaskType;
      /** the id of the entry to edit */
      String m_Id;
      /**
       * the position of the attribute or labeled data
       * to edit (starting with 1)
       */
      String m_Pos;
};

} }

#endif
