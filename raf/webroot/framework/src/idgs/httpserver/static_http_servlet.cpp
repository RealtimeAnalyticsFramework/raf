/*
 Copyright (c) <2012>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#include "static_http_servlet.h"

#include <fstream>

#include "idgs/idgslogging.h"
#include "http_mime.h"

namespace idgs {
namespace httpserver {

StaticHttpServlet::StaticHttpServlet() {
  webroot = "webroot";
}

StaticHttpServlet::~StaticHttpServlet() {
}


static int trySendFile(const std::string& realfile, HttpResponse& response) {
  std::ifstream ifs;
  ifs.open(realfile.c_str(), std::ifstream::in);

  // whether file not found
  if (!ifs.is_open()) {
    return 1;
  }

  VLOG(2) << "absolute file path: " << realfile;

  // get file length
  ifs.seekg(0, std::ios::end);
  int length = ifs.tellg();
  ifs.seekg(0, std::ios::beg);

  // read file content
  std::string buff;
  buff.resize(length);
  ifs.read(const_cast<char*>(buff.data()), length);
  ifs.close();

  response.setContent(buff);
  response.setStatus(HttpResponse::ok);
  std::string ext;
  std::string::size_type pos1 = realfile.rfind('.');
  if (pos1 != realfile.npos) {
    ext = realfile.substr(pos1 + 1, realfile.length() - pos1 - 1);
  }
  auto& mime = HttpMime::getMimeFromExt(ext);
  response.addHeader("Content-Type", mime);

  return 0;
}

void StaticHttpServlet::doGet(HttpRequest& request, HttpResponse& response) {
  LOG(INFO) << "Static content, URI: " << request.getUri();
  std::string requestUri = request.getUri();
  auto pos = requestUri.rfind("?");
  if (pos == std::string::npos) {
    pos = requestUri.rfind("#");
  }
  if (pos != std::string::npos) {
    requestUri = requestUri.substr(0, pos);
  }

  for (auto& p : virtual_dir) {
    if (requestUri.find(p.first) == 0) {
      std::string realfile = p.second + requestUri.substr(p.first.size());
      if (!realfile.empty() && realfile.at(realfile.length() - 1) == '/') {
        realfile += "index.html";
      }
      if (trySendFile(realfile, response) == 0) {
        DVLOG(2) << p.first << " => " << p.second;
        LOG(INFO) << "Absolute file path: " << realfile;
        return;
      }
    }
  }
  // default web root
  std::string realfile = webroot + requestUri;
  if (!realfile.empty() && realfile.at(realfile.length() - 1) == '/') {
    realfile += "index.html";
  }
  if (trySendFile(realfile, response) == 0) {
    LOG(INFO) << "Absolute file path: " << realfile;
    return;
  }

  // Not Found
  response = HttpResponse::createReply(HttpResponse::not_found);
}

} // namespace httpserver
} // namespace idgs
