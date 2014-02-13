
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include <gtest/gtest.h>
#include "idgs/application.h"
#include <fstream>

using namespace idgs;
using namespace std;
namespace {
  struct ApplicationSetting {
    ApplicationSetting():clusterConfig("") {}
    std::string clusterConfig;
  };

//  class ActorA : StatelessActor {
//    public:
//    ActorA() {
//      setActorId("Actor A");
//      idgs::actor::ActorDescriptorPtr descriptor(new idgs::actor::ActorDescriptorWrapper);
//      descriptor->setName(getActorId());
//      idgs::actor::ActorOperationDescriporWrapper in_opA1;
//      in_opA1.setName("in_opA1");
//      descriptor->setInOperation(in_opA1.getName(), in_opA1);
//      idgs::actor::ActorOperationDescriporWrapper in_opA2;
//      in_opA2.setName("in_opA2");
//      descriptor->setInOperation(in_opA2.getName(), in_opA2);
//
//      idgs::actor::ActorOperationDescriporWrapper out_opA1;
//      out_opA1.setName("out_opA1");
//      descriptor->setOutOperation(out_opA1.getName(), out_opA1);
//      idgs::actor::ActorOperationDescriporWrapper out_opA2;
//      out_opA2.setName("out_opA2");
//      descriptor->setOutOperation(out_opA2.getName(), out_opA2);
//      this->descriptor = descriptor;
//
//      this->descriptor->addConsumeActor("Actor A");
//      this->descriptor->addConsumeActor("Actor B");
//      this->descriptor->addConsumeActor("Actor C");
//
//      ::idgs::util::singleton<idgs::actor::RpcFramework>::getInstance().getActorFramework()->Register(this->getActorId(), this);
//      ::idgs::util::singleton<idgs::actor::ActorDescriptorMgr>::getInstance().registerActorDescriptor(this->descriptor->getName(), this->descriptor);
//    }
//
//    void innerProcess(ActorMessagePtr& msg) override {
//
//    }
//  };
//
//  class ActorB : StatelessActor {
//    public:
//    ActorB() {
//      setActorId("Actor B");
//      idgs::actor::ActorDescriptorPtr descriptor(new idgs::actor::ActorDescriptorWrapper);
//      descriptor->setName(getActorId());
//      idgs::actor::ActorOperationDescriporWrapper in_opB1;
//      in_opB1.setName("in_opB1");
//      descriptor->setInOperation(in_opB1.getName(), in_opB1);
//      idgs::actor::ActorOperationDescriporWrapper in_opB2;
//      in_opB2.setName("in_opB2");
//      descriptor->setInOperation(in_opB2.getName(), in_opB2);
//
//      idgs::actor::ActorOperationDescriporWrapper out_opB1;
//      out_opB1.setName("out_opB1");
//      descriptor->setOutOperation(out_opB1.getName(), out_opB1);
//      idgs::actor::ActorOperationDescriporWrapper out_opB2;
//      out_opB2.setName("out_opB2");
//      descriptor->setOutOperation(out_opB2.getName(), out_opB2);
//      this->descriptor = descriptor;
//
//      this->descriptor->addConsumeActor("Actor B");
//      this->descriptor->addConsumeActor("Actor A");
//      this->descriptor->addConsumeActor("Actor C");
//
//      ::idgs::util::singleton<idgs::actor::RpcFramework>::getInstance().getActorFramework()->Register(this->getActorId(), this);
//      ::idgs::util::singleton<idgs::actor::ActorDescriptorMgr>::getInstance().registerActorDescriptor(this->descriptor->getName(), this->descriptor);
//    }
//
//    void innerProcess(ActorMessagePtr& msg) override {
//
//    }
//  };
//
//  class ActorC : StatelessActor {
//    public:
//    ActorC() {
//      setActorId("Actor C");
//      idgs::actor::ActorDescriptorPtr descriptor(new idgs::actor::ActorDescriptorWrapper);
//      descriptor->setName(getActorId());
//      idgs::actor::ActorOperationDescriporWrapper in_opC1;
//      in_opC1.setName("in_opC1");
//      descriptor->setInOperation(in_opC1.getName(), in_opC1);
//      idgs::actor::ActorOperationDescriporWrapper in_opC2;
//      in_opC2.setName("in_opC2");
//      descriptor->setInOperation(in_opC2.getName(), in_opC2);
//
//      idgs::actor::ActorOperationDescriporWrapper out_opC1;
//      out_opC1.setName("out_opC1");
//      descriptor->setOutOperation(out_opC1.getName(), out_opC1);
//      idgs::actor::ActorOperationDescriporWrapper out_opB2;
//      out_opB2.setName("out_opC2");
//      descriptor->setOutOperation(out_opB2.getName(), out_opB2);
//      this->descriptor = descriptor;
//
//      this->descriptor->addConsumeActor("Actor C");
//      this->descriptor->addConsumeActor("Actor A");
//      this->descriptor->addConsumeActor("Actor B");
//      ::idgs::util::singleton<idgs::actor::RpcFramework>::getInstance().getActorFramework()->Register(this->getActorId(), this);
//      ::idgs::util::singleton<idgs::actor::ActorDescriptorMgr>::getInstance().registerActorDescriptor(this->descriptor->getName(), this->descriptor);
//    }
//
//    virtual void innerProcess(ActorMessagePtr& msg) override {
//
//    }
//  };
//
} // namespace
///// check if expect and actual is matched
//static void check(const std::string expect_result_in[], const std::string expect_result_out[],const idgs::actor::ActorDescriptorPtr& actorDescriptor) {
//  const std::map<std::string, idgs::actor::ActorOperationDescriporWrapper>& ins = actorDescriptor->getInOperations();
//  const std::map<std::string, idgs::actor::ActorOperationDescriporWrapper>& outs = actorDescriptor->getOutOperations();
//  for (int i = 0, size = ins.size(); i < size; ++i) {
//    EXPECT_TRUE(ins.find(expect_result_in[i])!=ins.end());
//    EXPECT_TRUE(outs.find(expect_result_out[i])!=outs.end());
//  }
//}

//TEST(module_descriptor_dump, consume) {
//
//  // Scenario:
//  // A consume A, B, C
//  // B consume A, B, C
//  // C consume A, B, C
//
//  ActorA* A = new ActorA;
//
//  ActorB* B = new ActorB;
//
//  ActorC* C = new ActorC;
//
//  // expect result
//  const std::string expect_actorA_in[] = {"in_opA1", "in_opA2", "out_opA1", "out_opA2", "out_opB1", "out_opB2", "out_opC1", "out_opC2"};
//  const std::string expect_actorA_out[] = {"in_opA1", "in_opA2", "in_opB1", "in_opB2", "in_opC1", "in_opC2", "out_opA1", "out_opA2"};
//
//  const std::string expect_actorB_in[] = {"in_opB1", "in_opB2", "out_opA1", "out_opA2", "out_opB1", "out_opB2", "out_opC1", "out_opC2"};
//  const std::string expect_actorB_out[] = {"in_opA1", "in_opA2", "in_opB1", "in_opB2", "in_opC1", "in_opC2", "out_opB1", "out_opB2"};
//
//  const std::string expect_actorC_in[] = {"in_opC1", "in_opC2", "out_opA1", "out_opA2", "out_opB1", "out_opB2", "out_opC1", "out_opC2"};
//  const std::string expect_actorC_out[] = {"in_opA1", "in_opA2", "in_opB1", "in_opB2", "in_opC1", "in_opC2", "out_opC1", "out_opC2"};
//
//  const idgs::actor::ActorDescriptorPtr& actorDescriptorA = ::idgs::util::singleton<idgs::actor::ActorDescriptorMgr>::getInstance().getActorDescriptor("Actor A");
//  const idgs::actor::ActorDescriptorPtr& actorDescriptorB = ::idgs::util::singleton<idgs::actor::ActorDescriptorMgr>::getInstance().getActorDescriptor("Actor B");
//  const idgs::actor::ActorDescriptorPtr& actorDescriptorC = ::idgs::util::singleton<idgs::actor::ActorDescriptorMgr>::getInstance().getActorDescriptor("Actor C");
//
//  // check
//
//  check(expect_actorA_in, expect_actorA_out, actorDescriptorA);
//  check(expect_actorB_in, expect_actorB_out, actorDescriptorB);
//  check(expect_actorC_in, expect_actorC_out, actorDescriptorC);
//}


TEST(module_descriptor_dump, dumpModuleDesc) {
  ApplicationSetting setting;
  // default valuel
  setting.clusterConfig = "framework/conf/cluster.conf";

  Application& app = ::idgs::util::singleton<Application>::getInstance();
  ResultCode rc;

  DVLOG(0) << "Loading configuration .";
  rc = app.init(setting.clusterConfig);
  if (rc != RC_SUCCESS) {
    LOG(ERROR) << "Failed to initialize server: " << getErrorDescription(rc);
    exit(1);
  }

  DVLOG(0) << "Server is starting.";
  rc = app.start();
  if (rc != RC_SUCCESS) {
    LOG(ERROR) << "Failed to start server: " << getErrorDescription(rc);
    return;
  }

  DVLOG(0) << "begin to dump module descriptor.";
  sleep(4);
  cerr << "##########################################################################" << endl;
  cerr << "            Module Descriptors" << endl;
  cerr << "##########################################################################" << endl;
  auto it = ::idgs::util::singleton<idgs::actor::ActorDescriptorMgr>::getInstance().getModuleDescriptors().begin();
  auto end = ::idgs::util::singleton<idgs::actor::ActorDescriptorMgr>::getInstance().getModuleDescriptors().end();
  for(; it != end; ++it) {
    cerr << "Module: " << it->first << endl;
    cerr << it->second->toModuleDescriptor()->DebugString() << endl;
    const std::string& file_name = it->first + ".mod.json";
    std::string file_content = protobuf::JsonMessage().toPrettyJsonString(it->second->toModuleDescriptor().get());
    const std::string& file_path = file_name;
    std::ofstream os(file_path);
    os << file_content;
    os.close();
  }

  DVLOG(1) << "Server is shutting down.";
  rc = app.stop();
  if (rc != RC_SUCCESS) {
    LOG(ERROR) << "Failed to shutdown server: " << getErrorDescription(rc);
  }
}
