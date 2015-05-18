
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

extern "C" {
#include <yajl/yajl_tree.h>
}
#include <sstream>
#include <google/protobuf/message.h>


namespace protobuf {

class ProtobufJson {
public:

  static std::ostream& toJsonStream(std::ostream& os, const google::protobuf::Message* message, bool format = false, int indent = 0);

  /// @brief  Covert protobuf message to json.
  /// @param  message The protobuf message.
  /// @return Json string
  static std::string toJsonString(const google::protobuf::Message* message) {
    std::ostringstream oss;
    toJsonStream(oss, message, false);
    return oss.str();
  }

  /// @brief  to pretty string with indent
  static std::string toPrettyJsonString(const google::protobuf::Message* message) {
    std::ostringstream oss;
    toJsonStream(oss, message, true);
    return oss.str();
  }

  /// @brief  Covert json to protobuf message.
  /// @param  message Protobuf object.
  /// @param  json Json string.
  /// @return A new protobuf message by json, if failed return NULL.
  static int parseJsonFromString(google::protobuf::Message* message, const std::string& json);

  /// @brief  Covert json to protobuf message.
  /// @param  message Protobuf object.
  /// @param  filePath Path of config file with json format.
  /// @return A new protobuf message by json, if failed return NULL.
  static int parseJsonFromFile(google::protobuf::Message* message, const std::string& filePath);

private:
  static int jsonToField(google::protobuf::Message* message, const yajl_val node);

};

} /* namespace idgs */
