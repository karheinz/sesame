INCLUDE_DIRECTORIES( . )
INCLUDE_DIRECTORIES( lib/util lib/crypto )
INCLUDE_DIRECTORIES( libcperciva/alg libcperciva/cpusupport libcperciva/crypto libcperciva/util )

ADD_LIBRARY(
    scrypt
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    ${CMAKE_CURRENT_BINARY_DIR}/cpusupport-config.h
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
    "CONFIG_H_FILE=\"${CMAKE_CURRENT_BINARY_DIR}/config.h\";CPUSUPPORT_CONFIG_FILE=\"${CMAKE_CURRENT_BINARY_DIR}/cpusupport-config.h\"" )

ADD_CUSTOM_COMMAND(
    OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
    ${CMAKE_CURRENT_BINARY_DIR}/cpusupport-config.h
    COMMAND sh -c 'if [ ! -f libscrypt.a ]\; then
            ${CMAKE_CURRENT_SOURCE_DIR}/configure\;
            make config.h\;
            make cpusupport-config.h\;
        fi'
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} 
    )
