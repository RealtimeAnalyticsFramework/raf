/*
 Copyright (c) <2012>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#pragma once
#include <memory>
#include <assert.h>

namespace idgs {
namespace net {

class ByteBuffer : public std::enable_shared_from_this<ByteBuffer> {
public:
  ByteBuffer() {
  }

  ~ByteBuffer() {
    if (!parent) {
      delete [] buffer_;
    }
    buffer_   = NULL;
    capacity_ = 0;
  }

public:
  std::shared_ptr<ByteBuffer> slice (int offset, int size) {
    assert(offset + size <= this->capacity_);
    std::shared_ptr<ByteBuffer> buf = std::make_shared<ByteBuffer>();
    buf->parent = this->shared_from_this();
    buf->buffer_ = this->buffer_ + offset;
    buf->capacity_ = size;
    return buf;
  }

  /// allocate a new buffer
  static std::shared_ptr<ByteBuffer> allocate(int capacity)  {
    std::shared_ptr<ByteBuffer> buf = std::make_shared<ByteBuffer>();
    buf->buffer_ = new char[capacity];
    buf->capacity_ = capacity;
    return buf;
  }

  /// ByteBuffer will own the buffer and free it in the end.
  /// @param buffer_ buffer to be wrapped, must be allocated via `new char[]`
//  static std::shared_ptr<ByteBuffer> wrap(char* buffer, int capacity) {
//    std::shared_ptr<ByteBuffer> buf = std::make_shared<ByteBuffer>();
//    buf->buffer_ = buffer;
//    buf->capacity_ = capacity;
//    return buf;
//  }

  char* data() const{
    return buffer_;
  }

  int capacity() const {
    return this->capacity_;
  }

  int size() const {
    return this->size_;
  }

  void size(int size) {
    size_ = size;
  }

private:
  std::shared_ptr<ByteBuffer> parent;

  char* buffer_   = NULL;
  int   capacity_ = 0;
  int   size_     = 0;
};

} // namespace net
} // namespace idgs
