/*
 * rdd_cluster_listener.h
 *
 *  Created on: Jan 21, 2015
 *      Author: root
 */

#pragma once

#include "idgs/cluster/member_event_listener.h"

namespace idgs {
namespace rdd {

class RddMemberEventListener : public idgs::cluster::MemberEventListener {
public:
  RddMemberEventListener();
  virtual ~RddMemberEventListener();

  void memberStatusChanged(const idgs::cluster::MemberWrapper& member) override;

};

} /* namespace rdd */
} /* namespace idgs */
