
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "type_composer.h"

#include "idgs/util/utillity.h"

namespace protobuf {
void DynamicMessage::addField(DynamicField& field) {
  field.number = fields.size() + 1;
  fields.push_back(field);
}

DynamicTypeComposer::DynamicTypeComposer() {
  // TODO Auto-generated constructor stub

}

DynamicTypeComposer::~DynamicTypeComposer() {
  // TODO Auto-generated destructor stub
}

void DynamicTypeComposer::addMessage(const DynamicMessage& msg) {
  messages[msg.name] = msg;
}

void DynamicTypeComposer::saveFile(const std::string& fileName) {
  std::string content = toString();
  idgs::sys::saveFile(fileName, content);
}

} // namespace protobuf 
