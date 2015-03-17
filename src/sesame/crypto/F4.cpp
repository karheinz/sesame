#include <stdexcept>

#include "sesame/crypto/F4.hpp"
#include "sesame/crypto/jpegtranf4.h"
#include "sesame/utils/filesystem.hpp"


namespace sesame { namespace crypto {

F4::F4( const String& pathToFile ) :
   m_PathToFile( pathToFile )
{
}

const String F4::embed( const Vector<char>& data ) const
{
   if ( data.size() == 0 )
   {
      throw std::runtime_error( "nothing to embed" );
   }

   auto index( m_PathToFile.find_last_of( "." ) );
   auto indexDelim( m_PathToFile.find_last_of( "/" ) );
   if ( index != String::npos && indexDelim != String::npos && indexDelim > index )
   {
      index = String::npos;
   }

   String pathToFile;
   uint32_t count( 1 );
   do
   {
      StringStream s;

      if ( index == String::npos )
      {
         s << m_PathToFile << "." << count++ << ".jpeg";
      }
      else
      {
         s << m_PathToFile.substr( 0, index );
         s << "." << count++;
         if ( ( index + 1 ) < m_PathToFile.size() )
         {
            s << "." << m_PathToFile.substr( index + 1 );
         }
      }

      pathToFile = s.str();
   }
   while ( utils::exists( pathToFile ) );


   if ( ! f4_embed( m_PathToFile.c_str(), pathToFile.c_str(), data.data(), data.size() ) )
   {
      if ( utils::isFile( pathToFile ) )
      {
         utils::removeFile( pathToFile );
      }

      throw std::runtime_error( "failed to embed data" );
   }

   return pathToFile;
}

void F4::extract( Vector<char>& data ) const
{
   // We expect max 20% of the image as data.
   data.resize( utils::getFileSize( m_PathToFile ) / 5 );

   if ( data.size() == 0 )
   {
      throw std::runtime_error( "failed to extract data" );
   }

   if ( ! f4_extract( m_PathToFile.c_str(), data.data(), data.size() ) )
   {
      throw std::runtime_error( "failed to extract data" );
   }
}

} }
