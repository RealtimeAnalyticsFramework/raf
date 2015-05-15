
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#pragma once

#include "http_request.h"
#include "http_response.h"

namespace idgs{
namespace httpserver {


static const char GET[] = "GET";
static const char DELETE[] = "DELETE";
static const char HEAD[] = "HEAD";
static const char OPTIONS[] = "OPTIONS";
static const char POST[] = "POST";
static const char PUT[] = "PUT";

class HttpServlet {
public:
  virtual ~HttpServlet() {}
  virtual void init() {}
  virtual std::string& getName() {
    static std::string name = "HttpServlet";
    return name;
  }

public:
  virtual void service(HttpRequest& request, HttpResponse& response);
  virtual void doGet(HttpRequest& request, HttpResponse& response);
  virtual void doDelete(HttpRequest& request, HttpResponse& response);
  virtual void doHead(HttpRequest& request, HttpResponse& response);
  virtual void doOptions(HttpRequest& request, HttpResponse& response);
  virtual void doPost(HttpRequest& request, HttpResponse& response);
  virtual void doPut(HttpRequest& request, HttpResponse& response);
protected:
  void notSupportedAction(HttpRequest& request, HttpResponse& response);
};


typedef std::shared_ptr<idgs::httpserver::HttpServlet> HttpServletPtr;

} // namespace httpserver
} // namespace idgs
