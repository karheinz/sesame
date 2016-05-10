INCLUDE_DIRECTORIES( . )
INCLUDE_DIRECTORIES( lib/util lib/crypto )
INCLUDE_DIRECTORIES( libcperciva/alg libcperciva/cpusupport libcperciva/crypto libcperciva/util )

# Add build type specific CFLAGS because of try_compile().
SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE_UPPER}}" )

ADD_LIBRARY(
    scrypt
    STATIC
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    lib/crypto/crypto_scrypt.c
    lib/crypto/crypto_scrypt_smix.c
    lib/crypto/crypto_scrypt_smix_sse2.c
    libcperciva/alg/sha256.c
    libcperciva/cpusupport/cpusupport_x86_aesni.c
    libcperciva/cpusupport/cpusupport_x86_sse2.c
    libcperciva/util/insecure_memzero.c
    libcperciva/util/warnp.c
    )
SET_TARGET_PROPERTIES( scrypt PROPERTIES COMPILE_DEFINITIONS
    "CONFIG_H_FILE=\"${CMAKE_CURRENT_BINARY_DIR}/config.h\"" )

ADD_CUSTOM_COMMAND(
    OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    COMMAND sh -c 'if [ ! -f libscrypt.a ]\; then
            ${CMAKE_CURRENT_SOURCE_DIR}/configure\;
            make config.h\;
        fi'
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} 
    )


################################################################################

TRY_COMPILE(
    CPU_SUPPORTS_X86_CPUID
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/libcperciva/cpusupport/Build/cpusupport-X86-CPUID.c
    COMPILE_DEFINITIONS -D_POSIX_C_SOURCE=200809L
    )

IF( ${CPU_SUPPORTS_X86_CPUID} )
    MESSAGE( "CPU supports X86 CPUID" )
    SET_PROPERTY( TARGET scrypt APPEND PROPERTY COMPILE_DEFINITIONS "CPUSUPPORT_X86_CPUID" )
ELSE()
    MESSAGE( "CPU does not support X86 CPUID" )
ENDIF()

SET( CMAKE_C_FLAGS_SAVE "${CMAKE_C_FLAGS}" )
FOREACH( FLAG "" "-msse2" "-msse2 -Wno-cast-align" )
    SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS_SAVE} ${FLAG}" )
    TRY_COMPILE(
        CPU_SUPPORTS_X86_SSE2
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/libcperciva/cpusupport/Build/cpusupport-X86-SSE2.c
        COMPILE_DEFINITIONS -D_POSIX_C_SOURCE=200809L
        )

    IF( ${CPU_SUPPORTS_X86_SSE2} )
        IF( "${FLAG}" STREQUAL "" )
            MESSAGE( "CPU supports X86 SSE2" )
        ELSE()
            MESSAGE( "CPU supports X86 SSE2, flag(s) used: ${FLAG}" )
        ENDIF()
        SET_PROPERTY( TARGET scrypt APPEND PROPERTY COMPILE_DEFINITIONS "CPUSUPPORT_X86_SSE2" )
        BREAK()
    ENDIF()
ENDFOREACH( FLAG )
IF( NOT ${CPU_SUPPORTS_X86_SSE2} )
    MESSAGE( "CPU does not support X86 SSE2" )
    SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS_SAVE}" )
ENDIF()

SET( CMAKE_C_FLAGS_SAVE "${CMAKE_C_FLAGS}" )
FOREACH( FLAG "" "-maes" "-maes -Wno-cast-align" "-maes -Wno-missing-prototypes -Wno-cast-qual" )
    SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS_SAVE} ${FLAG}" )
    TRY_COMPILE(
        CPU_SUPPORTS_X86_AESNI
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/libcperciva/cpusupport/Build/cpusupport-X86-AESNI.c
        COMPILE_DEFINITIONS -D_POSIX_C_SOURCE=200809L
        )

    IF( ${CPU_SUPPORTS_X86_AESNI} )
        IF( "${FLAG}" STREQUAL "" )
            MESSAGE( "CPU supports X86 AESNI" )
        ELSE()
            MESSAGE( "CPU supports X86 AESNI, flag(s) used: ${FLAG}" )
        ENDIF()
        SET_PROPERTY( TARGET scrypt APPEND PROPERTY COMPILE_DEFINITIONS "CPUSUPPORT_X86_AESNI" )
        BREAK()
    ENDIF()
ENDFOREACH( FLAG )
IF( NOT ${CPU_SUPPORTS_X86_AESNI} )
    MESSAGE( "CPU does not support X86 AESNI" )
    SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS_SAVE}" )
ENDIF()

MESSAGE( "scrypt: CFLAGS=\"${CMAKE_C_FLAGS}\"" )

