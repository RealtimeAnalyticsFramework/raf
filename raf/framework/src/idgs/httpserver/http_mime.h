/*
 Copyright (c) <2012>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#pragma once
#include <string>
#include <map>

namespace idgs {
namespace httpserver {

class HttpMime {
public:
  HttpMime();
  ~HttpMime();

  static const std::string& getMimeFromExt(const std::string& ext) {
    static std::string default_mime("application/octet-stream");
    auto it = ext_to_mime.find(ext);
    if (it != ext_to_mime.end()) {
      return it->second;
    }
    return default_mime;
  }
private:
  static std::map<std::string, std::string> ext_to_mime;
};

} // namespace httpserver
} // namespace idgs
