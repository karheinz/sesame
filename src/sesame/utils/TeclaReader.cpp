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
#include <stdexcept>
#include "sesame/utils/string.hpp"
#include "sesame/utils/TeclaReader.hpp"

extern "C"
{
static CPL_MATCH_FN( cpl_no_completion )
{
   return 0;
}
}

namespace sesame { namespace utils {

TeclaReader::TeclaReader( std::size_t lineSize, std::size_t historyBuffer ) :
   m_Gl( nullptr )
{
   m_Gl = new_GetLine( lineSize, historyBuffer );
   if ( ! m_Gl )
   {
      throw std::runtime_error( "failed to init tecla GetLine" );
   }
   if ( ! disableCompletion() )
   {
      throw std::runtime_error( "failed to disable auto completion" );
   }
}

TeclaReader::~TeclaReader()
{
   if ( m_Gl )
   {
      del_GetLine( m_Gl );
   }
}

bool TeclaReader::setEditMode( const String& mode )
{
   return ( ! gl_configure_getline( m_Gl, ( String( "edit-mode " ) + mode ).c_str(), nullptr, nullptr ) );
}

String TeclaReader::readLine( const String& prompt, bool hideText )
{
   // Set.
   gl_echo_mode( m_Gl, hideText ? 0 : 1 );
   // Set again. Should return echo_off=>0 and echo_on=>1.
   if ( gl_echo_mode( m_Gl, hideText ? 0 : 1 ) != ( hideText ? 0 : 1 ) )
   {
      throw std::runtime_error( "failed to turn echo mode off" );
   }
   char *line = gl_get_line( m_Gl, prompt.c_str(), nullptr, -1 );
   if ( ! line )
   {
      throw std::runtime_error( "failed to get line" );
   }
   if ( hideText )
   {
      std::cout << std::endl;
   }

   // TODO: iconv to utf8!
   return ( strip( String( line ) ) + "\n" );
}

bool TeclaReader::clear()
{
   return ( ! gl_erase_terminal( m_Gl ) );
}

bool TeclaReader::addCompletion( CPL_MATCH_FN( cpl_function ), void* data )
{
   return ( ! gl_customize_completion( m_Gl, data, cpl_function ) );
}

bool TeclaReader::disableCompletion()
{
   return ( ! gl_customize_completion( m_Gl, nullptr, cpl_no_completion ) );
}

bool TeclaReader::enableHistory( std::size_t bufferSize )
{
   return ( ! gl_resize_history( m_Gl, bufferSize ) );
}

bool TeclaReader::disableHistory()
{
   return ( ! gl_resize_history( m_Gl, 0 ) );
}

} }
