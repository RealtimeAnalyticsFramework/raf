
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "idgs/net/rpc_member_listener.h"
#include "idgs/net/inner_tcp_server.h"
#include "idgs/cluster/cluster_framework.h"

using namespace idgs::cluster;

namespace idgs {
namespace net {

RpcMemberListener::RpcMemberListener(NetworkModelAsio* network_):network(network_) {

}
RpcMemberListener::~RpcMemberListener() {
  function_footprint();
}

void RpcMemberListener::statusChanged(const MemberWrapper& changedMember) {
  if(changedMember.isPrepared()) {
    auto& prepared_member = const_cast<MemberWrapper&>(changedMember);
    auto prepared_endPoint = prepared_member.getMember().inneraddress();
    DVLOG(3) << "Member[" << prepared_endPoint.host() << ":" << prepared_endPoint.port() << "] is prepared";
    int32_t localMemberId = ::idgs::util::singleton<ClusterFramework>::getInstance().getMemberManager()->getLocalMemberId();
    if(localMemberId == -1) {
      return;
    }
    auto localMember = ::idgs::util::singleton<ClusterFramework>::getInstance().getMemberManager()->getLocalMember();
    /// local member loop to build connection with other prepared members
    if(prepared_member.getId() == localMemberId) {
      auto memberTable = ::idgs::util::singleton<ClusterFramework>::getInstance().getMemberManager()->getMemberTable();
      for(MemberWrapper member : memberTable) {
        if(member.getId() == prepared_member.getId()) {
          continue; /// ignore connect to itself
        }
        if(member.isPrepared() || member.isActive()) {
          auto endPoint = member.getMember().inneraddress();
          DVLOG(0) << "local prepared member: " << localMemberId << " will create end point with member: " << member.getId();
          network->putEndPoint(member.getId(), endPoint);
          if(member.getId() < localMemberId) {
            network->getInnerTcpServer()->getConnection(member.getId())->connect(member.getId());
          }
        }
      } /// end for
    } /// end if
    /// not local member, build connection with this prepared member
    else if(localMember && (localMember->isPrepared() || localMember->isActive())){
      DVLOG(0) << "local member: " << localMemberId << " will create end point with prepared member: " << prepared_member.getId();
      network->putEndPoint(prepared_member.getId(), prepared_endPoint);
      if(prepared_member.getId() < localMemberId) {
        network->getInnerTcpServer()->getConnection(prepared_member.getId())->connect(prepared_member.getId());
      }
    } /// end else
  }
}

} // namespace net
} // namespace idgs
