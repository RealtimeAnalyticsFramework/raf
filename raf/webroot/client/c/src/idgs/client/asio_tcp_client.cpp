/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "idgs/client/asio_tcp_client.h"
#include "idgs/net/rpc_buffer.h"

using asio::ip::tcp;
using namespace idgs::pb;

namespace idgs {
namespace client {

AsioTcpClient::AsioTcpClient(const idgs::client::pb::Endpoint& _server) :
    io_service(), server(_server), id(0) {
  m_socket = std::make_shared<asio::ip::tcp::socket>(io_service);
  // @todo c++14
  // deadline = std::make_unique<asio::steady_timer>(io_service);
  deadline.reset(new asio::steady_timer(io_service));
  deadline->expires_from_now(std::chrono::seconds::max());
  check_deadline();
}

idgs::ResultCode AsioTcpClient::initialize() {
  idgs::ResultCode rc;
  rc = nonblockingConnect();
  if (rc) {
    return rc;
  }

  // login
  idgs::net::ClientLogin clientLogin;
  DVLOG(2) << "send client login " << sizeof(clientLogin) << " byte size, content is : "
              << dumpBinaryBuffer2(reinterpret_cast<char*>(&clientLogin), sizeof(clientLogin));
  asio::write(*getSocket(), asio::buffer(reinterpret_cast<char*>(&clientLogin), sizeof(clientLogin)),
      asio::transfer_all());
  return rc;

}

idgs::ResultCode AsioTcpClient::close() {
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


idgs::ResultCode AsioTcpClient::send(ClientActorMessagePtr& actorMsg, int timeout_sec)  {
  return nonblockingWrite(actorMsg, timeout_sec);
}

idgs::ResultCode AsioTcpClient::receive(ClientActorMessagePtr& msg, int timeout_sec) {
  return nonblockingRead(msg, timeout_sec);
}

idgs::ResultCode AsioTcpClient::sendRecv(ClientActorMessagePtr& actorMsg, ClientActorMessagePtr& response, int timeout_sec) {
  idgs::ResultCode code;
  code = nonblockingWrite(actorMsg, timeout_sec);
  if (RC_OK != code) {
    return code;
  }
  code = nonblockingRead(response, timeout_sec);
  return code;
}


void AsioTcpClient::check_deadline() {
  deadline->async_wait([this] (const asio::error_code& error) {
    if (error) {
      // DVLOG_FIRST_N(2, 20) << "deadline.async_wait error code: " << error.message();
    } else {
      this->m_socket->cancel();
      this->deadline->expires_from_now(std::chrono::seconds::max());
    }

    this->check_deadline();
  });
}


idgs::ResultCode AsioTcpClient::nonblockingConnect() {
  if (m_socket->is_open()) {
    return RC_CLIENT_SOCKET_IS_ALREADY_OPEN;
  }

  idgs::ResultCode code = RC_CLIENT_ALL_SERVERS_NOT_AVAILABLE;

  try {
    tcp::resolver resolver(io_service);

    idgs::client::pb::Endpoint endPoint = getServerEndpoint();
    tcp::resolver::query query(tcp::v4(), endPoint.host(), endPoint.port());
    tcp::resolver::iterator iter = resolver.resolve(query);

    deadline->expires_from_now(std::chrono::seconds(20));

    asio::error_code ec;

    for (; iter != tcp::resolver::iterator() && code == RC_CLIENT_ALL_SERVERS_NOT_AVAILABLE; ++iter) {
      m_socket->close();
      //DVLOG(2) << "try to async connect to " << m_socket->remote_endpoint().address().to_string();

      ec = asio::error::would_block;
      m_socket->async_connect(iter->endpoint(),
          [this, &ec] (const asio::error_code& error) {
            ec = error;
            DVLOG(2) << "Connect to remote peer result: "<< ec << " => " << ec.message();
            LOG_IF(ERROR, error) << toString()
                << "Failed to  connect to peer: " << m_socket->remote_endpoint().address().to_string()
                << ", error " << ec << "("<< ec.message() << ")";
          });

      while (ec == asio::error::try_again || ec == asio::error::would_block) {
        DVLOG(2) << "run_once begin";
        io_service.run_one();
        DVLOG(2) << "run_once end";
      }
      DVLOG(2) << "Connect to peer: " << iter->endpoint() << ", result: " << ec << "(" << ec.message() << ")";

      setSocketOption(m_socket.get());

      if (!ec && m_socket->is_open()) {
        code = RC_SUCCESS;
      }
    }

    DVLOG(2) << " Connect result => " << ec << " => " << ec.message();

  } catch (std::exception& error) {
    LOG(ERROR)<< " Get exception " << error.what();
    deadline->expires_from_now(std::chrono::seconds::max());
    return RC_ASYNCH_CLIENT_CONNECT_ERROR;
  }

  deadline->expires_from_now(std::chrono::seconds::max());
  return code;
}

idgs::ResultCode AsioTcpClient::nonblockingRead(ClientActorMessagePtr& response, int timeout_sec) {
  idgs::net::RpcBuffer readBuffer;

  if(!m_socket->non_blocking()) {
    m_socket->non_blocking(true);
  }

  try {
    asio::error_code ec = asio::error::would_block;
    std::size_t len = 0;
    deadline->expires_from_now(std::chrono::seconds(timeout_sec));

    asio::async_read(*m_socket.get(), asio::buffer(readBuffer.getHeader(), sizeof(idgs::net::TcpHeader)),
        asio::transfer_all(),
        [&ec, &len, this] (const asio::error_code& error, std::size_t bytes_transferred) {
          ec = error;
          if(ec){
            LOG_FIRST_N(ERROR, 100) << toString() << ", TCP Read head error: " << ec << "(" << ec.message() << "), socket is opened: " << m_socket->is_open();
          }
          len = bytes_transferred;
        });

    while (ec == asio::error::try_again || ec == asio::error::would_block) {
      io_service.run_one();
    }
    LOG_IF(ERROR, ec) << "after blocking, get TCP Read head error code " << ec << " => " << ec.message();

    if (ec || !m_socket->is_open()) {
      deadline->expires_from_now(std::chrono::seconds::max());
      return RC_ERROR;
    }

    const size_t bodyLength = readBuffer.getBodyLength();
    auto& byteBuffer = readBuffer.byteBuffer();
    auto b = idgs::net::ByteBuffer::allocate(bodyLength);
    byteBuffer.swap(b);

    // reset error code.
    ec = asio::error::would_block;
    asio::async_read(*m_socket.get(), asio::buffer(byteBuffer->data(), bodyLength), asio::transfer_all(),
        [&ec, &len, this] (const asio::error_code& error, std::size_t bytes_transferred ) {
          ec = error;
          LOG_IF(ERROR, error) << toString() << " failed to  read body from server, error " << ec << "(" << ec.message() << ")";
          LOG_IF(ERROR, ec) << "get TCP Read body error code " << ec << " => " << ec.message();
          len = bytes_transferred;
        });
    while (ec == asio::error::try_again || ec == asio::error::would_block) {
      io_service.run_one();
    }
    LOG_IF(ERROR, ec) << "after blocking, get TCP Read body error code " << ec << " => " << ec.message();

    if (ec || !m_socket->is_open()) {
      LOG(ERROR)<< toString() << " TCP Read body error: " << ec << " => " << ec.message();
      deadline->expires_from_now(std::chrono::seconds::max());
      return RC_ERROR;
    }

    std::shared_ptr<RpcMessage> rpcMessage = std::make_shared<RpcMessage>();
    protobuf::ProtoSerdes<DEFAULT_PB_SERDES>::deserializeFromArray(byteBuffer->data(), len, rpcMessage.get());
    response = std::make_shared<ClientActorMessage>(rpcMessage);
    //DVLOG(2) << "response.get " << response.get();
    DVLOG(2) << "Get message from peer " << response->toString();

  } catch (std::exception& error) {
    LOG(ERROR)<< " Get exception " << error.what();
    deadline->expires_from_now(std::chrono::seconds::max());
    return RC_ERROR;
  }
  deadline->expires_from_now(std::chrono::seconds::max());
  return RC_OK;
}

idgs::ResultCode AsioTcpClient::nonblockingWrite(ClientActorMessagePtr& actorMsg, int timeout_sec) {
  actorMsg->setChannel(idgs::pb::TC_TCP);
  std::string msgContent = actorMsg->toBuffer();
  DVLOG_FIRST_N(4, 20) << "serialized length: " << msgContent.length() << ", serialized msg content: " << msgContent;
  DVLOG_FIRST_N(4, 20) << "serialized length: " << msgContent.length() << ", binary msg content: "
              << dumpBinaryBuffer2(msgContent.c_str(), msgContent.length());
  idgs::ResultCode code;

  if(!m_socket->non_blocking()) {
    m_socket->non_blocking(true);
  }

  idgs::net::RpcBuffer writeBuffer;

  writeBuffer.setBodyLength(msgContent.length());

  std::size_t len = 0;

  try {
    deadline->expires_from_now(std::chrono::seconds(timeout_sec));

    DVLOG_FIRST_N(2, 20) << "try to nonblockingWrite " << actorMsg->toString();
    std::vector<asio::const_buffer> outBuffers = {
        asio::buffer(reinterpret_cast<void*>(writeBuffer.getHeader()), sizeof(idgs::net::TcpHeader)),
        asio::buffer(msgContent.c_str(), msgContent.length())
    };

    asio::error_code ec = asio::error::would_block;

    asio::async_write(*m_socket.get(), outBuffers, asio::transfer_all(),
        [&ec, &len, this] (const asio::error_code& error, std::size_t bytes_transferred ) {
          ec = error;
          if(ec) {
            LOG_FIRST_N(ERROR, 20) << toString() << " failed to write data: " << ec << "("<< ec.message() << ")";
          }
          //DVLOG(2) << "get TCP Read body error code " << ec;
          len = bytes_transferred;
        });

    while (ec == asio::error::try_again || ec == asio::error::would_block) {
      io_service.run_one();
    }

    DVLOG_FIRST_N(2, 20) << "after blocking, get TCP nonblockingWrite error code " << ec << " => " << ec.message()
        << ", bytes transferred: " << len;

    if (ec || !m_socket->is_open()) {
      LOG_FIRST_N(ERROR, 20)<< "TCP Read body error: " << ec << "(" << ec.message() << ")";
      code = RC_ERROR;
      // goto SYNC_WRITE_END;
    }
  } catch (std::exception& error) {
    LOG_FIRST_N(ERROR, 20) << " Get exception " << error.what();
    code = RC_ERROR;
    // goto SYNC_WRITE_END;
  }
  code = RC_SUCCESS;

  deadline->expires_from_now(std::chrono::seconds::max());
  return code;

}
void AsioTcpClient::encode(std::string &data, ResultCode &code, char* data_) {
  // set body length
  size_t body_length_ = data.length();

  // encode header
  idgs::net::TcpHeader* tcpHeader = reinterpret_cast<idgs::net::TcpHeader*>(data_);
  tcpHeader->size = body_length_;

  // set body
  std::memcpy(data_ + sizeof(idgs::net::TcpHeader), data.c_str(), data.length());

  code = RC_SUCCESS;
}

size_t AsioTcpClient::decodeHeader(char* data_, ResultCode &code) {
  idgs::net::TcpHeader* tcpHeader = reinterpret_cast<idgs::net::TcpHeader*>(data_);
  uint32_t body_length_ = tcpHeader->size;
  return body_length_;
}

void AsioTcpClient::setSocketOption (asio::ip::tcp::socket* sock) {
  auto handle = sock->native_handle();
  DVLOG(5) << "set socket option, socket fd: " << handle << ", is_open: " << sock->is_open() << ", non_block: " << sock->non_blocking();

  if(handle > 0) {
    if(!sock->non_blocking()) {
      // sock->non_blocking(true);
    }
#if defined(__unix__)
    // Linux or Unix
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    if (setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))) {
      LOG(INFO) << "Failed to set socket option SO_RCVTIMEO: " << errno << '(' << strerror(errno) << ')';
    }
    if (setsockopt(handle, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv))) {
      LOG(INFO) << "Failed to set socket option SO_SNDTIMEO: " << errno << '(' << strerror(errno) << ')';
    }
#else
    // Windows
    int32_t timeout = 5000;
    if (setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout))) {
      LOG(INFO) << "Failed to set socket option SO_RCVTIMEO: " << errno << '(' << strerror(errno) << ')';
    }
    if (setsockopt(handle, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout))) {
      LOG(INFO) << "Failed to set socket option SO_SNDTIMEO: " << errno << '(' << strerror(errno) << ')';
    }
#endif // defined(__unix__)
  }

}


} // namespace client
} // namespace idgs

