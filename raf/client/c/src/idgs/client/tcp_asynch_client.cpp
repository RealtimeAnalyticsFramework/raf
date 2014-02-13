/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(__GNUC__) || defined(__clang__) $
#include "tcp_asynch_client.h"

using asio::ip::tcp;
using namespace idgs::pb;

namespace idgs {
namespace client {
TcpAsynchClient::TcpAsynchClient(const idgs::client::pb::Endpoint& _server) :
    TcpClient(_server), timeout_sec(10) {
  m_socket.reset(new asio::ip::tcp::socket(io_service));
  deadline.reset(new asio::deadline_timer(io_service));
  deadline->expires_at(boost::posix_time::pos_infin);
  check_deadline();

//#if defined(__unix__)
//	// Linux or Unix
//	struct timeval tv;
//	tv.tv_sec = 5;
//	tv.tv_usec = 0;
//	setsockopt(m_socket->native(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
//	setsockopt(m_socket->native(), SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
//#else
//	// Windows
//	int32_t timeout_sec = 5000;
//	setsockopt(m_socket->native(), SOL_SOCKET, SO_RCVTIMEO,
//			(const char*) &timeout_sec, sizeof(timeout_sec));
//	setsockopt(m_socket->native(), SOL_SOCKET, SO_SNDTIMEO,
//			(const char*) &timeout_sec, sizeof(timeout_sec));
//#endif // defined(__unix__)
}

idgs::ResultCode TcpAsynchClient::initialize() {
  DVLOG(2) << "#### TcpAsynchClient::initialize() ";
  idgs::ResultCode code = connect();
  DVLOG(2) << "#### finish TcpAsynchClient::initialize() ";
  return code;
}

ClientActorMessagePtr TcpAsynchClient::receive(ResultCode* code, int timeout_sec) {
  this->timeout_sec = timeout_sec;
  DVLOG(2) << "#### TcpAsynchClient::receive() ";
  ClientActorMessagePtr ptr = synchRead(*code);
  DVLOG(2) << "#### finish TcpAsynchClient::receive() ";
  return ptr;
}

void TcpAsynchClient::send(ClientActorMessagePtr& actorMsg, idgs::ResultCode* code, int timeout_sec) {
  this->timeout_sec = timeout_sec;
  DVLOG(2) << "#### TcpAsynchClient::sendRecv() ";
  DVLOG(2) << "syncWrite";
  *code = synchWrite(actorMsg);
  if (*code != RC_SUCCESS) {
    return;
  }
}

ClientActorMessagePtr TcpAsynchClient::sendRecv(ClientActorMessagePtr& actorMsg, idgs::ResultCode* code,
    int timeout_sec) {
  this->timeout_sec = timeout_sec;
  DVLOG(2) << "#### TcpAsynchClient::sendRecv() ";
  DVLOG(2) << "syncWrite";
  *code = synchWrite(actorMsg);
  if (*code != RC_SUCCESS) {
    return ClientActorMessagePtr(NULL);
  }
  DVLOG(2) << "synchRead";
  ClientActorMessagePtr ptr = synchRead(*code);
  DVLOG(2) << "#### finish TcpAsynchClient::sendRecv() ";
  return ptr;
}

idgs::ResultCode TcpAsynchClient::connect() {
  if (m_socket->is_open()) {
    return RC_CLIENT_SOCKET_IS_ALREADY_OPEN;
  }

  idgs::ResultCode code = RC_CLIENT_ALL_SERVERS_NOT_AVAILABLE;

  try {
    tcp::resolver resolver(io_service);

    idgs::client::pb::Endpoint endPoint = getServerAddress();
    tcp::resolver::query query(tcp::v4(), endPoint.address(), endPoint.port());
    tcp::resolver::iterator iter = resolver.resolve(query);

    DVLOG(2) << "after resovle";
    deadline->expires_from_now(boost::posix_time::seconds(timeout_sec));
    DVLOG(2) << "start time with " << timeout_sec;

    asio::error_code ec;

    for (; iter != tcp::resolver::iterator() && code == RC_CLIENT_ALL_SERVERS_NOT_AVAILABLE; ++iter) {
      m_socket->close();
      //DVLOG(2) << "try to async connect to " << m_socket->remote_endpoint().address().to_string();

      ec = asio::error::would_block;

      m_socket->async_connect(iter->endpoint(),
          [this, &ec] (const asio::error_code& error) {
            ec = error;
            LOG_IF(ERROR, error) << toString() << "failed to  connect to server " << m_socket->remote_endpoint().address().to_string()
            << ", error " << ec << "("<< ec.message() << ")";
          });

      DVLOG(2) << "get TCP connect outside code: " << ec << " => " << ec.message();
      while (ec == asio::error::would_block) {
        io_service.run_one();
      }

      DVLOG(2) << "after block" << ec.message() << " with socket is open " << m_socket->is_open();
      if (!ec && m_socket->is_open()) {
        code = RC_SUCCESS;
//            break;
      }
    }

    DVLOG(2) << " Connect result => " << ec << " => " << ec.message();

  } catch (std::exception& error) {
    LOG(ERROR)<< " Get exception " << error.what();
    deadline->expires_at(boost::posix_time::pos_infin);
    return RC_ASYNCH_CLIENT_CONNECT_ERROR;
  }

  deadline->expires_at(boost::posix_time::pos_infin);
  return code;
}

ClientActorMessagePtr TcpAsynchClient::synchRead(ResultCode& code) {
  ClientActorMessagePtr response(NULL);
  idgs::net::RpcBuffer readBuffer;

  try {
    asio::error_code ec = asio::error::would_block;
    std::size_t len = 0;
    deadline->expires_from_now(boost::posix_time::seconds(timeout_sec));

    asio::async_read(*m_socket.get(), asio::buffer(readBuffer.getHeader(), sizeof(idgs::net::TcpHeader)),
        asio::transfer_all(),
        [&ec, &len, this] (const asio::error_code& error, std::size_t bytes_transferred) {
          ec = error;
          if(ec){
            LOG_FIRST_N(ERROR, 1000) << toString() << ", TCP Read head error: " << ec << "(" << ec.message() << "), socket is opened: " << m_socket->is_open();
          }
          len = bytes_transferred;
        });

    DVLOG(2) << "get TCP Read head error code " << ec << "(" << ec.message() << ")";
    while (ec == asio::error::would_block) {
      io_service.run_one();
    }
    DVLOG(2) << "after blocking, get TCP Read head error code " << ec << " => " << ec.message();

    if (ec || !m_socket->is_open()) {
      code = RC_ERROR;
      deadline->expires_at(boost::posix_time::pos_infin);
      return response;
    }

    readBuffer.decodeHeader();
    const size_t bodyLength = readBuffer.getBodyLength();

    // reset error code.
    ec = asio::error::would_block;
    asio::async_read(*m_socket.get(), asio::buffer(readBuffer.getBody(), bodyLength), asio::transfer_all(),
        [&ec, &len, this] (const asio::error_code& error, std::size_t bytes_transferred ) {
          ec = error;
          LOG_IF(ERROR, error) << toString() << " failed to  read body from server, error " << ec << "(" << ec.message() << ")";
          DVLOG(2) << "get TCP Read body error code " << ec << " => " << ec.message();
          len = bytes_transferred;
        });
    DVLOG(2) << "get TCP Read body error code " << ec << " => " << ec.message();
    while (ec == asio::error::would_block) {
      io_service.run_one();
    }
    DVLOG(2) << "after blocking, get TCP Read body error code " << ec << " => " << ec.message();

    if (ec || !m_socket->is_open()) {
      LOG(ERROR)<< toString() << " TCP Read body error: " << ec << " => " << ec.message();
      code = RC_ERROR;
      deadline->expires_at(boost::posix_time::pos_infin);
      return ClientActorMessagePtr();
    }

    RpcMessagePtr rpc(new RpcMessage);
    protobuf::ProtoSerdes<DEFAULT_PB_SERDES>::deserializeFromArray(readBuffer.getBody(), len, rpc.get());
    response.reset(new ClientActorMessage(rpc));
    //DVLOG(2) << "response.get " << response.get();
    DVLOG(2) << "Get message from peer " << response->toString();

    code = RC_SUCCESS;
  } catch (std::exception& error) {
    LOG(ERROR)<< " Get exception " << error.what();
    code = RC_ERROR;
    deadline->expires_at(boost::posix_time::pos_infin);
    return ClientActorMessagePtr();;
  }
  deadline->expires_at(boost::posix_time::pos_infin);
  return response;
}

idgs::ResultCode TcpAsynchClient::synchWrite(ClientActorMessagePtr& actorMsg) {
  actorMsg->setChannel(idgs::pb::TC_TCP);
  std::string msgContent = actorMsg->toBuffer();
  DVLOG(4) << "serialized length: " << msgContent.length() << ", serialized msg content: " << msgContent;
  DVLOG(4) << "serialized length: " << msgContent.length() << ", binary msg content: "
              << dumpBinaryBuffer2(msgContent.c_str(), msgContent.length());
  idgs::ResultCode code;

  idgs::net::RpcBuffer writeBuffer;

  writeBuffer.setBodyLength(msgContent.length());
  writeBuffer.encodeHeader();

  std::size_t len = 0;

  try {
    deadline->expires_from_now(boost::posix_time::seconds(timeout_sec));
    asio::error_code ec = asio::error::would_block;

    DVLOG(2) << "try to synchWrite " << actorMsg->toString();
    std::vector<asio::const_buffer> outBuffers = { asio::buffer(reinterpret_cast<void*>(writeBuffer.getHeader()),
        sizeof(idgs::net::TcpHeader)), asio::buffer(msgContent.c_str(), msgContent.length()) };

    asio::async_write(*m_socket.get(), outBuffers, asio::transfer_all(),
        [&ec, &len, this] (const asio::error_code& error, std::size_t bytes_transferred ) {
          ec = error;
          if(ec) {
            LOG_FIRST_N(ERROR, 1000) << toString() << " failed to write data: " << ec << "("<< ec.message() << ")";
          }
          //DVLOG(2) << "get TCP Read body error code " << ec;
          len = bytes_transferred;
        });

    DVLOG(2) << "get TCP synchWrite error code " << ec << "(" << ec.message() << ")";

    while (ec == asio::error::would_block) {
      io_service.run_one();
    }

    DVLOG(2) << "after blocking, get TCP synchWrite error code " << ec << " => " << ec.message();

    if (ec || !m_socket->is_open()) {
      LOG_FIRST_N(ERROR, 1000)<< "TCP Read body error: " << ec << "(" << ec.message() << ")";
      code = RC_ERROR;
    }
  } catch (std::exception& error) {
    LOG_FIRST_N(ERROR, 20) << " Get exception " << error.what();
    code = RC_ERROR;
  }
  deadline->expires_at(boost::posix_time::pos_infin);
  code = RC_SUCCESS;
  return code;

}

void TcpAsynchClient::check_deadline() {
  DVLOG(2) << " TcpAsynchClient::check_deadline ";
  deadline->async_wait([this] (const asio::error_code& error) {
    this->timeoutHandler(error);
  });
}

void TcpAsynchClient::timeoutHandler(const asio::error_code& error) {
  DVLOG(2) << "deadline.async_wait error code: " << error.message();
  if (deadline->expires_at() <= asio::deadline_timer::traits_type::now()) {
    LOG_IF(ERROR, error) << " time out " << toString() << ": " << error << "(" << error.message() << ")";
    m_socket->cancel();
    deadline->expires_at(boost::posix_time::pos_infin);
  }

  check_deadline();
}
} // namespace client
} // namespace idgs
