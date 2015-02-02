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
#include <unistd.h>
#include <termios.h>

#include "sesame/utils/string.hpp"
#include "sesame/utils/Reader.hpp"


namespace sesame { namespace utils {

Reader::Reader( std::size_t lineSize ) :
   m_Buffer( lineSize )
{
}

String Reader::readLine( const String& prompt, bool echoOff )
{
   if ( ! prompt.empty() )
   {
      std::cout << prompt;
   }

   if ( echoOff )
   {
      EchoOffGuard g;
      std::cin.getline( m_Buffer.data(), m_Buffer.size(), '\n' );
      std::cout << '\n';
   }
   else
   {
      std::cin.getline( m_Buffer.data(), m_Buffer.size(), '\n' );
   }

   // Copy every read byte except null char at the end.
   return String( m_Buffer.data(), m_Buffer.data() + std::cin.gcount() - 1 );
}

Reader::EchoOffGuard::EchoOffGuard()
{
   termios t;
   tcgetattr(STDIN_FILENO, &t);
   t.c_lflag &= ~ECHO;
   tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

Reader::EchoOffGuard::~EchoOffGuard()
{
   termios t;
   tcgetattr(STDIN_FILENO, &t);
   t.c_lflag |= ECHO;
   tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

} }
