
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "protobuf/message_helper.h"
#include "idgs/util/utillity.h"

using namespace idgs;
using namespace google::protobuf;

namespace protobuf {

static string formatBoolValue(const string& value) {
  // convert string true/false to bool string
  if (strcmp(idgs::str::toUpper(value).c_str(), "TRUE") == 0) {
    return "1";
  } else if (strcmp(idgs::str::toUpper(value).c_str(), "FALSE") == 0) {
    return "0";
  } else {
    return value;
  }
}

MessageHelper::MessageHelper() :
    sourceTree(), pbErrorCollector(), importer(&sourceTree, &pbErrorCollector), factory(importer.pool()), dynamicFactory(&dynamicPool) {
  sourceTree.MapPath("", "");

  // force to load primitive types' descriptor
  ::idgs::pb::Boolean::descriptor();
}

MessageHelper::~MessageHelper() {
  function_footprint();
}

idgs::ResultCode MessageHelper::registerDynamicMessage(const FileDescriptorProto& file_proto) {
  const std::string& file_name = file_proto.name();
  if(dynamicPool.FindFileByName(file_name)) {
    LOG(INFO) << "FileDescriptorProto: " << "\"" << file_name << "\"" <<  " has registered";
    return RC_OK;
  }
  dynamicPool.BuildFile(file_proto);
  return RC_OK;
}

ResultCode MessageHelper::registerDynamicMessage(const string& protoFile) {
  // register protobuf message in proto file.
  const FileDescriptor* fDescriptor = importer.Import(protoFile);
  if (fDescriptor) {
    // discription
    DVLOG(2) << "Loading proto file : " << fDescriptor->name() << ".";
    for (int32_t i = 0; i < fDescriptor->message_type_count(); ++i) {
      DVLOG(2) << "Register proto message : " << fDescriptor->message_type(i)->full_name() << ".";
    }

    return RC_SUCCESS;
  } else {
    return RC_LOAD_PROTO_ERROR;
  }
}

/// @brief  Register protobuf message into system by proto in string.
/// @param  protoContent content of protobuf file.
/// @return Status code of result.
idgs::ResultCode MessageHelper::registerDynamicMessageFromString(const std::string& protoContent) {
  char buff[L_tmpnam];
  auto tmp = tmpnam(buff);
  if (!tmp) {
    LOG(ERROR) << "Failed to generate a temp file name.";
    return RC_ERROR;
  }
  std::string tmpfilename = std::string(buff) + std::to_string(getpid()) + ".proto";
  idgs::sys::saveFile(tmpfilename, protoContent);

  auto rc = registerDynamicMessage(tmpfilename);

  auto ret = remove(tmpfilename.c_str());
  if(ret != 0) {
    LOG(WARNING) << "Failed to remove file " << tmpfilename << ", errno: " << errno << " " << strerror(errno);
  }

  return rc;
}


//ResultCode MessageHelper::registerDynamicMessages(const std::string& path) {
//  ResultCode resultCode = RC_SUCCESS;
//  DIR* dir = opendir(path.c_str());
//
//  if (!dir) {
//    resultCode = RC_LOAD_PROTO_ERROR;
//  } else {
//    struct dirent* dt;
//
//    // loop proto file in path
//    while ((dt = readdir(dir)) != NULL) {
//      string s(dt->d_name);
//      if (s.rfind(".proto") != string::npos) {
//        if (registerDynamicMessage(path + dt->d_name) != RC_SUCCESS) {
//          resultCode = RC_LOAD_PROTO_ERROR;
//        }
//      }
//    }
//  }
//
//  return resultCode;
//}

std::shared_ptr<google::protobuf::Message> MessageHelper::createMessage(const string& typeName) {
  const Message* prototype = getPbPrototype(typeName);

  std::shared_ptr<google::protobuf::Message> result;
  if (prototype) {
    result.reset(prototype->New());
  } else {
    VLOG(1) << "Message type not found: " << typeName;
  }

  return result;
}

const Message* MessageHelper::getPbPrototype(const std::string& typeName) {
  const Message* result;
  tbb::concurrent_hash_map<std::string, google::protobuf::Message*>::const_accessor a;
  if (prototypeCache.find(a, typeName)) {
    result = a->second;
    return result;
  }
  const Descriptor* descriptor = NULL;

  // create dynamic descriptor
  descriptor = importer.pool()->FindMessageTypeByName(typeName);
  if (descriptor) {
    DVLOG(2) << "Creating dynamic proto message by " << typeName << ".";
    result = factory.GetPrototype(descriptor);
  } else {
    // create static descriptor
    descriptor = DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
    if (descriptor) {
      DVLOG(2) << "Creating static proto message by " << typeName << ".";
      result = MessageFactory::generated_factory()->GetPrototype(descriptor);
    } else {
      descriptor = dynamicPool.FindMessageTypeByName(typeName);
      if (descriptor) {
        result = dynamicFactory.GetPrototype(descriptor);
      } else {
        return NULL;
      }
    }
  }

  prototypeCache.insert(make_pair(typeName, const_cast<Message*>(result)));
  return result;
}

bool MessageHelper::isMessageRegistered(const string& typeName) {
  return (getPbPrototype(typeName) != NULL);
}

void MessageErrorCollector::AddError(const std::string& filename, int line, int column, const std::string& message) {
  google::LogMessage(filename.c_str(), line, google::GLOG_WARNING).stream() << ':' << column << ": error: " << message
      << '.';
  // LOG(WARNING) << "Load proto file " << filename << ':' << line <<  ':' << column << ": error: " << message << '.';
}

ResultCode MessageHelper::setMessageValue(Message* msg, const string& fieldName, const PbVariant& var) {
  if (!msg) {
    return RC_INVALID_MESSAGE;
  }

  auto fld = msg->GetDescriptor()->FindFieldByName(fieldName);
  if (!fld) {
    return RC_INVALID_MESSAGE;
  }

  return setMessageValue(msg, fld, var);
}

idgs::ResultCode MessageHelper::setMessageValue(Message* message, const FieldDescriptor* field,
    const std::string& value) {
  assert(message);
  assert(field);
  const Reflection* reflection = message->GetReflection();
  ResultCode rc;
  switch (field->cpp_type()) {
  // handle type unsigned int64
  case FieldDescriptor::CPPTYPE_UINT64: {
    if(value.empty()) {
      reflection->SetUInt64(message, field, 0);
      return RC_OK;
    }
    uint64_t valUInt64 = 0;
    rc = idgs::sys::convert<uint64_t>(value, valUInt64);
    if (rc != RC_OK) {
      LOG(ERROR)<< "convert value error, value: " << value << ", field name: " << field->name() << ", proto message name: " << message->GetTypeName();
      return rc;
    }
    reflection->SetUInt64(message, field, valUInt64);
    break;
  }
    // handle type int64
  case FieldDescriptor::CPPTYPE_INT64: {
    if(value.empty()) {
      reflection->SetInt64(message, field, 0);
      return RC_OK;
    }
    int64_t valInt64 = 0;
    rc = idgs::sys::convert<int64_t>(value, valInt64);
    if (rc != RC_OK) {
      LOG(ERROR)<< "convert value error, value: " << value << ", field name: " << field->name() << ", proto message name: " << message->GetTypeName();
      return rc;
    }
    reflection->SetInt64(message, field, valInt64);
    break;
  }
    // handle type unsigned int32
  case FieldDescriptor::CPPTYPE_UINT32: {
    if(value.empty()) {
      reflection->SetUInt32(message, field, 0);
      return RC_OK;
    }
    uint32_t valUInt32 = 0;
    rc = idgs::sys::convert<uint32_t>(value, valUInt32);
    if (rc != RC_OK) {
      LOG(ERROR)<< "convert value error, value: " << value << ", field name: " << field->name() << ", proto message name: " << message->GetTypeName();
      return rc;
    }
    reflection->SetUInt32(message, field, valUInt32);
    break;
  }
    // handle type int32
  case FieldDescriptor::CPPTYPE_INT32: {
    if(value.empty()) {
      reflection->SetInt32(message, field, 0);
      return RC_OK;
    }
    int32_t valInt32 = 0;
    rc = idgs::sys::convert<int32_t>(value, valInt32);
    if (rc != RC_OK) {
      LOG(ERROR)<< "convert value error, value: " << value << ", field name: " << field->name() << ", proto message name: " << message->GetTypeName();
      return rc;
    }
    reflection->SetInt32(message, field, valInt32);
    break;
  }
    // handle type string
  case FieldDescriptor::CPPTYPE_STRING: {
    if(value.empty()) {
      reflection->SetString(message, field, "");
      return RC_OK;
    }
    reflection->SetString(message, field, value);
    break;
  }
    // handle type double
  case FieldDescriptor::CPPTYPE_DOUBLE: {
    if(value.empty()) {
      reflection->SetDouble(message, field, 0);
      return RC_OK;
    }
    double valDouble = 0;
    rc = idgs::sys::convert<double>(value, valDouble);
    if (rc != RC_OK) {
      LOG(ERROR)<< "convert value error, value: " << value << ", field name: " << field->name() << ", proto message name: " << message->GetTypeName();
      return rc;
    }
    reflection->SetDouble(message, field, valDouble);
    break;
  }
    // handle type float
  case FieldDescriptor::CPPTYPE_FLOAT: {
    if(value.empty()) {
      reflection->SetFloat(message, field, 0);
      return RC_OK;
    }
    float valFloat = 0;
    rc = idgs::sys::convert<float>(value, valFloat);
    if (rc != RC_OK) {
      LOG(ERROR)<< "convert value error, value: " << value;
      return rc;
    }
    reflection->SetFloat(message, field, valFloat);
    break;
  }
    // handle type bool
  case FieldDescriptor::CPPTYPE_BOOL: {
    if(value.empty()) {
      reflection->SetBool(message, field, false);
      return RC_OK;
    }
    bool valBool = false;
    rc = idgs::sys::convert<bool>(formatBoolValue(value), valBool);
    if (rc != RC_OK) {
      LOG(ERROR)<< "convert value error, value: " << value << ", field name: " << field->name() << ", proto message name: " << message->GetTypeName();
      return rc;
    }
    reflection->SetBool(message, field, valBool);
    break;
  }
    // handle type enum
  case FieldDescriptor::CPPTYPE_ENUM: {
    if(value.empty()) {
      return RC_OK;
    }
    reflection->SetEnum(message, field, field->enum_type()->FindValueByName(value));
    break;
  }
  default:
    break;
  }
  return RC_SUCCESS;
}

ResultCode MessageHelper::setMessageValue(Message* msg, const FieldDescriptor* field, const PbVariant& var) {
  assert(msg);
  assert(field);
  auto ref = msg->GetReflection();
  switch (field->cpp_type()) {
  case FieldDescriptor::CPPTYPE_UINT64: {
    ref->SetUInt64(msg, field, (uint64_t) var);
    break;
  }
  case FieldDescriptor::CPPTYPE_INT64: {
    ref->SetInt64(msg, field, (int64_t) var);
    break;
  }
  case FieldDescriptor::CPPTYPE_UINT32: {
    ref->SetUInt32(msg, field, (uint32_t) var);
    break;
  }
  case FieldDescriptor::CPPTYPE_INT32: {
    ref->SetInt32(msg, field, (int32_t) var);
    break;
  }
  case FieldDescriptor::CPPTYPE_STRING: {
    ref->SetString(msg, field, (string) var);
    break;
  }
  case FieldDescriptor::CPPTYPE_DOUBLE: {
    ref->SetDouble(msg, field, (double) var);
    break;
  }
  case FieldDescriptor::CPPTYPE_FLOAT: {
    ref->SetFloat(msg, field, (float) var);
    break;
  }
  case FieldDescriptor::CPPTYPE_BOOL: {
    ref->SetBool(msg, field, (bool) var);
    break;
  }
  case FieldDescriptor::CPPTYPE_ENUM:
    return RC_NOT_SUPPORT;
  case FieldDescriptor::CPPTYPE_MESSAGE:
    return RC_NOT_SUPPORT;
  default:
    return RC_NOT_SUPPORT;
  }

  return RC_SUCCESS;
}

ResultCode MessageHelper::getMessageValue(const Message* msg, const string& fieldName, PbVariant& var) {
  assert(msg);
  auto fld = msg->GetDescriptor()->FindFieldByName(fieldName);
  assert(fld);
  return getMessageValue(msg, fld, var);
}

ResultCode MessageHelper::getMessageValue(const Message* msg, const FieldDescriptor* field, PbVariant& var) {
  if (!msg) {
    return RC_INVALID_MESSAGE;
  }

  auto ref = msg->GetReflection();
  switch (field->cpp_type()) {
  case FieldDescriptor::CPPTYPE_UINT64: {
    var = ref->GetUInt64(*msg, field);
    break;
  }
  case FieldDescriptor::CPPTYPE_INT64: {
    var = ref->GetInt64(*msg, field);
    break;
  }
  case FieldDescriptor::CPPTYPE_UINT32: {
    var = ref->GetUInt32(*msg, field);
    break;
  }
  case FieldDescriptor::CPPTYPE_INT32: {
    var = ref->GetInt32(*msg, field);
    break;
  }
  case FieldDescriptor::CPPTYPE_STRING: {
    std::string s;
    s = ref->GetStringReference(*msg, field, &s);
    var = s;
    break;
  }
  case FieldDescriptor::CPPTYPE_DOUBLE: {
    var = ref->GetDouble(*msg, field);
    break;
  }
  case FieldDescriptor::CPPTYPE_FLOAT: {
    var = ref->GetFloat(*msg, field);
    break;
  }
  case FieldDescriptor::CPPTYPE_BOOL: {
    var = ref->GetBool(*msg, field);
    break;
  }
  case FieldDescriptor::CPPTYPE_ENUM:
    return RC_NOT_SUPPORT;
  case FieldDescriptor::CPPTYPE_MESSAGE:
    return RC_NOT_SUPPORT;
  default:
    return RC_NOT_SUPPORT;
  }

  return RC_SUCCESS;
}

} /* namespace idgs */
