INCLUDE_DIRECTORIES( . lib/util lib/crypto )

IF( EXISTS /proc/cpuinfo )
   EXECUTE_PROCESS(
      COMMAND grep -q "sse2" /proc/cpuinfo
      RESULT_VARIABLE NO_SSE2
      OUTPUT_QUIET
      ERROR_QUIET
      )
ENDIF()

IF( NOT ${NO_SSE2} )
   ADD_LIBRARY(
      scrypt
      ${CMAKE_CURRENT_BINARY_DIR}/config.h
      lib/crypto/crypto_scrypt-sse.c lib/crypto/sha256.c
      )
   SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse2" )
ELSE()
   ADD_LIBRARY(
      scrypt
      ${CMAKE_CURRENT_BINARY_DIR}/config.h
      lib/crypto/crypto_scrypt-nosse.c lib/crypto/sha256.c
      )
ENDIF()
SET_TARGET_PROPERTIES( scrypt PROPERTIES COMPILE_DEFINITIONS CONFIG_H_FILE="${CMAKE_CURRENT_BINARY_DIR}/config.h" )

ADD_CUSTOM_COMMAND(
   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/config.h
   COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/configure
   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} 
   )
