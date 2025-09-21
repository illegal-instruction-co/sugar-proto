option(BUILD_TESTS "Build the tests" ON)

if(BUILD_TESTS)
    find_package(Protobuf REQUIRED)
    find_package(GTest REQUIRED)

    include_directories(
        ${GTEST_INCLUDE_DIRS}
        ${Protobuf_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}
    )

    link_libraries(
        ${Protobuf_LIBRARIES}
        GTest::GTest
        GTest::Main
        pthread
    )

    set(PROTO_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/test_messages.proto
        ${CMAKE_CURRENT_SOURCE_DIR}/solo.proto
    )
    protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILE})

    add_executable(unit_test_emit_header
        emit_header_unit_test.cpp
        ${PROTO_SRCS}
        ${PROTO_HDRS}
        $<TARGET_OBJECTS:emit_header_obj>
    )

    add_executable(unit_test_sugar_runtime
        sugar_runtime_unit_test.cpp
        ${PROTO_SRCS}
        ${PROTO_HDRS}
    )
endif()
