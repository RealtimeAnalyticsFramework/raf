
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <memory>
#include "tbb/concurrent_hash_map.h"

#include "google/protobuf/dynamic_message.h"
#include "google/protobuf/compiler/importer.h"

#include "idgs/result_code.h"
#include "protobuf/pbvariant.h"

namespace protobuf {

/// Error message collector. <br>
/// To collect error message when register dynamic protobuf. <br>
class MessageErrorCollector: public google::protobuf::compiler::MultiFileErrorCollector {

  /// @brief  Handle error.
  /// @param  filename The name of protobuf file.
  /// @param  line The line of error.
  /// @param  column The column of error.
  /// @param  message The message of error.
  void AddError(const std::string& filename, int line, int column, const std::string& message);
};

/// Helper of protobuf message. <br>
/// Support several common function of protobuf message. <br>
class MessageHelper {
public:
  MessageHelper();
  virtual ~MessageHelper();

  /// Copy constructor, called by containers.
  /// This should be singleton, copy constructor should be deleted.
  MessageHelper(const MessageHelper& other) = delete;
  MessageHelper(MessageHelper&& other) = delete;
  MessageHelper& operator()(const MessageHelper& other) = delete;
  MessageHelper& operator()(MessageHelper&& other) = delete;

  /// @brief  Register protobuf message
  /// @param  protoFileDesc
  /// @return Status code of result.
  idgs::ResultCode registerDynamicMessage(const ::google::protobuf::FileDescriptorProto& file_proto);

  /// @brief  Register protobuf message into system by proto file.
  /// @param  protoFile The name of protobuf file.
  /// @return Status code of result.
  idgs::ResultCode registerDynamicMessage(const std::string& protoFile);

  /// @brief  Register protobuf message into system by proto in string.
  /// @param  protoContent content of protobuf file.
  /// @return Status code of result.
  idgs::ResultCode registerDynamicMessageFromString(const std::string& protoContent);

  /// @brief  Register protobuf messages into system by all proto files in given path.
  /// @param  path The proto path include many proto files.
  /// @return Status code of result.
//  idgs::ResultCode registerDynamicMessages(const std::string& path);

  /// @brief  Create a protobuf message object by message type name.
  /// @param  typeName The name of message type inlcude package and name.
  /// @return A new protobuf message, if create failed return NULL.
  std::shared_ptr<google::protobuf::Message> createMessage(const std::string& typeName);

  bool isMessageRegistered(const std::string& typeName);

  const google::protobuf::Message* getPbPrototype(const std::string& type);

  idgs::ResultCode setMessageValue(google::protobuf::Message* msg, const std::string& fieldName, const PbVariant& var);

  idgs::ResultCode setMessageValue(google::protobuf::Message* msg, const google::protobuf::FieldDescriptor* field,
      const PbVariant& var);

  /// set message field value
  idgs::ResultCode setMessageValue(google::protobuf::Message* message, const google::protobuf::FieldDescriptor* field,
      const std::string& value);

  idgs::ResultCode getMessageValue(const google::protobuf::Message* msg, const std::string& fieldName, PbVariant& var);

  idgs::ResultCode getMessageValue(const google::protobuf::Message* msg, const google::protobuf::FieldDescriptor* field,
      PbVariant& var);

private:
  google::protobuf::compiler::DiskSourceTree sourceTree;
  MessageErrorCollector pbErrorCollector;
  google::protobuf::compiler::Importer importer;
  google::protobuf::DescriptorPool dynamicPool;
  google::protobuf::DynamicMessageFactory factory, dynamicFactory;

  tbb::concurrent_hash_map<std::string, google::protobuf::Message*> prototypeCache;
};
} /* namespace idgs */
