/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "tcp_client.h"

using asio::ip::tcp;

namespace idgs {
namespace client {

void TcpClient::encode(std::string &data, ResultCode &code, char* data_) {
  // set body length
  size_t body_length_ = data.length();

  // encode header
  idgs::net::TcpHeader* tcpHeader = reinterpret_cast<idgs::net::TcpHeader*>(data_);
  tcpHeader->size = body_length_;

  // set body
  std::memcpy(data_ + sizeof(idgs::net::TcpHeader), data.c_str(), data.length());

  code = RC_SUCCESS;
}

size_t TcpClient::decodeHeader(char* data_, ResultCode &code) {
  idgs::net::TcpHeader* tcpHeader = reinterpret_cast<idgs::net::TcpHeader*>(data_);
  uint32_t body_length_ = tcpHeader->size;
  return body_length_;
}
}
}
