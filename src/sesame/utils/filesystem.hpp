#ifndef SESAME_UTILS_FILESYSTEM_HPP
#define SESAME_UTILS_FILESYSTEM_HPP

#include <cstddef>
#include "types.hpp"

namespace sesame { namespace utils {

bool exists( const String& path );
bool isFile( const String& path );
std::size_t getFileSize( const String& path );
bool removeFile( const String& path );

} }

#endif
