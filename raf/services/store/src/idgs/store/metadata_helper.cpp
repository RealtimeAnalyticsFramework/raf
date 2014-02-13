
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "metadata_helper.h"
#include "idgs/store/data_store.h"
#include "protobuf/message_helper.h"

using namespace idgs::util;
using namespace protobuf;
using namespace google::protobuf;

namespace idgs {
namespace store {

ResultCode MetadataHelper::messageToMetadata(const Message* message, pb::Metadata* metadata) {
  if (message == NULL) {
    LOG(ERROR) << "invalid message.";
    return RC_INVALID_MESSAGE;
  }

  if (metadata == NULL) {
    LOG(ERROR) << "invalid metadata.";
    return RC_INVALID_MESSAGE;
  }

  auto descriptor = message->GetDescriptor();
  metadata->set_type_name(descriptor->full_name());
  for (int32_t i = 0; i < descriptor->field_count(); ++ i) {
    auto field = descriptor->field(i);
    auto fld = metadata->add_field();
    fld->set_label(static_cast<pb::FieldLabel>(field->label()));
    fld->set_type(static_cast<idgs::pb::DataType>(field->type()));
    fld->set_name(field->name());
    fld->set_number(field->number());
    if (field->type() == FieldDescriptor::TYPE_MESSAGE) {
      fld->set_type_name(field->message_type()->full_name());
    }
  }

  return RC_SUCCESS;
}

ResultCode MetadataHelper::loadStoreMetadata(const string& storeName, pb::MetadataPair* metadata) {
  DataStore& store = singleton<DataStore>::getInstance();
  shared_ptr<StoreConfigWrapper> config(new StoreConfigWrapper);
  ResultCode code = store.loadStoreConfig(storeName, config);
  if (code != RC_SUCCESS) {
    return code;
  }

  MessageHelper& helper = singleton<MessageHelper>::getInstance();
  auto key = helper.createMessage(config->getStoreConfig().key_type());
  code = messageToMetadata(key.get(), metadata->mutable_key());
  if (code != RC_SUCCESS) {
    return code;
  }

  auto value = helper.createMessage(config->getStoreConfig().value_type());
  code = messageToMetadata(value.get(), metadata->mutable_value());
  if (code != RC_SUCCESS) {
    return code;
  }

  return code;
}

ResultCode MetadataHelper::registerByMetadata(const pb::Metadata* metadata) {
  return RC_SUCCESS;
}

} // namespace store
} // namespace idgs
