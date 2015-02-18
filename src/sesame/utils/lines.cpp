#include "types.hpp"
#include "sesame/utils/lines.hpp"


namespace sesame { namespace utils {

namespace
{
   const uint8_t INDENTATION( 2 );
}

String buildLine( uint8_t level, const String& elem )
{
   StringStream s;
   for ( uint8_t i = 0; i < ( level * INDENTATION ); ++i )
   {
      s << u8" ";
   }
   s << elem;
   for ( uint8_t i = 0; i < INDENTATION; ++i )
   {
      s << u8"\u2500"; 
   }

   return utils::fromUtf8( s.str() );
}

String empty( uint8_t level )
{
   StringStream s;
   for ( uint8_t i = 0; i <= ( level * INDENTATION ); ++i )
   {
      s << u8" ";
   }

   return utils::fromUtf8( s.str() );
}

String down( uint8_t level )
{
   StringStream s;
   for ( uint8_t i = 0; i < ( level * INDENTATION ); ++i )
   {
      s << u8" ";
   }
   s << u8"\u2502";

   return utils::fromUtf8( s.str() );
}

String branch( uint8_t level )
{
   return buildLine( level, u8"\u251c" );
}

String corner( uint8_t level )
{
   return buildLine( level, u8"\u2514" );
}

} }
