
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "attribute_operation.h"
#include <string>
#include <vector>
#include <memory>
#include "idgs/actor/actor_message.h"
#include "idgs/httpserver/http_request.h"
#include "idgs/httpserver/http_response.h"

namespace idgs {
namespace admin {

typedef std::shared_ptr<idgs::admin::AttributePath> AttributePathPtr;

struct OperationContext {
  std::string module_name;
  AttributePathPtr attr;
  idgs::actor::ActorMessagePtr actorMsg;
};

struct HttpRequestContext {
  std::string module_name;
  AttributePathPtr attr;
  std::string operation_name;
};

typedef std::map<std::string, std::function<void(OperationContext& context)> > AdminOpFunctionMapType; // the key is the attribute
typedef std::map<std::string, std::function<void(const idgs::http::server::HttpRequest& req,
  idgs::http::server::HttpResponse& rep,
  HttpRequestContext& context)> > RestFunctionMapType; // the key is the attribute

class ManageableNode {
public:
  virtual ~ManageableNode() {};

  virtual bool processRequest(OperationContext& context);

  virtual bool init() = 0;

  std::vector<std::string>& getAllAttributes() {
    return attributesPath;
  }

  void setModuleName(const std::string& _moduleName) {
    moduleName = _moduleName;
  }

  const std::string& getModuleName() const {
    return moduleName;
  }

  void addAttribute(const std::string& attrPath) {
    attributesPath.push_back(attrPath);
  }

protected:

  bool checkOperationName(OperationContext& context,
      const std::string& operationName);

  bool checkParameterValue(OperationContext& context,
      const std::string& key, std::string& value);

  std::string moduleName;
  std::vector<std::string> attributesPath;
  AdminOpFunctionMapType opMap;
};
}
}
