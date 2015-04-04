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
#include "sesame/commands/HelpTask.hpp"
#include "sesame/utils/colors.hpp"

using sesame::utils::ESC_SEQ_RESET;
using sesame::utils::ESC_SEQ_BOLD;
using sesame::utils::ESC_SEQ_ULINE;

namespace sesame { namespace commands {

HelpTask::HelpTask( const Type type, const String& program ) :
   m_Type( type ),
   m_Program( program )
{
}

void HelpTask::run( std::shared_ptr<Instance>& instance )
{
   switch ( m_Type )
   {
      case HELP:
      {
         if ( ! instance )
         {
            std::cout << "Currently available commands:";

            std::cout << "\n\n" << std::setw( 7 ) << " " << ESC_SEQ_BOLD << "help" << ESC_SEQ_RESET;
            std::cout << "\n" << std::setw( 14 ) << " " << "prints this message";

            std::cout << "\n\n" << std::setw( 7 ) << " " << ESC_SEQ_BOLD << "edit-mode" << ESC_SEQ_RESET;
            std::cout << " (" << ESC_SEQ_BOLD << "emacs" << ESC_SEQ_RESET << "|";
            std::cout << ESC_SEQ_BOLD << "vi" << ESC_SEQ_RESET << ")";
            std::cout << "\n" << std::setw( 14 ) << " " << "sets editing mode to either ";
            std::cout << ESC_SEQ_BOLD << "emacs" << ESC_SEQ_RESET << " or ";
            std::cout << ESC_SEQ_BOLD << "vi" << ESC_SEQ_RESET;

            std::cout << "\n\n" << std::setw( 7 ) << " " << ESC_SEQ_BOLD << "clear" << ESC_SEQ_RESET;
            std::cout << "\n" << std::setw( 14 ) << " " << "clears the screen";

            std::cout << "\n\n" << std::setw( 7 ) << " " << ESC_SEQ_BOLD << "quit" << ESC_SEQ_RESET;
            std::cout << "\n" << std::setw( 14 ) << " " << "quits sesame";

            std::cout << "\n\n" << std::setw( 7 ) << " " << ESC_SEQ_BOLD << "new" << ESC_SEQ_RESET;
            std::cout << "\n" << std::setw( 14 ) << " " << "creates a new empty container";

            std::cout << "\n\n" << std::setw( 7 ) << " " << ESC_SEQ_BOLD << "open" << ESC_SEQ_RESET;
            std::cout << " " << ESC_SEQ_ULINE << "FILE" << ESC_SEQ_RESET;
            std::cout << "\n" << std::setw( 14 ) << " " << "opens an existing container stored in ";
            std::cout << ESC_SEQ_ULINE << "FILE" << ESC_SEQ_RESET;

            std::cout << "\n" << std::endl;
         }
         else
         {
            std::cout << "Currently available commands:";

            std::cout << "\n\n" << std::setw( 7 ) << " " << ESC_SEQ_BOLD << "help" << ESC_SEQ_RESET;
            std::cout << "\n" << std::setw( 14 ) << " " << "prints this message";

            std::cout << "\n\n" << std::setw( 7 ) << " " << ESC_SEQ_BOLD << "edit-mode" << ESC_SEQ_RESET;
            std::cout << " (" << ESC_SEQ_BOLD << "emacs" << ESC_SEQ_RESET << "|";
            std::cout << ESC_SEQ_BOLD << "vi" << ESC_SEQ_RESET << ")";
            std::cout << "\n" << std::setw( 14 ) << " " << "sets editing mode to either ";
            std::cout << ESC_SEQ_BOLD << "emacs" << ESC_SEQ_RESET << " or ";
            std::cout << ESC_SEQ_BOLD << "vi" << ESC_SEQ_RESET;

            std::cout << "\n\n" << std::setw( 7 ) << " " << ESC_SEQ_BOLD << "clear" << ESC_SEQ_RESET;
            std::cout << "\n" << std::setw( 14 ) << " " << "clears the screen";

            std::cout << "\n\n" << std::setw( 7 ) << " " << ESC_SEQ_BOLD << "write " << ESC_SEQ_RESET;
            std::cout << ESC_SEQ_ULINE << "FILE" << ESC_SEQ_RESET;
            std::cout << "\n" << std::setw( 14 ) << " " << "writes current container to ";
            std::cout << ESC_SEQ_ULINE << "FILE" << ESC_SEQ_RESET << " or";
            std::cout << "\n" << std::setw( 14 ) << " " << "embeds container, if ";
            std::cout << ESC_SEQ_ULINE << "FILE" << ESC_SEQ_RESET;
            std::cout << " is a JPEG image";

            std::cout << "\n\n" << std::setw( 7 ) << " " << ESC_SEQ_BOLD << "recrypt" << ESC_SEQ_RESET;
            std::cout << "\n" << std::setw( 14 ) << " ";
            std::cout << "recrypts the container with new crypto params and/or password/phrase";

            std::cout << "\n\n" << std::setw( 7 ) << " " << ESC_SEQ_BOLD << "close" << ESC_SEQ_RESET;
            std::cout << "\n" << std::setw( 14 ) << " " << "closes the current container";

            std::cout << "\n\n" << std::setw( 7 ) << " " << ESC_SEQ_BOLD << "quit" << ESC_SEQ_RESET;
            std::cout << "\n" << std::setw( 14 ) << " " << "quits sesame";

            std::cout << "\n\n" << std::setw( 7 ) << " " << ESC_SEQ_BOLD << "apg" << ESC_SEQ_RESET;
            std::cout << " [" << ESC_SEQ_ULINE << "OPTION" << ESC_SEQ_RESET << "...]";
            std::cout << "\n" << std::setw( 14 ) << " " << "runs (a) (p)assword (g)enerator, ";
            std::cout << "use " << ESC_SEQ_BOLD << "-h" << ESC_SEQ_RESET << " for help";

            std::cout << "\n\n" << std::setw( 7 ) << " " << ESC_SEQ_BOLD << "list" << ESC_SEQ_RESET;
            std::cout << " [" << ESC_SEQ_ULINE << "TAG" << ESC_SEQ_RESET << "]";
            std::cout << "\n" << std::setw( 14 ) << " " << "lists entries of the container (tagged";
            std::cout << " with " << ESC_SEQ_ULINE << "TAG" << ESC_SEQ_RESET << ")";

            std::cout << "\n\n" << std::setw( 7 ) << " " << ESC_SEQ_BOLD << "tree" << ESC_SEQ_RESET;
            std::cout << "\n" << std::setw( 14 ) << " " << "lists all entries by tags";

            std::cout << "\n\n" << std::setw( 7 ) << " " << ESC_SEQ_BOLD << "tags" << ESC_SEQ_RESET;
            std::cout << "\n" << std::setw( 14 ) << " " << "lists all tags assigned to entries";

            std::cout << "\n\n" << std::setw( 7 ) << " " << ESC_SEQ_BOLD << "add" << ESC_SEQ_RESET;
            std::cout << "\n" << std::setw( 14 ) << " " << "adds an entry to the current container";

            std::cout << "\n\n" << std::setw( 7 ) << " (";
            std::cout << ESC_SEQ_BOLD << "decrypt" << ESC_SEQ_RESET << "|";
            std::cout << ESC_SEQ_BOLD << "show" << ESC_SEQ_RESET << "|";
            std::cout << ESC_SEQ_BOLD << "update" << ESC_SEQ_RESET << "|";
            std::cout << ESC_SEQ_BOLD << "delete" << ESC_SEQ_RESET << ")";
            std::cout << " " << ESC_SEQ_ULINE << "ENTRY" << ESC_SEQ_RESET;
            std::cout << "\n" << std::setw( 14 ) << " " << "decrypts, shows, updates or deletes ";
            std::cout << "the specified " << ESC_SEQ_ULINE << "ENTRY" << ESC_SEQ_RESET;

            std::cout << "\n\n" << std::setw( 7 ) << " ";
            std::cout << ESC_SEQ_BOLD << "update" << ESC_SEQ_RESET << " ";
            std::cout << ESC_SEQ_ULINE << "ENTRY" << ESC_SEQ_RESET << " (";
            std::cout << ESC_SEQ_BOLD << "add_tag" << ESC_SEQ_RESET << "|";
            std::cout << ESC_SEQ_BOLD << "add_attribute" << ESC_SEQ_RESET << "|";
            std::cout << ESC_SEQ_BOLD << "add_password" << ESC_SEQ_RESET << "|";
            std::cout << ESC_SEQ_BOLD << "add_key" << ESC_SEQ_RESET << ")";
            std::cout << "\n" << std::setw( 14 ) << " ";
            std::cout << "adds a tag, attribute, password or key to ";
            std::cout << "the specified " << ESC_SEQ_ULINE << "ENTRY" << ESC_SEQ_RESET;

            std::cout << "\n\n" << std::setw( 7 ) << " ";
            std::cout << ESC_SEQ_BOLD << "update" << ESC_SEQ_RESET << " ";
            std::cout << ESC_SEQ_ULINE << "ENTRY" << ESC_SEQ_RESET << " (";
            std::cout << ESC_SEQ_BOLD << "update_tag" << ESC_SEQ_RESET << "|";
            std::cout << ESC_SEQ_BOLD << "update_attribute" << ESC_SEQ_RESET << "|";
            std::cout << ESC_SEQ_BOLD << "update_password_or_key" << ESC_SEQ_RESET << ") ";
            std::cout << ESC_SEQ_ULINE << "ID" << ESC_SEQ_RESET;
            std::cout << "\n" << std::setw( 14 ) << " ";
            std::cout << "updates tag, attribute, password or key with ";
            std::cout << ESC_SEQ_ULINE << "ID" << ESC_SEQ_RESET << " of the specified ";
            std::cout << ESC_SEQ_ULINE << "ENTRY" << ESC_SEQ_RESET;

            std::cout << "\n\n" << std::setw( 7 ) << " ";
            std::cout << ESC_SEQ_BOLD << "update" << ESC_SEQ_RESET << " ";
            std::cout << ESC_SEQ_ULINE << "ENTRY" << ESC_SEQ_RESET << " (";
            std::cout << ESC_SEQ_BOLD << "delete_tag" << ESC_SEQ_RESET << "|";
            std::cout << ESC_SEQ_BOLD << "delete_attribute" << ESC_SEQ_RESET << "|";
            std::cout << ESC_SEQ_BOLD << "delete_password_or_key" << ESC_SEQ_RESET << ") ";
            std::cout << ESC_SEQ_ULINE << "ID" << ESC_SEQ_RESET;
            std::cout << "\n" << std::setw( 14 ) << " ";
            std::cout << "deletes tag, attribute, password or key with ";
            std::cout << ESC_SEQ_ULINE << "ID" << ESC_SEQ_RESET << " of the specified ";
            std::cout << ESC_SEQ_ULINE << "ENTRY" << ESC_SEQ_RESET;

            std::cout << "\n\n" << std::setw( 7 ) << " ";
            std::cout << ESC_SEQ_BOLD << "select" << ESC_SEQ_RESET << " ";
            std::cout << ESC_SEQ_ULINE << "ENTRY" << ESC_SEQ_RESET << " ";
            std::cout << ESC_SEQ_BOLD << "export_password_or_key" << ESC_SEQ_RESET << " ";
            std::cout << ESC_SEQ_ULINE << "ID" << ESC_SEQ_RESET;
            std::cout << "\n" << std::setw( 14 ) << " ";
            std::cout << "exports password with ";
            std::cout << ESC_SEQ_ULINE << "ID" << ESC_SEQ_RESET << " of the specified ";
            std::cout << ESC_SEQ_ULINE << "ENTRY" << ESC_SEQ_RESET << " to clipboard or";
            std::cout << "\n" << std::setw( 14 ) << " ";
            std::cout << "exports key with ";
            std::cout << ESC_SEQ_ULINE << "ID" << ESC_SEQ_RESET << " of the specified ";
            std::cout << ESC_SEQ_ULINE << "ENTRY" << ESC_SEQ_RESET << " to file";

            std::cout << "\n" << std::endl;
         }

         break;
      }
      case USAGE:
      {
         std::cout << "Usage:";

         std::cout << "\n\n" << std::setw( 7 ) << " ";
         std::cout << ESC_SEQ_BOLD << m_Program << ESC_SEQ_RESET;
         std::cout << " [" << ESC_SEQ_ULINE << "FILE" << ESC_SEQ_RESET << "]";

         std::cout << "\n" << std::endl;

         break;
      }
      default:
      {
         throw std::runtime_error( "command not implemented yet" );
      }
   }
}

} }
