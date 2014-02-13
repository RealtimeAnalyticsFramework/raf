
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <string>
#include <vector>
#include <map>

namespace idgs {
namespace admin {

class AttributePath {
public:

  AttributePath():isParsed(false),nextId(0), fullPath("") {}

  //parse "actor_framework.stateful_actor.queue.size;stateful_actor=XXXX,queue=XXXX"
  bool parse(const std::string& _fullPath);

  bool hasNext() {
    return nextId < attrPath.size();
  }

  const std::string& next() {
    return attrPath.at(nextId++);
  }

  const std::string& prev() {
    return attrPath.at(nextId--);
  }

  std::string& getAttributePath() {
    return attrPathString;
  }

  const std::string& getFullPath() {
    return fullPath;
  }

  std::map<std::string, std::string>& getAllParameters() {
    return attributeParam;
  }

  bool getParameterValue(const std::string& key, std::string& value);

private:
  bool isParsed;
  std::vector<std::string> attrPath;
  int nextId;
  std::string fullPath;
  std::string attrPathString; // the whole string without parameters
  std::map<std::string, std::string> attributeParam;
};

class AttributeOperation {
public:
  ~AttributeOperation();

  // parse "[{"attribute":"actor_framework.stateful_actor.queue.size;stateful_actor=XXXX"}, {"attribute":"actor_framework.stateless_actors"}]"
  bool parse(const std::string& attributesPath);

  const std::string& getModuleName() const {
    return module_name;
  }

  void setModuleName(const std::string& moduleName) {
    module_name = moduleName;
  }

  const std::string& getOperationName() const {
    return operation_name;
  }

  void setOperationName(const std::string& operationName) {
    operation_name = operationName;
  }

  const std::vector<AttributePath>& getAttributes() const {
    return attributes;
  }

private:
  //for example:
  // command is : get [{"module":"actor","attributes":[{"attribute":"actor_framework.stateful_actor.queue.size;stateful_actor=XXXX"}, {"attribute":"actor_framework.stateless_actors"}]}]
  // the operation name is : get
  // the module name is : actor
  // the attributes are : actor_framework.stateful_actor.queue.size;stateful_actor=XXXX and actor_framework.stateless_actors
  std::string operation_name;
  std::string module_name;
  std::vector<AttributePath> attributes;
};
}
}
