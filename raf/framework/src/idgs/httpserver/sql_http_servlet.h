/*
 Copyright (c) <2012>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#pragma once
#include <mutex>

#include "http_servlet.h"

namespace idgs {
namespace httpserver {

class DaemonProcess {
public:
  DaemonProcess();
  ~DaemonProcess();

  int start(const std::string& cmd);
  int stop();

  std::string recv();
  int sendrecv(const std::string& request, std::string& response);
private:
  int pipe_in[2];
  int pipe_out[2];
  pid_t pid;
};

class DaemonProcessFactory {
public:
  std::shared_ptr<DaemonProcess> getProcess();

private:
  std::shared_ptr<DaemonProcess> proc;
};




class SqlHttpServlet: public idgs::httpserver::HttpServlet {
public:
  SqlHttpServlet();
  virtual ~SqlHttpServlet();

public:
  virtual std::string& getName() override {
    static std::string name = "DaemonHttpServlet";
    return name;
  }

  virtual void doPost(HttpRequest& request, HttpResponse& response) override;

private:
  std::mutex lock;
  DaemonProcessFactory factory;
};

} // namespace httpserver
} // namespace idgs
