
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <vector>
#include "http_header.h"

namespace idgs{
namespace http {
namespace server {

class HttpRequest {
public:
  std::string& getBody() {
    return body;
  }

  void setBody(const std::string& body) {
    this->body = body;
  }

  std::vector<HttpHeader>& getHeaders() {
    return headers;
  }

  void setHeaders(const std::vector<HttpHeader>& headers) {
    this->headers = headers;
  }

  int getHttpVersionMajor() const {
    return http_version_major;
  }

  void setHttpVersionMajor(int httpVersionMajor) {
    http_version_major = httpVersionMajor;
  }

  int getHttpVersionMinor() const {
    return http_version_minor;
  }

  void setHttpVersionMinor(int httpVersionMinor) {
    http_version_minor = httpVersionMinor;
  }

  std::string& getMethod() {
    return method;
  }

  void setMethod(const std::string& method) {
    this->method = method;
  }

  std::string& getRequestPath() {
    return request_path;
  }

  void setRequestPath(const std::string& requestPath) {
    request_path = requestPath;
  }

  std::string& getRootPath() {
    return root_path;
  }

  void setRootPath(const std::string& rootPath) {
    root_path = rootPath;
  }

  std::string& getUri() {
    return uri;
  }

  void setUri(const std::string& uri) {
    this->uri = uri;
  }

private:
  std::vector<HttpHeader> headers;
  std::string method;
  std::string uri;
  int http_version_major;
  std::string request_path; // if the uri is /admin/cluster/member, the request_path is cluster/member
  std::string root_path; // if the uri is /admin/cluster/member, the root_path is admin
  int http_version_minor;
  std::string body;
};

} // namespace server
} // namespace http
}
