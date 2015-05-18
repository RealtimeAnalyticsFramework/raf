/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "command.h"
#include "client_const.h"


namespace idgs {
namespace client {
Command::Command() {
}

Command::~Command() {
}

ClientActorMessagePtr Command::toClientActorMsg(ResultCode& rc) {
  rc = idgs::RC_OK;

  ClientActorMessagePtr actorMsg(std::make_shared<ClientActorMessage>());
  actorMsg->setOperationName(opName);
  actorMsg->setDestActorId(actorId);
  actorMsg->setSourceActorId(CLIENT_ACTOR_ID);
  actorMsg->setDestMemberId(idgs::pb::ANY_MEMBER);
  actorMsg->setSourceMemberId(idgs::pb::CLIENT_MEMBER);
  PbMessagePtr payload = parsePayload(rc);
  if (rc) {
    return ClientActorMessagePtr(NULL);
  }
  actorMsg->setPayload(payload);
  AttachmentMap map = parseAttachment(rc);
  if (rc) {
    return ClientActorMessagePtr(NULL);
  }
  for (auto it = map.begin(); it != map.end(); ++it) {
    DVLOG(2) << "parsed Attachment " << it->first << " | " << it->second;
    actorMsg->setAttachment(it->first, it->second);
  }
  return actorMsg;
}

idgs::ResultCode Command::toActorMsg(ClientActorMessagePtr& actorMsg) {
  actorMsg->setOperationName(opName);
  actorMsg->setDestActorId(actorId);
  actorMsg->setSourceActorId(CLIENT_ACTOR_ID);
  actorMsg->setDestMemberId(idgs::pb::ANY_MEMBER);
  actorMsg->setSourceMemberId(idgs::pb::CLIENT_MEMBER);

  // payload
  idgs::pb::RpcMessage* rpc = actorMsg->getRpcMessage().get();
  // serdes: JSON
  rpc->set_serdes_type(idgs::pb::PB_JSON);
  // payload
  rpc->set_payload(payload);

  // attachments
  for (auto it = attachments.begin(); it != attachments.end(); ++it) {
    DVLOG(2) << "Attachment " << it->first << " | " << it->second;
    auto att = rpc->add_attachments();
    att->set_name(it->first);
    att->set_value(it->second);
  }

  return RC_OK;
}

std::string Command::toString() {
  std::stringstream ss;
  ss << "actor id: " << actorId << std::endl;
  ss << "operation name: " << opName << std::endl;
  ss << "payload: " << payload << std::endl;
  ss << "attachments: " << std::endl;
  for (auto it = attachments.begin(); it != attachments.end(); ++it) {
    ss << it->first << " = " << it->second << std::endl;
  }
  return ss.str();
}
}
}
