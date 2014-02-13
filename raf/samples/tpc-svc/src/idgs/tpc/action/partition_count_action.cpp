/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "partition_count_action.h"
#include "idgs/util/utillity.h"
#include "idgs/rdd/rdd_const.h"
#include "idgs/cluster/cluster_framework.h"
#include "idgs/tpc/pb/tpc_rdd_action.pb.h"

using namespace std;
using namespace protobuf;
using namespace idgs::rdd;
using namespace idgs::rdd::pb;
using namespace idgs::tpc::pb;

namespace idgs {
namespace tpc {
namespace action {

PartitionCountAction::PartitionCountAction() {
}

PartitionCountAction::~PartitionCountAction() {
}

RddResultCode PartitionCountAction::action(const idgs::actor::ActorMessagePtr& msg, const BaseRddPartition* input,
    std::vector<PbVariant>& output) {
  auto start = sys::getCurrentTime();
  static int32_t local_member_id =
      ::idgs::util::singleton<idgs::cluster::ClusterFramework>::getInstance().getMemberManager()->getLocalMemberId();
  uint32_t partition_id = input->getPartition();
  DVLOG(3) << "Execute partition count action in member: " << local_member_id << ", partition: "
              << input->getPartition();
  uint32_t size = input->size();

  PartitionCountResult result;
  result.set_member_id(local_member_id);
  result.set_partition_id(partition_id);
  result.set_size(size);
  string str;
  ProtoSerdes<DEFAULT_PB_SERDES>::serialize(&result, &str);
  output.push_back(str);
  VLOG(2) << "Execute partition count action in partition: " << input->getPartition() << ", spent time: "
             << sys::formatTime((sys::getCurrentTime() - start));
  return RRC_SUCCESS;
}

RddResultCode PartitionCountAction::aggregate(const idgs::actor::ActorMessagePtr& actionRequest, idgs::actor::ActorMessagePtr& actionResponse, const vector<vector<string>>& input) {
  auto start = sys::getCurrentTime();
  static int32_t local_member_id =
      ::idgs::util::singleton<idgs::cluster::ClusterFramework>::getInstance().getMemberManager()->getLocalMemberId();
  DVLOG(3) << "Aggregate partition count action in member: " << local_member_id;
  shared_ptr<PartitionCountActionResult> result(new PartitionCountActionResult);
  for (auto it = input.begin(); it != input.end(); ++it) {
    if (it->empty()) {
      continue;
    }
    DVLOG(3) << "@@@@@@@@@@@" << it->at(0);
    auto partition_result = result->add_partition_results();
    ProtoSerdes<DEFAULT_PB_SERDES>::deserialize(it->at(0), partition_result);
    DVLOG(3) << "@@@@@@@@@@@" << partition_result->ShortDebugString();
  }
  actionResponse->setAttachment(ACTION_RESULT, result);
  VLOG(2) << "Aggregate partition count action spent time: " << sys::formatTime((sys::getCurrentTime() - start));
  return RRC_SUCCESS;
}
} // namespace action 
} // namespace rdd 
} // namespace idgs 
