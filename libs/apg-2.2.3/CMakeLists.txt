SET( SOURCES "bloom.cpp;./sha/sha.cpp;./cast/cast.cpp;rnd.cpp;pronpass.cpp;randpass.cpp;restrict.cpp;errors.cpp;apg.cpp;getopt.cpp;convert.cpp" )
ADD_LIBRARY( apg STATIC ${SOURCES} )
TARGET_LINK_LIBRARIES( apg crypt m )
SET_PROPERTY( TARGET apg APPEND PROPERTY COMPILE_DEFINITIONS "APG_DONOTUSE_CRYPT;APG_DONOTUSE_SHA" )
