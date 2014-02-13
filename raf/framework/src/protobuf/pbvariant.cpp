
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "protobuf/pbvariant.h"

#define V_LT <
#define V_LE <=
#define V_EQ ==
#define V_GT >
#define V_GE >=
#define V_NE !=

//
// macro to define compare operator.
#define PB_VARIANT_CMP_OP(OP) bool PbVariant::operator OP (const PbVariant& rhs) const {\
  switch(type) {\
  case vt_int32:\
    return value.v_int32 OP ((int32_t) rhs);\
  case vt_int64:\
    return value.v_int64 OP ((int64_t) rhs);\
  case vt_uint32:\
    return value.v_uint32 OP ((uint32_t) rhs);\
  case vt_uint64:\
    return value.v_uint64 OP ((uint64_t) rhs);\
  case vt_double:\
    return value.v_double OP ((double) rhs);\
  case vt_float:\
    return value.v_float OP ((float) rhs);\
  case vt_string:\
    return (*(value.v_string)) OP ((std::string) rhs);\
  default:\
    /* @todo enum & message*/\
    assert(false);\
    break;\
  }\
  return false;\
};

#define double_t double
#define float_t float
#define bool_t bool

// MACRO to define constructor
#define PB_VARIANT_CTOR(TYPE) PbVariant::PbVariant(TYPE##_t v):type(vt_##TYPE) {\
  value.v_##TYPE = v;\
}

// macro to define assignment operator
#define PB_VARIANT_ASSIGN(TYPE)  PbVariant& PbVariant::operator = (TYPE##_t v) { \
      freeString();\
      type = vt_##TYPE;\
      value.v_##TYPE = v;\
      return *this;\
    }

// cast operator
#define PB_VARIANT_CAST_OP(TYPE, CAST)  PbVariant::operator TYPE##_t () const { \
      TYPE##_t result; \
      switch (type) { \
      case vt_int32: \
        result = (TYPE##_t) value.v_int32; \
        break; \
      case vt_int64: \
        result = (TYPE##_t) value.v_int64; \
        break; \
      case vt_uint32: \
        result = (TYPE##_t) value.v_uint32; \
        break; \
      case vt_uint64: \
        result = (TYPE##_t) value.v_uint64; \
        break; \
      case vt_double: \
        result = (TYPE##_t) value.v_double; \
        break; \
      case vt_float: \
        result = (TYPE##_t) value.v_float; \
        break; \
      case vt_bool: \
        result = (TYPE##_t) value.v_bool; \
        break; \
      case vt_string: \
        result = value.v_string? CAST((*(value.v_string))) : 0; \
        break; \
      default: \
        result = 0; \
        break; \
      } \
      return result; \
    }

namespace protobuf {

PbVariant::PbVariant():type(vt_int32) {
};

PB_VARIANT_CTOR(int32);
PB_VARIANT_CTOR(int64);
PB_VARIANT_CTOR(uint32);
PB_VARIANT_CTOR(uint64);
PB_VARIANT_CTOR(double);
PB_VARIANT_CTOR(float);
PB_VARIANT_CTOR(bool);
PbVariant::PbVariant(const std::string& v):type(vt_string){
  value.v_string = new std::string(v);
};
PbVariant::PbVariant(std::string* v) : type(vt_string) {
  value.v_string = v;
};
PbVariant::PbVariant(const PbVariant& v):type(v.type){
  if(v.type == vt_string) {
    value.v_string = new std::string(*(v.value.v_string));
  } else {
    value.v_uint64 = v.value.v_uint64;
  }
};
PbVariant::PbVariant(PbVariant&& v):type(v.type){
  if(v.type == vt_string) {
    value.v_string = v.value.v_string;
    v.value.v_string = NULL;
  } else {
    value.v_uint64 = v.value.v_uint64;
  }
};

PbVariant::~PbVariant(){
  freeString();
};

PB_VARIANT_ASSIGN(int32);
PB_VARIANT_ASSIGN(int64);
PB_VARIANT_ASSIGN(uint32);
PB_VARIANT_ASSIGN(uint64);
PB_VARIANT_ASSIGN(double);
PB_VARIANT_ASSIGN(float);
PB_VARIANT_ASSIGN(bool);
PbVariant& PbVariant::operator = (const std::string& v) {
  if(type == vt_string) {
    *(value.v_string) = v;
  } else {
    type = vt_string;
    value.v_string = new std::string(v);
  }
  return *this;
};
PbVariant& PbVariant::operator = (std::string* v) {
  if(type == vt_string) {
    value.v_string = v;
  } else {
    type = vt_string;
    value.v_string = v;
  }
  return *this;
};
PbVariant& PbVariant::operator = (const PbVariant& v) {
  if(this == &v)
    return *this;
  if(type == vt_string) {
    freeString();
  }
  if(v.type == vt_string) {
    value.v_string = new std::string(*(v.value.v_string));
  } else {
    value.v_uint64 = v.value.v_uint64;
  }

  type = v.type;
  return *this;
};
PbVariant& PbVariant::operator = (PbVariant&& v) {
  if(this == &v)
    return *this;
  if(type == vt_string) {
    freeString();
  }
  if(v.type == vt_string) {
    value.v_string = v.value.v_string;
    v.value.v_string = NULL;
  } else {
    value.v_uint64 = v.value.v_uint64;
  }
  type = v.type;
  return *this;
};

std::string PbVariant::toString() const {
  static std::string emptyString;
  switch (type) {
  case vt_int32:
    return std::to_string(value.v_int32);
  case vt_int64:
    return std::to_string(value.v_int64);
  case vt_uint32:
    return std::to_string(value.v_uint32);
  case vt_uint64:
    return std::to_string(value.v_uint64);
  case vt_double:
    return std::to_string(value.v_double);
  case vt_float:
    return std::to_string(value.v_float);
  case vt_bool:
    return std::to_string(value.v_bool);
  case vt_string:
    if (value.v_string) {
      return *value.v_string;
    } else {
      return emptyString;
    }
  default:
    /// @todo error log.
    break;
  }
  return emptyString;
};

size_t PbVariant::hashcode() const {
  // size_t code;
  switch (type) {
  case vt_int32:
    return std::hash<int32_t>()(value.v_int32);
  case vt_int64:
    return std::hash<int64_t>()(value.v_int64);
  case vt_uint32:
    return std::hash<uint32_t>()(value.v_uint32);
  case vt_uint64:
    return std::hash<uint64_t>()(value.v_uint64);
  case vt_double:
    return std::hash<double>()(value.v_double);
  case vt_float:
    return std::hash<float>()(value.v_float);
  case vt_bool:
    return std::hash<bool>()(value.v_bool);
  case vt_string:
    if (value.v_string) {
      return std::hash<std::string>()(*value.v_string);
    }
  default:
    /// @todo error log.
    break;
  }
  return 0;
};

#define CAST_TO_BOOL(s) (s.empty())
PB_VARIANT_CAST_OP(bool, CAST_TO_BOOL);
PB_VARIANT_CAST_OP(int32, std::stoi);
PB_VARIANT_CAST_OP(int64, std::stol);
PB_VARIANT_CAST_OP(uint32, std::stoul);
PB_VARIANT_CAST_OP(uint64, std::stoul);
PB_VARIANT_CAST_OP(float, std::stof);
PB_VARIANT_CAST_OP(double, std::stod);
PbVariant::operator std::string() const {
  return toString();
};

PB_VARIANT_CMP_OP(V_LT);
PB_VARIANT_CMP_OP(V_LE);
PB_VARIANT_CMP_OP(V_EQ);
PB_VARIANT_CMP_OP(V_GT);
PB_VARIANT_CMP_OP(V_GE);
PB_VARIANT_CMP_OP(V_NE);


void PbVariant::freeString() {
  if(type == vt_string && value.v_string) {
    delete value.v_string;
    value.v_string = NULL;
  }
}




} // namespace protobuf

