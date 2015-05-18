/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/

#include "enum_def.h"
#include "idgs/util/utillity.h"



namespace idgs {
namespace util {
/// parse a enum body, e.g. "ONE=1, TWO, THREE"
std::map<std::string, int> parseEnumBody(const std::string& s) {
  std::map<std::string, int> result;
  int currentIndex = 0;
  std::vector<std::string> values;
  std::vector<std::string> nvpair;
  ::idgs::str::split(s, ",", values);
  for (std::string& value: values) {
    value = idgs::str::trim(value);
    nvpair.clear();
    idgs::str::split(value, "=", nvpair);
    if (nvpair.size() == 1) {
      result.insert(std::make_pair(value, currentIndex));
    } else {
      std::string index = idgs::str::trim(nvpair[1]);
      // NAME = index
      if(index.at(0) >= '0' && index.at(0) <= '9') {
        currentIndex = std::stol(index);
      } else {
        LOG(ERROR) << "Unsupported enum format: " << value;
      }
      result.insert(std::make_pair(idgs::str::trim(nvpair[0]), currentIndex));
    }
    ++currentIndex;
  }

  return result;
}

std::map<int, std::string> revertEnumMap(const std::map<std::string, int>& nvmap) {
  std::map<int, std::string> result;
  for (auto& p: nvmap) {
    result.insert(std::make_pair(p.second, p.first));
  }
  return result;
}

} // namespace util
} // namespace idgs


