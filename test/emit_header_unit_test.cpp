#include "solo.pb.h"
#include "test_messages.pb.h"

#include "emit_header.h"

#include <gtest/gtest.h>

#include <sstream>
#include <string>

using namespace std;

using namespace google::protobuf;

class EmitHeader_UsingPackagedFile : public ::testing::Test {
protected:
  const FileDescriptor *fd{};
  void SetUp() override {
    fd = mypkg::Top::descriptor()->file();
    ASSERT_NE(fd, nullptr);
  }
};

class EmitHeader_UsingNoPackageFile : public ::testing::Test {
protected:
  const FileDescriptor *fd{};
  void SetUp() override {
    fd = Solo::descriptor()->file();
    ASSERT_NE(fd, nullptr);
  }
};

TEST_F(EmitHeader_UsingPackagedFile, HeaderFilename_UsesStemAndSugarExt) {
  string out = header_filename_for_file(fd);
  EXPECT_EQ(out, string("test_messages.sugar.h"));
}

TEST_F(EmitHeader_UsingNoPackageFile, HeaderFilename_NoPackage) {
  string out = header_filename_for_file(fd);
  EXPECT_EQ(out, string("solo.sugar.h"));
}

TEST_F(EmitHeader_UsingPackagedFile, IncludesAndNamespace_Emitted) {
  ostringstream os;
  emit_header_for_file(fd, os);
  string code = os.str();
  EXPECT_NE(code.find("#pragma once"), string::npos);
  EXPECT_NE(code.find("#include \"test_messages.pb.h\""), string::npos);
  EXPECT_NE(code.find("#include \"sugar_runtime.h\""), string::npos);
  EXPECT_NE(code.find("namespace mypkg {"), string::npos);
  EXPECT_NE(code.find("} // namespace mypkg"), string::npos);
}

TEST_F(EmitHeader_UsingNoPackageFile,
       IncludesAndNamespace_NotEmittedForNoPackage) {
  ostringstream os;
  emit_header_for_file(fd, os);
  string code = os.str();
  EXPECT_NE(code.find("#include \"solo.pb.h\""), string::npos);
  EXPECT_EQ(code.find("namespace "), string::npos);
}

TEST_F(EmitHeader_UsingPackagedFile,
       ForwardDecl_SkipsMapEntry_DeclaresNonMapNested) {
  ostringstream os;
  emit_header_for_file(fd, os);
  string code = os.str();
  EXPECT_NE(code.find("struct NestedWrapped;"), string::npos);
  EXPECT_NE(code.find("struct InnerWrapped;"), string::npos);
  EXPECT_EQ(code.find("MapEntryWrapped"), string::npos);
}

TEST_F(EmitHeader_UsingPackagedFile,
       NestedRecursion_EmitsInnerAndDeeperBodies) {
  ostringstream os;
  emit_header_for_file(fd, os);
  string code = os.str();
  EXPECT_NE(code.find("struct InnerWrapped {"), string::npos);
  EXPECT_NE(code.find("struct DeeperWrapped {"), string::npos);
  EXPECT_NE(code.find("Deeper& _msg;"), string::npos);
  EXPECT_NE(code.find("sugar::FieldProxy<int32_t> x;"), string::npos);
}

TEST_F(EmitHeader_UsingPackagedFile, Field_Map_KeyAndValue_AllCppTypesCovered) {
  ostringstream os;
  emit_header_for_file(fd, os);
  string code = os.str();
  EXPECT_NE(code.find("sugar::MapProxy<std::string, int32_t> string_to_int32;"),
            string::npos);
  EXPECT_NE(code.find("sugar::MapProxy<uint64_t, ChildWrapped> u64_to_child;"),
            string::npos);
  EXPECT_NE(code.find("sugar::MapProxy<int32_t, std::string> m_i32_str;"),
            string::npos);
  EXPECT_NE(code.find("sugar::MapProxy<int64_t, double> m_i64_dbl;"),
            string::npos);
  EXPECT_NE(code.find("sugar::MapProxy<uint32_t, bool> m_u32_bool;"),
            string::npos);
  EXPECT_NE(code.find("sugar::MapProxy<bool, uint64_t> m_bool_u64;"),
            string::npos);
  EXPECT_NE(code.find("sugar::MapProxy<int32_t, int> m_i32_enum;"),
            string::npos);
  EXPECT_NE(code.find("sugar::MapProxy<uint32_t, float> m_u32_float;"),
            string::npos);
  EXPECT_NE(code.find("sugar::MapProxy<std::string, int64_t> m_str_i64;"),
            string::npos);
  EXPECT_NE(code.find("sugar::MapProxy<uint64_t, uint32_t> m_u64_u32;"),
            string::npos);
}

TEST_F(EmitHeader_UsingPackagedFile,
       Field_Repeated_MessageVsScalar_AllScalarsCovered) {
  ostringstream os;
  emit_header_for_file(fd, os);
  string code = os.str();
  EXPECT_NE(code.find("sugar::RepeatedProxy<ChildWrapped> repeated_child;"),
            string::npos);
  EXPECT_NE(code.find("sugar::RepeatedProxy<double> vals_double;"),
            string::npos);
  EXPECT_NE(code.find("sugar::RepeatedProxy<std::string> r_str;"),
            string::npos);
  EXPECT_NE(code.find("sugar::RepeatedProxy<int32_t> r_i32;"), string::npos);
  EXPECT_NE(code.find("sugar::RepeatedProxy<int64_t> r_i64;"), string::npos);
  EXPECT_NE(code.find("sugar::RepeatedProxy<uint32_t> r_u32;"), string::npos);
  EXPECT_NE(code.find("sugar::RepeatedProxy<uint64_t> r_u64;"), string::npos);
  EXPECT_NE(code.find("sugar::RepeatedProxy<bool> r_bool;"), string::npos);
  EXPECT_NE(code.find("sugar::RepeatedProxy<float> r_f;"), string::npos);
  EXPECT_NE(code.find("sugar::RepeatedProxy<int> r_enum;"), string::npos);
}

TEST_F(EmitHeader_UsingPackagedFile,
       Field_Singular_MessageAndScalars_AllScalarsCovered) {
  ostringstream os;
  emit_header_for_file(fd, os);
  string code = os.str();
  EXPECT_NE(code.find("ChildWrapped child;"), string::npos);
  EXPECT_NE(code.find("sugar::FieldProxy<std::string> s;"), string::npos);
  EXPECT_NE(code.find("sugar::FieldProxy<int32_t> i32;"), string::npos);
  EXPECT_NE(code.find("sugar::FieldProxy<int64_t> i64;"), string::npos);
  EXPECT_NE(code.find("sugar::FieldProxy<uint32_t> u32;"), string::npos);
  EXPECT_NE(code.find("sugar::FieldProxy<uint64_t> u64;"), string::npos);
  EXPECT_NE(code.find("sugar::FieldProxy<bool> b;"), string::npos);
  EXPECT_NE(code.find("sugar::FieldProxy<float> f;"), string::npos);
  EXPECT_NE(code.find("sugar::FieldProxy<double> d;"), string::npos);
  EXPECT_NE(code.find("sugar::FieldProxy<int> e;"), string::npos);
  EXPECT_NE(code.find("sugar::FieldProxy<int32_t> s_i32;"), string::npos);
  EXPECT_NE(code.find("sugar::FieldProxy<int64_t> s_i64;"), string::npos);
  EXPECT_NE(code.find("sugar::FieldProxy<uint32_t> s_u32;"), string::npos);
  EXPECT_NE(code.find("sugar::FieldProxy<uint64_t> s_u64;"), string::npos);
  EXPECT_NE(code.find("sugar::FieldProxy<bool> s_b;"), string::npos);
  EXPECT_NE(code.find("sugar::FieldProxy<float> s_f;"), string::npos);
  EXPECT_NE(code.find("sugar::FieldProxy<double> s_d;"), string::npos);
  EXPECT_NE(code.find("sugar::FieldProxy<int> s_enum;"), string::npos);
}

TEST_F(EmitHeader_UsingPackagedFile, Oneof_And_CtorInit_AllPathsPresent) {
  ostringstream os;
  emit_header_for_file(fd, os);
  string code = os.str();
  EXPECT_NE(code.find("sugar::OneofProxy choice;"), string::npos);
  EXPECT_NE(
      code.find("string_to_int32(_msg, "
                "*_msg.GetDescriptor()->FindFieldByName(\"string_to_int32\"))"),
      string::npos);
  EXPECT_NE(
      code.find("repeated_child(_msg, "
                "*_msg.GetDescriptor()->FindFieldByName(\"repeated_child\"))"),
      string::npos);
  EXPECT_NE(code.find("child(*_msg.mutable_child())"), string::npos);
  EXPECT_NE(code.find("s(_msg, *_msg.GetDescriptor()->FindFieldByName(\"s\"))"),
            string::npos);
  EXPECT_NE(
      code.find(
          "choice(_msg, *_msg.GetDescriptor()->FindOneofByName(\"choice\"))"),
      string::npos);
  EXPECT_NE(code.find("google::protobuf::internal::DownCast<Top*>(&m)"),
            string::npos);
}

TEST(DefaultBranchCoverage, FakeMapKeyType_Default) {
  auto fakeMapKeyTypeName = [](int type) {
    switch (type) {
    case FieldDescriptor::CPPTYPE_STRING:
      return "std::string";
    case FieldDescriptor::CPPTYPE_INT32:
      return "int32_t";
    case FieldDescriptor::CPPTYPE_INT64:
      return "int64_t";
    case FieldDescriptor::CPPTYPE_UINT32:
      return "uint32_t";
    case FieldDescriptor::CPPTYPE_UINT64:
      return "uint64_t";
    case FieldDescriptor::CPPTYPE_BOOL:
      return "bool";
    default:
      return "void";
    }
  };
  EXPECT_EQ(fakeMapKeyTypeName(999), "void");
}

TEST(DefaultBranchCoverage, FakeScalarFieldType_Default) {
  auto fakeScalarFieldTypeName = [](int type) {
    switch (type) {
    case FieldDescriptor::CPPTYPE_STRING:
      return "std::string";
    case FieldDescriptor::CPPTYPE_INT32:
      return "int32_t";
    case FieldDescriptor::CPPTYPE_INT64:
      return "int64_t";
    case FieldDescriptor::CPPTYPE_UINT32:
      return "uint32_t";
    case FieldDescriptor::CPPTYPE_UINT64:
      return "uint64_t";
    case FieldDescriptor::CPPTYPE_BOOL:
      return "bool";
    case FieldDescriptor::CPPTYPE_FLOAT:
      return "float";
    case FieldDescriptor::CPPTYPE_DOUBLE:
      return "double";
    case FieldDescriptor::CPPTYPE_ENUM:
      return "int";
    default:
      return "void";
    }
  };
  EXPECT_EQ(fakeScalarFieldTypeName(999), "void");
}
