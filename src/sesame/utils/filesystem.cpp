#include <cerrno>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "sesame/utils/filesystem.hpp"

namespace sesame { namespace utils {

bool exists( const String& path )
{
   struct stat buf;
   return ( lstat( path.c_str(), &buf ) != -1 );
}

bool isFile( const String& path )
{
   bool yes( true );
   struct stat buf;
   if ( lstat( path.c_str(), &buf ) != -1 )
   {
      yes = ( S_ISREG( buf.st_mode ) > 0 );
   }
   else
   {
      if ( errno != ENOENT )
      {
         throw std::runtime_error( "failed to stat file" );
      }
      else
      {
         yes = false;
      }
   }

   return yes;
}

std::size_t getFileSize( const String& path )
{
   if ( isFile( path ) )
   {
      struct stat buf;
      if ( lstat( path.c_str(), &buf ) == -1 )
      {
         throw std::runtime_error( "failed to stat file" );
      }
      return buf.st_size;
   }
   else
   {
      return 0;
   }
}

bool removeFile( const String& path )
{
   if ( isFile( path ) )
   {
      return ( unlink( path.c_str() ) != -1 );
   }
   else
   {
      return false;
   }
}

} }
