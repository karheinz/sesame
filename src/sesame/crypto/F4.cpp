#include <stdexcept>

#include "sesame/crypto/F4.hpp"
#include "sesame/crypto/jpegtranf4.h"
#include "sesame/utils/filesystem.hpp"


namespace sesame { namespace crypto {

void F4::embed(
   const String& fileNameIn,
   const String& fileNameOut,
   const Vector<char>& data
   )
   const
{
   if ( data.size() == 0 )
   {
      throw std::runtime_error( "nothing to embed" );
   }

   if ( f4_embed( fileNameIn.c_str(), fileNameOut.c_str(), data.data(), data.size() ) != 0 )
   {
      if ( utils::isFile( fileNameOut ) )
      {
         utils::removeFile( fileNameOut );
      }

      throw std::runtime_error( "failed to embed data" );
   }
}

void F4::extract(
   const String& fileNameIn,
   Vector<char>& data
   )
   const
{
   // We expect max 20% of the image as data.
   data.resize( utils::getFileSize( fileNameIn ) / 5 );

   if ( data.size() == 0 )
   {
      throw std::runtime_error( "failed to extract data" );
   }

   if ( f4_extract( fileNameIn.c_str(), data.data(), data.size() ) != 0 )
   {
      throw std::runtime_error( "failed to extract data" );
   }
}

const String F4::calcOutFileName(
   const String& fileNameIn,
   const String& delimiter
   )
   const
{
   auto index( fileNameIn.find_last_of( "." ) );
   auto indexDelim( fileNameIn.find_last_of( delimiter ) );

   if ( index != String::npos && indexDelim != String::npos && indexDelim > index )
   {
      index = String::npos;
   }

   String fileNameOut;
   uint32_t count( 1 );
   do
   {
      StringStream s;

      if ( index == String::npos )
      {
         s << fileNameIn << "." << count++ << ".jpeg";
      }
      else
      {
         s << fileNameIn.substr( 0, index );
         s << "." << count++;
         if ( ( index + 1 ) < fileNameIn.size() )
         {
            s << "." << fileNameIn.substr( index + 1 );
         }
      }

      fileNameOut = s.str();
   }
   while ( utils::exists( fileNameOut ) );

   return fileNameOut;
}

} }
