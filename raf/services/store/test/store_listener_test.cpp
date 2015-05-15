/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__)
#include "idgs_gch.h"
#endif // GNUC_ $
#include "gtest/gtest.h"

#include "idgs/store/listener/store_listener_factory.h"

namespace idgs {
namespace store {

class TestListener : public StoreListener, public idgs::util::CloneEnabler<TestListener, StoreListener> {
public:

  /// used by factory class.
  virtual const std::string& getName() const override {
    static std::string name = "TestListener";
    return name;
  }

  /// the description of current listener
  virtual const std::string& getDescription() const override {
    static std::string description = "Test store listener";
    return description;
  }

  virtual idgs::ResultCode init(std::map<std::string, std::string> props) override {
    ip = props["PROP_IP"];
    port = props["PROP_PORT"];
    user = props["PROP_USER"];
    password = props["PROP_PASSWORD"];
    return idgs::RC_SUCCESS;
  }

  virtual ListenerResultCode insert(ListenerContext* ctx) override {
    LOG(INFO) << "insert data to store " << storeName;
    return LRC_END;
  }

  virtual ListenerResultCode get(ListenerContext* ctx) override {
    LOG(INFO) << "get data from store " << storeName;
    return LRC_END;
  }

  virtual ListenerResultCode update(ListenerContext* ctx) override {
    LOG(INFO) << "update data to store " << storeName;
    return LRC_END;
  }

  virtual ListenerResultCode remove(ListenerContext* ctx) override {
    LOG(INFO) << "remove data to store " << storeName;
    return LRC_END;
  }

  virtual ListenerResultCode truncate(ListenerContext* ctx) override {
    LOG(INFO) << "truncate store " << storeName;
    return LRC_END;
  }

public:
  std::string ip;
  std::string port;
  std::string user;
  std::string password;

  std::string storeName;

};

} // namespace store
} // namespace idgs

std::map<std::string, std::string> props;

TEST(store_listener, registered) {
  props["PROP_IP"] = "127.0.0.1";
  props["PROP_PORT"] = "7788";
  props["PROP_USER"] = "root";
  props["PROP_PASSWORD"] = "intel.123";

  idgs::store::StoreListener* listener = new idgs::store::TestListener;
  listener->init(props);
  idgs::store::StoreListenerFactory::registerStoreListener(listener);

  idgs::store::TestListener* testListener = dynamic_cast<idgs::store::TestListener*>(listener);
  EXPECT_TRUE(testListener != NULL);
  EXPECT_EQ("127.0.0.1", testListener->ip);
  EXPECT_EQ("7788", testListener->port);
  EXPECT_EQ("root", testListener->user);
  EXPECT_EQ("intel.123", testListener->password);
}

TEST(store_listener, build_listener) {
  idgs::store::StoreListener* listener = NULL;
  auto code = idgs::store::StoreListenerFactory::build("TestListener", std::shared_ptr<idgs::store::StoreConfig>(), &listener, props);
  EXPECT_EQ(idgs::RC_SUCCESS, code);

  idgs::store::TestListener* testListener = dynamic_cast<idgs::store::TestListener*>(listener);
  EXPECT_TRUE(testListener != NULL);
  EXPECT_EQ("127.0.0.1", testListener->ip);
  EXPECT_EQ("7788", testListener->port);
  EXPECT_EQ("root", testListener->user);
  EXPECT_EQ("intel.123", testListener->password);

  idgs::store::ListenerContext ctx;
  listener->insert(&ctx);
  listener->update(&ctx);
  listener->get(&ctx);
  listener->remove(&ctx);
  listener->truncate(&ctx);
}
