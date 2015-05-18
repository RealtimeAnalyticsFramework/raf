
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $


namespace {
  struct CorosyncCpgTest : private CorosyncCpg::CorosyncCpgHandler {
  public:
    CorosyncCpgTest():cpg(*this) {};

    void deliver(
        cpg_handle_t handle,
        const struct cpg_name *group,
        uint32_t nodeid,
        uint32_t pid,
        void* msg,
        int msg_len) {
      DVLOG(1) << "message deliver";
    }

    /**
     *  Corosync rpc message call back function, occur when function cpg_join or cpg_leave is called,
     *  this function only handle
     */
    void configChange(
        cpg_handle_t handle,
        const struct cpg_name *group,
        const struct cpg_address *members, int nMembers,
        const struct cpg_address *left, int nLeft,
        const struct cpg_address *joined, int nJoined
    ) {
      DVLOG(1) << "cluster configuration changed";
    }

    CorosyncCpg& getCpg() {
      return cpg;
    }
  private:
    CorosyncCpg cpg;
  };
}

void mcastMsg(const string& msg, CorosyncCpg& cpg) {
  ::iovec d;
  d.iov_base = (void*)msg.data();
  d.iov_len = msg.size();
  cpg.multicast(&d, 1);
}

TEST(corosync_cpg, getFd){
  CorosyncCpgTest test;
  string group("hello world");
  test.getCpg().init();
  test.getCpg().join(group);
  test.getCpg().getFd();
}

TEST(corosync_cpg, dispatchOneNonBlocking){
  CorosyncCpgTest test;
  string group("hello world");
  test.getCpg().init();
  test.getCpg().join(group);
  mcastMsg(group, test.getCpg());
  test.getCpg().dispatchOneNonBlocking();
}

TEST(corosync_cpg, dispatchAll){
  CorosyncCpgTest test;
  string group("hello world");
  test.getCpg().init();
  test.getCpg().join(group);

  mcastMsg(group, test.getCpg());
  test.getCpg().dispatchAll();
}

TEST(corosync_cpg, dispatchBlocking){
  CorosyncCpgTest* test = new CorosyncCpgTest();
  string group("hello world");
  test->getCpg().init();
  test->getCpg().join(group);
  mcastMsg(group, test->getCpg());

  std::thread* t = new std::thread([&test](void* d){
    test->getCpg().dispatchBlocking();
  }, (void*)NULL);

  mcastMsg(group, test->getCpg());
  mcastMsg(group, test->getCpg());
  sleep(1);
  // delete t;
  // delete test;

}
