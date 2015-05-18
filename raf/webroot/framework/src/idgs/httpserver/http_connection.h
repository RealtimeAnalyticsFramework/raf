/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#pragma once

#include "http_request_parser.h"
#include "http_servlet.h"

namespace idgs {
namespace httpserver {
class HttpResponse;
class HttpConnection;

class HttpConnection: public std::enable_shared_from_this<HttpConnection> {
public:
  HttpConnection(asio::ip::tcp::socket* socket_);
  ~HttpConnection();
  asio::ip::tcp::socket& socket();

  void start(char* preLoaded, size_t _preLoadedSize);
  void stop();

  HttpRequest& getRequest() {
    return request_;
  }

  HttpResponse& getResponse() {
    return reply_;
  }

  std::shared_ptr<AsyncContext> startAsync();

  bool isAsync();


  void sendResponse(const HttpResponse& response);

private:
  void handle_read(const asio::error_code& e, const std::size_t bytes_transferred);
  void handle_write(const asio::error_code& e, const std::size_t& bytes_transferred);
  bool handle_request(HttpRequest& request, HttpResponse& response);
  bool url_decode(const std::string& in, std::string& out);


private:
  asio::ip::tcp::socket* socket_;

  uint32_t bufferSize = 0;
  size_t preLoadedSize;
  char* buffer;

  HttpRequest request_;
  HttpRequestParser request_parser_;
  HttpResponse reply_;

  std::shared_ptr<AsyncContext> asyncContext;
};
typedef std::shared_ptr<HttpConnection> HttpConnectionPtr;

///
/// Async Http Servlet Context.
///
class AsyncContext {
public:
  AsyncContext(std::weak_ptr<HttpConnection> connection_) :
    connection(connection_) {}

public:
  HttpRequest& getRequest() {
    return connection.lock()->getRequest();
  }

  HttpResponse& getResponse() {
    return connection.lock()->getResponse();
  }

  void done() {
    connection.lock()->sendResponse(getResponse());
  }
private:
  std::weak_ptr<HttpConnection> connection;
};

} // namespace httpserver
} // namespace idgs

