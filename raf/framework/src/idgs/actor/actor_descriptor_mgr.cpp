
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(__GNUC__) || defined(__clang__) $
#include "idgs/actor/actor_descriptor_mgr.h"

namespace idgs {

namespace actor {

ActorDescriptorMgr::ActorDescriptorMgr() {

}

ActorDescriptorMgr::~ActorDescriptorMgr() {
  function_footprint();
}

bool ActorDescriptorMgr::registerModuleDescriptor(const std::string& module_name,
    const idgs::actor::ModuleDescriptorPtr& module_descriptor) {
  LOG(INFO) << "registerModuleDescriptor " << module_name;
  const std::map<std::string, idgs::actor::ActorDescriptorPtr>& map = module_descriptor->getActorDescriptors();
  for (auto it = map.begin(); it != map.end(); ++it) {
    auto rc = registerActorDescriptor(it->first, it->second, module_descriptor.get());
    if (!rc) {
      LOG(ERROR) << "Failed to register module: " << module_name;
      return false;
    }
  }
  moduleDescriptors.insert(std::pair<std::string, idgs::actor::ModuleDescriptorPtr>(module_name, module_descriptor));
  return true;
}

void ActorDescriptorMgr::unRegisterModuleDescriptor(const std::string& module_name) {
  function_footprint();

  auto itr = moduleDescriptors.find(module_name);
  if (itr == moduleDescriptors.end()) {
    return;
  }
  const std::map<std::string, idgs::actor::ActorDescriptorPtr>& map = itr->second->getActorDescriptors();
  for (auto it = map.begin(); it != map.end(); ++it) {
    removeActorDescriptor(it->first);
  }
  moduleDescriptors.erase(module_name);
}

const std::map<std::string, idgs::actor::ActorDescriptorPtr>& ActorDescriptorMgr::getModuleActorDescriptors(
    const std::string& module_name) const {
  auto it = moduleDescriptors.find(module_name);
  if (it == moduleDescriptors.end()) {
    static std::map<std::string, idgs::actor::ActorDescriptorPtr> null_map;
    return null_map;
  }
  return it->second->getActorDescriptors();
}

idgs::ResultCode ActorDescriptorMgr::loadModuleActorDescriptor(const std::string& mod_desc_file) {
  std::shared_ptr<idgs::pb::ModuleDescriptor> proto_module_descriptor(new idgs::pb::ModuleDescriptor);
  idgs::ResultCode rc = protobuf::JsonMessage().parseJsonFromFile(proto_module_descriptor.get(), mod_desc_file);
  if (rc != RC_SUCCESS) {
    return rc;
  }
  size_t begin = mod_desc_file.find_last_of('/') + 1;
  size_t end = mod_desc_file.find('.');
  const std::string& mod_name = mod_desc_file.substr(begin, end - begin);
  DVLOG(2) << "loading module " << mod_name << " 's descriptor(s)";
  proto_module_descriptor->set_name(mod_name);
  idgs::actor::ModuleDescriptorPtr mod_descriptor(new idgs::actor::ModuleDescriptorWrapper(proto_module_descriptor));
  moduleDescriptors.insert(std::pair<std::string, idgs::actor::ModuleDescriptorPtr>(mod_name, mod_descriptor));
  return RC_SUCCESS;
}

bool ActorDescriptorMgr::registerActorDescriptor(const std::string& name, const idgs::actor::ActorDescriptorPtr& descriptor, ModuleDescriptorWrapper* module) {
  std::map<std::string, ActorOperationDescriporWrapper>& resolved = descriptor->getResolvedInOperations ();

  const std::map<std::string, ActorOperationDescriporWrapper>& in = descriptor->getInOperations();
  resolved.insert(in.begin(), in.end());

  const std::set<std::string>& consumes = descriptor->getConsumeActors();
  ActorDescriptorPtr actor;
  for(const std::string& n : consumes) {
    if (n == name) {
      // depends on itself
      actor = descriptor;
    } else {
      actor = getActorDescriptor(n);
      if(module && ! actor) {
        actor = module->getActorDescriptor(n);
      }
      if (!actor) {
        LOG(ERROR) << "Depended actor descriptor: " << n << " not found";
        return false;
      }
    }
    const std::map<std::string, ActorOperationDescriporWrapper>& out = actor->getOutOperations();
    resolved.insert(out.begin(), out.end());
  }


  DVLOG(4) << "register actor descriptor: " << descriptor->toString();

  actorDescriptors.insert(std::pair<std::string, idgs::actor::ActorDescriptorPtr>(name, descriptor));
  return true;
}

void ActorDescriptorMgr::removeActorDescriptor(const std::string& name) {
  actorDescriptors.erase(name);
}

/// get actor descriptor, NULL for not found.
const idgs::actor::ActorDescriptorPtr& ActorDescriptorMgr::getActorDescriptor(const std::string& name) {
  static idgs::actor::ActorDescriptorPtr nullDesc(NULL);
  auto it = actorDescriptors.find(name);
  if (it != actorDescriptors.end()) {
    return (it->second);
  }
  return nullDesc;
}
}
}
