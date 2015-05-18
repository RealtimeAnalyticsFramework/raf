
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/actor/actor_descriptor.h"
namespace idgs {

namespace actor {
class ActorDescriptorMgr {
public:
  ActorDescriptorMgr();
  ActorDescriptorMgr(const ActorDescriptorMgr& mgr) = delete;
  ActorDescriptorMgr(const ActorDescriptorMgr&& mgr) = delete;
  ~ActorDescriptorMgr();

  /// Register module descriptor
  bool registerModuleDescriptor(const std::string& module_name, const idgs::actor::ModuleDescriptorPtr& desc);

  /// UnRegister module descriptor
  void unRegisterModuleDescriptor(const std::string& module_name);


  /// Load module's actor descriptors in file
  idgs::ResultCode loadModuleActorDescriptor(const std::string& mod_descriptor_file);

  void clear() {
    actorDescriptors.clear();
    moduleDescriptors.clear();
  }
  /// get actor descriptor, NULL for not found.
  const idgs::actor::ActorDescriptorPtr& getActorDescriptor(const std::string& name);

  bool registerActorDescriptor(const std::string& name, const idgs::actor::ActorDescriptorPtr& desc, ModuleDescriptorWrapper* module = NULL);

  const std::map<std::string, idgs::actor::ModuleDescriptorPtr>& getModuleDescriptors() const {
    return moduleDescriptors;
  }

  void removeActorDescriptor(const std::string& name);

private:

  const std::map<std::string, idgs::actor::ActorDescriptorPtr>& getActorDescriptors() {
    return actorDescriptors;
  }

  /// get one module's all actor descriptors by module name
  const std::map<std::string, idgs::actor::ActorDescriptorPtr>& getModuleActorDescriptors(
      const std::string& module_name) const;


private:
  std::map<std::string, idgs::actor::ActorDescriptorPtr> actorDescriptors; // actor descriptor
  std::map<std::string, idgs::actor::ModuleDescriptorPtr> moduleDescriptors; // module descriptor
}; // class ActorDescriptorMgr
} // namespace idgs
} // namespace actor
