/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#pragma once

#include <atomic>
#include "idgs/actor/stateless_actor.h"
#include "idgs/store/store_config.h"

#include "idgs/store/parsed_store_descriptor.h"

namespace idgs {
namespace tpc {
namespace actor {

extern const char LINECRUD_ACTOR_ID[]; // = "linecrud_actor";
extern const char LINECRUD_ACTOR_DESCRIPTION[]; // = "linecrud_actor_description";
extern const char OP_CRUD_MAPPER[]; // = "linecrud_mapper_operation";
extern const char OP_CRUD_MULTICAST_MAPPER[]; // = "linecrud_multicast_mapper_operation";
extern const char OP_CRUD_REQUEST[]; // = "linecrud_request_operation";
extern const char OP_CRUD_RESPONSE[]; // = "linecrud_response_operation";

class LineCrudActor: public idgs::actor::StatelessActor {
public:
  LineCrudActor();
  ~LineCrudActor();
  void init();
  static idgs::actor::ActorDescriptorPtr generateActorDescriptor();
  const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override;

private:
  /// key: store_name, value: struct descriptor
  idgs::store::StoreDescriptorMap store_descriptor_cache;
  /// results
  std::atomic_ulong total_line_count, total_resp_count, total_error_resp_count;
  /// generator actor message
  idgs::actor::ActorMessagePtr genInsertActorMsg(const std::string& store_name, const std::string& line,
      idgs::ResultCode *rc, uint32_t option = 0);
  idgs::actor::ActorMessagePtr genUpdateActorMsg(const std::string& store_name, const std::string& line,
      idgs::ResultCode *rc, uint32_t option = 0);
  idgs::actor::ActorMessagePtr genDeleteActorMsg(const std::string& store_name, const std::string& line,
      idgs::ResultCode *rc, uint32_t option = 0);

  /// parse line
  idgs::store::KeyValueMessagePair parseLine(const std::string& store_name, const std::string& line,
      idgs::ResultCode* rc);

  void handleStoreMapper(const idgs::actor::ActorMessagePtr& actorMsg);
  void handleMulticastStoreMapper(const idgs::actor::ActorMessagePtr& actorMsg);
  void handleLineCRUD(const idgs::actor::ActorMessagePtr& actorMsg);
  void handleLineInsert(const idgs::actor::ActorMessagePtr& requestActorMsg);
  void handleLineUpdate(const idgs::actor::ActorMessagePtr& requestActorMsg);
  void handleLineDelete(const idgs::actor::ActorMessagePtr& requestActorMsg);

  void handleInsertResponse(const idgs::actor::ActorMessagePtr& requestActorMsg);
};
} // namespace op 
} // namespace rdd 
} // namespace idgs 
