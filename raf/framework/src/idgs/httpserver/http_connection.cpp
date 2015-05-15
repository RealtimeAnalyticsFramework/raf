
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "http_connection.h"

#include "idgs/application.h"
#include "http_server.h"

namespace idgs {
namespace httpserver {

//////////////////////// HttpConnection /////////////////////////////////////

HttpConnection::HttpConnection(asio::ip::tcp::socket* _socket)
  : socket_(_socket),
    bufferSize(8192),
    preLoadedSize(0) {
  buffer = new char[bufferSize];
}

HttpConnection::~HttpConnection() {
  VLOG(3) << "HttpConnection " << this << " is deleted";

  if(socket_) {
    delete socket_;
    socket_ = NULL;
  }

  if(buffer) {
    delete[] buffer;
    buffer = NULL;
  }
}

asio::ip::tcp::socket& HttpConnection::socket() {
  return *socket_;
}

void HttpConnection::start(char* preLoaded, size_t _preLoadedSize) {
  VLOG(2)<<"HTTP Connection starts";
  auto conn = shared_from_this();
  request_.setConnection(conn);

  if (preLoaded != NULL && _preLoadedSize != 0) {
    memcpy(buffer, preLoaded, _preLoadedSize);
    handle_read(asio::error_code(), _preLoadedSize);
  } else {
    preLoadedSize = 0;
    socket().async_read_some(asio::buffer(buffer + preLoadedSize, bufferSize - preLoadedSize),
        [conn] (const asio::error_code& error, const std::size_t  bytes_transferred) {
            conn->handle_read(error, bytes_transferred);
          }
    );
  }
}

void HttpConnection::stop() {
  VLOG(2)<<"HTTP Connection" << this << " is stopped";
  socket_->close();
}

void HttpConnection::handle_read(const asio::error_code& e, const std::size_t  bytes_transferred) {
  auto pHttpCon = shared_from_this();
  VLOG(2) << "Async_read with [error_code " << e.message() << "] [bytes_transferred:" << bytes_transferred << "]";
  if (!e) {
    idgs::httpserver::ParseStatus result;
    VLOG(2) << "get Http request: " << std::string(buffer, buffer + preLoadedSize + bytes_transferred);

    char* pos;
    std::tie(result, pos) = request_parser_.parse(
        request_, buffer + preLoadedSize, buffer + preLoadedSize + bytes_transferred);

    VLOG(2) << "Parse http request with result: " << result;

    preLoadedSize += bytes_transferred;
    if (result == Complete) {
      if (pos <  buffer + preLoadedSize) {
        std::string payload = std::string(pos, buffer + preLoadedSize - pos);
        request_.setBody(payload);
      }
      if (!handle_request(request_, reply_)) {
        sendResponse(reply_);
      }
    } else if (result == Invalid) {
      reply_ = HttpResponse::createReply(HttpResponse::bad_request);
      sendResponse(reply_);
    } else {
      socket().async_read_some(asio::buffer(buffer + preLoadedSize, bufferSize - preLoadedSize),
          [pHttpCon] (const asio::error_code& error, const std::size_t  bytes_transferred) {
              pHttpCon->handle_read(error, bytes_transferred);
            }
      );
    }
  } else if (e != asio::error::operation_aborted) {
    VLOG(2) << "http connection aborted";
  }
}

void HttpConnection::handle_write(const asio::error_code& e, const std::size_t& bytes_transferred) {
  auto conn = shared_from_this();
  if (!e) {
    asio::error_code ignored_ec;
    socket_->shutdown(asio::ip::tcp::socket::shutdown_both, ignored_ec);
  }

  if (e != asio::error::operation_aborted) {
    VLOG(3) << "HttpConnection is stopped";
    HttpServer* httpServer = idgs_application()->getRpcFramework()->getNetwork()->getHttpServer();
    httpServer->removeConnection(conn);
  }
}

void HttpConnection::sendResponse(const HttpResponse& response) {
  auto conn = shared_from_this();
  reply_ = response;
  auto buffers = conn->reply_.toBuffers();
  if (VLOG_IS_ON(2)) {
    std::stringstream ss;
    for (auto& buf: buffers) {
      ss.write(asio::buffer_cast<const char*>(buf), asio::buffer_size(buf));
    }
    DVLOG(2) << "Http Response: " << ss.str();
  }
  asio::async_write(*(conn->socket_), buffers,
      asio::transfer_all(),
      [conn] (const asio::error_code& error, const std::size_t& bytes_transferred) {
                conn->handle_write(error, bytes_transferred);
              }
  );

}


bool HttpConnection::handle_request(HttpRequest& request, HttpResponse& response) {
  // Decode url to path.
  std::string request_path;
  if (!url_decode(request.getUri(), request_path)) {
    LOG(ERROR) << "can not decode URL for req " << request.getUri();
    response = HttpResponse::createReply(HttpResponse::bad_request);
    return false;
  }

  std::string requestUri = request_path;
  request.setUri(requestUri);

  VLOG(2) << "Get Http Request URI: " << request_path;
  LOG(INFO) << "URI: " << request_path;

  // Request path must be absolute and not contain "..".
  if (request_path.empty() || request_path[0] != '/'
      || request_path.find("..") != std::string::npos) {
    LOG(ERROR) << "Request path must be absolute and not contain \"..\": " << request.getUri();
    response = HttpResponse::createReply(HttpResponse::bad_request);
    return false;
  }

  int root_pos = request_path.find('/', 1);
  std::string root = request_path.substr(1,root_pos - 1);
  VLOG(2) << "get root " << root;

  HttpServer* httpServer = idgs_application()->getRpcFramework()->getNetwork()->getHttpServer();


  std::string reqPath = request_path.substr(root_pos + 1);
  request.setRootPath(root);
  request.setRequestPath(reqPath);

  HttpServlet* servlet = httpServer->lookupServlet(requestUri);
  if (servlet) {
    servlet->service(request, response);
    if (!request.isAsync()) {
      sendResponse(response);
    }
    return true;
  } else {
    LOG(ERROR) << "can not find http servlet for root " << root;
    response = HttpResponse::createReply(HttpResponse::bad_request, "can not find http servlet for root " + root);
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

std::shared_ptr<AsyncContext> HttpConnection::startAsync() {
  if (!asyncContext) {
    asyncContext = std::make_shared<AsyncContext>(shared_from_this());
  }
  return asyncContext;
}

bool HttpConnection::isAsync() {
  return (bool)asyncContext;
}

} // namespace httpserver
} // namespace idgs
