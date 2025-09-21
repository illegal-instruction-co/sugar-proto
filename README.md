# sugar-proto

`sugar-proto` is a lightweight wrapper and plugin for Protocol Buffers. The main goal is to make working with protobuf in C++ feel simpler and more intuitive, closer to plain struct and stl like APIs.  

## Why
Using the default protobuf C++ API can feel verbose and rigid. With sugar-proto, you can interact with your messages in a much friendlier way, for example:

```cpp
u.id = 123;
u.tags.push_back("cpp");
u.profile.city = "Berlin";

cout << u.id << endl;
cout << u.tags[0] << endl;
cout << u.profile.city << endl;
```  

Instead of juggling with reflection and getters/setters everywhere, you get a concise and readable interface.  

## Installation

Build and install with:

```bash
git clone https://github.com/illegal-instruction-co/sugar-proto.git
cd sugar-proto
mkdir build && cd build
cmake ..
make -j
sudo make install
```

After installation:  
- The plugin binary will be placed at `/usr/local/bin/protoc-gen-sugar`  
- The runtime header will be installed at `/usr/local/include/sugar/sugar_runtime.h`  
- CMake config files will be available so you can simply use `find_package(sugar-proto REQUIRED)` in your own projects  

## Usage

In your project’s `CMakeLists.txt`:

```cmake
find_package(Protobuf REQUIRED)
find_package(sugar-proto REQUIRED)

set(PROTO_FILE ${CMAKE_SOURCE_DIR}/my.proto)
set(GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

add_custom_command(
    OUTPUT ${GENERATED_DIR}/my.pb.cc ${GENERATED_DIR}/my.pb.h
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
        --cpp_out=${GENERATED_DIR}
        -I ${CMAKE_SOURCE_DIR}
        ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
)

add_custom_command(
    OUTPUT ${GENERATED_DIR}/my.sugar.h
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
        --sugar_out=${GENERATED_DIR}
        -I ${CMAKE_SOURCE_DIR}
        ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
)

add_executable(my_app
    main.cpp
    ${GENERATED_DIR}/my.pb.cc
)

target_include_directories(my_app PRIVATE ${GENERATED_DIR})
target_link_libraries(my_app PRIVATE Protobuf::libprotobuf)
```

## Example Code

```cpp
#include "my.pb.h"
#include "my.sugar.h"
#include <iostream>

int main() {
    MyMessage msg;
    MyMessageWrapped u(msg);

    u.id = 42;
    u.name = "hello world";
    u.tags.push_back("first");
    u.tags.push_back("test");

    std::cout << msg.DebugString() << std::endl;

    std::cout << u.id << std::endl;
    std::cout << u.name << std::endl;

    return 0;
}
```

## Notes

- Minimum required CMake version is 3.16 (recommended 3.21 or newer)  
- Currently supports: scalar fields, repeated fields, maps, and oneofs  
- API is not considered stable yet, small breaking changes may occur  

If you run into issues or missing features, please open an issue. The ultimate goal is to make protobuf usage in C++ enjoyable and developer friendly.