
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

namespace idgs {
namespace net {
#define IDGS_COOKIE (*((uint32_t*)("INTC")))

#pragma pack(push)
#pragma pack(1)
///
/// TCP packet header
///
struct TcpHeader {
  // uint32_t cookie;  // must be IDGS_COOKIE('INTC')
  uint32_t size;
};

///
/// Client login packet
///
struct ClientLogin {
  uint32_t cookie = IDGS_COOKIE;    // must be IDGS_COOKIE('INTC')
  uint8_t serdes  = 0 ;             // 0: protobuf binary, 1: protobuf text, 2: json
};

#pragma pack(pop)

///
/// RPC packet transfered in network
///
class RpcBuffer {
public:

  RpcBuffer() :
      body(NULL), bodyCapacity(0){
  }

  ~RpcBuffer() {
    freeBuffer();
  }

  void freeBuffer() {
    if (body) {
      delete[] body;
      body = NULL;
    }
    bodyCapacity = 0;
  }

  void reserveBuffer() {
    bodyCapacity = header.size;
    try {
      body = new char[bodyCapacity];
      if (!body) {
        LOG(ERROR) << "Can't alloc memory";
      }
    } catch (std::bad_alloc& e) {
      LOG(ERROR) << "Can't alloc memory: " << e.what();
    }
  }

  char* getBody() {
    return body;
  }

  size_t getBodyLength() const {
    return header.size;
  }

  void setBodyLength(size_t length) {
    header.size = length;
  }

  /// decode header and alloc memory for body.
  bool decodeHeader() {
    reserveBuffer();
    return true;
  }

  void encodeHeader() {
    reserveBuffer();
  }

  TcpHeader* getHeader() {
    return &header;
  }

private:
  TcpHeader header;

  // body buffer
  char* body;
  uint32_t bodyCapacity;

  // attachments
};

typedef std::shared_ptr<RpcBuffer> RpcBufferPtr;
} // namespace net
} // namespace idgs
