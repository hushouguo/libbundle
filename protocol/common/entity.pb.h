// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: common/entity.proto

#ifndef PROTOBUF_common_2fentity_2eproto__INCLUDED
#define PROTOBUF_common_2fentity_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3003000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3003000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/map.h>  // IWYU pragma: export
#include <google/protobuf/map_field_inl.h>
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
namespace bundle {
class Entity;
class EntityDefaultTypeInternal;
extern EntityDefaultTypeInternal _Entity_default_instance_;
class Entity_ValuesEntry;
class Entity_ValuesEntryDefaultTypeInternal;
extern Entity_ValuesEntryDefaultTypeInternal _Entity_ValuesEntry_default_instance_;
class Value;
class ValueDefaultTypeInternal;
extern ValueDefaultTypeInternal _Value_default_instance_;
}  // namespace bundle

namespace bundle {

namespace protobuf_common_2fentity_2eproto {
// Internal implementation detail -- do not call these.
struct TableStruct {
  static const ::google::protobuf::internal::ParseTableField entries[];
  static const ::google::protobuf::internal::AuxillaryParseTableField aux[];
  static const ::google::protobuf::internal::ParseTable schema[];
  static const ::google::protobuf::uint32 offsets[];
  static void InitDefaultsImpl();
  static void Shutdown();
};
void AddDescriptors();
void InitDefaults();
}  // namespace protobuf_common_2fentity_2eproto

enum Value_ValueType {
  Value_ValueType_type_s8 = 0,
  Value_ValueType_type_u8 = 1,
  Value_ValueType_type_s32 = 2,
  Value_ValueType_type_u32 = 3,
  Value_ValueType_type_s64 = 4,
  Value_ValueType_type_u64 = 5,
  Value_ValueType_type_bool = 6,
  Value_ValueType_type_float = 7,
  Value_ValueType_type_double = 8,
  Value_ValueType_type_string = 9,
  Value_ValueType_type_bytes = 10,
  Value_ValueType_type_datetime = 11,
  Value_ValueType_Value_ValueType_INT_MIN_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32min,
  Value_ValueType_Value_ValueType_INT_MAX_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32max
};
bool Value_ValueType_IsValid(int value);
const Value_ValueType Value_ValueType_ValueType_MIN = Value_ValueType_type_s8;
const Value_ValueType Value_ValueType_ValueType_MAX = Value_ValueType_type_datetime;
const int Value_ValueType_ValueType_ARRAYSIZE = Value_ValueType_ValueType_MAX + 1;

const ::google::protobuf::EnumDescriptor* Value_ValueType_descriptor();
inline const ::std::string& Value_ValueType_Name(Value_ValueType value) {
  return ::google::protobuf::internal::NameOfEnum(
    Value_ValueType_descriptor(), value);
}
inline bool Value_ValueType_Parse(
    const ::std::string& name, Value_ValueType* value) {
  return ::google::protobuf::internal::ParseNamedEnum<Value_ValueType>(
    Value_ValueType_descriptor(), name, value);
}
// ===================================================================

class Value : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:bundle.Value) */ {
 public:
  Value();
  virtual ~Value();

  Value(const Value& from);

  inline Value& operator=(const Value& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const Value& default_instance();

  static inline const Value* internal_default_instance() {
    return reinterpret_cast<const Value*>(
               &_Value_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    0;

  void Swap(Value* other);

  // implements Message ----------------------------------------------

  inline Value* New() const PROTOBUF_FINAL { return New(NULL); }

  Value* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const Value& from);
  void MergeFrom(const Value& from);
  void Clear() PROTOBUF_FINAL;
  bool IsInitialized() const PROTOBUF_FINAL;

  size_t ByteSizeLong() const PROTOBUF_FINAL;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) PROTOBUF_FINAL;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const PROTOBUF_FINAL;
  int GetCachedSize() const PROTOBUF_FINAL { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const PROTOBUF_FINAL;
  void InternalSwap(Value* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const PROTOBUF_FINAL;

  // nested types ----------------------------------------------------

  typedef Value_ValueType ValueType;
  static const ValueType type_s8 =
    Value_ValueType_type_s8;
  static const ValueType type_u8 =
    Value_ValueType_type_u8;
  static const ValueType type_s32 =
    Value_ValueType_type_s32;
  static const ValueType type_u32 =
    Value_ValueType_type_u32;
  static const ValueType type_s64 =
    Value_ValueType_type_s64;
  static const ValueType type_u64 =
    Value_ValueType_type_u64;
  static const ValueType type_bool =
    Value_ValueType_type_bool;
  static const ValueType type_float =
    Value_ValueType_type_float;
  static const ValueType type_double =
    Value_ValueType_type_double;
  static const ValueType type_string =
    Value_ValueType_type_string;
  static const ValueType type_bytes =
    Value_ValueType_type_bytes;
  static const ValueType type_datetime =
    Value_ValueType_type_datetime;
  static inline bool ValueType_IsValid(int value) {
    return Value_ValueType_IsValid(value);
  }
  static const ValueType ValueType_MIN =
    Value_ValueType_ValueType_MIN;
  static const ValueType ValueType_MAX =
    Value_ValueType_ValueType_MAX;
  static const int ValueType_ARRAYSIZE =
    Value_ValueType_ValueType_ARRAYSIZE;
  static inline const ::google::protobuf::EnumDescriptor*
  ValueType_descriptor() {
    return Value_ValueType_descriptor();
  }
  static inline const ::std::string& ValueType_Name(ValueType value) {
    return Value_ValueType_Name(value);
  }
  static inline bool ValueType_Parse(const ::std::string& name,
      ValueType* value) {
    return Value_ValueType_Parse(name, value);
  }

  // accessors -------------------------------------------------------

  // string value_string = 11;
  void clear_value_string();
  static const int kValueStringFieldNumber = 11;
  const ::std::string& value_string() const;
  void set_value_string(const ::std::string& value);
  #if LANG_CXX11
  void set_value_string(::std::string&& value);
  #endif
  void set_value_string(const char* value);
  void set_value_string(const char* value, size_t size);
  ::std::string* mutable_value_string();
  ::std::string* release_value_string();
  void set_allocated_value_string(::std::string* value_string);

  // bytes value_bytes = 12;
  void clear_value_bytes();
  static const int kValueBytesFieldNumber = 12;
  const ::std::string& value_bytes() const;
  void set_value_bytes(const ::std::string& value);
  #if LANG_CXX11
  void set_value_bytes(::std::string&& value);
  #endif
  void set_value_bytes(const char* value);
  void set_value_bytes(const void* value, size_t size);
  ::std::string* mutable_value_bytes();
  ::std::string* release_value_bytes();
  void set_allocated_value_bytes(::std::string* value_bytes);

  // string value_datetime = 13;
  void clear_value_datetime();
  static const int kValueDatetimeFieldNumber = 13;
  const ::std::string& value_datetime() const;
  void set_value_datetime(const ::std::string& value);
  #if LANG_CXX11
  void set_value_datetime(::std::string&& value);
  #endif
  void set_value_datetime(const char* value);
  void set_value_datetime(const char* value, size_t size);
  ::std::string* mutable_value_datetime();
  ::std::string* release_value_datetime();
  void set_allocated_value_datetime(::std::string* value_datetime);

  // .bundle.Value.ValueType type = 1;
  void clear_type();
  static const int kTypeFieldNumber = 1;
  ::bundle::Value_ValueType type() const;
  void set_type(::bundle::Value_ValueType value);

  // sint32 value_s8 = 2;
  void clear_value_s8();
  static const int kValueS8FieldNumber = 2;
  ::google::protobuf::int32 value_s8() const;
  void set_value_s8(::google::protobuf::int32 value);

  // uint32 value_u8 = 3;
  void clear_value_u8();
  static const int kValueU8FieldNumber = 3;
  ::google::protobuf::uint32 value_u8() const;
  void set_value_u8(::google::protobuf::uint32 value);

  // sint32 value_s32 = 4;
  void clear_value_s32();
  static const int kValueS32FieldNumber = 4;
  ::google::protobuf::int32 value_s32() const;
  void set_value_s32(::google::protobuf::int32 value);

  // sint64 value_s64 = 6;
  void clear_value_s64();
  static const int kValueS64FieldNumber = 6;
  ::google::protobuf::int64 value_s64() const;
  void set_value_s64(::google::protobuf::int64 value);

  // uint32 value_u32 = 5;
  void clear_value_u32();
  static const int kValueU32FieldNumber = 5;
  ::google::protobuf::uint32 value_u32() const;
  void set_value_u32(::google::protobuf::uint32 value);

  // bool value_bool = 8;
  void clear_value_bool();
  static const int kValueBoolFieldNumber = 8;
  bool value_bool() const;
  void set_value_bool(bool value);

  // uint64 value_u64 = 7;
  void clear_value_u64();
  static const int kValueU64FieldNumber = 7;
  ::google::protobuf::uint64 value_u64() const;
  void set_value_u64(::google::protobuf::uint64 value);

  // double value_double = 10;
  void clear_value_double();
  static const int kValueDoubleFieldNumber = 10;
  double value_double() const;
  void set_value_double(double value);

  // float value_float = 9;
  void clear_value_float();
  static const int kValueFloatFieldNumber = 9;
  float value_float() const;
  void set_value_float(float value);

  // @@protoc_insertion_point(class_scope:bundle.Value)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr value_string_;
  ::google::protobuf::internal::ArenaStringPtr value_bytes_;
  ::google::protobuf::internal::ArenaStringPtr value_datetime_;
  int type_;
  ::google::protobuf::int32 value_s8_;
  ::google::protobuf::uint32 value_u8_;
  ::google::protobuf::int32 value_s32_;
  ::google::protobuf::int64 value_s64_;
  ::google::protobuf::uint32 value_u32_;
  bool value_bool_;
  ::google::protobuf::uint64 value_u64_;
  double value_double_;
  float value_float_;
  mutable int _cached_size_;
  friend struct protobuf_common_2fentity_2eproto::TableStruct;
};
// -------------------------------------------------------------------


// -------------------------------------------------------------------

class Entity : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:bundle.Entity) */ {
 public:
  Entity();
  virtual ~Entity();

  Entity(const Entity& from);

  inline Entity& operator=(const Entity& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const Entity& default_instance();

  static inline const Entity* internal_default_instance() {
    return reinterpret_cast<const Entity*>(
               &_Entity_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    2;

  void Swap(Entity* other);

  // implements Message ----------------------------------------------

  inline Entity* New() const PROTOBUF_FINAL { return New(NULL); }

  Entity* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const Entity& from);
  void MergeFrom(const Entity& from);
  void Clear() PROTOBUF_FINAL;
  bool IsInitialized() const PROTOBUF_FINAL;

  size_t ByteSizeLong() const PROTOBUF_FINAL;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) PROTOBUF_FINAL;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const PROTOBUF_FINAL;
  int GetCachedSize() const PROTOBUF_FINAL { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const PROTOBUF_FINAL;
  void InternalSwap(Entity* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const PROTOBUF_FINAL;

  // nested types ----------------------------------------------------


  // accessors -------------------------------------------------------

  // map<string, .bundle.Value> values = 2;
  int values_size() const;
  void clear_values();
  static const int kValuesFieldNumber = 2;
  const ::google::protobuf::Map< ::std::string, ::bundle::Value >&
      values() const;
  ::google::protobuf::Map< ::std::string, ::bundle::Value >*
      mutable_values();

  // uint64 id = 1;
  void clear_id();
  static const int kIdFieldNumber = 1;
  ::google::protobuf::uint64 id() const;
  void set_id(::google::protobuf::uint64 value);

  // @@protoc_insertion_point(class_scope:bundle.Entity)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  public:
  class Entity_ValuesEntry : public ::google::protobuf::internal::MapEntry<Entity_ValuesEntry, 
      ::std::string, ::bundle::Value,
      ::google::protobuf::internal::WireFormatLite::TYPE_STRING,
      ::google::protobuf::internal::WireFormatLite::TYPE_MESSAGE,
      0 > {
  public:
    typedef ::google::protobuf::internal::MapEntry<Entity_ValuesEntry, 
      ::std::string, ::bundle::Value,
      ::google::protobuf::internal::WireFormatLite::TYPE_STRING,
      ::google::protobuf::internal::WireFormatLite::TYPE_MESSAGE,
      0 > SuperType;
    Entity_ValuesEntry();
    Entity_ValuesEntry(::google::protobuf::Arena* arena);
    void MergeFrom(const ::google::protobuf::Message& other) PROTOBUF_FINAL;
    void MergeFrom(const Entity_ValuesEntry& other);
    static const Message* internal_default_instance() { return reinterpret_cast<const Message*>(&_Entity_ValuesEntry_default_instance_); }
    ::google::protobuf::Metadata GetMetadata() const;
  };
  ::google::protobuf::internal::MapField<
      Entity_ValuesEntry,
      ::std::string, ::bundle::Value,
      ::google::protobuf::internal::WireFormatLite::TYPE_STRING,
      ::google::protobuf::internal::WireFormatLite::TYPE_MESSAGE,
      0 > values_;
  private:
  ::google::protobuf::uint64 id_;
  mutable int _cached_size_;
  friend struct protobuf_common_2fentity_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// Value

// .bundle.Value.ValueType type = 1;
inline void Value::clear_type() {
  type_ = 0;
}
inline ::bundle::Value_ValueType Value::type() const {
  // @@protoc_insertion_point(field_get:bundle.Value.type)
  return static_cast< ::bundle::Value_ValueType >(type_);
}
inline void Value::set_type(::bundle::Value_ValueType value) {
  
  type_ = value;
  // @@protoc_insertion_point(field_set:bundle.Value.type)
}

// sint32 value_s8 = 2;
inline void Value::clear_value_s8() {
  value_s8_ = 0;
}
inline ::google::protobuf::int32 Value::value_s8() const {
  // @@protoc_insertion_point(field_get:bundle.Value.value_s8)
  return value_s8_;
}
inline void Value::set_value_s8(::google::protobuf::int32 value) {
  
  value_s8_ = value;
  // @@protoc_insertion_point(field_set:bundle.Value.value_s8)
}

// uint32 value_u8 = 3;
inline void Value::clear_value_u8() {
  value_u8_ = 0u;
}
inline ::google::protobuf::uint32 Value::value_u8() const {
  // @@protoc_insertion_point(field_get:bundle.Value.value_u8)
  return value_u8_;
}
inline void Value::set_value_u8(::google::protobuf::uint32 value) {
  
  value_u8_ = value;
  // @@protoc_insertion_point(field_set:bundle.Value.value_u8)
}

// sint32 value_s32 = 4;
inline void Value::clear_value_s32() {
  value_s32_ = 0;
}
inline ::google::protobuf::int32 Value::value_s32() const {
  // @@protoc_insertion_point(field_get:bundle.Value.value_s32)
  return value_s32_;
}
inline void Value::set_value_s32(::google::protobuf::int32 value) {
  
  value_s32_ = value;
  // @@protoc_insertion_point(field_set:bundle.Value.value_s32)
}

// uint32 value_u32 = 5;
inline void Value::clear_value_u32() {
  value_u32_ = 0u;
}
inline ::google::protobuf::uint32 Value::value_u32() const {
  // @@protoc_insertion_point(field_get:bundle.Value.value_u32)
  return value_u32_;
}
inline void Value::set_value_u32(::google::protobuf::uint32 value) {
  
  value_u32_ = value;
  // @@protoc_insertion_point(field_set:bundle.Value.value_u32)
}

// sint64 value_s64 = 6;
inline void Value::clear_value_s64() {
  value_s64_ = GOOGLE_LONGLONG(0);
}
inline ::google::protobuf::int64 Value::value_s64() const {
  // @@protoc_insertion_point(field_get:bundle.Value.value_s64)
  return value_s64_;
}
inline void Value::set_value_s64(::google::protobuf::int64 value) {
  
  value_s64_ = value;
  // @@protoc_insertion_point(field_set:bundle.Value.value_s64)
}

// uint64 value_u64 = 7;
inline void Value::clear_value_u64() {
  value_u64_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 Value::value_u64() const {
  // @@protoc_insertion_point(field_get:bundle.Value.value_u64)
  return value_u64_;
}
inline void Value::set_value_u64(::google::protobuf::uint64 value) {
  
  value_u64_ = value;
  // @@protoc_insertion_point(field_set:bundle.Value.value_u64)
}

// bool value_bool = 8;
inline void Value::clear_value_bool() {
  value_bool_ = false;
}
inline bool Value::value_bool() const {
  // @@protoc_insertion_point(field_get:bundle.Value.value_bool)
  return value_bool_;
}
inline void Value::set_value_bool(bool value) {
  
  value_bool_ = value;
  // @@protoc_insertion_point(field_set:bundle.Value.value_bool)
}

// float value_float = 9;
inline void Value::clear_value_float() {
  value_float_ = 0;
}
inline float Value::value_float() const {
  // @@protoc_insertion_point(field_get:bundle.Value.value_float)
  return value_float_;
}
inline void Value::set_value_float(float value) {
  
  value_float_ = value;
  // @@protoc_insertion_point(field_set:bundle.Value.value_float)
}

// double value_double = 10;
inline void Value::clear_value_double() {
  value_double_ = 0;
}
inline double Value::value_double() const {
  // @@protoc_insertion_point(field_get:bundle.Value.value_double)
  return value_double_;
}
inline void Value::set_value_double(double value) {
  
  value_double_ = value;
  // @@protoc_insertion_point(field_set:bundle.Value.value_double)
}

// string value_string = 11;
inline void Value::clear_value_string() {
  value_string_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& Value::value_string() const {
  // @@protoc_insertion_point(field_get:bundle.Value.value_string)
  return value_string_.GetNoArena();
}
inline void Value::set_value_string(const ::std::string& value) {
  
  value_string_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:bundle.Value.value_string)
}
#if LANG_CXX11
inline void Value::set_value_string(::std::string&& value) {
  
  value_string_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:bundle.Value.value_string)
}
#endif
inline void Value::set_value_string(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  value_string_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:bundle.Value.value_string)
}
inline void Value::set_value_string(const char* value, size_t size) {
  
  value_string_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:bundle.Value.value_string)
}
inline ::std::string* Value::mutable_value_string() {
  
  // @@protoc_insertion_point(field_mutable:bundle.Value.value_string)
  return value_string_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Value::release_value_string() {
  // @@protoc_insertion_point(field_release:bundle.Value.value_string)
  
  return value_string_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Value::set_allocated_value_string(::std::string* value_string) {
  if (value_string != NULL) {
    
  } else {
    
  }
  value_string_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value_string);
  // @@protoc_insertion_point(field_set_allocated:bundle.Value.value_string)
}

// bytes value_bytes = 12;
inline void Value::clear_value_bytes() {
  value_bytes_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& Value::value_bytes() const {
  // @@protoc_insertion_point(field_get:bundle.Value.value_bytes)
  return value_bytes_.GetNoArena();
}
inline void Value::set_value_bytes(const ::std::string& value) {
  
  value_bytes_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:bundle.Value.value_bytes)
}
#if LANG_CXX11
inline void Value::set_value_bytes(::std::string&& value) {
  
  value_bytes_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:bundle.Value.value_bytes)
}
#endif
inline void Value::set_value_bytes(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  value_bytes_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:bundle.Value.value_bytes)
}
inline void Value::set_value_bytes(const void* value, size_t size) {
  
  value_bytes_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:bundle.Value.value_bytes)
}
inline ::std::string* Value::mutable_value_bytes() {
  
  // @@protoc_insertion_point(field_mutable:bundle.Value.value_bytes)
  return value_bytes_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Value::release_value_bytes() {
  // @@protoc_insertion_point(field_release:bundle.Value.value_bytes)
  
  return value_bytes_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Value::set_allocated_value_bytes(::std::string* value_bytes) {
  if (value_bytes != NULL) {
    
  } else {
    
  }
  value_bytes_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value_bytes);
  // @@protoc_insertion_point(field_set_allocated:bundle.Value.value_bytes)
}

// string value_datetime = 13;
inline void Value::clear_value_datetime() {
  value_datetime_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& Value::value_datetime() const {
  // @@protoc_insertion_point(field_get:bundle.Value.value_datetime)
  return value_datetime_.GetNoArena();
}
inline void Value::set_value_datetime(const ::std::string& value) {
  
  value_datetime_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:bundle.Value.value_datetime)
}
#if LANG_CXX11
inline void Value::set_value_datetime(::std::string&& value) {
  
  value_datetime_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:bundle.Value.value_datetime)
}
#endif
inline void Value::set_value_datetime(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  value_datetime_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:bundle.Value.value_datetime)
}
inline void Value::set_value_datetime(const char* value, size_t size) {
  
  value_datetime_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:bundle.Value.value_datetime)
}
inline ::std::string* Value::mutable_value_datetime() {
  
  // @@protoc_insertion_point(field_mutable:bundle.Value.value_datetime)
  return value_datetime_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Value::release_value_datetime() {
  // @@protoc_insertion_point(field_release:bundle.Value.value_datetime)
  
  return value_datetime_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Value::set_allocated_value_datetime(::std::string* value_datetime) {
  if (value_datetime != NULL) {
    
  } else {
    
  }
  value_datetime_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value_datetime);
  // @@protoc_insertion_point(field_set_allocated:bundle.Value.value_datetime)
}

// -------------------------------------------------------------------

// -------------------------------------------------------------------

// Entity

// uint64 id = 1;
inline void Entity::clear_id() {
  id_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 Entity::id() const {
  // @@protoc_insertion_point(field_get:bundle.Entity.id)
  return id_;
}
inline void Entity::set_id(::google::protobuf::uint64 value) {
  
  id_ = value;
  // @@protoc_insertion_point(field_set:bundle.Entity.id)
}

// map<string, .bundle.Value> values = 2;
inline int Entity::values_size() const {
  return values_.size();
}
inline void Entity::clear_values() {
  values_.Clear();
}
inline const ::google::protobuf::Map< ::std::string, ::bundle::Value >&
Entity::values() const {
  // @@protoc_insertion_point(field_map:bundle.Entity.values)
  return values_.GetMap();
}
inline ::google::protobuf::Map< ::std::string, ::bundle::Value >*
Entity::mutable_values() {
  // @@protoc_insertion_point(field_mutable_map:bundle.Entity.values)
  return values_.MutableMap();
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)


}  // namespace bundle

#ifndef SWIG
namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::bundle::Value_ValueType> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::bundle::Value_ValueType>() {
  return ::bundle::Value_ValueType_descriptor();
}

}  // namespace protobuf
}  // namespace google
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_common_2fentity_2eproto__INCLUDED
