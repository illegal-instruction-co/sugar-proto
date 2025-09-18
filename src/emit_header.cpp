#include "emit_header.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>

#include <ostream>
#include <string>

using google::protobuf::Descriptor;
using google::protobuf::FieldDescriptor;

static void emit_message_wrapper(const Descriptor *d, std::ostream &os);

static void emit_field_member(const FieldDescriptor *f, std::ostream &os) {
  const std::string &fname = f->name();

  if (f->is_map()) {
    const auto *kv = f->message_type();
    const auto *kf = kv->FindFieldByName("key");
    const auto *vf = kv->FindFieldByName("value");

    std::string ktype = "void";
    switch (kf->cpp_type()) {
    case FieldDescriptor::CPPTYPE_STRING:
      ktype = "std::string";
      break;
    case FieldDescriptor::CPPTYPE_INT32:
      ktype = "int32_t";
      break;
    case FieldDescriptor::CPPTYPE_INT64:
      ktype = "int64_t";
      break;
    case FieldDescriptor::CPPTYPE_UINT32:
      ktype = "uint32_t";
      break;
    case FieldDescriptor::CPPTYPE_UINT64:
      ktype = "uint64_t";
      break;
    case FieldDescriptor::CPPTYPE_BOOL:
      ktype = "bool";
      break;
    default:
      break;
    }

    std::string vtype = "void";
    switch (vf->cpp_type()) {
    case FieldDescriptor::CPPTYPE_STRING:
      vtype = "std::string";
      break;
    case FieldDescriptor::CPPTYPE_INT32:
      vtype = "int32_t";
      break;
    case FieldDescriptor::CPPTYPE_INT64:
      vtype = "int64_t";
      break;
    case FieldDescriptor::CPPTYPE_UINT32:
      vtype = "uint32_t";
      break;
    case FieldDescriptor::CPPTYPE_UINT64:
      vtype = "uint64_t";
      break;
    case FieldDescriptor::CPPTYPE_BOOL:
      vtype = "bool";
      break;
    case FieldDescriptor::CPPTYPE_FLOAT:
      vtype = "float";
      break;
    case FieldDescriptor::CPPTYPE_DOUBLE:
      vtype = "double";
      break;
    case FieldDescriptor::CPPTYPE_ENUM:
      vtype = "int";
      break;
    case FieldDescriptor::CPPTYPE_MESSAGE:
      vtype = vf->message_type()->name() + std::string("Wrapped");
      break;
    default:
      break;
    }

    os << "    sugar::MapProxy<" << ktype << ", " << vtype << "> " << fname
       << ";\n";
    return;
  }

  if (f->is_repeated()) {
    if (f->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
      os << "    sugar::RepeatedProxy<" << f->message_type()->name()
         << "Wrapped> " << fname << ";\n";
    } else {
      std::string elemType = "void";
      switch (f->cpp_type()) {
      case FieldDescriptor::CPPTYPE_STRING:
        elemType = "std::string";
        break;
      case FieldDescriptor::CPPTYPE_INT32:
        elemType = "int32_t";
        break;
      case FieldDescriptor::CPPTYPE_INT64:
        elemType = "int64_t";
        break;
      case FieldDescriptor::CPPTYPE_UINT32:
        elemType = "uint32_t";
        break;
      case FieldDescriptor::CPPTYPE_UINT64:
        elemType = "uint64_t";
        break;
      case FieldDescriptor::CPPTYPE_BOOL:
        elemType = "bool";
        break;
      case FieldDescriptor::CPPTYPE_FLOAT:
        elemType = "float";
        break;
      case FieldDescriptor::CPPTYPE_DOUBLE:
        elemType = "double";
        break;
      case FieldDescriptor::CPPTYPE_ENUM:
        elemType = "int";
        break;
      default:
        break;
      }
      os << "    sugar::RepeatedProxy<" << elemType << "> " << fname << ";\n";
    }
    return;
  }

  if (f->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
    const auto *md = f->message_type();
    os << "    " << md->name() << "Wrapped " << fname << ";\n";
    return;
  }

  std::string fieldType = "void";
  switch (f->cpp_type()) {
  case FieldDescriptor::CPPTYPE_STRING:
    fieldType = "std::string";
    break;
  case FieldDescriptor::CPPTYPE_INT32:
    fieldType = "int32_t";
    break;
  case FieldDescriptor::CPPTYPE_INT64:
    fieldType = "int64_t";
    break;
  case FieldDescriptor::CPPTYPE_UINT32:
    fieldType = "uint32_t";
    break;
  case FieldDescriptor::CPPTYPE_UINT64:
    fieldType = "uint64_t";
    break;
  case FieldDescriptor::CPPTYPE_BOOL:
    fieldType = "bool";
    break;
  case FieldDescriptor::CPPTYPE_FLOAT:
    fieldType = "float";
    break;
  case FieldDescriptor::CPPTYPE_DOUBLE:
    fieldType = "double";
    break;
  case FieldDescriptor::CPPTYPE_ENUM:
    fieldType = "int";
    break;
  default:
    break;
  }
  os << "    sugar::FieldProxy<" << fieldType << "> " << fname << ";\n";
}

static void emit_ctor_init(const Descriptor *d, std::ostream &os) {
  // 1) Normal ctor (Foo& m)
  os << "    explicit " << d->name() << "Wrapped(" << d->name() << "& m)\n";
  os << "        : _msg(m)";
  for (int i = 0; i < d->field_count(); ++i) {
    const auto *f = d->field(i);
    const std::string &fname = f->name();
    if (f->is_map())
      os << ",\n          " << fname
         << "(_msg, *_msg.GetDescriptor()->FindFieldByName(\"" << fname
         << "\"))";
    else if (f->is_repeated())
      os << ",\n          " << fname
         << "(_msg, *_msg.GetDescriptor()->FindFieldByName(\"" << fname
         << "\"))";
    else if (f->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE)
      os << ",\n          " << fname << "(*_msg.mutable_" << fname << "())";
    else
      os << ",\n          " << fname
         << "(_msg, *_msg.GetDescriptor()->FindFieldByName(\"" << fname
         << "\"))";
  }
  for (int i = 0; i < d->oneof_decl_count(); ++i) {
    const auto *o = d->oneof_decl(i);
    os << ",\n          " << o->name()
       << "(_msg, *_msg.GetDescriptor()->FindOneofByName(\"" << o->name()
       << "\"))";
  }
  os << " {}\n";

  os << "    explicit " << d->name()
     << "Wrapped(google::protobuf::Message& m)\n"
     << "        : " << d->name()
     << "Wrapped(*google::protobuf::internal::DownCast<" << d->name()
     << "*>(&m)) {}\n";
}

static void emit_oneofs(const Descriptor *d, std::ostream &os) {
  for (int i = 0; i < d->oneof_decl_count(); ++i) {
    const auto *o = d->oneof_decl(i);
    os << "    sugar::OneofProxy " << o->name() << ";\n";
  }
}

static void emit_message_wrapper(const Descriptor *d, std::ostream &os) {
  for (int i = 0; i < d->nested_type_count(); ++i) {
    const auto *nested = d->nested_type(i);
    if (nested->options().map_entry())
      continue;
    os << "struct " << nested->name() << "Wrapped;\n";
  }

  os << "struct " << d->name() << "Wrapped {\n";
  os << "    " << d->name() << "& _msg;\n";

  for (int i = 0; i < d->field_count(); ++i)
    emit_field_member(d->field(i), os);

  emit_oneofs(d, os);
  emit_ctor_init(d, os);
  os << "};\n\n";

  for (int i = 0; i < d->nested_type_count(); ++i) {
    const auto *nested = d->nested_type(i);
    if (nested->options().map_entry())
      continue;
    emit_message_wrapper(nested, os);
  }
}

void emit_header_for_file(const google::protobuf::FileDescriptor *file,
                          std::ostream &os) {
  os << "#pragma once\n";
  os << "#include \"" << file->name().substr(0, file->name().find_last_of('.'))
     << ".pb.h\"\n";
  os << "#include \"sugar_runtime.h\"\n\n";

  if (!file->package().empty())
    os << "namespace " << file->package() << " {\n";

  for (int i = 0; i < file->message_type_count(); ++i)
    emit_message_wrapper(file->message_type(i), os);

  if (!file->package().empty())
    os << "} // namespace " << file->package() << "\n";
}

std::string
header_filename_for_file(const google::protobuf::FileDescriptor *file) {
  const std::string stem =
      file->name().substr(0, file->name().find_last_of('.'));
  return stem + ".sugar.h";
}
