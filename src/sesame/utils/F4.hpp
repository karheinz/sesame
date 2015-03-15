#ifndef SESAME_UTILS_F4_HPP
#define SESAME_UTILS_F4_HPP

#include <iostream>

#include "types.hpp"


namespace sesame { namespace utils {

class F4
{
   public:
      F4( const String& fileName );

      virtual ~F4() = default;

      void embed( std::istream& s );

      void extract( std::ostream& s );

   private:
      const String m_FileName;
};

} }

#endif
