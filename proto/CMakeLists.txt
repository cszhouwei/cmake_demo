FIND_PACKAGE(extlib REQUIRED)

ADD_CUSTOM_COMMAND(OUTPUT demo.pb.cc demo.pb.h
    COMMAND ${CMAKE_BINARY_DIR}/extlib/install/bin/protoc
    ARGS --cpp_out . ${CMAKE_CURRENT_SOURCE_DIR}/demo.proto -I${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS demo.proto)

ADD_LIBRARY(proto demo.pb.cc)
TARGET_LINK_LIBRARIES(proto protobuf pthread)
