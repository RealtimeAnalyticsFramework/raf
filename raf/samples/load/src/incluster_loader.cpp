/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "incluster_loader.h"
#include "idgs/util/utillity.h"
#include "idgs/application.h"
#include "idgs/store/datastore_const.h"
#include "idgs/signal_handler.h"

using namespace idgs::store;
using namespace idgs::store::pb;
using namespace idgs::actor;
using namespace idgs::pb;

namespace idgs {
namespace client {

const std::string InClusterLoader::ACOTR_ID = "tpch.loader";

InClusterLoader::InClusterLoader() :
    cancelTimer(NULL) {
  setActorId(ACOTR_ID);
  activeRequest.store(0);
  doneRecords.store(0);
}

InClusterLoader::~InClusterLoader() {
}

const idgs::actor::ActorMessageHandlerMap& InClusterLoader::getMessageHandlerMap() const {
  static std::map<std::string, idgs::actor::ActorMessageHandler> handlerMap = { { DATA_STORE_INSERT_RESPONSE,
      static_cast<idgs::actor::ActorMessageHandler>(&InClusterLoader::handleInsertResponse) }, };
  return handlerMap;
}

idgs::ResultCode InClusterLoader::startMember() {
  Application& app = ::idgs::util::singleton<Application>::getInstance();

  DVLOG(1) << "Loading configuration.";
  CHECK_RC(app.init(settings->client_cfg_file));

  DVLOG(1) << "Server is starting.";
  CHECK_RC(app.start());

  return idgs::RC_OK;
}

idgs::ResultCode InClusterLoader::stopMember() {
  // unregister itself
  ::idgs::util::singleton<idgs::actor::RpcFramework>::getInstance().getActorFramework()->unRegisterStatelessActor(
      ACOTR_ID);

  // unregister actor descriptor
  ::idgs::util::singleton<idgs::actor::ActorDescriptorMgr>::getInstance().removeActorDescriptor(descriptor->getName());

  DVLOG(1) << "Server is shutting down.";
  Application& app = ::idgs::util::singleton<Application>::getInstance();
  CHECK_RC(app.stop());

  return idgs::RC_OK;
}

idgs::ResultCode InClusterLoader::runMember() {
  SignalHandler sh;
  sh.setup();

  Application& app = ::idgs::util::singleton<Application>::getInstance();
  // dead loop
  while (app.isRunning()) {
    sleep(1);
  }
  return RC_OK;
}

idgs::ResultCode InClusterLoader::init(LoaderSettings* settings) {
  Loader::init(settings); /// super init

  // register actor descriptor
  this->descriptor = generateActorDescriptor();
  ::idgs::util::singleton<idgs::actor::ActorDescriptorMgr>::getInstance().registerActorDescriptor(descriptor->getName(),
      descriptor);

  // register itself
  ::idgs::util::singleton<idgs::actor::RpcFramework>::getInstance().getActorFramework()->Register(ACOTR_ID, this);

  LOG(INFO)<< "Start member.";
  CHECK_RC(startMember());

  LOG(INFO)<< "Sleep 3 seconds.";
  std::chrono::seconds dura(3);
  std::this_thread::sleep_for(dura);
  return idgs::RC_OK;
}

void InClusterLoader::import() {

  LOG(INFO)<< "Begin to send request.";
  lastTime = startTime = idgs::sys::getCurrentTime();
  sendRequest();

  LOG(INFO) << "Main loop.";
  runMember();

  LOG(INFO) << "Stop member.";
  stopMember();
}

::idgs::actor::ActorDescriptorPtr InClusterLoader::generateActorDescriptor() {
  static std::shared_ptr<ActorDescriptorWrapper> descriptor;
  if (descriptor)
    return descriptor;
  descriptor.reset(new ::ActorDescriptorWrapper);

  descriptor->setName(ACOTR_ID);
  descriptor->setDescription("TPCH data loader");
  descriptor->setType(::idgs::pb::AT_STATELESS);

  ::idgs::actor::ActorOperationDescriporWrapper insertName;
  insertName.setName(idgs::store::DATA_STORE_INSERT_RESPONSE);
  insertName.setDescription("Insert response.");
  insertName.setPayloadType("idgs.store.pb.InsertResponse");
  descriptor->setInOperation(insertName.getName(), insertName);

  return descriptor;
}

void InClusterLoader::handleInsertResponse(const idgs::actor::ActorMessagePtr& msg) {
  DVLOG(5) << "Get response: " << msg->toString();
  idgs::store::pb::InsertResponse* resp = dynamic_cast<idgs::store::pb::InsertResponse*>(msg->getPayload().get());
  if (resp->result_code() != idgs::store::pb::SRC_SUCCESS) {
    LOG(ERROR)<< "Insert data error, caused by code " << resp->result_code();
  }
  {
    // cppcheck-suppress unreadVariable
    std::lock_guard < std::mutex > lockGuard(lock);
    doneRecords.fetch_add(1);
    activeRequest.fetch_sub(1);
  }
  sendRequest();
}

ActorMessagePtr InClusterLoader::genInsertActorMsg(const std::string& store_name, const std::string& line,
    idgs::ResultCode* rc, uint32_t option) {
  KeyValueMessagePair key_value = parseLine(store_name, line, rc);
  if (*rc != RC_SUCCESS) {
    LOG(ERROR)<< "parseLine error, error code: " << *rc << ", error message: " << getErrorDescription(*rc);
    return ActorMessagePtr(NULL);
  }
  ActorMessagePtr actorMsg = createActorMessage();
  std::shared_ptr < InsertRequest > payload(new InsertRequest());
  actorMsg->setOperationName(OP_INSERT);
  payload->set_store_name(store_name);
  payload->set_options(option);
  actorMsg->setDestActorId(ACTORID_STORE_SERVCIE);
  actorMsg->setDestMemberId(ANY_MEMBER);
  actorMsg->setPayload(payload);
  actorMsg->setAttachment(STORE_ATTACH_KEY, key_value.first);
  actorMsg->setAttachment(STORE_ATTACH_VALUE, key_value.second);

  return actorMsg;
}

idgs::ResultCode InClusterLoader::sendRequest() {
  int active;
  int ACTIVE_REQUESTS = settings->thread_count;
  while ((active = activeRequest.load()) < ACTIVE_REQUESTS) {
    std::string line;
    int rc;
    int tmp_file_index = -1;
    {
      // cppcheck-suppress unreadVariable
      std::lock_guard < std::mutex > lockGuard(lock);
      rc = readline(line);
      tmp_file_index = file_index; /// new file index
    }
    switch (rc) {
    case 0:
      break;
    case 1:
      continue;
    case 2:
      exit(1);
      break;
    case 3: {
      // cppcheck-suppress unreadVariable
      std::lock_guard < std::mutex > lockGuard(lock);
      DVLOG(2) << "Total responses: " << doneRecords.load() << ", records left: " << active;
      if (active <= 0) {
        if (cancelTimer) {
          delete cancelTimer;
          cancelTimer = NULL;
        }
        // complete
        LOG(INFO)<< "All files are done: " << file_index;
        unsigned long end = idgs::sys::getCurrentTime();
        std::ofstream ofs(settings->output_file);
        ofs << "YVALUE" << "=" << ((double) records.load() * 1000 / (end - startTime)) << std::endl;
        ofs.close();
        LOG(INFO)<< "Total records = " << records.load() << ", TPS = " << ((double)records.load() * 1000 / (end - startTime));
        Application& app = ::idgs::util::singleton<Application>::getInstance();
        app.shutdown();
      } else {
        if (!cancelTimer) {
          cancelTimer = new idgs::cancelable_timer(15, []() {
            std::cerr << "Timeout: program exit.";
            Application& app = ::idgs::util::singleton<Application>::getInstance();
            app.shutdown();
          });
        }
      }
      return idgs::RC_OK;
//          break;
    }
    default:
      LOG(ERROR)<< "unknown err.";
    }
    if (line.empty()) {
      LOG(WARNING)<< "read an empty line, ignored";
      continue;
    }
    idgs::ResultCode result;
    std::string file_name;
    try {
      file_name = all_load_files.at(tmp_file_index);
    } catch (std::exception& e) {
      LOG(ERROR)<< e.what() << "file_index: " << tmp_file_index;
    }
    const std::string& store_name = file_store_map.at(file_name);
    idgs::actor::ActorMessagePtr actorMsg = genInsertActorMsg(store_name, line, &result);
    if (result != RC_OK) {
      LOG(ERROR)<< "genInsertActorMsg error, caused by " << getErrorDescription(result) << "\n line: " << line;
      continue;
    }
    rc = idgs::actor::sendMessage(actorMsg);
    {
      // cppcheck-suppress unreadVariable
      std::lock_guard < std::mutex > lockGuard(lock);
      if (rc == RC_OK) {
        // active request
        activeRequest.fetch_add(1);
      } else {
        LOG(ERROR)<< "Failed to send message: " << actorMsg->getPayload()->DebugString();
      }
        // total records
      ulong t = records.fetch_add(1);
      if ((t % LOG_PER_RECORDS) == 0 && t > 0) {
        unsigned long now = idgs::sys::getCurrentTime();
        LOG(INFO)<< "total line inserted = "<< t << " tps = " << ((double)LOG_PER_RECORDS * 1000 / (now - lastTime));
        lastTime = now;
      }
    }
  }
  return idgs::RC_OK;
}
}
}

