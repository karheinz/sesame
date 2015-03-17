#ifndef SESAME_CRYPTO_F4_HPP
#define SESAME_CRYPTO_F4_HPP

#include <cstddef>

#include "types.hpp"


namespace sesame { namespace crypto {

class F4
{
   public:
      F4() = default;

      virtual ~F4() = default;

      void embed(
         const String& fileNameIn,
         const String& fileNameOut,
         const Vector<char>& data
         ) const;

      void extract(
         const String& fileNameIn,
         Vector<char>& data
         ) const;

      const String calcOutFileName(
         const String& fileNameIn,
         const String& delimiter = "/"
         ) const;

};

} }

#endif
