#ifndef SESAME_CRYPTO_F4_HPP
#define SESAME_CRYPTO_F4_HPP

#include <cstddef>

#include "types.hpp"


namespace sesame { namespace crypto {

class F4
{
   public:
      F4( const String& fileName );

      virtual ~F4() = default;

      const String embed( const Vector<char>& data ) const;

      void extract( Vector<char>& data ) const;

   private:
      const String m_PathToFile;
};

} }

#endif
