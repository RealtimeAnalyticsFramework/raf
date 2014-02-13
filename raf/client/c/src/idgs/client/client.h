/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#pragma once

#include "command.h"
#include "client_setting.h"
#include "pb/client_config.pb.h"
#include <asio.hpp>
#include "idgs/net/rpc_buffer.h"

namespace idgs {
namespace client {
class Client {
public:
  Client(const idgs::client::pb::Endpoint& _server) :
      server(_server), io_service(), id(0) {
  }

  const idgs::client::pb::Endpoint& getServerAddress() {
    return server;
  }

  virtual ~Client() {
  }

  void setId(int id_) {
    id = id_;
  }

  int getId() const {
    return id;
  }

  std::string toString() const;

protected:
  const idgs::client::pb::Endpoint& server;
  asio::io_service io_service;
  int id = 0;
};
} // namespace client
} // namespace idgs
