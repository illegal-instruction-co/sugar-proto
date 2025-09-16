cmake_minimum_required(VERSION 3.16)
project(example_app LANGUAGES CXX)

find_package(Protobuf REQUIRED)

set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/user.proto)
set(GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

add_custom_command(
    OUTPUT ${GENERATED_DIR}/user.pb.cc ${GENERATED_DIR}/user.pb.h
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
        --cpp_out=${GENERATED_DIR}
        -I ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
)

add_custom_command(
    OUTPUT ${GENERATED_DIR}/user.sugar.h
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
        --plugin=protoc-gen-sugar=$<TARGET_FILE:protoc-gen-sugar>
        --sugar_out=${GENERATED_DIR}
        -I ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROTO_FILE}
    DEPENDS protoc-gen-sugar ${PROTO_FILE}
)

add_custom_target(generate_user ALL
    DEPENDS
        ${GENERATED_DIR}/user.pb.cc
        ${GENERATED_DIR}/user.pb.h
        ${GENERATED_DIR}/user.sugar.h
)

add_executable(example_app
    main.cpp
    ${GENERATED_DIR}/user.pb.cc
)

add_dependencies(example_app generate_user)

target_include_directories(example_app PRIVATE
    ${GENERATED_DIR}
    ${Protobuf_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(example_app PRIVATE ${Protobuf_LIBRARIES})
