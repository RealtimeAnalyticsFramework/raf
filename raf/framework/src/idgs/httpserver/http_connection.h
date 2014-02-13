
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "http_request_parser.h"
#include "http_servlet.h"

namespace idgs {
namespace http {
namespace server {


class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
public:
  HttpConnection(asio::ip::tcp::socket* socket_);

  ~HttpConnection();

  asio::ip::tcp::socket& socket();

  void start(char* preLoaded, size_t _preLoadedSize);

  void stop();

  std::shared_ptr<HttpConnection> getHttpConnection() {
    return shared_from_this();
  }

private:

  void handle_read(const asio::error_code& e,
      const std::size_t bytes_transferred);

  void handle_write(const asio::error_code& e, const std::size_t& bytes_transferred);

  bool handle_request(HttpRequest& req, HttpResponse& rep);

  bool url_decode(const std::string& in, std::string& out);

  //void actorMessageHandler(const idgs::actor::ActorMessagePtr& msg);

  asio::ip::tcp::socket* socket_;

  //HttpRequestHandler& request_handler_;

  uint32_t bufferSize = 0;

  size_t preLoadedSize;

  char* buffer;

  HttpRequest request_;

  HttpRequestParser request_parser_;

  HttpResponse reply_;

  idgs::actor::ActorMessagePtr actorReq;

  HttpAsyncServletPtr httpHanlder;
};
typedef std::shared_ptr<HttpConnection> HttpConnectionPtr;
} // namespace server
} // namespace http
} // idgs


