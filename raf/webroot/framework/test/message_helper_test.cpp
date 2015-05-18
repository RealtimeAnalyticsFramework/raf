
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "gtest/gtest.h"

#include "protobuf/message_helper.h"

using namespace idgs::pb;
using namespace protobuf;

TEST(message_helper, primitive) {
  MessageHelper mh;
  auto l = mh.createMessage("idgs.pb.Long");
  ASSERT_NE((void*)NULL, l.get());
  ((Long&)(*l)).set_value(1000);
  DVLOG(1) << l->DebugString();
}


TEST(message_helper, dynamic) {
  MessageHelper mh;
  mh.registerDynamicMessage("./framework/test/employee.proto");
}

TEST(message_helper, create_dynamic_shared_ptr) {
  MessageHelper mh;
  auto l = mh.createMessage("idgs.pb.Long");
  ASSERT_NE((void*)NULL, l.get());
  ((Long&)(*l)).set_value(1000);
  DVLOG(1) << l->DebugString();
}

template<typename T>
struct Allocator
{
    typedef T value_type;

    Allocator() noexcept {};

    template<typename U>
    Allocator(const Allocator<U>& other) throw() {};

    T* allocate(std::size_t n, const void* hint = 0) {
//      return static_cast<T*>(::operator new(n * sizeof(T)));

      T* ptr = static_cast<T*>(::operator new(n * sizeof(T)));
      LOG(INFO) << "hint " << hint;
      LOG(INFO) << "allocate " << n << ", ptr = " << ptr;
      return ptr;
    }

    void deallocate(T* ptr, size_t n)
    {
      LOG(INFO) << "deallocate " << n << ", ptr = " << ptr;
      ::operator delete(ptr);
    }
};

template <typename T, typename U>
inline bool operator == (const Allocator<T>&, const Allocator<U>&)
{
    return true;
}

template <typename T, typename U>
inline bool operator != (const Allocator<T>& a, const Allocator<U>& b)
{
    return !(a == b);
}


class DummyMessage : public google::protobuf::Message {
 public:
  DummyMessage() {};
  ~DummyMessage() {};

  // implements Message ----------------------------------------------

  google::protobuf::Message* New() const {
    return NULL;
  }

  int GetCachedSize() const {
    return 0;
  }
  void SetCachedSize(int) const {
  }

  google::protobuf::Metadata GetMetadata() const {
    google::protobuf::Metadata meta;
    return meta;
  }


 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(DummyMessage);
};


std::shared_ptr<google::protobuf::Message> make_shared_message(google::protobuf::Message* msg) {
  LOG(INFO) << "sizeof " << sizeof(*msg);
  std::allocator_traits<Allocator<google::protobuf::Message> >();

  Allocator<idgs::pb::Long> alloc;
//  auto result = std::allocate_shared<idgs::pb::Long>(alloc);
  auto result = std::allocate_shared<DummyMessage>(alloc);
  LOG(INFO) << "ptr " << result.get();

  return result;
}



TEST(message_helper, create_static_shared_ptr) {
  DummyMessage dummy;
  idgs::pb::Long lv;

  LOG(INFO) << "sizeof " << sizeof(lv);
  LOG(INFO) << "sizeof(idgs::pb::Long) " << sizeof(idgs::pb::Long);
  LOG(INFO) << "sizeof(google::protobuf::Message) " << sizeof(google::protobuf::Message);
  LOG(INFO) << "sizeof(DummyMessage) " << sizeof(DummyMessage);

  auto m = make_shared_message(&lv);
  void* temp = *((void**)((void*)(&m)));
  LOG(INFO) << "managed ptr " << temp;
  temp = *((void**)((void*)(&m)) + 1);
  LOG(INFO) << "control block " << temp;
  void **cb = (void **)temp;
  LOG(INFO) << "control content " << *cb << ", " << *(cb + 1) << ", " << *(cb + 2);

  LOG(INFO) << "==========================================";
  m = std::shared_ptr<google::protobuf::Message>(new idgs::pb::Long);
  LOG(INFO) << "ptr " << m.get();
  LOG(INFO) << sizeof(m);
  temp = *((void**)((void*)(&m)));
  LOG(INFO) << "managed ptr " << temp;
  temp = *((void**)((void*)(&m)) + 1);
  LOG(INFO) << "control block " << temp;
  cb = (void **)temp;
  LOG(INFO) << "control content " << *cb << ", " << *(cb + 1) << ", " << *(cb + 2);

}



TEST(message_helper, shutdown) {
  google::protobuf::ShutdownProtobufLibrary();
}
