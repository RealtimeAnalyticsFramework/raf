
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#if defined(USE_BOOST)
// #include <boost/lexical_cast.hpp>
#endif // defined(USE_BOOST)
#include "idgs/result_code.h"

namespace idgs {

template <typename Target, typename Source>
inline Target sinmple_lexical_cast(const Source &arg) {
  std::stringstream ss;
  ss << arg;
  Target result;
  ss >> result;
  return result;
}

struct sys {

  /// @brief is prime
  static bool isPrime(size_t number);

  /// @brief  Get current timestamp.
  /// @return Current timestamp.
  static unsigned long getCurrentTime();

  static std::string formatTime(double spent_time);




  /// @brief  Convert data type.
  /// @param  value The data would be converted.
  /// @param  result The converted data.
  /// @return New data type of data.
  /// @code
  /// // Example
  /// string value = "123";
  /// int32_t i;
  /// i = sys::convert<int>(value);
  /// @endcode
  template<typename out_type, typename in_value>
  static inline ResultCode convert(const in_value& value, out_type& result) {
#if defined(USE_BOOST)
    try {
      result = boost::lexical_cast<out_type>(value);
      return RC_SUCCESS;
    } catch (boost::bad_lexical_cast& e) {
      LOG(ERROR) << "Error in convert data : " << e.what();
      return RC_DATA_CONVERT_ERROR;
    } catch (...) {
      catchUnknownException();
      return RC_DATA_CONVERT_ERROR;
    }
#else // defined(USE_BOOST)
    result = sinmple_lexical_cast<out_type>(value);
    return RC_SUCCESS;
#endif // defined(USE_BOOST)
  }

  static void saveFile(const std::string& fileName, const std::string& content);
};

struct str {

  /// @brief  Trim a string with left and right space.
  /// @param  s The string would be trimed.
  /// @return Trimed string
  static std::string trim(const std::string& s);

  /// @brief  Split the string by seperattor
  /// @param  s The string would be splitted.
  /// @param  seperator The seperator for string.
  /// @param  result The result of split string.
  static void split(const std::string& s, const std::string& seperator, std::vector<std::string>& result);

  /// @brief  Convert a string to uppercase.
  /// @param  str The string would be converted.
  /// @return Uppered string
  static std::string toUpper(const std::string& str);

  /// @brief  Convert a string to lowercase.
  /// @param  str The string would be converted.
  /// @return Lowered string
  static std::string toLower(const std::string& str);
};
} // namespace idgs

/// dump a vector to stream
template<typename T>
std::ostream& operator <<(std::ostream& os, const std::vector<T>& v) {
  os << "[";
  bool first = true;
  for (auto it = v.begin(); it != v.end(); ++it) {
    if (first) {
      first = false;
    } else {
      os << ',';
    }
    os << *it;
  }
  os << "]";
  return os;
}
