
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "http_connection.h"
#include "idgs/application.h"

namespace idgs {
namespace http {
namespace server {

//////////////////////// HttpConnection /////////////////////////////////////

HttpConnection::HttpConnection(asio::ip::tcp::socket* _socket)
  : socket_(_socket),
    bufferSize(8192),
    preLoadedSize(0),
    actorReq(NULL),
    httpHanlder(NULL){
  buffer = new char[bufferSize];
}

HttpConnection::~HttpConnection() {
  VLOG(3) << "HttpConnection " << this << " is deleted";

  if(socket_) {
    delete socket_;
    socket_ = NULL;
  }

  if(buffer) {
    delete buffer;
    socket_ = NULL;
  }
}

asio::ip::tcp::socket& HttpConnection::socket() {
  return *socket_;
}

void HttpConnection::start(char* preLoaded, size_t _preLoadedSize) {
  if (preLoaded != NULL && _preLoadedSize != 0) {
    preLoadedSize = _preLoadedSize;
    memcpy(buffer, preLoaded, preLoadedSize);
  }

  VLOG(2) << "HttpConnection start";

  std::shared_ptr<HttpConnection> pHttpCon = this->getHttpConnection();
  auto handler = [pHttpCon] (const asio::error_code& error, const std::size_t  bytes_transferred) {
    pHttpCon->handle_read(error, bytes_transferred);
  };
  socket().async_read_some(asio::buffer(buffer + preLoadedSize, bufferSize - preLoadedSize),
      handler);
}

void HttpConnection::stop() {
  VLOG(2)<<"HTTP Connection" << this << " is stopped";
  socket_->close();
}

void HttpConnection::handle_read(const asio::error_code& e, const std::size_t  bytes_transferred) {
  VLOG(2) << "Async_read with [error_code " << e.message() << "] [bytes_transferred:" << bytes_transferred << "]";
  if (!e) {
    idgs::http::server::ParseStatus result;
    std::string ss(buffer, buffer + preLoadedSize + bytes_transferred);
    VLOG(2) << "get Http request: " << ss;

    std::tie(result, std::ignore) = request_parser_.parse(
        request_, buffer, buffer + preLoadedSize + bytes_transferred);

    VLOG(2) << "Parse http request with result: " << result;

    std::shared_ptr<HttpConnection> pHttpCon = this->getHttpConnection();
    auto handler = [pHttpCon] (const asio::error_code& error, const std::size_t& bytes_transferred) {
      pHttpCon->handle_write(error, bytes_transferred);
    };

    if (result == Complete) {
      if (!handle_request(request_, reply_)) {
        asio::async_write(*socket_, reply_.toBuffers(),
            asio::transfer_all(),
            handler);
      }
    } else if (result == Invalid) {
      reply_ = HttpResponse::createReply(HttpResponse::bad_request);
      asio::async_write(*socket_, reply_.toBuffers(),
          asio::transfer_all(),
          handler);
    } else {
      socket().async_read_some(asio::buffer(buffer + preLoadedSize, bufferSize - preLoadedSize),
           handler);
    }
  } else if (e != asio::error::operation_aborted) {
    VLOG(2) << "http connection aborted";
  }
}

void HttpConnection::handle_write(const asio::error_code& e, const std::size_t& bytes_transferred) {
  if (!e) {
    asio::error_code ignored_ec;
    socket_->shutdown(asio::ip::tcp::socket::shutdown_both, ignored_ec);
  }

  if (e != asio::error::operation_aborted) {
    VLOG(3) << "HttpConnection is stopped";
    //connection_manager_.stop(shared_from_this());
    HttpServer* httpServer = idgs::util::singleton<idgs::actor::RpcFramework>::getInstance().getNetwork()->getHttpServer();
    httpServer->removeConnection(shared_from_this());
  }
}

bool HttpConnection::handle_request(HttpRequest& req, HttpResponse& rep) {
  // Decode url to path.
  std::string request_path;
  if (!url_decode(req.getUri(), request_path)) {
    LOG(ERROR) << "can not decode URL for req " << req.getUri();
    rep = HttpResponse::createReply(HttpResponse::bad_request);
    return false;
  }

  VLOG(2) << "Get Http Request URI: " << request_path;

  // Request path must be absolute and not contain "..".
  if (request_path.empty() || request_path[0] != '/'
      || request_path.find("..") != std::string::npos) {
    LOG(ERROR) << "Request path must be absolute and not contain \"..\": " << req.getUri();
    rep = HttpResponse::createReply(HttpResponse::bad_request);
    return false;
  }

  int root_pos = request_path.find('/', 1);
  std::string root = request_path.substr(1,root_pos - 1);
  VLOG(2) << "get root " << root;

  HttpServer* httpServer = idgs::util::singleton<idgs::actor::RpcFramework>::getInstance().getNetwork()->getHttpServer();


  std::string reqPath = request_path.substr(root_pos + 1);
  req.setRootPath(root);
  req.setRequestPath(reqPath);

  httpHanlder = httpServer->getHttpAsyncServlet(root);
  if (httpHanlder.get() == NULL) {
    LOG(ERROR) << "can not find http servlet for root " << root;
    rep = HttpResponse::createReply(HttpResponse::bad_request, "can not find http servlet for root " + root);
    return false;
  }

  VLOG(3) << "handler " << httpHanlder->getName() << " will service request";

  if (httpHanlder.get() != NULL) {
    auto f = [this] (const HttpResponse& msg) {
      reply_ = msg;
      std::shared_ptr<HttpConnection> pHttpCon = this->getHttpConnection();
      auto handler = [pHttpCon] (const asio::error_code& error, const std::size_t& bytes_transferred) {
        pHttpCon->handle_write(error, bytes_transferred);
      };
      asio::async_write(*socket_, reply_.toBuffers(),
          asio::transfer_all(),
          handler);

      return;
    };
    httpHanlder->registerHandler(f);
    httpHanlder->service(req);
  } else {
    LOG(ERROR) << "can not handler this request: " << req.getUri();
    rep = HttpResponse::createReply(HttpResponse::bad_request, "can not handler this request");
    return false;
  }

  return true;
}

bool HttpConnection::url_decode(const std::string& in, std::string& out) {
  out.clear();
  out.reserve(in.size());
  for (std::size_t i = 0; i < in.size(); ++i) {
    if (in[i] == '%') {
      if (i + 3 <= in.size()) {
        int value = 0;
        std::istringstream is(in.substr(i + 1, 2));
        if (is >> std::hex >> value) {
          out += static_cast<char>(value);
          i += 2;
        }
        else {
          return false;
        }
      }
      else {
        return false;
      }
    }
    else if (in[i] == '+') {
      out += ' ';
    } else {
      out += in[i];
    }
  }
  return true;
}

} // namespace server
} // namespace http
}
