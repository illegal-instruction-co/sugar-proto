#pragma once

/*
 * sugar_runtime.h
 *
 * Copyright 2025 M.Berkay Karatas
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/reflection.h>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace sugar {

namespace detail {
template <typename T>
inline constexpr bool is_string_like_v =
    std::is_same_v<std::decay_t<T>, std::string> ||
    std::is_same_v<std::decay_t<T>, std::string_view> ||
    std::is_same_v<std::decay_t<T>, const char *>;

template <typename T, std::size_t N>
inline constexpr bool is_string_like_v<T[N]> = true;

template <typename T> inline std::string to_string_any(T &&v) {
  if constexpr (std::is_same_v<std::decay_t<T>, std::string>)
    return v;
  else if constexpr (std::is_same_v<std::decay_t<T>, std::string_view>)
    return std::string(v);
  else if constexpr (std::is_same_v<T, const char *>)
    return std::string(v);
  else
    static_assert(sizeof(T) == 0, "unsupported string-like type");
  return {};
}

template <std::size_t N>
inline std::string to_string_any(const char (&arr)[N]) {
  return std::string(arr);
}

[[nodiscard]] inline const google::protobuf::FieldDescriptor *
find_field(const google::protobuf::Descriptor *d,
           std::string_view name) noexcept {
  if (!d)
    return nullptr;
  return d->FindFieldByName(std::string{name});
}

template <typename T>
inline constexpr bool is_signed_int_v =
    std::is_integral_v<std::decay_t<T>> && std::is_signed_v<std::decay_t<T>> &&
    !std::is_same_v<std::decay_t<T>, bool>;

template <typename T>
inline constexpr bool is_unsigned_int_v =
    std::is_integral_v<std::decay_t<T>> && std::is_unsigned_v<std::decay_t<T>>;

template <typename T>
inline constexpr bool is_float_v = std::is_floating_point_v<std::decay_t<T>>;
} // namespace detail

template <typename MsgT> class MessageWrapped;

template <typename T> class FieldProxy {
public:
  FieldProxy(google::protobuf::Message &m,
             const google::protobuf::FieldDescriptor &f) noexcept
      : msg_(m), field_(f) {}

  template <typename V> FieldProxy &operator=(V &&v) {
    auto *r = msg_.GetReflection();
    using FD = google::protobuf::FieldDescriptor;
    if (field_.is_repeated())
      throw std::runtime_error("assignment on repeated field");
    switch (field_.cpp_type()) {
    case FD::CPPTYPE_INT32:
      if constexpr (detail::is_signed_int_v<V>)
        r->SetInt32(&msg_, &field_, static_cast<int32_t>(v));
      else
        throw std::runtime_error("type mismatch: expected int32");
      break;
    case FD::CPPTYPE_INT64:
      if constexpr (detail::is_signed_int_v<V>)
        r->SetInt64(&msg_, &field_, static_cast<int64_t>(v));
      else
        throw std::runtime_error("type mismatch: expected int64");
      break;
    case FD::CPPTYPE_UINT32:
      if constexpr (detail::is_unsigned_int_v<V>)
        r->SetUInt32(&msg_, &field_, static_cast<uint32_t>(v));
      else
        throw std::runtime_error("type mismatch: expected uint32");
      break;
    case FD::CPPTYPE_UINT64:
      if constexpr (detail::is_unsigned_int_v<V>)
        r->SetUInt64(&msg_, &field_, static_cast<uint64_t>(v));
      else
        throw std::runtime_error("type mismatch: expected uint64");
      break;
    case FD::CPPTYPE_FLOAT:
      if constexpr (detail::is_float_v<V> || detail::is_signed_int_v<V> ||
                    detail::is_unsigned_int_v<V>)
        r->SetFloat(&msg_, &field_, static_cast<float>(v));
      else
        throw std::runtime_error("type mismatch: expected float-like");
      break;
    case FD::CPPTYPE_DOUBLE:
      if constexpr (detail::is_float_v<V> || detail::is_signed_int_v<V> ||
                    detail::is_unsigned_int_v<V>)
        r->SetDouble(&msg_, &field_, static_cast<double>(v));
      else
        throw std::runtime_error("type mismatch: expected double-like");
      break;
    case FD::CPPTYPE_BOOL:
      if constexpr (std::is_same_v<std::decay_t<V>, bool> ||
                    detail::is_signed_int_v<V> || detail::is_unsigned_int_v<V>)
        r->SetBool(&msg_, &field_, static_cast<bool>(v));
      else
        throw std::runtime_error("type mismatch: expected bool-like");
      break;
    case FD::CPPTYPE_STRING:
      if constexpr (detail::is_string_like_v<V>)
        r->SetString(&msg_, &field_, detail::to_string_any(std::forward<V>(v)));
      else
        throw std::runtime_error("type mismatch: expected string");
      break;
    case FD::CPPTYPE_ENUM:
      if constexpr (detail::is_signed_int_v<V> ||
                    detail::is_unsigned_int_v<V>) {
        const int n = static_cast<int>(v);
        const auto *ev = field_.enum_type()->FindValueByNumber(n);
        if (!ev)
          throw std::runtime_error("invalid enum value");
        r->SetEnum(&msg_, &field_, ev);
      } else
        throw std::runtime_error("type mismatch: expected enum number");
      break;
    case FD::CPPTYPE_MESSAGE:
      throw std::runtime_error("assign to message not allowed");
    }
    return *this;
  }

  [[nodiscard]] operator T() const {
    auto *r = msg_.GetReflection();
    using FD = google::protobuf::FieldDescriptor;
    if (field_.is_repeated())
      throw std::runtime_error("read on repeated field");
    if constexpr (std::is_same_v<T, std::string>) {
      if (field_.cpp_type() != FD::CPPTYPE_STRING)
        throw std::runtime_error("type mismatch");
      return r->GetString(msg_, &field_);
    } else if constexpr (std::is_same_v<T, bool>) {
      if (field_.cpp_type() != FD::CPPTYPE_BOOL)
        throw std::runtime_error("type mismatch");
      return r->GetBool(msg_, &field_);
    } else if constexpr (detail::is_signed_int_v<T>) {
      if (field_.cpp_type() == FD::CPPTYPE_INT32)
        return static_cast<T>(r->GetInt32(msg_, &field_));
      if (field_.cpp_type() == FD::CPPTYPE_INT64)
        return static_cast<T>(r->GetInt64(msg_, &field_));
      if (field_.cpp_type() == FD::CPPTYPE_ENUM)
        return static_cast<T>(r->GetEnum(msg_, &field_)->number());
      throw std::runtime_error("type mismatch");
    } else if constexpr (detail::is_unsigned_int_v<T>) {
      if (field_.cpp_type() == FD::CPPTYPE_UINT32)
        return static_cast<T>(r->GetUInt32(msg_, &field_));
      if (field_.cpp_type() == FD::CPPTYPE_UINT64)
        return static_cast<T>(r->GetUInt64(msg_, &field_));
      throw std::runtime_error("type mismatch");
    } else if constexpr (detail::is_float_v<T>) {
      if (field_.cpp_type() == FD::CPPTYPE_FLOAT)
        return static_cast<T>(r->GetFloat(msg_, &field_));
      if (field_.cpp_type() == FD::CPPTYPE_DOUBLE)
        return static_cast<T>(r->GetDouble(msg_, &field_));
      throw std::runtime_error("type mismatch");
    } else
      static_assert(sizeof(T) == 0, "unsupported FieldProxy read type");
  }

  FieldProxy operator[](std::string_view) = delete;

private:
  google::protobuf::Message &msg_;
  const google::protobuf::FieldDescriptor &field_;
};

template <typename ElemT> class RepeatedProxy {
public:
  RepeatedProxy(google::protobuf::Message &m,
                const google::protobuf::FieldDescriptor &f)
      : msg_(m), field_(f) {
    if (!field_.is_repeated())
      throw std::runtime_error("RepeatedProxy on non-repeated field");
  }

  [[nodiscard]] int size() const noexcept {
    return msg_.GetReflection()->FieldSize(msg_, &field_);
  }

  template <typename V> void push_back(V &&v) {
    auto *r = msg_.GetReflection();
    using FD = google::protobuf::FieldDescriptor;
    switch (field_.cpp_type()) {
    case FD::CPPTYPE_INT32:
      if constexpr (detail::is_signed_int_v<V>)
        r->AddInt32(&msg_, &field_, static_cast<int32_t>(v));
      else
        throw std::runtime_error("type mismatch: expected int32");
      break;
    case FD::CPPTYPE_INT64:
      if constexpr (detail::is_signed_int_v<V>)
        r->AddInt64(&msg_, &field_, static_cast<int64_t>(v));
      else
        throw std::runtime_error("type mismatch: expected int64");
      break;
    case FD::CPPTYPE_UINT32:
      if constexpr (detail::is_unsigned_int_v<V>)
        r->AddUInt32(&msg_, &field_, static_cast<uint32_t>(v));
      else
        throw std::runtime_error("type mismatch: expected uint32");
      break;
    case FD::CPPTYPE_UINT64:
      if constexpr (detail::is_unsigned_int_v<V>)
        r->AddUInt64(&msg_, &field_, static_cast<uint64_t>(v));
      else
        throw std::runtime_error("type mismatch: expected uint64");
      break;
    case FD::CPPTYPE_FLOAT:
      if constexpr (detail::is_float_v<V> || detail::is_signed_int_v<V> ||
                    detail::is_unsigned_int_v<V>)
        r->AddFloat(&msg_, &field_, static_cast<float>(v));
      else
        throw std::runtime_error("type mismatch: expected float-like");
      break;
    case FD::CPPTYPE_DOUBLE:
      if constexpr (detail::is_float_v<V> || detail::is_signed_int_v<V> ||
                    detail::is_unsigned_int_v<V>)
        r->AddDouble(&msg_, &field_, static_cast<double>(v));
      else
        throw std::runtime_error("type mismatch: expected double-like");
      break;
    case FD::CPPTYPE_BOOL:
      if constexpr (std::is_same_v<std::decay_t<V>, bool> ||
                    detail::is_signed_int_v<V> || detail::is_unsigned_int_v<V>)
        r->AddBool(&msg_, &field_, static_cast<bool>(v));
      else
        throw std::runtime_error("type mismatch: expected bool-like");
      break;
    case FD::CPPTYPE_STRING:
      if constexpr (detail::is_string_like_v<V>)
        r->AddString(&msg_, &field_, detail::to_string_any(std::forward<V>(v)));
      else
        throw std::runtime_error("type mismatch: expected string");
      break;
    case FD::CPPTYPE_ENUM:
      if constexpr (detail::is_signed_int_v<V> ||
                    detail::is_unsigned_int_v<V>) {
        const int n = static_cast<int>(v);
        const auto *ev = field_.enum_type()->FindValueByNumber(n);
        if (!ev)
          throw std::runtime_error("invalid enum value");
        r->AddEnum(&msg_, &field_, ev);
      } else
        throw std::runtime_error("type mismatch: expected enum number");
      break;
    case FD::CPPTYPE_MESSAGE:
      throw std::runtime_error("use add_message() for repeated message");
    }
  }

  [[nodiscard]] google::protobuf::Message &add_message() {
    if (field_.cpp_type() != google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE)
      throw std::runtime_error("add_message only for message");
    return *msg_.GetReflection()->AddMessage(&msg_, &field_);
  }

private:
  google::protobuf::Message &msg_;
  const google::protobuf::FieldDescriptor &field_;
};

template <typename K, typename V> class MapProxy {
public:
  MapProxy(google::protobuf::Message &m,
           const google::protobuf::FieldDescriptor &f) noexcept
      : msg_(m), field_(f) {
    if (!field_.is_map())
      throw std::runtime_error("MapProxy on non-map field");
  }

  template <typename KeyLike, typename ValLike>
  void set(KeyLike &&k, ValLike &&v) {
    auto *r = msg_.GetReflection();
    auto *entry = r->AddMessage(&msg_, &field_);
    const auto *kd = entry->GetDescriptor()->FindFieldByName("key");
    const auto *vd = entry->GetDescriptor()->FindFieldByName("value");
    if (!kd || !vd)
      throw std::runtime_error("map entry schema invalid");
    set_field(*entry, *kd, std::forward<KeyLike>(k));
    set_field(*entry, *vd, std::forward<ValLike>(v));
  }

private:
  template <typename X>
  static void set_field(google::protobuf::Message &m,
                        const google::protobuf::FieldDescriptor &f, X &&value) {
    auto *r = m.GetReflection();
    using FD = google::protobuf::FieldDescriptor;
    switch (f.cpp_type()) {
    case FD::CPPTYPE_INT32:
      if constexpr (detail::is_signed_int_v<X>)
        r->SetInt32(&m, &f, static_cast<int32_t>(value));
      else
        throw std::runtime_error("type mismatch: expected int32");
      break;
    case FD::CPPTYPE_INT64:
      if constexpr (detail::is_signed_int_v<X>)
        r->SetInt64(&m, &f, static_cast<int64_t>(value));
      else
        throw std::runtime_error("type mismatch: expected int64");
      break;
    case FD::CPPTYPE_UINT32:
      if constexpr (detail::is_unsigned_int_v<X>)
        r->SetUInt32(&m, &f, static_cast<uint32_t>(value));
      else
        throw std::runtime_error("type mismatch: expected uint32");
      break;
    case FD::CPPTYPE_UINT64:
      if constexpr (detail::is_unsigned_int_v<X>)
        r->SetUInt64(&m, &f, static_cast<uint64_t>(value));
      else
        throw std::runtime_error("type mismatch: expected uint64");
      break;
    case FD::CPPTYPE_FLOAT:
      if constexpr (detail::is_float_v<X> || detail::is_signed_int_v<X> ||
                    detail::is_unsigned_int_v<X>)
        r->SetFloat(&m, &f, static_cast<float>(value));
      else
        throw std::runtime_error("type mismatch: expected float-like");
      break;
    case FD::CPPTYPE_DOUBLE:
      if constexpr (detail::is_float_v<X> || detail::is_signed_int_v<X> ||
                    detail::is_unsigned_int_v<X>)
        r->SetDouble(&m, &f, static_cast<double>(value));
      else
        throw std::runtime_error("type mismatch: expected double-like");
      break;
    case FD::CPPTYPE_BOOL:
      if constexpr (std::is_same_v<std::decay_t<X>, bool> ||
                    detail::is_signed_int_v<X> || detail::is_unsigned_int_v<X>)
        r->SetBool(&m, &f, static_cast<bool>(value));
      else
        throw std::runtime_error("type mismatch: expected bool-like");
      break;
    case FD::CPPTYPE_STRING:
      if constexpr (detail::is_string_like_v<X>)
        r->SetString(&m, &f, detail::to_string_any(std::forward<X>(value)));
      else
        throw std::runtime_error("type mismatch: expected string");
      break;
    case FD::CPPTYPE_ENUM:
      if constexpr (detail::is_signed_int_v<X> ||
                    detail::is_unsigned_int_v<X>) {
        const int n = static_cast<int>(value);
        const auto *ev = f.enum_type()->FindValueByNumber(n);
        if (!ev)
          throw std::runtime_error("invalid enum");
        r->SetEnum(&m, &f, ev);
      } else
        throw std::runtime_error("type mismatch: expected enum number");
      break;
    case FD::CPPTYPE_MESSAGE:
      throw std::runtime_error("map submessage not supported");
    }
  }

  google::protobuf::Message &msg_;
  const google::protobuf::FieldDescriptor &field_;
};

class OneofProxy {
public:
  OneofProxy(google::protobuf::Message &m,
             const google::protobuf::OneofDescriptor &o) noexcept
      : msg_(m), oneof_(o) {}

  [[nodiscard]] const google::protobuf::FieldDescriptor *
  active_field() const noexcept {
    auto *r = msg_.GetReflection();
    return r->GetOneofFieldDescriptor(msg_, &oneof_);
  }

  void clear() { msg_.GetReflection()->ClearOneof(&msg_, &oneof_); }

  template <typename F> void set(std::string_view field_name, F &&setter) {
    const auto *f = detail::find_field(msg_.GetDescriptor(), field_name);
    if (!f || f->containing_oneof() != &oneof_)
      throw std::runtime_error("field not in this oneof");
    setter(msg_, *f);
  }

private:
  google::protobuf::Message &msg_;
  const google::protobuf::OneofDescriptor &oneof_;
};

} // namespace sugar
