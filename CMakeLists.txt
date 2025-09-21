cmake_minimum_required(VERSION 3.16)
project(sugar_proto LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Protobuf REQUIRED)

option(BUILD_TESTS "Build the tests" ON)
option(COVERAGE "Enable coverage reporting" OFF)

if(COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS "Building with coverage flags")
        add_compile_options(-O0 -g --coverage -fprofile-arcs -ftest-coverage)
        add_link_options(--coverage)
    endif()
endif()

add_library(protoc_gen_sugar_obj OBJECT src/protoc_gen_sugar.cpp)
add_library(emit_header_obj OBJECT src/emit_header.cpp src/emit_header.h)

add_executable(protoc-gen-sugar
    $<TARGET_OBJECTS:protoc_gen_sugar_obj>
    $<TARGET_OBJECTS:emit_header_obj>
)
target_include_directories(protoc-gen-sugar PRIVATE
    ${Protobuf_INCLUDE_DIRS}
    src
)
target_link_libraries(protoc-gen-sugar PRIVATE
    protobuf::libprotobuf
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

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
