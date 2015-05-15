/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#include "http_server.h"
#include "idgs/idgslogging.h"

#include "static_http_servlet.h"
#include "actor_http_servlet.h"
#include "shell_http_servlet.h"
#include "sql_http_servlet.h"

namespace idgs {
namespace httpserver {

HttpServer::HttpServer() :
    HttpServer("webroot") {
}

HttpServer::HttpServer(const std::string& _webRoot) :
    webRoot(_webRoot) {
  LOG(INFO)<< "HTTP Server started with WEB root: " << webRoot;

  // default servlet, static contents
  std::unique_ptr<StaticHttpServlet> static_content_servlet(new StaticHttpServlet());
  static_content_servlet->setWebroot(webRoot);
  defaultServlet = std::move(static_content_servlet);

  {
    // actor agent http servlet
    std::string ctx("/actor");
    auto servlet = std::unique_ptr<HttpServlet>(new ActorHttpServlet);
    registerHttpServlet(ctx, servlet);
  }

  {
    // shell script http servlet
    std::string ctx("/shell");
    auto servlet = std::unique_ptr<HttpServlet>(new ShellHttpServlet);
    registerHttpServlet(ctx, servlet);
  }

  {
    // SQL http servlet
    std::string ctx("/sql");
    auto servlet = std::unique_ptr<HttpServlet>(new SqlHttpServlet);
    registerHttpServlet(ctx, servlet);
  }

}

HttpServlet* HttpServer::lookupServlet(const std::string& requestUri) {
  for (auto& p : servletMap) {
    if (requestUri.find(p.first) == 0) {
      return p.second.get();
    }
  }
  return defaultServlet.get();
}

void HttpServer::addVirtualDir(const std::string& virtual_dir, const std::string& abs_dir) {
  dynamic_cast<StaticHttpServlet*>(defaultServlet.get())->addVirtualDir(virtual_dir, abs_dir);
}


void HttpServer::process(asio::ip::tcp::socket *sock, char* preLoad, size_t preLoadSize) {
  VLOG(2) << "HttpServer process request: [socket:" << sock << "]  [buffer:" << std::string(preLoad, preLoadSize) << "] [size:"
             << preLoadSize << "]";
  std::shared_ptr<HttpConnection> conn = std::make_shared < HttpConnection > (sock);

  {
    std::lock_guard<std::mutex> guard(connection_lock);
    connectionPool.insert(conn);
    VLOG(2) << "Http connections: " << connectionPool.size();
  }

  conn->start(preLoad, preLoadSize);

}

} // namespace httpserver
} // idgs

