
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <map>
#include <string>

namespace idgs {
namespace util {

template<typename T>
class resource_manager {
public:
  resource_manager();
  ~resource_manager();

  resource_manager(const resource_manager&) = delete;
  resource_manager(resource_manager&&) = delete;
  resource_manager& operator =(const resource_manager&) = delete;
  resource_manager& operator =(resource_manager&&) = delete;

  void put(const std::string& name, const T& action);
  const T& get(const std::string& name);
  void remove(const std::string& name);

private:
  std::map<std::string, T> resources;
};
// class resource_manager

template<typename T>
inline resource_manager<T>::resource_manager() {
}

template<typename T>
inline resource_manager<T>::~resource_manager() {
}

template<typename T>
inline void resource_manager<T>::put(const std::string& name, const T& v) {
  resources[name] = v;
}

template<typename T>
inline const T& resource_manager<T>::get(const std::string& name) {
  return resources[name];
}

template<typename T>
inline void resource_manager<T>::remove(const std::string& name) {
  resources.erase(name);
}

} // namespace util
} // namespace idgs
