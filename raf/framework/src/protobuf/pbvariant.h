
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once
#include <google/protobuf/descriptor.h>
#include "idgs/util/enum_def.h"

namespace protobuf {

struct PbVariant {
public:
  PbVariant();
  PbVariant(int32_t);
  PbVariant(int64_t);
  PbVariant(uint32_t);
  PbVariant(uint64_t);
  PbVariant(double);
  PbVariant(float);
  PbVariant(bool);
  PbVariant(const std::string& v);
  PbVariant(std::string* v);
  PbVariant(const PbVariant& v);
  PbVariant(PbVariant&& v);

  ~PbVariant();

  PbVariant& operator = (int32_t);
  PbVariant& operator = (int64_t);
  PbVariant& operator = (uint32_t);
  PbVariant& operator = (uint64_t);
  PbVariant& operator = (double);
  PbVariant& operator = (float);
  PbVariant& operator = (bool);
  PbVariant& operator = (const std::string& v);
  PbVariant& operator = (std::string* v);
  PbVariant& operator = (const PbVariant& v);
  PbVariant& operator = (PbVariant&& v);

  std::string toString() const;

  size_t hashcode() const;

  operator bool () const;
  operator int32_t () const;
  operator int64_t () const;
  operator uint32_t () const;
  operator uint64_t () const;
  operator float () const;
  operator double () const;
  operator std::string() const;


  bool operator < (const PbVariant& ) const;
  bool operator <= (const PbVariant& ) const;
  bool operator > (const PbVariant& ) const;
  bool operator >= (const PbVariant& ) const;
  bool operator == (const PbVariant& ) const;
  bool operator != (const PbVariant& ) const;


private:
  inline void freeString();

public:
  union {
    int32_t v_int32;
    int64_t v_int64;
    uint32_t v_uint32;
    uint64_t v_uint64;
    float v_float;
    double v_double;
    bool v_bool;

    std::string* v_string;
  } value;
  DEF_ENUM (Type,
    vt_int32   = 1, // google::protobuf::FieldDescriptor::CPPTYPE_INT32,    // TYPE_INT32, TYPE_SINT32, TYPE_SFIXED32
    vt_int64   = 2, // google::protobuf::FieldDescriptor::CPPTYPE_INT64,    // TYPE_INT64, TYPE_SINT64, TYPE_SFIXED64
    vt_uint32  = 3, // google::protobuf::FieldDescriptor::CPPTYPE_UINT32,   // TYPE_UINT32, TYPE_FIXED32
    vt_uint64  = 4, // google::protobuf::FieldDescriptor::CPPTYPE_UINT64,   // TYPE_UINT64, TYPE_FIXED64
    vt_double  = 5, // google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE,   // TYPE_DOUBLE
    vt_float   = 6, // google::protobuf::FieldDescriptor::CPPTYPE_FLOAT,    // TYPE_FLOAT
    vt_bool    = 7, // google::protobuf::FieldDescriptor::CPPTYPE_BOOL,     // TYPE_BOOL
    vt_enum    = 8, // google::protobuf::FieldDescriptor::CPPTYPE_ENUM,     // TYPE_ENUM
    vt_string  = 9, // google::protobuf::FieldDescriptor::CPPTYPE_STRING,   // TYPE_STRING, TYPE_BYTES
    vt_message = 10,// google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE,  // TYPE_MESSAGE, TYPE_GROUP
  );
  Type type;

  std::string debugString() {
    return std::string("type: ").append(TypeToString(type)).append(", value: ").append(toString());
  }

};
} // namespace protobuf

