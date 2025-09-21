#include "sugar_runtime.h"
#include "test_messages.pb.h"

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

using namespace std;

using namespace sugar;

namespace sugar {
template <typename MsgT> class MessageWrapped {
public:
  explicit MessageWrapped(::google::protobuf::Message &m) : _msg(m) {}
  ::google::protobuf::Message &_msg;
};
} // namespace sugar

namespace {
using FD = google::protobuf::FieldDescriptor;
using Msg = google::protobuf::Message;
using Top = mypkg::Top;

static const FD *F(const google::protobuf::Descriptor *d, string name) {
  return d->FindFieldByName(name);
}

template <typename T> static FieldProxy<T> FP(Msg &m, const FD *f) {
  return FieldProxy<T>(m, *f);
}
template <typename T> static RepeatedProxy<T> RP(Msg &m, const FD *f) {
  return RepeatedProxy<T>(m, *f);
}
template <typename K, typename V>
static MapProxy<K, V> MP(Msg &m, const FD *f) {
  return MapProxy<K, V>(m, *f);
}

TEST(FieldProxy_AssignAndRead, ScalarsAndEnum) {
  Top msg;
  auto *d = msg.GetDescriptor();
  FP<int32_t>(msg, F(d, "i32")) = -5;
  EXPECT_EQ(static_cast<int32_t>(FP<int32_t>(msg, F(d, "i32"))), -5);
  FP<uint32_t>(msg, F(d, "u32")) = 7u;
  EXPECT_EQ(static_cast<uint32_t>(FP<uint32_t>(msg, F(d, "u32"))), 7u);
  FP<int64_t>(msg, F(d, "i64")) = -12345;
  EXPECT_EQ(static_cast<int64_t>(FP<int64_t>(msg, F(d, "i64"))), -12345);
  FP<uint64_t>(msg, F(d, "u64")) = 12345ull;
  EXPECT_EQ(static_cast<uint64_t>(FP<uint64_t>(msg, F(d, "u64"))), 12345ull);
  FP<float>(msg, F(d, "f")) = 1.5f;
  EXPECT_FLOAT_EQ(static_cast<float>(FP<float>(msg, F(d, "f"))), 1.5f);
  FP<double>(msg, F(d, "d")) = 2.5;
  EXPECT_DOUBLE_EQ(static_cast<double>(FP<double>(msg, F(d, "d"))), 2.5);
  FP<bool>(msg, F(d, "b")) = true;
  EXPECT_TRUE(static_cast<bool>(FP<bool>(msg, F(d, "b"))));
  FP<string>(msg, F(d, "s")) = "abc";
  EXPECT_EQ(static_cast<string>(FP<string>(msg, F(d, "s"))), "abc");
  FP<int32_t>(msg, F(d, "e")) = static_cast<int>(mypkg::ONE);
  EXPECT_EQ(static_cast<int32_t>(FP<int32_t>(msg, F(d, "e"))), 1);
}

TEST(FieldProxy_AssignInvalid, WrongTypes) {
  Top msg;
  auto *d = msg.GetDescriptor();
  EXPECT_THROW(FP<int32_t>(msg, F(d, "s")) = 5, runtime_error);
  EXPECT_THROW(FP<string>(msg, F(d, "i32")) = "bad", runtime_error);
  EXPECT_THROW(FP<int32_t>(msg, F(d, "child")) = 1, runtime_error);
  EXPECT_THROW(FP<int32_t>(msg, F(d, "e")) = 999, runtime_error);
}

TEST(FieldProxy_Read_NegativePaths, WrongTypeAndRepeatedRead) {
  Top msg;
  auto *d = msg.GetDescriptor();
  EXPECT_THROW(static_cast<int32_t>(FP<int32_t>(msg, F(d, "r_i32"))),
               runtime_error);
  EXPECT_THROW(static_cast<bool>(FP<bool>(msg, F(d, "i32"))), runtime_error);
}

TEST(RepeatedProxy_PushBackAndIterate, Scalars) {
  Top msg;
  auto *d = msg.GetDescriptor();
  auto rr = RP<int32_t>(msg, F(d, "r_i32"));
  rr.push_back(-1);
  rr.push_back(-2);
  EXPECT_EQ(rr.size(), 2);
  EXPECT_EQ(rr[0], -1);
  EXPECT_EQ(rr.back(), -2);
  int sum = 0;
  for (auto v : rr)
    sum += v;
  EXPECT_EQ(sum, -3);
}

TEST(RepeatedProxy_PushBackMessage, FnInitAndMessageErrorBranch) {
  Top msg;
  auto *d = msg.GetDescriptor();
  auto rr = RP<MessageWrapped<mypkg::Child>>(msg, F(d, "repeated_child"));
  auto &child = *msg.add_repeated_child();
  child.set_child_str("ok");
  EXPECT_EQ(rr.size(), 1);
  EXPECT_EQ(msg.repeated_child(0).child_str(), "ok");
  EXPECT_THROW((RP<MessageWrapped<mypkg::Child>>(msg, F(d, "repeated_child"))
                    .push_back(1)),
               runtime_error);
}

TEST(RepeatedProxy_Set, AllScalarSetAndMessageSetError) {
  Top msg;
  auto *d = msg.GetDescriptor();
  auto ri32 = RP<int32_t>(msg, F(d, "r_i32"));
  ri32.push_back(0);
  ri32.set(0, -9);
  EXPECT_EQ(ri32[0], -9);
  EXPECT_THROW(
      (RP<MessageWrapped<mypkg::Child>>(msg, F(d, "repeated_child")).set(0, 5)),
      runtime_error);
}

TEST(MapProxy_ConstructAndSet, HappyPathsAndMessageValueError) {
  Top msg;
  auto *d = msg.GetDescriptor();
  auto m1 = MP<string, int>(msg, F(d, "string_to_int32"));
  m1.set("k", 42);
  auto m2 = MP<int, string>(msg, F(d, "m_i32_str"));
  m2.set(7, "v");
  auto m3 = MP<int64_t, double>(msg, F(d, "m_i64_dbl"));
  m3.set(1, 3.14);
  auto m4 = MP<uint32_t, bool>(msg, F(d, "m_u32_bool"));
  m4.set(1u, true);
  auto m5 = MP<bool, uint64_t>(msg, F(d, "m_bool_u64"));
  m5.set(false, 99ull);
  auto m6 = MP<uint32_t, float>(msg, F(d, "m_u32_float"));
  m6.set(2u, 2.5f);
  auto m7 = MP<string, int64_t>(msg, F(d, "m_str_i64"));
  m7.set("kk", -9);
  auto m8 = MP<uint64_t, uint32_t>(msg, F(d, "m_u64_u32"));
  m8.set(10ull, 20u);
  EXPECT_THROW(
      (MP<uint64_t, MessageWrapped<mypkg::Child>>(msg, F(d, "u64_to_child"))
           .set(1ull, MessageWrapped<mypkg::Child>(*msg.mutable_child()))),
      runtime_error);
}

TEST(OneofProxy_Usage, ActiveClearAndSet) {
  Top msg;
  auto *d = msg.GetDescriptor();
  OneofProxy o(msg, *d->FindOneofByName("choice"));
  EXPECT_EQ(o.active_field(), nullptr);
  o.set("o_i32", [](Msg &m, const FD &f) { FieldProxy<int32_t>(m, f) = 5; });
  EXPECT_EQ(o.active_field()->name(), "o_i32");
  o.clear();
  EXPECT_EQ(o.active_field(), nullptr);
  EXPECT_THROW(o.set("bad", [](Msg &, const FD &) {}), runtime_error);
}

TEST(FieldProxy_ReadMismatchBranches, UnsignedOnSignedAndFloatOnIntEtc) {
  Top msg;
  auto *d = msg.GetDescriptor();
  FP<int32_t>(msg, F(d, "i32")) = -5;
  EXPECT_THROW(static_cast<uint32_t>(FP<uint32_t>(msg, F(d, "i32"))),
               runtime_error);
  FP<uint32_t>(msg, F(d, "u32")) = 7u;
  EXPECT_THROW(static_cast<int32_t>(FP<int32_t>(msg, F(d, "u32"))),
               runtime_error);
  EXPECT_THROW(static_cast<float>(FP<float>(msg, F(d, "i32"))), runtime_error);
}

TEST(FieldProxy_AssignAndRead, StringAndBool) {
  Top msg;
  auto *d = msg.GetDescriptor();
  FP<string>(msg, F(d, "s")) = "xyz";
  EXPECT_EQ(static_cast<string>(FP<string>(msg, F(d, "s"))), "xyz");
  FP<bool>(msg, F(d, "b")) = true;
  EXPECT_TRUE(static_cast<bool>(FP<bool>(msg, F(d, "b"))));
}

TEST(FieldProxy_AssignInvalid, MessageAssignNotAllowed) {
  Top msg;
  auto *d = msg.GetDescriptor();
  EXPECT_THROW(FP<int32_t>(msg, F(d, "child")) = 123, runtime_error);
}

TEST(RepeatedProxy_PushBack, AllScalarTypes) {
  Top msg;
  auto *d = msg.GetDescriptor();
  RP<int32_t>(msg, F(d, "r_i32")).push_back(-10);
  RP<int64_t>(msg, F(d, "r_i64")).push_back(-1000);
  RP<uint32_t>(msg, F(d, "r_u32")).push_back(55u);
  RP<uint64_t>(msg, F(d, "r_u64")).push_back(999ull);
  RP<float>(msg, F(d, "r_f")).push_back(1.25f);
  RP<double>(msg, F(d, "vals_double")).push_back(2.5);
  RP<bool>(msg, F(d, "r_bool")).push_back(true);
  RP<string>(msg, F(d, "r_str")).push_back("hello");
  RP<int32_t>(msg, F(d, "r_enum")).push_back(static_cast<int>(mypkg::ZERO));
  EXPECT_EQ(msg.r_str(0), "hello");
}

TEST(RepeatedProxy_Set, Scalars) {
  Top msg;
  auto *d = msg.GetDescriptor();
  auto r = RP<uint32_t>(msg, F(d, "r_u32"));
  r.push_back(1u);
  r.set(0, 42u);
  EXPECT_EQ(r[0], 42u);
}

TEST(MapProxy_SetField, AllTypes) {
  Top msg;
  auto *d = msg.GetDescriptor();
  MP<int32_t, string>(msg, F(d, "m_i32_str")).set(1, "x");
  MP<int64_t, double>(msg, F(d, "m_i64_dbl")).set(123, 4.56);
  MP<uint32_t, bool>(msg, F(d, "m_u32_bool")).set(2u, true);
  MP<bool, uint64_t>(msg, F(d, "m_bool_u64")).set(false, 777ull);
  MP<string, int64_t>(msg, F(d, "m_str_i64")).set("k", -5);
  MP<uint64_t, uint32_t>(msg, F(d, "m_u64_u32")).set(100ull, 200u);
  MP<uint32_t, float>(msg, F(d, "m_u32_float")).set(3u, 1.5f);
  MP<int32_t, int>(msg, F(d, "m_i32_enum"))
      .set(10, static_cast<int>(mypkg::ONE));
  EXPECT_THROW(
      (MP<uint64_t, MessageWrapped<mypkg::Child>>(msg, F(d, "u64_to_child"))
           .set(5ull, MessageWrapped<mypkg::Child>(*msg.mutable_child()))),
      runtime_error);
}

TEST(FieldProxy_ReadMismatchBranches, WrongCasts) {
  Top msg;
  auto *d = msg.GetDescriptor();
  FP<string>(msg, F(d, "s")) = "zzz";
  EXPECT_THROW(static_cast<int32_t>(FP<int32_t>(msg, F(d, "s"))),
               runtime_error);
  FP<bool>(msg, F(d, "b")) = true;
  EXPECT_THROW(static_cast<string>(FP<string>(msg, F(d, "b"))), runtime_error);
}

TEST(FieldProxy_RepeatedAssign, ThrowsOnAssignToRepeatedField) {
  Top msg;
  auto *d = msg.GetDescriptor();
  auto f = d->FindFieldByName("r_i32");
  EXPECT_THROW(FP<int32_t>(msg, f) = 1, runtime_error);
}

TEST(RepeatedProxy_PushBackFn, ThrowsOnNonMessageField) {
  Top msg;
  auto *d = msg.GetDescriptor();
  auto rr = RP<int32_t>(msg, F(d, "r_i32"));
  EXPECT_THROW(rr.push_back([](int &) {}), runtime_error);
}

TEST(RepeatedProxy_SetMessage, ThrowsOnMessageSet) {
  Top msg;
  auto *d = msg.GetDescriptor();
  auto rr = RP<MessageWrapped<mypkg::Child>>(msg, F(d, "repeated_child"));
  EXPECT_THROW(rr.set(0, MessageWrapped<mypkg::Child>(*msg.mutable_child())),
               runtime_error);
}

TEST(OneofProxy_InvalidSet, ThrowsOnBadField) {
  Top msg;
  auto *d = msg.GetDescriptor();
  OneofProxy o(msg, *d->FindOneofByName("choice"));
  EXPECT_THROW(o.set("nonexistent", [](Msg &, const FD &) {}), runtime_error);
}

TEST(FieldProxy_StreamOutput, OstreamCoversStringAndInt) {
  Top msg;
  auto *d = msg.GetDescriptor();
  FP<int32_t>(msg, F(d, "i32")) = 123;
  std::ostringstream oss1;
  oss1 << FP<int32_t>(msg, F(d, "i32"));
  EXPECT_NE(oss1.str().find("123"), std::string::npos);

  FP<string>(msg, F(d, "s")) = "stream";
  std::ostringstream oss2;
  oss2 << FP<string>(msg, F(d, "s"));
  EXPECT_EQ(oss2.str(), "stream");
}

TEST(FieldProxy_StringViewCast, WorksForStringField) {
  Top msg;
  auto *d = msg.GetDescriptor();
  FP<string>(msg, F(d, "s")) = "hello";
  std::string_view sv = FP<string>(msg, F(d, "s"));
  EXPECT_EQ(sv, "hello");
}

TEST(RepeatedProxy_AddMessage, HappyAndError) {
  Top msg;
  auto *d = msg.GetDescriptor();
  auto rr = RP<MessageWrapped<mypkg::Child>>(msg, F(d, "repeated_child"));
  auto &sub = rr.add_message();
  sub.GetReflection()->SetString(&sub, F(sub.GetDescriptor(), "child_str"),
                                 "ok");
  EXPECT_EQ(msg.repeated_child(0).child_str(), "ok");
  EXPECT_THROW((RP<int32_t>(msg, F(d, "r_i32")).add_message()), runtime_error);
}

TEST(RepeatedProxy_IndexOutOfRange, Throws) {
  Top msg;
  auto *d = msg.GetDescriptor();
  auto rr = RP<int32_t>(msg, F(d, "r_i32"));
  EXPECT_THROW(rr[0], out_of_range);
}

TEST(MapProxy_SubmessageNotSupported, Throws) {
  Top msg;
  auto *d = msg.GetDescriptor();
  auto f = d->FindFieldByName("u64_to_child");
  auto m = MP<uint64_t, MessageWrapped<mypkg::Child>>(msg, f);
  EXPECT_THROW(m.set(1ull, MessageWrapped<mypkg::Child>(*msg.mutable_child())),
               runtime_error);
}

} // namespace
