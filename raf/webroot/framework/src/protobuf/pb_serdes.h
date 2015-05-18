
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <stdexcept>
#include <google/protobuf/text_format.h>

#include "protobuf/protobuf_json.h"

namespace protobuf {

enum SerdesMode {
  PB_BINARY = 0,
  PB_JSON   = 1,
  PB_TEXT   = 2
};

#if !defined(DEFAULT_PB_SERDES)
#define DEFAULT_PB_SERDES 0 // PB_BINARY
//#define DEFAULT_PB_SERDES 1 // PB_JSON
//#define DEFAULT_PB_SERDES 2 // PB_TEXT
#endif // !defined(DEFAULT_PB_SERDES)

template < int MODE >
struct ProtoSerdes {
  static inline bool serialize(const google::protobuf::Message* , std::string* ) {
    throw std::runtime_error(std::string().append(__FUNCTION__).append(" not supported"));
    return false;
  };
  static inline bool serialize(const google::protobuf::Message* , char*, size_t) {
    throw std::runtime_error(std::string().append(__FUNCTION__).append(" not supported"));
    return false;
  };
  static inline bool deserialize(const std::string& , google::protobuf::Message* ) {
    throw std::runtime_error(std::string().append(__FUNCTION__).append(" not supported"));
    return false;
  };
  static inline bool deserializeFromArray(const char*, size_t, google::protobuf::Message* ) {
    throw std::runtime_error(std::string().append(__FUNCTION__).append(" not supported"));
    return false;
  };
}; // struct ProtoSerdes

/// serdes to/from binary
template <>
struct ProtoSerdes<PB_BINARY> {
  static inline bool serialize(const google::protobuf::Message* message, std::string* buffer) {
    *buffer = message->SerializeAsString();
    return true;
  };
  static inline bool serialize(const google::protobuf::Message* message, char* buff, size_t size) {
    return message->SerializeToArray(buff, size);
  };
  static inline bool deserialize(const std::string& buffer, google::protobuf::Message* message) {
    return message->ParseFromString(buffer);
  };
  static inline bool deserializeFromArray(const char* buffer, size_t len, google::protobuf::Message* message) {
    return message->ParseFromArray(buffer, len);
  };
}; // struct ProtoSerdes

/// serdes to/from json
template <>
struct ProtoSerdes<PB_JSON> {
  static inline bool serialize(const google::protobuf::Message* message, std::string* buffer) {
    *buffer = protobuf::ProtobufJson::toJsonString(message);
    return true;
  };
  static inline bool serialize(const google::protobuf::Message* message, char* buff, size_t size) {
    std::string s = protobuf::ProtobufJson::toJsonString(message);
    assert(size > s.size());
    memcpy(buff, s.data(), s.size());
    return true;
  };
  static inline bool deserialize(const std::string& buffer, google::protobuf::Message* message) {
    return !protobuf::ProtobufJson::parseJsonFromString(message, buffer);
  };
  static inline bool deserializeFromArray(const char* buffer, size_t len, google::protobuf::Message* message) {
    std::string s(buffer, len);
    return !protobuf::ProtobufJson::parseJsonFromString(message, s);
  };
}; // struct ProtoSerdes

/// serdes to/from text
template <>
struct ProtoSerdes<PB_TEXT> {
  static inline bool serialize(const google::protobuf::Message* message, std::string* buffer) {
    return google::protobuf::TextFormat::PrintToString(*message, buffer);
  };
  static inline bool serialize(const google::protobuf::Message* message, char* buff, size_t size) {
    std::string s;
    google::protobuf::TextFormat::PrintToString(*message, &s);
    assert(size > s.size());
    memcpy(buff, s.data(), s.size());
    return true;
  };
  static inline bool deserialize(const std::string& buffer, google::protobuf::Message* message) {
    return google::protobuf::TextFormat::ParseFromString(buffer, message);
  };
  static inline bool deserializeFromArray(const char* buffer, size_t len, google::protobuf::Message* message) {
    std::string s(buffer, len);
    return google::protobuf::TextFormat::ParseFromString(s, message);
  };
}; // struct ProtoSerdes

/// serdes helper: no method call but branch
struct ProtoSerdesHelper {
  static inline bool serialize(SerdesMode mode, const google::protobuf::Message* message, std::string* buffer) {
    switch (mode) {
    case PB_BINARY:
      return ProtoSerdes<PB_BINARY>::serialize(message, buffer);
    case PB_JSON:
      return ProtoSerdes<PB_JSON>::serialize(message, buffer);
    case PB_TEXT:
      return ProtoSerdes<PB_TEXT>::serialize(message, buffer);
    }
    return false;
  };
  static inline bool serialize(SerdesMode mode, const google::protobuf::Message* message, char* buff, size_t size) {
    switch (mode) {
    case PB_BINARY:
      return ProtoSerdes<PB_BINARY>::serialize(message, buff, size);
    case PB_JSON:
      return ProtoSerdes<PB_JSON>::serialize(message, buff, size);
    case PB_TEXT:
      return ProtoSerdes<PB_TEXT>::serialize(message, buff, size);
    }
    return false;
  };
  static inline bool deserialize(SerdesMode mode, const std::string& buffer, google::protobuf::Message* message) {
    switch (mode) {
    case PB_BINARY:
      return ProtoSerdes<PB_BINARY>::deserialize(buffer, message);
    case PB_JSON:
      return ProtoSerdes<PB_JSON>::deserialize(buffer, message);
    case PB_TEXT:
      return ProtoSerdes<PB_TEXT>::deserialize(buffer, message);
    }
    return false;
  };
  static inline bool deserializeFromArray(SerdesMode mode, const char* buffer, size_t len, google::protobuf::Message* message) {
    switch (mode) {
    case PB_BINARY:
      return ProtoSerdes<PB_BINARY>::deserializeFromArray(buffer, len, message);
    case PB_JSON:
      return ProtoSerdes<PB_JSON>::deserializeFromArray(buffer, len, message);
    case PB_TEXT:
      return ProtoSerdes<PB_TEXT>::deserializeFromArray(buffer, len, message);
    }
    return false;
  };
}; // struct ProtoSerdesHelper

/// serdes helper: no branch but method call
struct ProtoSerdesHelper1 {
  static inline bool serialize(SerdesMode mode, const google::protobuf::Message* message, char* buff, size_t size) {
    typedef bool (*FUNC_SER)(const google::protobuf::Message*, char*, size_t);
    static FUNC_SER sers[] = {ProtoSerdes<PB_BINARY>::serialize, ProtoSerdes<PB_JSON>::serialize, ProtoSerdes<PB_TEXT>::serialize};
    return sers[mode](message, buff, size);
  };

  static inline bool serialize(SerdesMode mode, const google::protobuf::Message* message, std::string* buffer) {
    typedef bool (*FUNC_SER)(const google::protobuf::Message*, std::string*);
    static FUNC_SER sers[] = {ProtoSerdes<PB_BINARY>::serialize, ProtoSerdes<PB_JSON>::serialize, ProtoSerdes<PB_TEXT>::serialize};
    return sers[mode](message, buffer);
  };

  static inline bool deserialize(SerdesMode mode, const std::string& buffer, google::protobuf::Message* message) {
    typedef bool (*FUNC_DES)(const std::string& buffer, google::protobuf::Message* message);
    static FUNC_DES deses[] = {ProtoSerdes<PB_BINARY>::deserialize, ProtoSerdes<PB_JSON>::deserialize, ProtoSerdes<PB_TEXT>::deserialize};
    return deses[mode](buffer, message);
  };
  static inline bool deserializeFromArray(SerdesMode mode, const char* buffer, size_t len, google::protobuf::Message* message) {
    typedef bool (*FUNC_DES)(const char* buffer, size_t len, google::protobuf::Message* message);
    static FUNC_DES deses[] = {ProtoSerdes<PB_BINARY>::deserializeFromArray, ProtoSerdes<PB_JSON>::deserializeFromArray, ProtoSerdes<PB_TEXT>::deserializeFromArray};
    return deses[mode](buffer, len, message);
  }
}; // struct ProtoSerdesHelper1
} // namespace protobuf
