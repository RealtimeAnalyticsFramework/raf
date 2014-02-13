/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#pragma once

#include "tcp_client.h"

namespace idgs {
namespace client {
class TcpAsynchClient: public TcpClient {
  friend class TcpClientPool;
public:
  TcpAsynchClient(const idgs::client::pb::Endpoint& _server);
  TcpAsynchClient(const TcpAsynchClient&) = default;
  TcpAsynchClient(TcpAsynchClient&&) = default;

  idgs::ResultCode initialize();

  ClientActorMessagePtr receive(ResultCode* code, int timeout_sec = 10);
  ClientActorMessagePtr sendRecv(ClientActorMessagePtr& actorMsg, idgs::ResultCode* code, int timeout_sec = 10);

  void send(ClientActorMessagePtr& actorMsg, idgs::ResultCode* code, int timeout_sec = 10);

private:
  idgs::ResultCode connect();
  ClientActorMessagePtr synchRead(ResultCode& code);
  idgs::ResultCode synchWrite(ClientActorMessagePtr& actorMsg);

  void check_deadline();
  void timeoutHandler(const asio::error_code& error);

  std::unique_ptr<asio::deadline_timer> deadline;

  int timeout_sec;
};
} // namespace client
} // namespace idgs
