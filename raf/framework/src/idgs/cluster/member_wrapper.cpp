
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include <sstream>
#include "idgs/cluster/member_wrapper.h"

using namespace std;
using namespace idgs::pb;

namespace idgs {

namespace cluster {

MemberWrapper::MemberWrapper() {
  setStatus(INITIAL);
}

void MemberWrapper::setPartitionCount(size_t pos, size_t partition_count) {
  size_t size = partitionCount.size();
  if (pos >= size) {
    partitionCount.resize(size + 1);
  }
  partitionCount[pos] = partition_count;
}

bool MemberWrapper::canBalanced() const {
  return getId() >= 0 && isLocalStore() && (isJoined() || isPrepared() || isActive());
}

bool MemberWrapper::isLocalMember(const idgs::pb::Member& cfg_member) const {
  return member.publicaddress().host() == cfg_member.publicaddress().host()
      && member.publicaddress().port() == cfg_member.publicaddress().port();
}

ostream& operator << (ostream& os, const MemberWrapper& mw) {
  os << mw.toShortString();
  return os;
}

std::string MemberWrapper::toString() const {
  stringstream s;
  s << member.DebugString();
  return s.str();
}

std::string MemberWrapper::toShortString() const {
  stringstream s;
  auto publicAddr = member.publicaddress();
  auto innerAddr = member.inneraddress();
  s << getId();
  if(publicAddr.host().compare(innerAddr.host())) {
    s << ", " << "[" << publicAddr.host() << ":" << publicAddr.port() << "," << innerAddr.host() << ":" << innerAddr.port() << "]";
  } else {
    s << ", " << "[" << publicAddr.host() << ":" << publicAddr.port() << "," << innerAddr.port() << "]";
  }
  s << ", " << pb::MemberStatus_Name(getStatus());
  s << ", " << "store" << "(" << (isLocalStore() ? "Y" : "N") << ")";
  s << ", " << "leading" << "(" << (isLeading() ? "Y" : "N") << ")";
  return s.str();
}

} // end namespace cluster
} // end namespace idgs
