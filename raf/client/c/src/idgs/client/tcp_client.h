/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#pragma once

#include "client.h"

namespace idgs {
namespace client {
class TcpClient: public Client {
public:

  TcpClient(const idgs::client::pb::Endpoint& _server) :
      Client(_server) {
  }
  virtual ~TcpClient() {
    function_footprint();
    close();
  }

  idgs::ResultCode close() {
    if (m_socket.get() == NULL) {
      DVLOG(2) << "Client is not started";
      return RC_CLIENT_SOCKET_IS_ALREADY_CLOSED;
    }
    if (!m_socket->is_open()) {
      DVLOG(2) << "Client is not started or is already closed";
      return RC_CLIENT_SOCKET_IS_ALREADY_CLOSED;
    }

    DVLOG(2) << "Client " << this << " will be closed";
    asio::error_code ec;
    m_socket->close(ec);
    if (ec) {
      DVLOG(2) << " close error " << ec;
      return RC_ERROR;
    }
    return RC_SUCCESS;
  }

  std::shared_ptr<asio::ip::tcp::socket> getSocket() {
    return m_socket;
  }

  virtual idgs::ResultCode initialize() = 0;

  virtual ClientActorMessagePtr receive(ResultCode* code, int timeout_sec = 10) = 0;

  virtual ClientActorMessagePtr sendRecv(ClientActorMessagePtr& actorMsg, idgs::ResultCode* code,
      int timeout_sec = 10) = 0;

  /// append by deyin no need to response
  virtual void send(ClientActorMessagePtr& actorMsg, idgs::ResultCode* code, int timeout_sec = 10) = 0;

protected:

  /// @todo remove this function to avoid memory copy.
  static void encode(std::string &data, ResultCode &code, char* data_);

  /// @brief  Decode the header data, received from network, to the body length.
  /// @return The body length.
  static size_t decodeHeader(char* data_, ResultCode &code);

  std::shared_ptr<asio::ip::tcp::socket> m_socket;

};
} // namespace client
} // namespace idgs
