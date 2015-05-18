
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <asio.hpp>
#include <tbb/concurrent_queue.h>
#include "idgs/actor/actor_message.h"

namespace idgs {
namespace net {
class InnerTcpServer;
class NetworkModelAsio;


#pragma pack(push)
#pragma pack(1)
struct InnerTcpHandShake {
  uint32_t memberId;
};
#pragma pack(pop)


class InnerTcpConnection : public std::enable_shared_from_this<InnerTcpConnection> {
public:
  friend class InnerTcpServer;
  InnerTcpConnection(asio::io_service& ios);
  virtual ~InnerTcpConnection();

public:
  uint32_t getPeerMemberId() const {
    return peerMemberId;
  }

  int32_t sendMessage(idgs::actor::ActorMessagePtr& message);

  std::string toString();
  uint32_t connect(uint32_t memberId, int retry = 0);
  void terminate();

private: /// called by inner TCP server

  asio::ip::tcp::socket& getSocket() {
    return socket;
  }

  void handleConnect(const asio::error_code& error, int retry);
  int accept();

private:
  void startRecvHeader();
  void handleRecvHeader(const asio::error_code& error, std::shared_ptr<RpcBuffer> readBuffer);
  void handleRecvBody(const asio::error_code& error, std::shared_ptr<RpcBuffer> readBuffer);

  void realSendMessage();
  void handleSendMessage(const asio::error_code& error);

  std::shared_ptr<tbb::concurrent_queue<idgs::actor::ActorMessagePtr> > getQueue();

  void setPeerMemberId(const uint32_t memberId_) {
    peerMemberId = memberId_;
  }

private:
  asio::io_service& io_service;
  asio::ip::tcp::socket socket;

  DEF_ENUM (InnerTcpConnectionState,
    INITIAL,
    CONNECTING,
    READY,
    WRITING,
    TERMINATED
  );

  std::atomic<InnerTcpConnectionState> state;
  int peerMemberId;

  std::shared_ptr<tbb::concurrent_queue<idgs::actor::ActorMessagePtr> > queue;
  int try_pop_count = 0;

  static InnerTcpServer* innerTcpServer;
};

} // namespace net 
} // namespace idgs 
