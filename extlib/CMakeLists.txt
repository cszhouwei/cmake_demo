INCLUDE(ExternalProject)

SET(EXTLIB_DOWNLOAD_URL "${CMAKE_CURRENT_SOURCE_DIR}/tarballs")
SET(EXTLIB_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/install")

SET(CONFIGURE_FLAGS --enable-static=yes --enable-shared=no)
SET(CMAKE_FLAGS -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})

IF(USE_32BIT)
    SET(CONFIGURE_FLAGS ${CONFIGURE_FLAGS} CFLAGS=-m32 CXXFLAGS=-m32)
    SET(CMAKE_FLAGS ${CMAKE_FLAGS} -DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32)
ENDIF()

ExternalProject_Add(
    protobuf_proj 
    URL ${EXTLIB_DOWNLOAD_URL}/protobuf-2.6.1.tar.bz2
    URL_MD5 11aaac2d704eef8efd1867a807865d85
    CONFIGURE_COMMAND ./configure --prefix=<INSTALL_DIR> ${CONFIGURE_FLAGS}
    TEST_BEFORE_INSTALL 0
    BUILD_IN_SOURCE 1
    INSTALL_DIR ${EXTLIB_INSTALL_PREFIX}
    INSTALL_COMMAND make install
    )

IF(USE_32BIT)
    SET(ADDRESS_MODEL address-model=32)
ENDIF()
ExternalProject_Add(
    boost_proj 
    URL ${EXTLIB_DOWNLOAD_URL}/boost_1_56_0.tar.bz2
    URL_MD5 a744cf167b05d72335f27c88115f211d
    CONFIGURE_COMMAND ./bootstrap.sh --prefix=<INSTALL_DIR> --with-libraries=thread
    BUILD_IN_SOURCE 1
    BUILD_COMMAND ./b2 link=static runtime-link=static ${ADDRESS_MODEL} install
    INSTALL_DIR ${EXTLIB_INSTALL_PREFIX}
    INSTALL_COMMAND ""
    )

ExternalProject_Add(
    gmock_proj
    SVN_REPOSITORY http://googlemock.googlecode.com/svn/trunk/
    SVN_REVISION -r HEAD
    #SVN_USERNAME
	#SVN_PASSWORD
	CMAKE_ARGS ${CMAKE_FLAGS}
    INSTALL_DIR ${EXTLIB_INSTALL_PREFIX}
    INSTALL_COMMAND ""
    )
ExternalProject_Get_Property(gmock_proj SOURCE_DIR)
SET(GMOCK_INC ${SOURCE_DIR}/include)
SET(GTEST_INC ${SOURCE_DIR}/gtest/include)
ExternalProject_Get_Property(gmock_proj BINARY_DIR)
SET(GMOCK_LIB ${BINARY_DIR})
SET(GTEST_LIB ${BINARY_DIR}/gtest)

ADD_SUBDIRECTORY(libattrapi)

EXPORT(PACKAGE extlib)
EXPORT(TARGETS attrapi FILE extlib-exports.cmake)
CONFIGURE_FILE(extlib-config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/extlib-config.cmake)
