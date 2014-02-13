/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#pragma once

#include "idgs/store/pb/store_service.pb.h"

namespace idgs {
namespace client {
class ClientSetting {
public:
  ClientSetting() :
      silient(false) {
    msgs.resize(20);
    msgs[idgs::store::pb::SRC_SUCCESS] = "Success";
    msgs[idgs::store::pb::SRC_KEY_EXIST] = "Key is already exists.";
    msgs[idgs::store::pb::SRC_KEY_NOT_EXIST] = "Key is not exists";
    msgs[idgs::store::pb::SRC_VERSION_CONFLICT] = "Version conflict.";
    msgs[idgs::store::pb::SRC_TABLE_NOT_EXIST] = "Table not found.";
    msgs[idgs::store::pb::SRC_DATA_NOT_FOUND] = "Data not found.";
    msgs[idgs::store::pb::SRC_PARTITION_NOT_FOUND] = "Partition not found.";
    msgs[idgs::store::pb::SRC_PARTITION_NOT_READY] = "Partition is not ready.";
    msgs[idgs::store::pb::SRC_INVALID_KEY] = "Invalid key.";
    msgs[idgs::store::pb::SRC_INVALID_VALUE] = "Invalid value.";
    msgs[idgs::store::pb::SRC_INVALID_FILTER] = "Invalid filter.";
    msgs[idgs::store::pb::SRC_UNKNOWN_ERROR] = "Unknown error.";
  }

  ~ClientSetting() {
    msgs.clear();
  }

  std::string clientConfig;
  /// @todo remove
  std::string storeConfig;
  std::string scriptFile;
  bool silient = false;

  std::vector<std::string> msgs;
};
}
} /* namespace idgs */
