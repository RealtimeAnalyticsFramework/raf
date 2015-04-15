
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
//#if defined(__GNUC__) || defined(__clang__)
//#include "idgs_gch.h"
//#endif // GNUC_ $

#include "gtest/gtest.h"
#include "idgs/cancelable_timer.h"
#include "idgs/client/rdd/rdd_client.h"
#include "idgs/tpc/tpc_svc_const.h"
#include "idgs/tpc/pb/tpc_rdd_action.pb.h"

using namespace std;
using namespace idgs;
using namespace idgs::rdd::pb;
using namespace idgs::client::rdd;
using namespace idgs::tpc::pb;


TEST(partition_count_action_test, create_store_delegate) {
  TEST_TIMEOUT(30);

  /// init RDD client
  RddClient client;
  ResultCode code = client.init("conf/client.conf");
  if (code != RC_SUCCESS) {
    exit(1);
  }

  /// create store delegate RDD
  DelegateRddRequestPtr request = std::make_shared<CreateDelegateRddRequest>();
  DelegateRddResponsePtr response = std::make_shared<CreateDelegateRddResponse>();
  std::string store_name = "ssb_lineorder"; // default
  char* env_store_name = getenv("RDD_CHECK_STORE_NAME");
  if(env_store_name) {
    store_name = env_store_name;
  }
  string delegate_rdd_name = "delegate_" + store_name;
  request->set_rdd_name(delegate_rdd_name);
  request->set_schema_name("ssb");
  request->set_store_name(store_name);
  client.createStoreDelegateRDD(request, response);
  LOG(INFO) << "sleep 3s, execute action";
  /// sleep 3, then exec action
  sleep(3);
  {
    ActionRequestPtr request = std::make_shared<ActionRequest>();
    ActionResponsePtr response = std::make_shared<ActionResponse>();
    ActionResultPtr result = std::make_shared<PartitionCountActionResult>();
    request->set_action_id("partition_count_action_test");
    request->set_action_op_name(idgs::tpc::PARTITION_COUNT_ACTION);
    request->set_rdd_name(delegate_rdd_name);
    client.sendAction(request, response,  result);
    PartitionCountActionResult* action_result = dynamic_cast<PartitionCountActionResult*>(result.get());
    assert(action_result);
    size_t partition_size = action_result->partition_results_size();
    /// group by member id
    std::map<int32_t, std::vector<PartitionCountResult> > member_partition_results;
    for(auto it = action_result->partition_results().begin(); it != action_result->partition_results().end(); ++it) {
      member_partition_results[it->member_id()].push_back(*it);
    }
    double sum = 0;
    for(auto it = member_partition_results.begin(); it != member_partition_results.end(); ++it) {
      DVLOG(3) << "==============member " << it->first << "==============";
      for(auto jt = it->second.begin(); jt != it->second.end(); ++jt) {
        sum += jt->size();
        DVLOG(3) << "partition id: " << jt->partition_id() << ", row count: " << jt->size();
      }
      DVLOG(3) << "==============//member " << it->first << "==============";
    }
    LOG(INFO) << "store: \"" << store_name << "\" , total row count: " << sum ;
    if(sum > 0) {
      double avg = sum / partition_size;
      double E = 0.00;
      for(auto it = action_result->partition_results().begin(); it != action_result->partition_results().end(); ++it) {
        auto size = it->size();
        E += (size - avg) * (size - avg);
      }
      //double sdc = sqrt(E/partition_size) / avg;
      //LOG(INFO) << "standard deviation coefficient = " << sdc;
    }
  }
}
