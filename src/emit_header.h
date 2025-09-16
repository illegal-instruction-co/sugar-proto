#pragma once

#include <google/protobuf/descriptor.h>

#include <ostream>
#include <string>

void emit_header_for_file(const google::protobuf::FileDescriptor *,
                          std::ostream &);
std::string header_filename_for_file(const google::protobuf::FileDescriptor *);
