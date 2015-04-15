
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "metadata_helper.h"

#include "idgs/store/store_module.h"

using namespace google::protobuf;

namespace idgs {
namespace store {

ResultCode MetadataHelper::messageToMetadata(const Message* message, FileDescriptorProto* metadata) {
  if (message == NULL) {
    LOG(ERROR) << "invalid message.";
    return RC_INVALID_MESSAGE;
  }

  if (metadata == NULL) {
    LOG(ERROR) << "invalid metadata.";
    return RC_INVALID_MESSAGE;
  }

  auto descriptor = message->GetDescriptor();
  auto& name = descriptor->full_name();
  auto pos = name.find_last_of(".");
  auto pgk = (pos == string::npos) ? "" : name.substr(0, pos);

  metadata->set_name(name + ".proto");
  metadata->set_package(pgk);
  descriptor->CopyTo(metadata->add_message_type());

  return RC_SUCCESS;
}

ResultCode MetadataHelper::loadStoreMetadata(const string& storeName, FileDescriptorProto* keyMetadata, FileDescriptorProto* valueMetadata) {
  auto store = idgs_store_module()->getDataStore()->getStore(storeName);
  if (!store) {
    return RC_STORE_NOT_FOUND;
  }

  auto& storeConfigWrapper = store->getStoreConfigWrapper();
  auto key = storeConfigWrapper->newKey();
  auto code = messageToMetadata(key.get(), keyMetadata);
  if (code != RC_SUCCESS) {
    return code;
  }

  auto value = storeConfigWrapper->newValue();
  code = messageToMetadata(value.get(), valueMetadata);
  if (code != RC_SUCCESS) {
    return code;
  }

  return code;
}

} // namespace store
} // namespace idgs
