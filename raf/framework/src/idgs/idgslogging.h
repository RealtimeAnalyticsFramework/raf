
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <glog/logging.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "idgs/pb/primitive_type.pb.h"



#if defined(NDEBUG)
#define DVLOG_IF(verboselevel, condition) \
  (true || !VLOG_IS_ON(verboselevel)) ?\
    (void) 0 : google::LogMessageVoidify() & LOG(INFO)
#define DVLOG_EVERY_N(verboselevel, n) \
  (true || !VLOG_IS_ON(verboselevel)) ? \
    (void) 0 : google::LogMessageVoidify() & LOG(INFO)

#else // defined(NDEBUG)
#define DVLOG_IF(verboselevel, condition) VLOG_IF(verboselevel, condition)
#define DVLOG_EVERY_N(verboselevel, n) VLOG_EVERY_N(verboselevel, n)
#endif // defined(NDEBUG)

// handle unknown exception
inline void catchUnknownException() {
  std::exception_ptr eptr = std::current_exception();
  if (eptr != std::exception_ptr()) {
    try {
      std::rethrow_exception(eptr);
    } catch(const std::exception& e) {
      LOG(ERROR) << "Caught exception: " << e.what();
    }
  } else {
    LOG(ERROR) << "Unknown Exception";
  }
}

// dump binary buffer
inline std::string dumpBinaryBuffer(const std::string& buffer) {
  idgs::pb::Bytes b;
  *(b.mutable_value()) = buffer;
  return b.DebugString();
}
// the same usage like Arrays.toString(byte[] ) in java, show binary
inline std::string dumpBinaryBuffer2(const char* a, size_t length) {
  if (a == NULL)
      return "null";
  int iMax = length - 1;
  if (iMax == -1)
      return "[]";
  std::stringstream s;
  s << ('[');
  for (int i = 0; ; i++) {
    s <<((int)a[i]);
    if (i == iMax) {
      s <<(']');
      return s.str();
    }
    s <<(", ");
  }
  return s.str();
}
// the same usage like Arrays.toString(char[] ) in java, show ascii
inline std::string dumpBinaryBuffer3(const char* a, size_t length) {
  if (a == NULL)
      return "null";
  int iMax = length - 1;
  if (iMax == -1)
      return "[]";
  std::stringstream s;
  s << ('[');
  for (int i = 0; ; i++) {
    s << a[i];
    if (i == iMax) {
      s <<(']');
      return s.str();
    }
    s <<(", ");
  }
  return s.str();
}

// function footprint
#if defined(NDEBUG)
  #define function_footprint()
#else
  struct FunctionFootprintHelper {
    FunctionFootprintHelper(const std::string& file, int line_, const std::string& func) : fileName(file), line(line_), functionName(func) {
      !(VLOG_IS_ON(3)) ? (void) 0 : google::LogMessageVoidify() & google::LogMessage(fileName.c_str(), line).stream() << functionName << " enter.";
    }
    ~FunctionFootprintHelper() {
      !(VLOG_IS_ON(3)) ? (void) 0 : google::LogMessageVoidify() & google::LogMessage(fileName.c_str(), line).stream() << functionName << " exit.";
    }
  private:
    const std::string fileName;
    const int line;
    const std::string functionName;
  };

#define function_footprint() FunctionFootprintHelper functionFootprint(__FILE__, __LINE__, __FUNCTION__)
#endif

