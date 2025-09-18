cmake_minimum_required(VERSION 3.16)
project(sugar_proto LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Protobuf REQUIRED)

add_executable(protoc-gen-sugar
    src/protoc_gen_sugar.cpp
    src/emit_header.cpp
    src/emit_header.h
)
target_include_directories(protoc-gen-sugar PRIVATE ${Protobuf_INCLUDE_DIRS} src)
target_link_libraries(protoc-gen-sugar PRIVATE
    ${Protobuf_LIBRARIES}
    protobuf::libprotoc
)

include(GNUInstallDirs)

install(TARGETS protoc-gen-sugar
    EXPORT sugar-protoTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES
    src/sugar_runtime.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sugar
)

install(EXPORT sugar-protoTargets
    FILE sugar-protoTargets.cmake
    NAMESPACE sugar-proto::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sugar-proto
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_SOURCE_DIR}/cmake/sugar-protoConfig.cmake.in
    ${CMAKE_BINARY_DIR}/sugar-protoConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sugar-proto
)

install(FILES
    ${CMAKE_BINARY_DIR}/sugar-protoConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sugar-proto
)

add_subdirectory(example)
