
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "config_parser.h"

#include "protobuf/protobuf_json.h"
#include "idgs/util/utillity.h"


namespace idgs {
namespace store {

ResultCode StoreConfigParser::parseStoreConfigFromFile(const std::string& path, pb::DataStoreConfig* config) {
  ResultCode code = (ResultCode) idgs::parseIdgsConfig(config, path);
  if (code != RC_SUCCESS) {
    LOG(ERROR)<< "parse store config error, caused by " << getErrorDescription(code);
    return code;
  }

  return parseStoreConfig(config);
}

ResultCode StoreConfigParser::parseStoreConfigFromString(const std::string& content, pb::DataStoreConfig* config) {
  ResultCode code = (ResultCode) protobuf::ProtobufJson::parseJsonFromString(config, content);
  if (code != RC_SUCCESS) {
    LOG(ERROR)<< "parse store config error, caused by " << getErrorDescription(code);
    return code;
  }

  return parseStoreConfig(config);
}

ResultCode StoreConfigParser::parseStoreConfig(pb::DataStoreConfig* config) {
  ResultCode code = RC_SUCCESS;

  // parsed by files
  auto size = config->schema_files_size();
  if (size > 0) {
    for (int32_t i = 0; i < config->schema_files_size(); ++ i) {
      auto storeSchema = config->add_schemas();
      code = (ResultCode) idgs::parseIdgsConfig(storeSchema, config->schema_files(i));
      if (code != RC_SUCCESS) {
        LOG(ERROR)<< "parse store config error, caused by " << getErrorDescription(code);
        return code;
      }
    }
    config->clear_schema_files();
  }

  // parsed by content
  size = config->schema_contents_size();
  if (size > 0) {
    for (int32_t i = 0; i < config->schema_contents_size(); ++ i) {
      auto storeSchema = config->add_schemas();
      code = (ResultCode) protobuf::ProtobufJson::parseJsonFromString(storeSchema, config->schema_contents(i));
      if (code != RC_SUCCESS) {
        LOG(ERROR)<< "parse store config error, caused by " << getErrorDescription(code);
        return code;
      }
    }
    config->clear_schema_contents();
  }

  return code;
}

} // namespace store
} // namespace idgs
