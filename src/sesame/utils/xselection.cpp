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


#include <mutex>
#include <thread>

#include "xselection.h"
#include "sesame/utils/xselection.hpp"


namespace sesame { namespace utils {

namespace
{
   std::thread xclipThread1;
   std::thread xclipThread2;
   std::mutex mutex;
   String currentSelection;
   char* currentSelectionPtr( 0 );
}

void xselect( const String& selection )
{
   mutex.lock();
   currentSelection = selection;
   currentSelectionPtr = const_cast<char*>( currentSelection.c_str() );
   mutex.unlock();

   if ( xclipThread1.joinable() )
   {
      xclipThread2 = std::thread( &xclip, mutex.native_handle(), currentSelectionPtr );
      xclipThread1.join();
   }
   else if ( xclipThread2.joinable() )
   {
      xclipThread1 = std::thread( &xclip, mutex.native_handle(), currentSelectionPtr );
      xclipThread2.join();
   }
   else
   {
      xclipThread1 = std::thread( &xclip, mutex.native_handle(), currentSelectionPtr );
   }
}

void xdeselect()
{
   mutex.lock();
   currentSelection.resize( 0 );
   currentSelectionPtr = const_cast<char*>( currentSelection.c_str() );
   mutex.unlock();

   if ( xclipThread1.joinable() )
   {
      xclipThread2 = std::thread( &xclip, mutex.native_handle(), currentSelectionPtr );
   }
   else if ( xclipThread2.joinable() )
   {
      xclipThread1 = std::thread( &xclip, mutex.native_handle(), currentSelectionPtr );
   }

   xclipThread1.join();
   xclipThread2.join();
}

} }