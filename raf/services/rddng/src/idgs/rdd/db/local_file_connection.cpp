
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "local_file_connection.h"

#include "idgs/util/utillity.h"
#include "protobuf/message_helper.h"

using namespace std;
using namespace idgs::actor;
using namespace protobuf;

namespace idgs {
namespace rdd {
namespace db {

LocalFileConnection::LocalFileConnection() {
}

LocalFileConnection::~LocalFileConnection() {
}

const std::string& LocalFileConnection::name() {
  static string name("LOCAL_FILE");
  return name;
}

BaseConnection* LocalFileConnection::clone() const {
  return new LocalFileConnection();
}

void LocalFileConnection::init(const std::map<std::string, std::string>& params) {
  auto it = params.find("LOCAL_FILE_NAME");
  if (it != params.end()) {
    fileFullName = it->second;
  } else {
    fileFullName = "./idgs_local.tbl";
  }

  it = params.find("FILE_SPLITTER");
  if (it != params.end()) {
    splitter = it->second;
  } else {
    splitter = "|";
  }

  it = params.find("PARTITION_ID");
  if (it != params.end()) {
    partition = it->second;
  }
}

idgs::ResultCode LocalFileConnection::connect() {
  if (fileFullName.empty()) {
    return RC_FILE_NOT_FOUND;
  }

  if (fileFullName.c_str()[fileFullName.length() - 1] == '/') {
    return RC_FILE_NOT_FOUND;
  }

  content = "";
  return RC_SUCCESS;
}

ResultCode LocalFileConnection::disconnect() {
  content = "";
  return RC_SUCCESS;
}

ResultCode LocalFileConnection::insert(const PbMessagePtr& key, const PbMessagePtr& value) {
  if (!key) {
    return RC_INVALID_KEY;
  }

  if (!value) {
    return RC_INVALID_VALUE;
  }

  string line = "";

  MessageHelper helper;
  auto keyDescriptor = key->GetDescriptor();
  for (int32_t i = 0; i < keyDescriptor->field_count(); ++i) {
    auto field = keyDescriptor->field(i);
    PbVariant var;
    helper.getMessageValue(key.get(), field, var);
    if (line.size() == 0) {
      line.append(splitter);
    }

    line.append(var.toString());
  }

  auto valueDescriptor = value->GetDescriptor();
  for (int32_t i = 0; i < valueDescriptor->field_count(); ++i) {
    auto field = valueDescriptor->field(i);
    PbVariant var;
    helper.getMessageValue(value.get(), field, var);
    if (line.size() == 0) {
      line.append(splitter);
    }

    line.append(var.toString());
  }

  if (content.size() > 0) {
    content.append("\n");
  }

  content.append(line);

  return RC_SUCCESS;
}

ResultCode LocalFileConnection::get(const PbMessagePtr& key, PbMessagePtr& value) {
  return RC_SUCCESS;
}

ResultCode LocalFileConnection::commit() {
  try {
    string suffix = "_" + partition;
    auto pathPos = fileFullName.find_last_of("/");
    string path, fileName;
    if (pathPos == string::npos) {
      path = "";
      fileName = fileFullName;
    } else {
      path = fileFullName.substr(0, pathPos + 1);
      fileName = fileFullName.substr(pathPos + 1);
    }

    auto pos = fileName.find_last_of(".");
    if (pos == string::npos) {
      fileName = fileName + suffix;
    } else {
      fileName.insert(pos, suffix, 0, suffix.size());
    }

    sys::saveFile(path + fileName, content);
  } catch (exception& ex) {
    LOG(ERROR)<< "Error commit for saving local file, caused by " << ex.what();
    return RC_ERROR;
  }

  return RC_SUCCESS;
}

ResultCode LocalFileConnection::rollback() {
  content = "";
  return RC_SUCCESS;
}

} // db
} // rdd
} // idgs
