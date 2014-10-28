INCLUDE(ExternalProject)

SET(EXTLIB_DOWNLOAD_URL "${CMAKE_CURRENT_SOURCE_DIR}/tarballs")
SET(EXTLIB_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/install")

SET(CONFIGURE_STATIC --enable-static=yes --enable-shared=no --disable-shared)
if(USE_32BIT)
    SET(CONFIGURE_FLAGS CFLAGS=-m32 CXXFLAGS=-m32 FFLAGS=-m32 FCFLAGS=-m32)
else()
endif(USE_32BIT)

ExternalProject_Add(
    protobuf_proj 
    URL ${EXTLIB_DOWNLOAD_URL}/protobuf-2.6.0.tar.gz
    URL_MD5 9959d86087e64524d7f91e7a5a6e4fd7
    CONFIGURE_COMMAND ./configure --prefix=<INSTALL_DIR> ${CONFIGURE_STATIC} ${CONFIGURE_FLAGS}
    TEST_BEFORE_INSTALL 0
    BUILD_IN_SOURCE 1
    INSTALL_DIR ${EXTLIB_INSTALL_PREFIX}
    INSTALL_COMMAND make install
    )

ADD_SUBDIRECTORY(libattrapi)

EXPORT(PACKAGE extlib)
EXPORT(TARGETS attrapi FILE extlib-exports.cmake)
CONFIGURE_FILE(extlib-config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/extlib-config.cmake)
