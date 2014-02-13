/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "tcp_synchronous_client.h"

using asio::ip::tcp;
using namespace idgs::pb;

namespace idgs {
namespace client {
enum {
  max_length = 1024
};

TcpSynchronousClient::TcpSynchronousClient(const idgs::client::pb::Endpoint& _server) :
    TcpClient(_server) {
  m_socket.reset(new asio::ip::tcp::socket(io_service));

#if defined(__unix__)
  // Linux or Unix
  struct timeval tv;
  tv.tv_sec = 5;
  tv.tv_usec = 0;
  setsockopt(m_socket->native(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
  setsockopt(m_socket->native(), SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#else
  // Windows
  int32_t timeout = 5000;
  setsockopt(m_socket->native(), SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
  setsockopt(m_socket->native(), SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
#endif // defined(__unix__)
}

idgs::ResultCode TcpSynchronousClient::initialize() {

  if (m_socket->is_open()) {
    return RC_CLIENT_SOCKET_IS_ALREADY_OPEN;
  }

  tcp::resolver resolver(io_service);

  idgs::client::pb::Endpoint endPoint = getServerAddress();
  tcp::resolver::query query(endPoint.address(), endPoint.port());
  tcp::resolver::iterator iterator = resolver.resolve(query);

  asio::error_code error;
  m_socket->connect(*iterator, error);
  if (error) {
    DVLOG(2) << " connect " << endPoint.address() << ":" << endPoint.port() << " with error " << error;
    return RC_CLIENT_SERVER_IS_NOT_AVAILABLE;
  }

#if defined(__unix__)
  // Linux or Unix
  struct timeval tv;
  tv.tv_sec = 5;
  tv.tv_usec = 0;
  setsockopt(m_socket->native(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
  setsockopt(m_socket->native(), SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#else
  // Windows
  int32_t timeout = 5000;
  setsockopt(m_socket->native(), SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
  setsockopt(m_socket->native(), SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
#endif // defined(__unix__)
  return RC_SUCCESS;
}

ClientActorMessagePtr TcpSynchronousClient::receive(ResultCode* code, int timeout_sec) {
  asio::error_code error;
  idgs::net::RpcBuffer buffer;

  // read header
  size_t len = asio::read(*m_socket.get(), asio::buffer(buffer.getHeader(), sizeof(idgs::net::TcpHeader)),
      asio::transfer_all(), error);
  DVLOG(2) << "read header byte size: " << len;
  if (error == asio::error::eof) {
    DVLOG(2) << "Connection closed cleanly by peer";
    *code = RC_ERROR;
    return ClientActorMessagePtr();
  } else if (error) {
    DLOG(INFO)<< "Connection error " << error;
    *code = RC_ERROR;
    return ClientActorMessagePtr();
  }

  buffer.decodeHeader();
  const size_t bodyLength = buffer.getHeader()->size;
  ;

  // read body
  len = asio::read(*m_socket.get(), asio::buffer(buffer.getBody(), bodyLength), asio::transfer_all(), error);
  DVLOG(2) << "read body byte size: " << len;
  if (error == asio::error::eof) {
    DVLOG(2) << "Connection closed cleanly by peer";
    *code = RC_ERROR;
    return ClientActorMessagePtr();
  } else if (error) {
    DLOG(INFO)<< "Connection error " << error;
    *code = RC_ERROR;
    return ClientActorMessagePtr();
  }

  ClientActorMessagePtr response(new ClientActorMessage);
  protobuf::ProtoSerdes<DEFAULT_PB_SERDES>::deserializeFromArray(buffer.getBody(), bodyLength,
      response->getRpcMessage().get());

  //DVLOG(2) << "response.get " << response.get();
  DVLOG(2) << "Get message from peer " << response->toString();

  *code = RC_SUCCESS;
  return response;
}

ClientActorMessagePtr TcpSynchronousClient::sendRecv(ClientActorMessagePtr& actorMsg, idgs::ResultCode* code,
    int timeout_sec) {
  actorMsg->setChannel(idgs::pb::TC_TCP);
  std::string msgContent = actorMsg->toBuffer();
  DVLOG(4) << dumpBinaryBuffer(msgContent);
  DVLOG(4) << dumpBinaryBuffer3(msgContent.c_str(), msgContent.length());

  idgs::net::RpcBuffer writeBuffer;
  writeBuffer.setBodyLength(msgContent.length());
  writeBuffer.encodeHeader();
  asio::error_code error;
  std::vector<asio::const_buffer> outBuffers = { asio::buffer(reinterpret_cast<void*>(writeBuffer.getHeader()),
      sizeof(idgs::net::TcpHeader)), asio::buffer(msgContent.c_str(), msgContent.length()) };

  asio::write(*m_socket.get(), outBuffers, asio::transfer_all(), error);
  DVLOG(2) << "client send size: " << (4 + msgContent.length()) << ", data: " << dumpBinaryBuffer(msgContent);

  if (error) {
    DVLOG(2) << "write message to server " << error;
    *code = RC_ERROR;
    return ClientActorMessagePtr();
  }

  idgs::net::RpcBuffer readBuffer;
  // read header
  asio::read(*m_socket.get(), asio::buffer(readBuffer.getHeader(), sizeof(idgs::net::TcpHeader)), asio::transfer_all(),
      error);
  if (error == asio::error::eof) {
    DVLOG(2) << "Connection closed cleanly by peer";
    *code = RC_ERROR;
    return ClientActorMessagePtr();
  } else if (error) {
    DLOG(INFO)<< "Connection error " << error;
    *code = RC_ERROR;
    return ClientActorMessagePtr();
  }

  readBuffer.decodeHeader();
  const size_t bodyLength = readBuffer.getBodyLength();

  // read body
  asio::read(*m_socket.get(), asio::buffer(readBuffer.getBody(), bodyLength), asio::transfer_all(), error);

  if (error == asio::error::eof) {
    DVLOG(2) << "Connection closed cleanly by peer";
    *code = RC_ERROR;
    return ClientActorMessagePtr();
  } else if (error) {
    DLOG(INFO)<< "Connection error " << error;
    *code = RC_ERROR;
    return ClientActorMessagePtr();
  }

  RpcMessagePtr response(new RpcMessage());
  protobuf::ProtoSerdes<DEFAULT_PB_SERDES>::deserializeFromArray(readBuffer.getBody(), bodyLength, response.get());

  //DVLOG(2) << "response.get " << response.get();
  DVLOG(2) << "Get message from peer " << response->DebugString();

  ClientActorMessagePtr result(new ClientActorMessage(response));
  *code = RC_SUCCESS;
  return result;
}

void TcpSynchronousClient::send(ClientActorMessagePtr& actorMsg, idgs::ResultCode* code, int timeout_sec) {
  actorMsg->setChannel(idgs::pb::TC_TCP);
  std::string msgContent = actorMsg->toBuffer();
  DVLOG(4) << dumpBinaryBuffer(msgContent);

  idgs::net::RpcBuffer writeBuffer;
  writeBuffer.setBodyLength(msgContent.length());
  writeBuffer.encodeHeader();
  asio::error_code error;
  std::vector<asio::const_buffer> outBuffers = { asio::buffer(reinterpret_cast<void*>(writeBuffer.getHeader()),
      sizeof(idgs::net::TcpHeader)), asio::buffer(msgContent.c_str(), msgContent.length()) };

  asio::write(*m_socket.get(), outBuffers, asio::transfer_all(), error);
  DVLOG(2) << "client send size: " << (4 + msgContent.length()) << ", data: " << dumpBinaryBuffer(msgContent);

  if (error) {
    DVLOG(2) << "write message to server " << error;
    *code = RC_ERROR;
    return;
  }
}
} // namespace client
} // namespace idgs

