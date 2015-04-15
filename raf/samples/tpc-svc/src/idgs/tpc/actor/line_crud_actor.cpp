/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(__GNUC__) || defined(__clang__) $
#include "line_crud_actor.h"
#include "idgs/store/store_module.h"
#include "idgs/tpc/pb/tpc_crud.pb.h"
#include "idgs/util/utillity.h"

using namespace idgs::tpc::pb;
using namespace idgs::pb;
using namespace idgs::actor;
using namespace std;
using namespace idgs::store::pb;
using namespace idgs::store;

namespace idgs {
namespace tpc {
namespace actor {

const std::string& LINECRUD_ACTOR_ID = "tpc.linecrud";
const std::string& LINECRUD_ACTOR_DESCRIPTION = "linecrud_actor_description";
const std::string& OP_CRUD_MAPPER = "linecrud_mapper_operation";
const std::string& OP_CRUD_MULTICAST_MAPPER = "linecrud_multicast_mapper_operation";
const std::string& OP_CRUD_REQUEST = "linecrud_request_operation";
const std::string& OP_CRUD_RESPONSE = "linecrud_response_operation";

LineCrudActor::LineCrudActor() :
    total_line_count(0), total_resp_count(0), total_error_resp_count(0) {
  setActorId(LINECRUD_ACTOR_ID);
}

const idgs::actor::ActorMessageHandlerMap& LineCrudActor::getMessageHandlerMap() const {
  static std::map<std::string, idgs::actor::ActorMessageHandler> handlerMap = { { OP_CRUD_MAPPER,
      static_cast<idgs::actor::ActorMessageHandler>(&LineCrudActor::handleStoreMapper) }, { OP_CRUD_MULTICAST_MAPPER,
      static_cast<idgs::actor::ActorMessageHandler>(&LineCrudActor::handleMulticastStoreMapper) }, { OP_CRUD_REQUEST,
      static_cast<idgs::actor::ActorMessageHandler>(&LineCrudActor::handleLineCRUD) }, { OP_INSERT_RESPONSE,
      static_cast<idgs::actor::ActorMessageHandler>(&LineCrudActor::handleInsertResponse) }, };
  return handlerMap;
}

ActorDescriptorPtr LineCrudActor::generateActorDescriptor() {
  static std::shared_ptr<ActorDescriptorWrapper> descriptor;
  if (descriptor) {
    return descriptor;
  }

  descriptor = std::make_shared<ActorDescriptorWrapper>();

  descriptor->setName(LINECRUD_ACTOR_ID);
  descriptor->setDescription(LINECRUD_ACTOR_DESCRIPTION);
  descriptor->setType(::idgs::pb::AT_STATELESS);

  // in operation
  ActorOperationDescriporWrapper op_crud_request;
  op_crud_request.setName(OP_CRUD_REQUEST);
  op_crud_request.setDescription("Line CRUD Request Operation");
  op_crud_request.setPayloadType("idgs.tpc.pb.RawlineCrudRequest");
  descriptor->setInOperation(op_crud_request.getName(), op_crud_request);

  // in operation
  ActorOperationDescriporWrapper op_crud_mapper;
  op_crud_mapper.setName(OP_CRUD_MAPPER);
  op_crud_mapper.setDescription("Line CRUD Mapper Operation");
  op_crud_mapper.setPayloadType("idgs.store.pb.StoreFileMapperConfig");
  descriptor->setInOperation(op_crud_mapper.getName(), op_crud_mapper);

  // in operation
  ActorOperationDescriporWrapper op_crud_multicast_mapper;
  op_crud_multicast_mapper.setName(OP_CRUD_MULTICAST_MAPPER);
  op_crud_multicast_mapper.setDescription("Line CRUD Multicast Mapper Operation");
  op_crud_multicast_mapper.setPayloadType("idgs.store.pb.StoreFileMapperConfig");
  descriptor->setInOperation(op_crud_multicast_mapper.getName(), op_crud_multicast_mapper);

  // out operation
  ActorOperationDescriporWrapper op_crud_response;
  op_crud_response.setName(OP_CRUD_REQUEST);
  op_crud_response.setDescription("Line CRUD Response Operation");
  op_crud_response.setPayloadType("idgs.tpc.pb.RawlineCrudResponse");
  descriptor->setOutOperation(op_crud_response.getName(), op_crud_response);

  descriptor->addConsumeActor(ACTORID_STORE_SERVCIE); // consume actor

  return descriptor;
}

LineCrudActor::~LineCrudActor() {

}

void LineCrudActor::init() {
  this->descriptor = generateActorDescriptor();
}

void LineCrudActor::handleInsertResponse(const idgs::actor::ActorMessagePtr& actorMsg) {
  InsertResponse* response = dynamic_cast<InsertResponse*>(actorMsg->getPayload().get());
  if (response->result_code() == SRC_SUCCESS) {
    ++total_resp_count;
  } else {
    LOG(ERROR)<< "DataStore insert data error, error code = " << response->result_code();
    ++total_error_resp_count;
  }
}

/// @todo
void LineCrudActor::handleLineUpdate(const ActorMessagePtr& requestActorMsg) {

}

/// @todo
void LineCrudActor::handleLineDelete(const ActorMessagePtr& requestActorMsg) {

}

void LineCrudActor::handleLineInsert(const ActorMessagePtr& requestActorMsg) {
  RawlineCrudRequest* payload = dynamic_cast<RawlineCrudRequest*>(requestActorMsg->getPayload().get());
  const int line_size = payload->lines_size();
  ActorMessagePtr responseActorMsg = requestActorMsg->createResponse();
  responseActorMsg->setDestActorId(requestActorMsg->getSourceActorId());
  responseActorMsg->setDestMemberId(requestActorMsg->getSourceMemberId());
  responseActorMsg->setOperationName(OP_CRUD_RESPONSE);
  std::shared_ptr<RawlineCrudResponse> response_payload = std::make_shared<RawlineCrudResponse>();
  total_line_count += line_size;
  response_payload->set_total_line_count(total_line_count);
  response_payload->set_total_resp_count(total_resp_count);
  response_payload->set_total_error_resp_count(total_error_resp_count);
  responseActorMsg->setPayload(response_payload);

  // response to client
  DVLOG(3) << "Response actor message: " << responseActorMsg->toString();
  idgs::actor::sendMessage(responseActorMsg);
  // forward to store
  idgs::ResultCode rc;
  for (int i = 0; i < line_size; ++i) {
    ActorMessagePtr forwardActorMsg = genInsertActorMsg(payload->store_name(), payload->lines(i), &rc,
        payload->option());
    if (rc != RC_SUCCESS) {
      LOG(ERROR)<< "generateInsertMessage error, error code: " << rc << ", error message: " << getErrorDescription(rc);
      continue;
    }
    DVLOG(3) << "Forward actor message: " << forwardActorMsg->toString();
    idgs::actor::postMessage(forwardActorMsg);
  }
}

void static createStoreFieldDescriptor(const std::string& field_name, const PbMessagePtr& key,
    const PbMessagePtr& value, ParsedStoreFieldDescriptor* descriptor) {
  for (size_t i = 0, count = key->GetDescriptor()->field_count(); i < count; ++i) {
    if (key->GetDescriptor()->field(i)->name().compare(field_name) == 0) {
      descriptor->type = KEY_TYPE;
      descriptor->descriptor = key->GetDescriptor()->field(i);
      break;
    }
  }
  for (size_t j = 0, count = value->GetDescriptor()->field_count(); j < count; ++j) {
    if (value->GetDescriptor()->field(j)->name().compare(field_name) == 0) {
      descriptor->type = VALUE_TYPE;
      descriptor->descriptor = value->GetDescriptor()->field(j);
      break;
    }
  }
}

void LineCrudActor::handleStoreMapper(const idgs::actor::ActorMessagePtr& actorMsg) {
  ActorMessagePtr new_actor_msg = createActorMessage();
  new_actor_msg->setPayload(actorMsg->getPayload());
  new_actor_msg->setDestActorId(getActorId());
  new_actor_msg->setDestMemberId(ALL_MEMBERS);
  new_actor_msg->setChannel(TC_MULTICAST);
  new_actor_msg->setOperationName(OP_CRUD_MULTICAST_MAPPER);
  /// multicast to all member
  DVLOG(3) << "member " << new_actor_msg->getSourceMemberId() << " recv and multicast to all members mapper config \n"
              << actorMsg->getPayload()->DebugString();
  idgs::actor::sendMessage(new_actor_msg);
}

void LineCrudActor::handleMulticastStoreMapper(const idgs::actor::ActorMessagePtr& actorMsg) {
  StoreFileMapperConfig* mapper_config = dynamic_cast<StoreFileMapperConfig*>(actorMsg->getPayload().get());
  DVLOG(3) << "Recv store mapper config from member  " << actorMsg->getSourceMemberId() << "\n"
              << mapper_config->DebugString();
  for (auto it = mapper_config->mapper().begin(); it != mapper_config->mapper().end(); ++it) {
//          const std::string& file_name = it->file_name();
    const std::string& store_name = it->store_name();
    auto store = idgs_store_module()->getDataStore()->getStore(store_name);
    if (!store) {
      LOG(ERROR)<< "store named " << store_name << " is not found.";
    }

    auto& store_config_wrapper_ptr = store->getStoreConfigWrapper();

    ParsedStoreDescriptor descriptor;
    descriptor.mapper->CopyFrom(*it);
    const std::string& key_type = store_config_wrapper_ptr->getStoreConfig().key_type();
    const std::string& value_type = store_config_wrapper_ptr->getStoreConfig().value_type();
    descriptor.key_type = key_type;
    descriptor.value_type = value_type;
    PbMessagePtr key_type_msg = store_config_wrapper_ptr->newKey();
    PbMessagePtr value_type_msg = store_config_wrapper_ptr->newValue();
    if (it->fields_size() > 0) { /// user defined fields
      descriptor.fieldDescriptor.reserve(it->fields_size());
      for (auto ft = it->fields().begin(); ft != it->fields().end(); ++ft) {
        ParsedStoreFieldDescriptor field_descriptor;
        createStoreFieldDescriptor(*ft, key_type_msg, value_type_msg, &field_descriptor);
        descriptor.fieldDescriptor.push_back(field_descriptor);
      }
    } else { /// user not define, using store config default fields
      for (size_t i = 0, count = key_type_msg->GetDescriptor()->field_count(); i < count; ++i) {
        ParsedStoreFieldDescriptor field_descriptor;
        field_descriptor.type = KEY_TYPE;
        field_descriptor.descriptor = key_type_msg->GetDescriptor()->field(i);
        descriptor.fieldDescriptor.push_back(field_descriptor);
      }
      for (size_t j = 0, count = value_type_msg->GetDescriptor()->field_count(); j < count; ++j) {
        ParsedStoreFieldDescriptor field_descriptor;
        field_descriptor.type = VALUE_TYPE;
        field_descriptor.descriptor = value_type_msg->GetDescriptor()->field(j);
        descriptor.fieldDescriptor.push_back(field_descriptor);
      }
    }
    store_descriptor_cache.insert(std::pair<std::string, ParsedStoreDescriptor>(it->store_name(), descriptor));
  }
}

void LineCrudActor::handleLineCRUD(const ActorMessagePtr& requestActorMsg) {
  function_footprint();
  RawlineCrudRequest* payload = dynamic_cast<RawlineCrudRequest*>(requestActorMsg->getPayload().get());
  switch (payload->type()) {
  case CrudType::INSERT:
    handleLineInsert(requestActorMsg);
    break;
  case CrudType::UPDATE:
    handleLineUpdate(requestActorMsg);
    break;
  case CrudType::DELETE:
    handleLineDelete(requestActorMsg);
    break;
  default:
    break;
  }
}

idgs::actor::ActorMessagePtr LineCrudActor::genInsertActorMsg(const std::string& store_name, const std::string& line,
    idgs::ResultCode *rc, uint32_t option) {
  KeyValueMessagePair pair = parseLine(store_name, line, rc);
  if (*rc != RC_SUCCESS) {
    LOG(ERROR)<< "parseLine error, error code: " << *rc << ", error message: " << getErrorDescription(*rc);
    return ActorMessagePtr(NULL);
  }
  std::shared_ptr<InsertRequest> pay_load = std::make_shared<InsertRequest>();
  pay_load->set_store_name(store_name);
  pay_load->set_options(option);
  ActorMessagePtr actorMsg = createActorMessage();
  actorMsg->setOperationName(OP_INSERT);
  actorMsg->setDestActorId(ACTORID_STORE_SERVCIE);
  actorMsg->setDestMemberId(ANY_MEMBER);
  actorMsg->setPayload(pay_load);
  actorMsg->setAttachment(STORE_ATTACH_KEY, pair.first);
  actorMsg->setAttachment(STORE_ATTACH_VALUE, pair.second);
  return actorMsg;
}

/// @todo
idgs::actor::ActorMessagePtr LineCrudActor::genUpdateActorMsg(const std::string& store_name, const std::string& line,
    idgs::ResultCode *rc, uint32_t option) {
  ActorMessagePtr actorMsg = createActorMessage();
  return actorMsg;
}

/// @todo
idgs::actor::ActorMessagePtr LineCrudActor::genDeleteActorMsg(const std::string& store_name, const std::string& line,
    idgs::ResultCode *rc, uint32_t option) {
  ActorMessagePtr actorMsg = createActorMessage();
  return actorMsg;
}

KeyValueMessagePair LineCrudActor::parseLine(const std::string& store_name, const std::string& line,
    idgs::ResultCode* rc) {
  auto start = sys::getCurrentTime();
  if (store_descriptor_cache.find(store_name) == store_descriptor_cache.end()) {
    LOG(ERROR)<< "store: " << store_name << " not found, line: " << line << " ignored";
    return KeyValueMessagePair(NULL, NULL);
  }
  const ParsedStoreDescriptor& descriptor = store_descriptor_cache.at(store_name);
  vector<string> result;
  idgs::str::split(line, descriptor.mapper->seperator(), result);

  auto store = idgs_store_module()->getDataStore()->getStore(store_name);
  auto& store_config_wrapper_ptr = store->getStoreConfigWrapper();

  PbMessagePtr key = store_config_wrapper_ptr->newKey();
  PbMessagePtr value = store_config_wrapper_ptr->newValue();
  if (!key.get()) {
    LOG(ERROR)<< "Parse line error, key message is null, store descriptor: " << descriptor.toString();
    *rc = RC_PARSE_LINE_ERROR;
    return KeyValueMessagePair(NULL, NULL);
  }
  if (!value.get()) {
    LOG(ERROR)<< "Parse line error, value message is null, store descriptor: " << descriptor.toString();
    *rc = RC_PARSE_LINE_ERROR;
    return KeyValueMessagePair(NULL, NULL);
  }
  if (result.size() != descriptor.fieldDescriptor.size()) {
    std::stringstream s;
    s << "########################fields:######################## " << std::endl;
    for (size_t index = 0, size = descriptor.fieldDescriptor.size(); index < size; ++index) {
      s << index << ": " << descriptor.fieldDescriptor[index].toString();
    }
    s << std::ends;
    std::stringstream ss;
    ss << "########################results:######################## " << std::endl;
    for (size_t index = 0, size = result.size(); index < size; ++index) {
      std::string result_value = result.at(index);
      ss << index << ": " << result_value << std::endl;
    }
    ss << std::ends;
    LOG(ERROR)<< "Parse store:" << store_name <<" error \n line: " << line << " \n caused by result size != field size" << ", result size:" << result.size() << ", field size:" << descriptor.fieldDescriptor.size()
    << " \n" << s.str() << ss.str();
    *rc = RC_PARSE_LINE_ERROR;
    return KeyValueMessagePair(NULL, NULL);
  }

  protobuf::MessageHelper helper;
  for (size_t index = 0, size = result.size(); index < size; ++index) {
    std::string result_value = str::trim(result.at(index));
    const ParsedStoreFieldDescriptor& field_descriptor = descriptor.fieldDescriptor.at(index);
    if (field_descriptor.type == KEY_TYPE) {
      *rc = helper.setMessageValue(key.get(), field_descriptor.descriptor, result_value);
      if (*rc != RC_SUCCESS) {
        LOG(ERROR)<< "set Message KEY's field value error, error code: " << *rc
        << ", field index: " << index << ", field name: " << field_descriptor.descriptor->name() << ", field value: " << result_value << ", line: " << line;
        return KeyValueMessagePair(NULL, NULL);
      }
    } else if(field_descriptor.type == VALUE_TYPE) {
      *rc = helper.setMessageValue(value.get(), field_descriptor.descriptor, result_value);
      if(*rc != RC_SUCCESS) {
        LOG(ERROR) << "set Message VALUE's field value error, error code: " << *rc
        << ", field index: " << index << ", field name: " << field_descriptor.descriptor->name() << ", field value: " << result_value << ", line: " << line;
        return KeyValueMessagePair(NULL, NULL);
      }
    }
  }
  LOG_FIRST_N(INFO, 1) << "parse one line will spent " << sys::formatTime((sys::getCurrentTime() - start));
  return KeyValueMessagePair(key, value);
}
} // namespace op 
} // namespace rdd 
} // namespace idgs 
