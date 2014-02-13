
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

//#define DUMP_SINGLETON
// #define DUMP_SINGLETON
#if !defined(NDEBUG) && defined(DUMP_SINGLETON)

namespace idgs {
  namespace util {
    struct SingletonNames {
      SingletonNames() {};
      ~SingletonNames() {
        function_footprint();
        std::cerr << toString() << std::endl;
      };
      std::string toString() {
        std::stringstream ss;
        ss << "############### Singleton Classes ##############" << std::endl;
        for(auto itr = names.begin(); itr != names.end(); ++itr) {
          ss << *itr << std::endl;
        }
        return ss.str();
      };
      void insert(const std::string& n) {
        names.insert(n);

      }
      std::set <std::string> names;
    };
  } // namespace util
} // namespace idgs
#endif // !defined(NDEBUG) && defined(DUMP_SINGLETON)
namespace idgs {
namespace util {
///
/// singleton class. <br>
/// Usage:<br>
/// @code
/// // Object singleton
/// ::idgs::util::singleton<SingletonClass>::getInstance().value = 1;
/// // Pointer singleton:
/// ::idgs::util::singleton<Interface*>::getInstance() = new Implementation();
/// ::idgs::util::singleton<Interface*>::getInstance()->sayHello();
/// @endcode
///
template<typename T>
class singleton {
public:
  /// get the reference to the singleton
  /// @return the singleton instance
  static inline T& getInstance() {
    static T instance;
#if !defined(NDEBUG) && defined(DUMP_SINGLETON)
    if(typeid(instance) != typeid(::idgs::util::SingletonNames)) {
      singleton< ::idgs::util::SingletonNames>::getInstance().insert(idgs::util::demangle(typeid(instance).name()));
    }
#endif // !defined(NDEBUG) && defined(DUMP_SINGLETON)
    return instance;
  }
  ;
};
// class singleton
}// namespace util
} // namespace idgs

