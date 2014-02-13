
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "idgs/actor/actor_descriptor.h"

namespace idgs {
namespace actor {

ActorDescriptorWrapper::ActorDescriptorWrapper(std::shared_ptr<idgs::pb::ActorDescriptor>& descriptor) :
    name(descriptor->name()), description(descriptor->description()), type(type = descriptor->type()) {
  // set in operations
  for (int i = 0, size = descriptor->in_operations_size(); i < size; ++i) {
    std::shared_ptr<idgs::pb::ActorOperationDescripor> op(
        new idgs::pb::ActorOperationDescripor(descriptor->in_operations(i)));
    ActorOperationDescriporWrapper op_wrapper(op);
    setInOperation(op_wrapper.getName(), op);
  }
  // set out operations
  for (int i = 0, size = descriptor->out_operations_size(); i < size; ++i) {
    std::shared_ptr<idgs::pb::ActorOperationDescripor> op(
        new idgs::pb::ActorOperationDescripor(descriptor->out_operations(i)));
    ActorOperationDescriporWrapper op_wrapper(op);
    setOutOperation(op_wrapper.getName(), op_wrapper);
  }
  // set consume actors
  for (int i = 0, size = descriptor->consume_actor_name_size(); i < size; ++i) {
    addConsumeActor(descriptor->consume_actor_name(i));
  }
}

std::shared_ptr<idgs::pb::ActorDescriptor> ActorDescriptorWrapper::toActorDescriptor() {
  std::shared_ptr<idgs::pb::ActorDescriptor> desc(new idgs::pb::ActorDescriptor);
  desc->set_name(name);
  desc->set_description(description);
  desc->set_type(type);
  // set in operations
  for (auto it = inOperations.begin(); it != inOperations.end(); ++it) {
    desc->add_in_operations()->CopyFrom(*(it->second.getDescriptor()));
  }
  // set out operations
  for (auto it = outOperations.begin(); it != outOperations.end(); ++it) {
    desc->add_out_operations()->CopyFrom(*(it->second.getDescriptor()));
  }
  // set consume actors
  for (auto it = consume_actor_name.begin(); it != consume_actor_name.end(); ++it) {
    desc->add_consume_actor_name(*it);
  }
  return desc;
}

/// get operation descriptor.
/// @return the operation descriptor, NUL means not found.
const ActorOperationDescriporWrapper* ActorDescriptorWrapper::getInOperation(const std::string& name) const {
  auto it = inOperations.find(name);
  if (it == inOperations.end()) {
    return NULL;
  }
  return &(it->second);
}

const ActorOperationDescriporWrapper* ActorDescriptorWrapper::getResolvedInOperation(const std::string& name) const {
  auto it = resolvedInOperations.find(name);
  if (it == resolvedInOperations.end()) {
    it = inOperations.find(name);
    if (it == inOperations.end()) {
      return NULL;
    }
  }
  return &(it->second);
}


void ActorDescriptorWrapper::setInOperation(const std::string& name, const ActorOperationDescriporWrapper& operation) {
  inOperations.insert(std::pair<std::string, ActorOperationDescriporWrapper>(name, operation));
}

const ActorOperationDescriporWrapper* ActorDescriptorWrapper::getOutOperation(const std::string& name) const {
  auto it = outOperations.find(name);
  if (it == outOperations.end()) {
    return NULL;
  }
  return &(it->second);
}

void ActorDescriptorWrapper::setOutOperation(const std::string& name, const ActorOperationDescriporWrapper& operation) {
  outOperations.insert(std::pair<std::string, ActorOperationDescriporWrapper>(name, operation));
}

void ActorDescriptorWrapper::addConsumeActor(const std::string& actor_name) {
  consume_actor_name.insert(actor_name);

}

std::string ActorDescriptorWrapper::toString() const {
  std::stringstream ss;
  ss << "address: " << this << " ";
  ss << "name: " << getName() << std::endl;
  ss << "In: ";
  for (const auto& p : inOperations) {
    ss << p.second.getName() << ", ";
  }
  ss << std::endl;

  ss << "Out: ";
  for (const auto& p : outOperations) {
    ss << p.second.getName() << ", ";
  }
  ss << std::endl;

  ss << "Consume: ";
  for (const auto& p : consume_actor_name) {
    ss << p << ", ";
  }
  ss << std::endl;

  ss << "Resolved: ";
  for (const auto& p : resolvedInOperations) {
    ss << p.second.getName() << ", ";
  }
  ss << std::endl;

  return ss.str();
}


ModuleDescriptorWrapper::ModuleDescriptorWrapper(std::shared_ptr<idgs::pb::ModuleDescriptor>& module_descriptor) :
    name(module_descriptor->name()), description(module_descriptor->description()) {
  for (const idgs::pb::ActorDescriptor actor : module_descriptor->actors()) {
    std::shared_ptr<idgs::pb::ActorDescriptor> actorPtr(new idgs::pb::ActorDescriptor(actor));
    ActorDescriptorPtr actor_descriptor(new ActorDescriptorWrapper(actorPtr));
    addActorDescriptor(actor_descriptor);
  }
}

void ModuleDescriptorWrapper::addActorDescriptor(const ActorDescriptorPtr& actor_desc) {
  actorDescriptors.insert(std::pair<std::string, ActorDescriptorPtr>(actor_desc->getName(), actor_desc));
}

void ModuleDescriptorWrapper::removeActorDescriptor(const std::string& actor_desc_name) {
  actorDescriptors.erase(actor_desc_name);
}

const std::map<std::string, ActorDescriptorPtr>& ModuleDescriptorWrapper::getActorDescriptors() const {
  return actorDescriptors;
}

const ActorDescriptorPtr& ModuleDescriptorWrapper::getActorDescriptor(const std::string& actor_desc_name) const {
  if (actorDescriptors.find(actor_desc_name) == actorDescriptors.end()) {
    static ActorDescriptorPtr descriptor(NULL);
    return descriptor;
  }
  return actorDescriptors.at(actor_desc_name);
}

std::shared_ptr<idgs::pb::ModuleDescriptor> ModuleDescriptorWrapper::toModuleDescriptor() const {
  std::shared_ptr<idgs::pb::ModuleDescriptor> module_descriptor(new idgs::pb::ModuleDescriptor);
  for (auto it = actorDescriptors.begin(); it != actorDescriptors.end(); ++it) {
    module_descriptor->set_name(name);
    module_descriptor->set_description(description);
    module_descriptor->add_actors()->CopyFrom(*it->second->toActorDescriptor().get());
  }
  return module_descriptor;
}
} // namespace actor
} // namespace idgs
