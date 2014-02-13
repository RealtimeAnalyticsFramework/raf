
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <sstream>
#include <iostream>
#include <string>

#include "idgs/store/pb/store_file_mapper.pb.h"

namespace idgs {
namespace store {
enum ParsedFieldType {
  UNKNOWN_TYPE, KEY_TYPE, VALUE_TYPE
};
struct ParsedStoreFieldDescriptor {
  ParsedFieldType type;
  const google::protobuf::FieldDescriptor* descriptor;
  ParsedStoreFieldDescriptor() :
      type(UNKNOWN_TYPE), descriptor(NULL) {

  }
  ParsedStoreFieldDescriptor(const ParsedStoreFieldDescriptor& descriptor) = default;
  ParsedStoreFieldDescriptor(ParsedStoreFieldDescriptor&& descriptor) = default;
  ParsedStoreFieldDescriptor& operator =(const ParsedStoreFieldDescriptor& other) = default;
  std::string toString() const {
    std::stringstream s;
    s << "type: " << type << std::ends;
    if (descriptor) {
      s << "proto descriptor: " << descriptor->DebugString();
    }
    return s.str();
  }
};
typedef std::shared_ptr<google::protobuf::Message> PbMessagePtr;
struct ParsedStoreDescriptor {
  std::string key_type;
  std::string value_type;
  idgs::store::pb::StoreFileMapper* mapper;
  std::vector<ParsedStoreFieldDescriptor> fieldDescriptor;
  ParsedStoreDescriptor(const ParsedStoreDescriptor& descriptor) = default;
  ParsedStoreDescriptor(ParsedStoreDescriptor&& descriptor) = default;
  ParsedStoreDescriptor& operator =(const ParsedStoreDescriptor& other) = default;
  ParsedStoreDescriptor() :
      key_type("unknown"), value_type("unknown"), mapper(
          new idgs::store::pb::StoreFileMapper) {

  }
  std::string toString() const {
    std::stringstream s;
    s << "key type: " << key_type << std::ends;
    s << "value type: " << value_type << std::ends;
    for (auto it = fieldDescriptor.begin(); it != fieldDescriptor.end(); ++it) {
      s << it->toString();
    }
    return s.str();
  }
};
typedef std::pair<std::shared_ptr<google::protobuf::Message>, std::shared_ptr<google::protobuf::Message>> KeyValueMessagePair;
typedef std::shared_ptr<idgs::store::StoreConfigWrapper> StoreConfigWrapperPtr;
typedef std::map<std::string, ParsedStoreDescriptor> StoreDescriptorMap;
}
}
