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


#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include <setjmp.h>
#include <signal.h>

#include <libtecla.h>

#include "types.hpp"
#include "sesame/Instance.hpp"
#include "sesame/commands/HelpTask.hpp"
#include "sesame/commands/InstanceTask.hpp"
#include "sesame/utils/completion.hpp"
#include "sesame/utils/filesystem.hpp"
#include "sesame/utils/resources.hpp"
#include "sesame/utils/string.hpp"
#include "sesame/utils/xselection.hpp"
#include "sesame/utils/Parser.hpp"
#include "sesame/utils/ParseResult.hpp"
#include "sesame/utils/TeclaReader.hpp"

using sesame::commands::HelpTask;
using sesame::commands::InstanceTask;

String buildPrompt( std::shared_ptr<sesame::Instance>& instance );

std::vector<std::pair<std::string,std::string>> apgCache;

bool stopRequested( false );
void signal_handler( int signal )
{
   std::cout << "\nGot signal " << signal <<
      ", will quit as soon as possible." << std::endl;

   stopRequested = true;
}

int main( int argc, char** argv)
{
   if ( sesame::utils::hasRootPrivileges() )
   {
#ifndef DEBUG
      if ( ! sesame::utils::disableCoreFiles() )
      {
         std::cerr << "ERROR: failed to disable core files ("
                   << std::strerror( errno ) << ")" << std::endl;
         return 1;
      }
#endif

      if ( ! sesame::utils::lockMemory() )
      {
         std::cerr << "ERROR: failed to lock memory ("
                   << std::strerror( errno ) << ")" << std::endl;
         return 1;
      }

      if ( ! sesame::utils::isRoot() )
      {
         if ( ! sesame::utils::dropPrivileges() )
         {
            std::cerr << "ERROR: failed to drop privileges ("
                      << std::strerror( errno ) << ")" << std::endl;
            return 1;
         }
      }
   }
   else
   {
      std::cerr << "WARNING: Failed to disable core files and swapping!\n\n"
                << "Consider to make root the owner of the program and to set the suid bit:\n\n"
                << "    # chown root:root /path/to/sesame\n"
                << "    # chmod u+s /path/to/sesame\n\n"
                << std::endl;
   }

   // Handle signals.
   struct sigaction action;
   std::memset( &action, 0, sizeof( action ) );
   action.sa_handler = signal_handler;
   action.sa_flags = ~SA_SIGINFO;

   sigaction( SIGHUP, &action, nullptr );
   sigaction( SIGINT, &action, nullptr );
   sigaction( SIGTERM, &action, nullptr );

   // Set locale!
   sesame::utils::setLocale();

   // Start.
   std::shared_ptr<sesame::Instance> instance;

   // Print version info.
   std::cout << sesame::VERSION_STRING << "\n" << std::endl;

   // More than one argument passed?
   if ( argc > 2 )
   {
      HelpTask task( HelpTask::USAGE, argv[ 0 ] );
      task.run( instance );
      return 1;
   }
   // File passed?
   // Abort immediately if opening the container fails.
   else if ( argc == 2 )
   {
      try
      {
         InstanceTask task( InstanceTask::OPEN, argv[ 1 ] );
         task.run( instance );
      }
      catch ( std::runtime_error& e )
      {
         std::cerr << "ERROR: " << e.what() << std::endl;;
         return 1;
      }
      catch ( std::exception& e )
      {
         if ( ! stopRequested )
         {
            std::cerr << "ERROR: " << e.what() << std::endl;;
         }
         return 1;
      }
   }

   sesame::utils::TeclaReader reader( 1024, 2048 );
   reader.addCompletion( cpl_complete_sesame, static_cast<void*>( &instance ) );
   sesame::utils::Parser parser;
   sesame::utils::ParseResult parseResult;
   String normalized;

   do
   {
      // Got a signal?
      if ( stopRequested )
      {
         break;
      }

      // Build prompt.
      String prompt( buildPrompt( instance ) );

      // Read line.
      try
      {
         normalized = reader.readLine( prompt );
      }
      catch ( std::exception& e )
      {
         if ( ! stopRequested )
         {
            std::cerr << "ERROR: " << e.what() << std::endl;;
         }

         // Abort on read line failure.
         break;
      }

      if ( normalized == "\n" )
      {
         continue;
      }
      else if ( normalized.find( "edit-mode " ) == 0 )
      {
         if ( normalized == "edit-mode vi\n" )
         {
            if ( ! reader.setEditMode( "vi" ) )
            {
               std::cerr << "ERROR: failed to set edit-mode" << std::endl;
            }
         }
         else if ( normalized == "edit-mode emacs\n" )
         {
            if ( ! reader.setEditMode( "emacs" ) )
            {
               std::cerr << "ERROR: failed to set edit-mode" << std::endl;
            }
         }
         else
         {
            std::cerr << "ERROR: edit-mode not supported" << std::endl;
         }
      }
      else if ( normalized == "clear\n" )
      {
         if ( ! reader.clear() )
         {
            std::cerr << "ERROR: failed to clear terminal" << std::endl;
         }
      }
      else if ( normalized == "quit\n" )
      {
         if ( instance )
         {
            try
            {
               InstanceTask task( InstanceTask::CLOSE );
               task.run( instance );
            }
            catch ( std::runtime_error& e )
            {
               std::cerr << "ERROR: " << e.what() << std::endl;;
            }
            catch ( std::exception& e )
            {
               if ( ! stopRequested )
               {
                  std::cerr << "ERROR: " << e.what() << std::endl;;
               }
            }
         }

         // Quit only if no instance is open!
         if ( ! instance )
         {
            break;
         }
      }
      else
      {
         parseResult = parser.parse( normalized );
         if ( parseResult.isValid() )
         {
            if ( ! parseResult.getCommandToken().empty() )
            {
               //std::cout << parseResult << std::endl;
               if ( parseResult.getCommand() != nullptr )
               {
                  try
                  {
                     parseResult.getCommand()->run( instance );
                  }
                  catch ( std::runtime_error& e )
                  {
                     std::cerr << "ERROR: " << e.what() << std::endl;
                  }
                  catch ( std::exception& e )
                  {
                     if ( ! stopRequested )
                     {
                        std::cerr << "ERROR: " << e.what() << std::endl;;
                     }
                  }
               }
            }
         }
         else
         {
            std::cerr << parseResult << std::endl;
         }
      }
   }
   while ( true );

   // Make sure instance is closed before quit!
   if ( instance )
   {
      const String tmp( instance->getIdAsHexString() );
      instance.reset();
      std::cout << "Closed container #" << tmp << "." << std::endl;
   }

   // Remove data from X clipboard before quit.
   sesame::utils::xdeselect();

   std::cout << "Goodbye!" << std::endl;

   return 0;
}

String buildPrompt( std::shared_ptr<sesame::Instance>& instance )
{
   String prompt( "sesame> " );
   if ( instance )
   {
      StringStream s;
      s << "sesame #" << std::hex << std::setw( 8 ) << std::setfill( '0' ) << instance->getId() << "> ";
      prompt = s.str();
   }

   return prompt;
}
