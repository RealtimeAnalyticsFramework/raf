/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#include "loader.h"
#include "protobuf/message_helper.h"
#include "idgs/util/utillity.h"
#include "idgs/store/data_store.h"

using namespace idgs::store;
using namespace google::protobuf;

#define DEFAULT_ONE_LINE_BUFFER_SIZE 1024

namespace idgs {
namespace client {

idgs::ResultCode Loader::init(LoaderSettings* settings) {
  this->settings = settings;
  idgs::ResultCode rc = loadMapperConfig();
  if (rc != RC_OK) {
    LOG(ERROR)<< "load store file mapper config file error, " << idgs::getErrorDescription(rc);
    return rc;
  }
  return RC_OK;
}

void static createStoreFieldDescriptor(const std::string& field_name, const PbMessagePtr& key,
    const PbMessagePtr& value, ParsedStoreFieldDescriptor* descriptor) {
  for (size_t i = 0, count = key->GetDescriptor()->field_count(); i < count; ++i) {
    if (key->GetDescriptor()->field(i)->name().compare(field_name) == 0) {
      descriptor->type = KEY_TYPE;
      descriptor->descriptor = key->GetDescriptor()->field(i);
      break;
    }
  }
  for (size_t j = 0, count = value->GetDescriptor()->field_count(); j < count; ++j) {
    if (value->GetDescriptor()->field(j)->name().compare(field_name) == 0) {
      descriptor->type = VALUE_TYPE;
      descriptor->descriptor = value->GetDescriptor()->field(j);
      break;
    }
  }
}

void Loader::config() {
  all_load_files.reserve(mapper_config->mapper_size()); /// reserve files
  for (auto it = mapper_config->mapper().begin(); it != mapper_config->mapper().end(); ++it) {
    const std::string& file_name = it->file_name();
    all_load_files.push_back(file_name);
    const std::string& store_name = it->store_name();
    file_store_map[file_name] = store_name;
    if (!isParseLine) {
      continue;
    }
    StoreConfigWrapperPtr store_config_wrapper_ptr;
    ResultCode rs = ::idgs::util::singleton<idgs::store::DataStore>::getInstance().loadStoreConfig(store_name,
        store_config_wrapper_ptr);
    if (rs != idgs::RC_OK) {
      LOG(ERROR)<< "load store config  error, " << getErrorDescription(rs) << ", store name: " << store_name;
    }
    ParsedStoreDescriptor descriptor;
    descriptor.mapper->CopyFrom(*it);
    const std::string& key_type = store_config_wrapper_ptr->getStoreConfig().key_type();
    const std::string& value_type = store_config_wrapper_ptr->getStoreConfig().value_type();
    descriptor.key_type = key_type;
    descriptor.value_type = value_type;
    PbMessagePtr key_type_msg = ::idgs::util::singleton<protobuf::MessageHelper>::getInstance().createMessage(key_type);
    PbMessagePtr value_type_msg = ::idgs::util::singleton<protobuf::MessageHelper>::getInstance().createMessage(
        value_type);
    if (it->fields_size() > 0) { /// user defined fields
      descriptor.fieldDescriptor.reserve(it->fields_size());
      for (auto ft = it->fields().begin(); ft != it->fields().end(); ++ft) {
        ParsedStoreFieldDescriptor field_descriptor;
        createStoreFieldDescriptor(*ft, key_type_msg, value_type_msg, &field_descriptor);
        descriptor.fieldDescriptor.push_back(field_descriptor);
      }
    } else { /// user not define, using store config default fields
      for (size_t i = 0, count = key_type_msg->GetDescriptor()->field_count(); i < count; ++i) {
        ParsedStoreFieldDescriptor field_descriptor;
        field_descriptor.type = KEY_TYPE;
        field_descriptor.descriptor = key_type_msg->GetDescriptor()->field(i);
        descriptor.fieldDescriptor.push_back(field_descriptor);
      }
      for (size_t j = 0, count = value_type_msg->GetDescriptor()->field_count(); j < count; ++j) {
        ParsedStoreFieldDescriptor field_descriptor;
        field_descriptor.type = VALUE_TYPE;
        field_descriptor.descriptor = value_type_msg->GetDescriptor()->field(j);
        descriptor.fieldDescriptor.push_back(field_descriptor);
      }
    }
    store_descriptor_cache.insert(std::pair<std::string, ParsedStoreDescriptor>(it->store_name(), descriptor));
  }
  stringstream s;
  s << "\n==================load file list==============" << "\n";
  string dir = settings->data_path;
  if (dir.substr(dir.length() - 1, 1) != "/") {
    dir = dir.append("/");
  }
  for (auto it = all_load_files.begin(); it != all_load_files.end(); ++it) {
    s << dir << *it << "\n";
  }
  s << "================//load file list==============" << "\n";
  LOG(INFO)<< s.str();
}

idgs::ResultCode Loader::loadMapperConfig() {
  idgs::ResultCode rc = protobuf::JsonMessage().parseJsonFromFile(mapper_config, settings->mapper_cfg_file);
  if (rc != RC_OK) {
    LOG(ERROR)<< "parse mapper file error, " << idgs::getErrorDescription(rc) << ", file path:" << settings->mapper_cfg_file;
    return rc;
  }
  return RC_OK;
}

KeyValueMessagePair Loader::parseLine(const std::string& store_name, const std::string& line, idgs::ResultCode* rc) {
  auto start = sys::getCurrentTime();
  const ParsedStoreDescriptor& descriptor = store_descriptor_cache.at(store_name);
  vector<string> result;
  idgs::str::split(line, descriptor.mapper->seperator(), result);
  PbMessagePtr key = ::idgs::util::singleton<protobuf::MessageHelper>::getInstance().createMessage(descriptor.key_type);
  PbMessagePtr value = ::idgs::util::singleton<protobuf::MessageHelper>::getInstance().createMessage(
      descriptor.value_type);
  if (!key.get()) {
    LOG(ERROR)<< "Parse line error, key message is null, store descriptor: " << descriptor.toString();
    *rc = RC_PARSE_LINE_ERROR;
    return KeyValueMessagePair(NULL, NULL);
  }
  if (!value.get()) {
    LOG(ERROR)<< "Parse line error, value message is null, store descriptor: " << descriptor.toString();
    *rc = RC_PARSE_LINE_ERROR;
    return KeyValueMessagePair(NULL, NULL);
  }
  if (result.size() != descriptor.fieldDescriptor.size()) {
    std::stringstream s;
    s << "########################fields:######################## " << std::endl;
    for (size_t index = 0, size = descriptor.fieldDescriptor.size(); index < size; ++index) {
      s << index << ": " << descriptor.fieldDescriptor[index].toString();
    }
    s << std::ends;
    std::stringstream ss;
    ss << "########################results:######################## " << std::endl;
    for (size_t index = 0, size = result.size(); index < size; ++index) {
      ss << index << ": " << result[index] << std::endl;
    }
    ss << std::ends;
    LOG(ERROR)<< "Parse store:" << store_name <<" error \n line: " << line << " \n caused by result size != field size" << ", result size:" << result.size() << ", field size:" << descriptor.fieldDescriptor.size()
    << " \n" << s.str() << ss.str();
    *rc = RC_PARSE_LINE_ERROR;
    return KeyValueMessagePair(NULL, NULL);
  }
  for (size_t index = 0, size = result.size(); index < size; ++index) {
    const ParsedStoreFieldDescriptor& field_descriptor = descriptor.fieldDescriptor.at(index);
    if (field_descriptor.type == KEY_TYPE) {
      *rc = ::idgs::util::singleton<protobuf::MessageHelper>::getInstance().setMessageValue(key.get(),
          field_descriptor.descriptor, result.at(index));
      if (*rc != RC_SUCCESS) {
        LOG(ERROR)<< "set Message KEY's field value error, error code: " << *rc
        << ", field index: " << index << ", field name: " << field_descriptor.descriptor->name() << ", field value: " << result[index] << ", line: " << line;
        return KeyValueMessagePair(NULL, NULL);
      }
    }
    else if(field_descriptor.type == VALUE_TYPE) {
      *rc = ::idgs::util::singleton<protobuf::MessageHelper>::getInstance().setMessageValue(value.get(), field_descriptor.descriptor, result.at(index));
      if(*rc != RC_SUCCESS) {
        LOG(ERROR) << "set Message VALUE's field value error, error code: " << *rc
        << ", field index: " << index << ", field name: " << field_descriptor.descriptor->name() << ", field value: " << result[index] << ", line: " << line;
        return KeyValueMessagePair(NULL, NULL);
      }
    }
  }
  LOG_FIRST_N(INFO, 1) << "parse one line spent " << sys::formatTime((sys::getCurrentTime() - start));
  return std::pair<PbMessagePtr, PbMessagePtr>(key, value);
}

int Loader::readline(string& line) {
  if (this->file_index >= all_load_files.size()) {
    return 3;
  }
  if ((!this->curr_file_stream.is_open()) && (this->file_index < all_load_files.size())) {
    string dir = settings->data_path;
    if (dir.substr(dir.length() - 1, 1) != "/") {
      dir = dir.append("/");
    }
    string filePath = dir + all_load_files[file_index];
    curr_file_stream.open(filePath.c_str()); /// open file
    if (!curr_file_stream.is_open()) {
      LOG(ERROR)<< " open file error, file path: " << filePath;
      return 2;
    }
    LOG(INFO)<< "Open file: " << filePath << ", store name: " << file_store_map[all_load_files[file_index]];
  }
  if (curr_file_stream.eof()) {
    curr_file_stream.close();
    LOG(INFO)<< "Table " << all_load_files[file_index] << " is done, total insert line(s) = "<< records.load();
    ++file_index; /// move to next file
    return 1;
  }
  std::getline(curr_file_stream, line);
  return 0;
}
}
}
