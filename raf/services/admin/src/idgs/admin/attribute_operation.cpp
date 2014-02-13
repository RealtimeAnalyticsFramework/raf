
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "attribute_operation.h"
#include "idgs/util/utillity.h"



namespace idgs {
namespace admin {
/**
 * the @_fullPath could be: actor_framework.stateful_actor.queue.size;stateful_actor=XXXX or actor_framework.stateless_actors
 */
bool AttributePath::parse(const std::string& _fullPath) {

  if (isParsed) {
    return true;
  }

  str::trim(_fullPath);
  fullPath = _fullPath;
  bool hasParam = (_fullPath.find(";") == std::string::npos) ? false : true;

  std::vector<std::string> pathAndParam;
  str::split(_fullPath, ";", pathAndParam);

  DVLOG(2) << "pathAndParam size: " << pathAndParam.size();
  if (pathAndParam.size() > 2) {
    LOG(ERROR)<< "the path is invalid, it should be no \";\" or just one \";\" : " << _fullPath;
    return false;
  }

  attrPathString = pathAndParam.at(0);
  attrPathString = str::trim(attrPathString);
  str::split(attrPathString, ".", attrPath);
  if (attrPath.empty()) {
    LOG(ERROR)<< "The attribute is empty";
    return false;
  }
  DVLOG(2) << "the attrPathString is " << attrPathString;
  DVLOG(2) << "the attribute size is " << attrPath.size();
  //assert(attrPathString.size() == fullPath.at(0).size());

  if (hasParam) {
    std::string& attrParams = pathAndParam.at(1);
    attrParams = str::trim(attrParams);
    std::vector<std::string> attrParamVector;
    str::split(attrParams, ",", attrParamVector);

    std::vector<std::string>::iterator it;
    for (it = attrParamVector.begin(); it != attrParamVector.end(); ++it) {
      std::string& attr_param_string = *it;
      attr_param_string = str::trim(attr_param_string);
      std::vector<std::string> attrParamValue;
      str::split(attr_param_string, "=", attrParamValue);
      if (attrParamValue.size() != 2) {
        return false;
      }
      attributeParam.insert(make_pair(str::trim(attrParamValue.at(0)), str::trim(attrParamValue.at(1))));
    }
  }

  return true;

}

bool AttributePath::getParameterValue(const std::string& key,
    std::string& value) {
  std::map<std::string, std::string>::iterator it = attributeParam.find(
      key);
  if (it == attributeParam.end()) {
    value = "";
    return false;
  }

  value = it->second;
  return true;
}
}
}



