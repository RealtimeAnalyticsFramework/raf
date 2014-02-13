/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/

#include "http_server.h"

namespace idgs{
namespace http {
namespace server {

HttpServer::HttpServer():webRoot(DEFAULT_WEB_ROOT){
  LOG(INFO) << "HTTP Server started with WEB root: " << webRoot;
}

HttpServer::HttpServer(const std::string& _webRoot):webRoot(_webRoot){
  LOG(INFO) << "HTTP Server started with WEB root: " << webRoot;
}

void HttpServer::process(asio::ip::tcp::socket *sock, char* preLoad, size_t preLoadSize) {
  VLOG(2) << "HttpServer process request: [socket:" << sock << "]  [preLoad:" << preLoad
      << "] [preLoadSize:" << preLoadSize << "]";
  std::shared_ptr<HttpConnection> pHttpCon = std::make_shared<HttpConnection>(sock);
  connectionPool.insert(pHttpCon->getHttpConnection());
  pHttpCon->start(preLoad,preLoadSize);
}

HttpAsyncServletPtr HttpServer::getHttpAsyncServlet(std::string rootName) {
  HttpAsyncServletPtr handler(NULL);

  std::map<std::string, HttpAsyncServletGenFunc>::iterator it = generators.find(rootName);
  if (it != generators.end()) {
    HttpAsyncServletGenFunc fuc = it->second;
    handler = fuc();
  }

  return handler;
}

} // namespace server
} // namespace http
} // idgs


