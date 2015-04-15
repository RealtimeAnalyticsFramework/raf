
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "idgs/net/rpc_member_listener.h"

#include "idgs/application.h"

#include "idgs/net/inner_tcp_server.h"

using namespace idgs::cluster;

namespace idgs {
namespace net {

RpcMemberListener::RpcMemberListener(NetworkModelAsio* network_):network(network_) {

}
RpcMemberListener::~RpcMemberListener() {
  function_footprint();
}

void RpcMemberListener::memberStatusChanged(const MemberWrapper& changedMember) {
  auto memberMgr = idgs_application()->getMemberManager();
  int32_t localMemberId = memberMgr->getLocalMemberId();
  auto localMember = memberMgr->getLocalMember();
  if(localMemberId < 0 || (localMember->getState() != idgs::pb::MS_PREPARED && localMember->getState() != idgs::pb::MS_ACTIVE)) {
    LOG(INFO) << "Local member is not ready.";
    return;
  }


  if(changedMember.getState() == idgs::pb::MS_PREPARED) {
    auto& prepared_member = const_cast<MemberWrapper&>(changedMember);
    auto prepared_endPoint = prepared_member.getMember().inner_address();
    DVLOG(3) << "Member[" << prepared_endPoint.host() << ":" << prepared_endPoint.port() << "] is prepared";
    if(prepared_member.getId() == localMemberId) {
      /// the joined is local member
      auto memberTable = memberMgr->getMemberTable();
      for(MemberWrapper member : memberTable) {
        if(member.getId() >= localMemberId) {
          // ignore member id >= local member
          continue;
        }
        // only connect to members with smaller id
        if((member.getState() == idgs::pb::MS_PREPARED || member.getState() == idgs::pb::MS_ACTIVE) && member.getId() >= 0) {
          auto endPoint = member.getMember().inner_address();
          DVLOG(0) << "local member (prepared, id: " << localMemberId << ") connect to remote member " << member.getId();
          network->putEndPoint(member.getId(), endPoint);
          network->getInnerTcpServer()->connect(member.getId());
        }
      } /// end for
    } else {
      DVLOG(0) << "local member: " << localMemberId << " connect to joined member: " << prepared_member.getId();
      network->putEndPoint(prepared_member.getId(), prepared_endPoint);
      if(prepared_member.getId() < localMemberId) {
        network->getInnerTcpServer()->connect(prepared_member.getId());
      }
    }
  }
}

} // namespace net
} // namespace idgs
