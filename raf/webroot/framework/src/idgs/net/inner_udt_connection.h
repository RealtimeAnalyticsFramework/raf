
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once
#if defined(WITH_UDT)



namespace idgs {
namespace net {

class InnerUdtServer;
class NetworkModelAsio;

class InnerUdtConnection : public std::enable_shared_from_this<InnerUdtConnection> {
public:
  friend class InnerUdtServer;
  InnerUdtConnection();
  virtual ~InnerUdtConnection();

public:
  int32_t sendMessage(idgs::actor::ActorMessagePtr& message);

  std::string toString();
  uint32_t connect(uint32_t memberId, int retry = 0);
  void terminate();

private:
  void handleConnect(int retry);
  int accept();

private:
  void startRecvHeader(UDTSOCKET* recver);
  void handleRecvHeader(UDTSOCKET* recver, std::shared_ptr<RpcBuffer> readBuffer);
  void handleRecvBody(UDTSOCKET* recver, std::shared_ptr<RpcBuffer> readBuffer);

  void realSendMessage();
  void handleSendMessage();

  std::shared_ptr<tbb::concurrent_queue<idgs::actor::ActorMessagePtr>> getQueue();

private:
  UDTSOCKET sendSocket;
  UDTSOCKET recvSocket;

  DEF_ENUM (InnerUdtConnectionState,
    INITIAL,
    CONNECTING,
    READY,
    WRITING,
    TERMINATED
  );

  std::atomic<InnerUdtConnectionState> state;
  std::shared_ptr<tbb::concurrent_queue<idgs::actor::ActorMessagePtr> > queue;
  int32_t peerMemberId;
  int32_t try_pop_count = 0;

  static InnerUdtServer* innerUdtServer;
};

} // namespace net 
} // namespace idgs 
#endif // defined(WITH_UDT)
