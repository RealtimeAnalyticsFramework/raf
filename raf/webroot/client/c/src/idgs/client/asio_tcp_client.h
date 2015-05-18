/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#pragma once

#include "tcp_client.h"
#include <asio.hpp>


namespace idgs {
namespace client {
class AsioTcpClient: public TcpClient {
public:
  AsioTcpClient(asio::io_service& io, const idgs::client::pb::Endpoint& _server);
  AsioTcpClient(const idgs::client::pb::Endpoint& _server);

  idgs::ResultCode initialize() override;
  idgs::ResultCode send(ClientActorMessagePtr& actorMsg, int timeout_sec = 10) override;
  idgs::ResultCode receive(ClientActorMessagePtr& msg, int timeout_sec = 10) override;
  idgs::ResultCode sendRecv(ClientActorMessagePtr& actorMsg, ClientActorMessagePtr& response, int timeout_sec = 10) override;
  idgs::ResultCode close() override;
  bool isOpen() const override {
    return m_socket && m_socket->is_open();
  }

  const idgs::client::pb::Endpoint& getServerEndpoint() const override {
    return server;
  }
  void setId(int id_) override{
    id = id_;
  }

  int getId() const override {
    return id;
  }


private:
  void check_deadline();

  idgs::ResultCode nonblockingConnect();
  idgs::ResultCode nonblockingRead(ClientActorMessagePtr& msg, int timeout_sec = 10);
  idgs::ResultCode nonblockingWrite(ClientActorMessagePtr& actorMsg, int timeout_sec = 10);

  std::shared_ptr<asio::ip::tcp::socket>& getSocket() {
    return m_socket;
  }
protected:
  static void setSocketOption (asio::ip::tcp::socket* sock) ;

  /// @todo remove this function to avoid memory copy.
  static void encode(std::string &data, ResultCode &code, char* data_);

  /// @brief  Decode the header data, received from network, to the body length.
  /// @return The body length.
  static size_t decodeHeader(char* data_, ResultCode &code);


private:
  asio::io_service io_service;
  idgs::client::pb::Endpoint server;
  std::shared_ptr<asio::ip::tcp::socket> m_socket;
  int id = 0;

  std::unique_ptr<asio::steady_timer> deadline;
};
} // namespace client
} // namespace idgs
