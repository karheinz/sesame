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


#include <clocale>
#include <locale>
#include <iostream>
#include <stdexcept>
#include "sesame/utils/string.hpp"
#include "sesame/utils/Transcoder.hpp"

namespace
{
   void stripIfEmpty( String& string )
   {
      if ( string.find_first_not_of( " \n" ) == String::npos )
      {
         string = "";
      }
   }
}

namespace sesame { namespace utils {

String lstrip( const String& string )
{
   String tmp( string );
   String::size_type hit;

   hit = tmp.find_first_not_of( " " );
   if ( hit > 0 )
   {
      tmp.erase( 0, hit );
   }
   stripIfEmpty( tmp );

   return tmp;
}

String rstrip( const String& string )
{
   String tmp( string );
   String::size_type hit;

   hit = tmp.find_last_not_of( " \n" );
   if ( hit != String::npos && ( hit + 1 ) < tmp.size() )
   {
      tmp.erase( hit + 1 );
   }
   stripIfEmpty( tmp );

   return tmp;
}

String strip( const String& string )
{
   String tmp( string );

   tmp = lstrip( tmp );
   tmp = rstrip( tmp );

   return tmp;
}

String reduce( const String& string )
{
   String::size_type hit;
   String tmp( string );

   while ( ( hit = tmp.find( "  " ) ) != String::npos )
   {
      tmp.replace( hit, 2, " " );
   }

   return tmp;
}

String normalize( const String& string )
{
   String tmp( string );
   tmp = strip( tmp );
   tmp = reduce( tmp );
   return tmp;
}

std::pair<String,String> split( const String& string )
{
   String left( string );
   String right;

   // Line does not end with whitespace? Split line.
   if ( ! ( left.empty() || left.back() == ' ' ) )
   {
      String::size_type leftStart( string.find_first_not_of( " " ) );
      String::size_type hit( left.rfind( " " ) );
      if ( hit != String::npos && hit > leftStart )
      {
         right = left.substr( hit + 1 );
         left.erase( hit );
         rstrip( left );
         left += " ";
      }
   }

   return std::make_pair( left, right );
}

String toUtf8( const char* text, const std::size_t length )
{
   static Transcoder transcoder( getEncoding(), "UTF-8" );

   return transcoder.transcode( text, length );
}

String toUtf8( const String& text )
{
   return toUtf8( text.data(), text.size() );
}

String fromUtf8( const char* text, const std::size_t length )
{
   static Transcoder transcoder( "UTF-8", getEncoding() );

   return transcoder.transcode( text, length );
}

String fromUtf8( const String& text )
{
   return fromUtf8( text.data(), text.size() );
}

void setLocale()
{
   char* currentLocaleName( std::setlocale( LC_ALL, "" ) );
   if ( ! currentLocaleName )
   {
      throw std::runtime_error( "failed to set locale" );
   }
   std::locale currentLocale( currentLocaleName );
   std::cout.imbue( currentLocale );
   std::cerr.imbue( currentLocale );
   std::cin.imbue( currentLocale );
}

String getEncoding()
{
   String curLoc( std::cin.getloc().name().c_str() );
   if ( curLoc == "*" )
   {
      throw std::runtime_error( "locale is unnamed" );
   }

   String::size_type dot( curLoc.find( "." ) );
   if ( dot == String::npos || ( dot + 1 ) >= curLoc.size() )
   {
      throw std::runtime_error( "encoding unknown" );
   }
   return curLoc.substr( dot + 1 );
}

} }
