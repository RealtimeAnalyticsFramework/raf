/*
 Copyright (c) <2012>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#pragma once

#include "http_servlet.h"

namespace idgs {
namespace httpserver {

class StaticHttpServlet: public idgs::httpserver::HttpServlet {
public:
  StaticHttpServlet();
  virtual ~StaticHttpServlet();

public:
  virtual std::string& getName() override {
    static std::string name = "StaticHttpServlet";
    return name;
  }

  virtual void doGet(HttpRequest& request, HttpResponse& response) override;

  void setWebroot (const std::string& webroot) {
    this->webroot = webroot;
  }

  void addVirtualDir(const std::string& vir_dir, const std::string& abs_dir) {
    virtual_dir.push_back(std::make_pair<std::string, std::string>(
        std::string(vir_dir),
        std::string(abs_dir)));
  }

private:
  std::string webroot;
  std::vector<std::pair<std::string, std::string> > virtual_dir;
};

} // namespace httpserver
} // namespace idgs
