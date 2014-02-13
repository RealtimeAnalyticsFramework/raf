/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <map>
#include <string>
#include "idgs/util/perf.h"

namespace idgs {
namespace util {
/// parse a enum body, e.g. "ONE=1, TWO, THREE"
std::map<std::string, int> parseEnumBody(const std::string& s);
std::map<int, std::string> revertEnumMap(const std::map<std::string, int>& nvmap);
} // namespace util
} // namespace idgs

/// define a enum type and two help method to covert between it and string.
#define DEF_ENUM(NAME, ...) \
  enum NAME { __VA_ARGS__ } ; \
  inline const std::string& NAME##ToString(NAME v) { \
    const static std::map<int, std::string> vnmap(idgs::util::revertEnumMap(idgs::util::parseEnumBody(#__VA_ARGS__ )));  \
    return (vnmap.at(v)); \
  } \
  inline bool StringTo##NAME(const std::string& name, NAME* v) { \
    const static std::map<std::string, int> nvmap(idgs::util::parseEnumBody(#__VA_ARGS__ ));  \
    auto it = nvmap.find(name); \
    if (likely(it != nvmap.end())) { \
      *v = static_cast<NAME>(it->second); \
      return true; \
    } \
    return false; \
  } \

