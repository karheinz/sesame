CMAKE_MINIMUM_REQUIRED( VERSION 2.8.0 )

ENABLE_TESTING()

SET( CMAKE_CXX_FLAGS "-std=c++11 -Wall" )

# flex for lexer
FIND_PROGRAM( BINFLEX "flex" )
FIND_LIBRARY( LIBFLEX "fl" )

# crypto libs
FIND_LIBRARY( LIBSSL "ssl" )
FIND_LIBRARY( LIBCRYPTO "crypto" )

# lib for editing command line
FIND_LIBRARY( LIBTECLA "tecla" )

# libs/scrypt
SET( LIBSCRYPT "scrypt" )
SET( LIBSCRYPT_DIR "libs/scrypt-1.1.6" )
SET( LIBSCRYPT_INCLUDE_DIR "libs/scrypt-1.1.6/lib/crypto" )
ADD_SUBDIRECTORY( ${LIBSCRYPT_DIR} )

# libs/gtest
SET( LIBGTEST "gtest" )
SET( LIBGTEST_MAIN "gtest_main" )
SET( LIBGTEST_DIR "libs/gtest-1.7.0" )
SET( LIBGTEST_INCLUDE_DIR "libs/gtest-1.7.0/include" )
ADD_SUBDIRECTORY( ${LIBGTEST_DIR} )

# libs/msgpack
SET( LIBMSGPACK "msgpack" )
SET( LIBMSGPACK_DIR "libs/msgpack-0.5.9" )
SET( LIBMSGPACK_INCLUDE_DIR "${LIBMSGPACK_DIR}/src" )
ADD_SUBDIRECTORY( ${LIBMSGPACK_DIR} )

# where to place generated sources
SET( GEN_SOURCE_DIR "${CMAKE_BINARY_DIR}/gen" )

# sesame
ADD_SUBDIRECTORY( "src" )

# test
ADD_SUBDIRECTORY( "test" )
