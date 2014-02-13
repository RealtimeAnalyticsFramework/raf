
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <asio.hpp>
#include "http_header.h"

namespace idgs{
namespace http {
namespace server {

/// A reply to be sent to a client.
class HttpResponse {
public:
  /// The status of the reply.
  enum StatusType {
    ok = 200,
    created = 201,
    accepted = 202,
    no_content = 204,
    multiple_choices = 300,
    moved_permanently = 301,
    moved_temporarily = 302,
    not_modified = 304,
    bad_request = 400,
    unauthorized = 401,
    forbidden = 403,
    not_found = 404,
    internal_server_error = 500,
    not_implemented = 501,
    bad_gateway = 502,
    service_unavailable = 503
  } status;

  const std::string& getContent() const {
    return content;
  }

  void setContent(const std::string& content) {
    this->content = content;
  }

  const std::vector<idgs::http::server::HttpHeader>& getHeaders() const {
    return headers;
  }

  void setHeaders(
      const std::vector<idgs::http::server::HttpHeader>& headers) {
    this->headers = headers;
  }

  /// Convert the reply into a vector of buffers. The buffers do not own the
  /// underlying memory blocks, therefore the reply object must remain valid and
  /// not be changed until the write operation has completed.
  std::vector<asio::const_buffer> toBuffers();

  /// Get a stock reply.
  static HttpResponse createReply(StatusType status);

  static HttpResponse createReply(StatusType status, const std::string& body);

  static HttpResponse stockRedirectRespons(std::string& redUri);

private:

  /// The headers to be included in the reply.
  std::vector<idgs::http::server::HttpHeader> headers;

  /// The content to be sent in the reply.
  std::string content;



};

} // namespace server
} // namespace http
}
