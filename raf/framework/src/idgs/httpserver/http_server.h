/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#pragma once

#include <mutex>
#include <set>
#include "http_connection.h"

namespace idgs{
namespace httpserver {


class HttpServer {
public:
  HttpServer();
  HttpServer(const std::string& _webRoot);

  void stop () {
    servletMap.clear();
    connectionPool.clear();
  }

public:
  void process(asio::ip::tcp::socket *sock, char* preLoad = 0, size_t preLoadSize = 0);
  void removeConnection(std::shared_ptr<HttpConnection> conn) {
    std::lock_guard<std::mutex> guard(connection_lock);
    connectionPool.erase(conn);
  }

  /// reqister http servlet
  /// @param context http URI prefix, e.g. /admin
  /// @param servlet pointer to the servlet, the pointer will be MOVED to the map.
  void registerHttpServlet(std::string& context, std::unique_ptr<HttpServlet>& servlet) {
    servletMap.push_back(std::make_pair(context, std::move(servlet)));
  }

  HttpServlet* lookupServlet(const std::string& requestUri);

  void addVirtualDir(const std::string& virtual_dir, const std::string& abs_dir);
private:
  std::string webRoot;

  std::set<std::shared_ptr<HttpConnection> > connectionPool;
  std::mutex connection_lock;

  std::vector<std::pair<std::string, std::unique_ptr<HttpServlet> > > servletMap;
  std::unique_ptr<HttpServlet> defaultServlet;

};

} // namespace httpserver
} // idgs
