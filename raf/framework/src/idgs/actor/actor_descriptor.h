
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once
#include <set>
#include "idgs/actor/actor_message.h"

namespace idgs {
namespace actor {
typedef std::function<void(ActorMessagePtr msg)> ActorOperation;


///
/// Descriptor of actor operation. <BR>
///
class ActorOperationDescriporWrapper {
public:
  ActorOperationDescriporWrapper() :
      descriptor(new idgs::pb::ActorOperationDescripor()) {
  }
  ActorOperationDescriporWrapper(std::shared_ptr<idgs::pb::ActorOperationDescripor>& desc) :
      descriptor(desc){
  }

  ActorOperationDescriporWrapper(const ActorOperationDescriporWrapper&) = default;
  ActorOperationDescriporWrapper(ActorOperationDescriporWrapper&&) = default;
  ActorOperationDescriporWrapper& operator =(const ActorOperationDescriporWrapper&) = default;
  ActorOperationDescriporWrapper& operator =(ActorOperationDescriporWrapper&&) = default;

  const std::string& getName() const {
    return descriptor->name();
  }

  void setName(const std::string& name) {
    descriptor->set_name(name);
  }


  const std::string& getDescription() const {
    return descriptor->description();
  }

  void setDescription(const std::string& description) {
    descriptor->set_description(description);
  }

  const std::string& getPayloadType() const {
    return descriptor->payload_type();
  }

  void setPayloadType(const std::string& type) {
    descriptor->set_payload_type(type);
  }


  std::shared_ptr<idgs::pb::ActorOperationDescripor> getDescriptor() const {
    return descriptor;
  }
private:
  std::shared_ptr<idgs::pb::ActorOperationDescripor> descriptor;
};

///
///
///
class ActorDescriptorWrapper {
public:
  ActorDescriptorWrapper() :
      name("Unknown"), description("Unknown"), type(idgs::pb::ActorType::AT_STATELESS) {
  }

  ActorDescriptorWrapper(const ActorDescriptorWrapper&) = delete;
  ActorDescriptorWrapper(ActorDescriptorWrapper&&) = delete;
  ActorDescriptorWrapper& operator =(const ActorDescriptorWrapper&) = delete;
  ActorDescriptorWrapper& operator =(ActorDescriptorWrapper&&) = delete;


  ActorDescriptorWrapper(std::shared_ptr<idgs::pb::ActorDescriptor>& descriptor);
  std::shared_ptr<idgs::pb::ActorDescriptor> toActorDescriptor();

  const std::string& getName() const {
    return name;
  }

  void setName(const std::string& name) {
    this->name = name;
  }


  idgs::pb::ActorType getType() const {
    return type;
  }

  void setType(idgs::pb::ActorType t) {
    type = t;
  }


  const std::string& getDescription() const {
    return description;
  }

  void setDescription(const std::string& description) {
    this->description = description;
  }


  /// get operation descriptor.
  /// @return the operation descriptor, NULL means not found.
  const ActorOperationDescriporWrapper* getInOperation(const std::string& name) const;

  const std::map<std::string, ActorOperationDescriporWrapper>& getInOperations() const {
    return inOperations;
  }
  void setInOperation(const std::string& name, const ActorOperationDescriporWrapper& operation);

  const ActorOperationDescriporWrapper* getOutOperation(const std::string& name) const;
  const std::map<std::string, ActorOperationDescriporWrapper>& getOutOperations() const {
    return outOperations;
  }
  void setOutOperation(const std::string& name, const ActorOperationDescriporWrapper& operation);

  void addConsumeActor(const std::string& actor_name);

  const std::set<std::string>& getConsumeActors() const {
    return consume_actor_name; // consume actors
  }

  /// get operation descriptor.
  /// @return the operation descriptor, NULL means not found.
  const ActorOperationDescriporWrapper* getResolvedInOperation(const std::string& name) const;

  std::map<std::string, ActorOperationDescriporWrapper>& getResolvedInOperations () {
    return resolvedInOperations;
  }

  std::string toString() const;

private:
  std::string name;
  std::string description;
  idgs::pb::ActorType type;
  std::map<std::string, ActorOperationDescriporWrapper> inOperations;
  std::map<std::string, ActorOperationDescriporWrapper> outOperations;

  std::map<std::string, ActorOperationDescriporWrapper> resolvedInOperations;

  std::set<std::string> consume_actor_name; // consume which actors
};

typedef std::shared_ptr<ActorDescriptorWrapper> ActorDescriptorPtr;


///
/// module descriptor
///
class ModuleDescriptorWrapper {

public:

  ModuleDescriptorWrapper() :
      name("Unknown"), description("Unknown") {

  }

  ModuleDescriptorWrapper(std::shared_ptr<idgs::pb::ModuleDescriptor>& module_descriptor);

  void setName(const std::string& module_name) {
    name = module_name;
  }
  void setDescription(const std::string& desc) {
    description = desc;
  }

  const std::string& getName() const {
    return name;
  }

  /// Add a actor descriptor
  void addActorDescriptor(const ActorDescriptorPtr& actor_desc);

  /// Remove some actor descriptor by actor descriptor name
  void removeActorDescriptor(const std::string& actor_desc_name);

  /// Return module's all actor descriptor
  const std::map<std::string, ActorDescriptorPtr>& getActorDescriptors() const;

  /// Return module's actor descriptor by actor_descriptor_name
  const ActorDescriptorPtr& getActorDescriptor(const std::string& actor_descriptor_name) const;

  /// To proto buffer format
  std::shared_ptr<idgs::pb::ModuleDescriptor> toModuleDescriptor() const;

private:
  std::map<std::string, idgs::actor::ActorDescriptorPtr> actorDescriptors;
  std::string name;
  std::string description;
};

typedef std::shared_ptr<ModuleDescriptorWrapper> ModuleDescriptorPtr;
} // namespace actor
} // namespace idgs

