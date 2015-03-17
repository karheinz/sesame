SET_PROPERTY( DIRECTORY APPEND PROPERTY INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/src" )
SET_PROPERTY( DIRECTORY APPEND PROPERTY INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/${LIBSCRYPT_INCLUDE_DIR}" )
SET_PROPERTY( DIRECTORY APPEND PROPERTY INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/${LIBMSGPACK_INCLUDE_DIR}" )
SET_PROPERTY( DIRECTORY APPEND PROPERTY INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/${LIBGTEST_INCLUDE_DIR}" )
SET_PROPERTY( DIRECTORY APPEND PROPERTY INCLUDE_DIRECTORIES "${GEN_SOURCE_DIR}" )
SET( SESAME_SOURCE_DIR "../src/sesame" )

ADD_CUSTOM_TARGET( tests )
ADD_DEPENDENCIES( tests gtest gtest_main )

ADD_EXECUTABLE( StringTest src/sesame/test/StringTest.cpp )
TARGET_LINK_LIBRARIES( StringTest ${LIBGTEST} ${LIBGTEST_MAIN} )
ADD_DEPENDENCIES( tests StringTest )
ADD_TEST( RunStringTest StringTest )

ADD_EXECUTABLE( string_test src/sesame/test/utils/string_test.cpp
   ${SESAME_SOURCE_DIR}/utils/string.cpp
   ${SESAME_SOURCE_DIR}/utils/Transcoder.cpp
   )
TARGET_LINK_LIBRARIES( string_test ${LIBGTEST} ${LIBGTEST_MAIN} )
ADD_DEPENDENCIES( tests string_test )
ADD_TEST( run_string_test string_test )

ADD_EXECUTABLE( TranscoderTest src/sesame/test/utils/TranscoderTest.cpp
   ${SESAME_SOURCE_DIR}/utils/string.cpp
   ${SESAME_SOURCE_DIR}/utils/Transcoder.cpp
   ${SESAME_SOURCE_DIR}/utils/Reader.cpp
   )
TARGET_LINK_LIBRARIES( TranscoderTest ${LIBGTEST} ${LIBGTEST_MAIN} )
ADD_DEPENDENCIES( tests TranscoderTest )
ADD_TEST( RunTranscoderTest TranscoderTest )

ADD_EXECUTABLE( DataTest src/sesame/test/DataTest.cpp ${SESAME_SOURCE_DIR}/Data.cpp )
TARGET_LINK_LIBRARIES( DataTest ${LIBGTEST} ${LIBGTEST_MAIN} ${LIBMSGPACK} )
ADD_DEPENDENCIES( tests DataTest )
ADD_TEST( RunDataTest DataTest )

ADD_EXECUTABLE( EntryTest src/sesame/test/EntryTest.cpp
   ${SESAME_SOURCE_DIR}/utils/string.cpp
   ${SESAME_SOURCE_DIR}/utils/Transcoder.cpp
   ${SESAME_SOURCE_DIR}/Data.cpp
   ${SESAME_SOURCE_DIR}/Entry.cpp
   )
TARGET_LINK_LIBRARIES( EntryTest ${LIBGTEST} ${LIBGTEST_MAIN} ${LIBMSGPACK} )
ADD_DEPENDENCIES( tests EntryTest )
ADD_TEST( RunEntryTest EntryTest )

ADD_EXECUTABLE( InstanceTest src/sesame/test/InstanceTest.cpp
   ${SESAME_SOURCE_DIR}/utils/string.cpp
   ${SESAME_SOURCE_DIR}/utils/Transcoder.cpp
   ${SESAME_SOURCE_DIR}/Data.cpp
   ${SESAME_SOURCE_DIR}/Entry.cpp
   ${SESAME_SOURCE_DIR}/Instance.cpp
   ${SESAME_SOURCE_DIR}/crypto/MachineFactory.cpp
   ${SESAME_SOURCE_DIR}/crypto/ScryptAesCbcShaV1Machine.cpp
   )
TARGET_LINK_LIBRARIES( InstanceTest ${LIBSSL} ${LIBCRYPTO} ${LIBSCRYPT} ${LIBGTEST} ${LIBGTEST_MAIN} ${LIBMSGPACK} )
ADD_DEPENDENCIES( tests InstanceTest )
ADD_TEST( RunInstanceTest InstanceTest )

ADD_EXECUTABLE( ScryptAesCbcShaV1MachineTest src/sesame/test/crypto/ScryptAesCbcShaV1MachineTest.cpp
   ${SESAME_SOURCE_DIR}/utils/string.cpp
   ${SESAME_SOURCE_DIR}/utils/Transcoder.cpp
   ${SESAME_SOURCE_DIR}/crypto/ScryptAesCbcShaV1Machine.cpp
   )
TARGET_LINK_LIBRARIES( ScryptAesCbcShaV1MachineTest ${LIBSSL} ${LIBCRYPTO} ${LIBSCRYPT} ${LIBGTEST} ${LIBGTEST_MAIN} ${LIBMSGPACK} )
ADD_DEPENDENCIES( tests ScryptAesCbcShaV1MachineTest )
ADD_TEST( RunScryptAesCbcShaV1MachineTest ScryptAesCbcShaV1MachineTest )

ADD_DEFINITIONS( -DAESAVS_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/src/sesame/test/crypto/AESAVS" )
ADD_EXECUTABLE( ScryptAesCbcShaV1MachineAesAvsTest src/sesame/test/crypto/ScryptAesCbcShaV1MachineAesAvsTest.cpp
   ${SESAME_SOURCE_DIR}/utils/string.cpp
   ${SESAME_SOURCE_DIR}/utils/Transcoder.cpp
   ${SESAME_SOURCE_DIR}/crypto/ScryptAesCbcShaV1Machine.cpp
   )
TARGET_LINK_LIBRARIES( ScryptAesCbcShaV1MachineAesAvsTest ${LIBSSL} ${LIBCRYPTO} ${LIBSCRYPT} ${LIBGTEST} ${LIBGTEST_MAIN} ${LIBMSGPACK} )
ADD_DEPENDENCIES( tests ScryptAesCbcShaV1MachineAesAvsTest )
ADD_TEST( RunScryptAesCbcShaV1MachineAesAvsTest ScryptAesCbcShaV1MachineAesAvsTest )

ADD_CUSTOM_TARGET(
    gentestdir
    mkdir -p "${CMAKE_CURRENT_BINARY_DIR}"
    )
ADD_CUSTOM_COMMAND(
    OUTPUT
    "${CMAKE_CURRENT_BINARY_DIR}/tux_by_Gabriel_dos_Santos.jpg"
    COMMAND cp
        "${CMAKE_CURRENT_SOURCE_DIR}/src/sesame/test/crypto/tux_by_Gabriel_dos_Santos.jpg"
        "${CMAKE_CURRENT_BINARY_DIR}/"
    DEPENDS gentestdir
    )
ADD_DEFINITIONS( -DF4_TEST_IMAGE="${CMAKE_CURRENT_BINARY_DIR}/tux_by_Gabriel_dos_Santos.jpg" )
ADD_EXECUTABLE( F4Test src/sesame/test/crypto/F4Test.cpp
   ${SESAME_SOURCE_DIR}/utils/filesystem.cpp
   ${SESAME_SOURCE_DIR}/utils/string.cpp
   ${SESAME_SOURCE_DIR}/utils/Transcoder.cpp
   ${SESAME_SOURCE_DIR}/crypto/F4.cpp
   "${CMAKE_CURRENT_BINARY_DIR}/tux_by_Gabriel_dos_Santos.jpg"
   $<TARGET_OBJECTS:${JPEGTRANF4}>
   )
TARGET_LINK_LIBRARIES( F4Test ${LIBGTEST} ${LIBGTEST_MAIN} ${LIBJPEGTURBO} )
ADD_DEPENDENCIES( tests F4Test )
ADD_TEST( RunF4Test F4Test )
