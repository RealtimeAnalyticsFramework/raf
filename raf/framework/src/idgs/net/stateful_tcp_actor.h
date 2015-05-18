
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once
#include <asio.hpp>

#include "idgs/actor/stateful_actor.h"

namespace idgs {
namespace net {

class AsyncTcpServer;

class StatefulTcpActor : public idgs::actor::StatefulActor {
public:
  friend class AsyncTcpServer;

  StatefulTcpActor(asio::io_service& io_service, asio::ip::tcp::socket *sock);

  ~StatefulTcpActor();


  virtual const idgs::actor::ActorDescriptorPtr& getDescriptor() const {
    static idgs::actor::ActorDescriptorPtr nullDesc(NULL);
    return nullDesc;
  }
  const std::string& getActorName() const override{
    static const std::string actorName("StatefulTcpActor");
    return actorName;
  }

  virtual void process(const idgs::actor::ActorMessagePtr& msg) override;

  void setClientSerdes(protobuf::SerdesMode serdes) {
    clientSerdes = serdes;
  }
  protobuf::SerdesMode getClientSerdes() const {
    return clientSerdes;
  }

private:
  asio::ip::tcp::socket& socket();
  inline void setActorId(const std::string& id) {
    actorId = id;
  }

  void startReceiveHeader();
  /// alloc memory for body, and start to receive body
  void handle_read_header(const asio::error_code& error, std::shared_ptr<RpcBuffer> readBuffer);

  void handle_read_body(const asio::error_code& error, std::shared_ptr<RpcBuffer> readBuffer);

  void handle_write(const asio::error_code& error, const idgs::actor::ActorMessagePtr& message, size_t bytes_transferred);

  void realSend();

private:
  asio::io_service& io_service_;
  asio::ip::tcp::socket* socket_ = NULL;
  protobuf::SerdesMode clientSerdes = protobuf::PB_BINARY;

  std::atomic_flag writing = ATOMIC_FLAG_INIT;

  static AsyncTcpServer* outerTcpServer;
};
} // namespace net
} // namespace idgs
