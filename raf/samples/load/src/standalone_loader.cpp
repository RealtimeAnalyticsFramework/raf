/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#include "standalone_loader.h"
#include "idgs/util/utillity.h"
#include "idgs/store/datastore_const.h"
#include <thread>

using namespace idgs::store::pb;
using namespace idgs::pb;
using namespace idgs::store;
using namespace idgs::pb;
using namespace google::protobuf;

namespace idgs {

namespace client {

StandaloneLoader::StandaloneLoader() :
    single_insert_time_out(10) {
  char* str_env_value = getenv("SINGLE_INSERT_TIME_OUT");
  if (str_env_value) {
    single_insert_time_out = atoi(str_env_value);
  }
  LOG(INFO)<< std::string(typeid(*this).name());
}

StandaloneLoader::~StandaloneLoader() {

}

ClientActorMessagePtr StandaloneLoader::genInsertClientActorMsg(const std::string& store_name, const std::string& line,
    idgs::ResultCode *rc, uint32_t option) {
  KeyValueMessagePair key_value = parseLine(store_name, line, rc);
  if (*rc != RC_SUCCESS) {
    LOG(ERROR)<< "parseLine error, error code: " << *rc << ", error message: " << getErrorDescription(*rc);
    return ClientActorMessagePtr(NULL);
  }
  ClientActorMessagePtr actorMsg = std::make_shared<ClientActorMessage>();
  std::shared_ptr<InsertRequest> pay_load = std::make_shared<InsertRequest>();
  actorMsg->setOperationName(OP_INSERT);
  pay_load->set_store_name(store_name);
  pay_load->set_options(option);
  actorMsg->setSourceActorId(CLIENT_ACTOR_ID);
  actorMsg->setSourceMemberId(CLIENT_MEMBER);
  actorMsg->setDestActorId(ACTORID_STORE_SERVCIE);
  actorMsg->setDestMemberId(ANY_MEMBER);
  actorMsg->setPayload(pay_load);
  actorMsg->setAttachment(STORE_ATTACH_KEY, key_value.first);
  actorMsg->setAttachment(STORE_ATTACH_VALUE, key_value.second);
  return actorMsg;
}

bool StandaloneLoader::loadTask() {
  std::string line;
  int rrc;
  int tmp_file_index = -1;
  {
    // cppcheck-suppress unreadVariable
    std::lock_guard < std::mutex > lockGuard(lock);
    rrc = readline(line);
    tmp_file_index = file_index; /// new file index
  }
  if (rrc > 1) {
    return false;
  } else if (rrc == 1) {
    return true;
  }
  if (line.empty()) {
    LOG(WARNING)<< "read an empty line, ignored";
    return true;
  }
  std::string file_name;
  try {
    file_name = all_load_files.at(tmp_file_index);
  } catch (std::exception& e) {
    LOG(ERROR)<< e.what() << "file_index: " << file_index;
  }
  const std::string& store_name = file_store_map.at(file_name);
  ResultCode rc;
  ClientActorMessagePtr clientActorMsg = genInsertClientActorMsg(store_name, line, &rc);
  if (rc != RC_SUCCESS) {
    LOG(ERROR)<< "genInsertClientActorMsg error, caused by " << getErrorDescription(rc) << "\n line: " << line;
    return true;
  }
  sendRecvMessage(clientActorMsg, &rc);
  if (rc == RC_SUCCESS) {
    ulong t = records.fetch_add(1);
    if ((t % LOG_PER_RECORDS) == 0 && t > 0) {
      unsigned long now = idgs::sys::getCurrentTime();
      LOG(INFO)<< "total line inserted = "<< t << " tps = " << ((double)LOG_PER_RECORDS * 1000 / (now - lastTime));
      lastTime = now;
    }
  }
  return true;
}

void StandaloneLoader::import() {
  lastTime = startTime = idgs::sys::getCurrentTime();
  vector<thread> thread_pool;
  int THREAD_COUNT = settings->thread_count;
  thread_pool.reserve(THREAD_COUNT);
  for (int i = 0; i < THREAD_COUNT; i++) {
    thread_pool.push_back(thread([this]() {
      while(this->loadTask()) {
        ;
      }
      DVLOG(2) << "load thread exit.";
    }));
  }
  for (auto it = thread_pool.begin(); it != thread_pool.end(); ++it) {
    it->join();
  }
  thread_pool.clear();
  unsigned long end = idgs::sys::getCurrentTime();
  std::ofstream ofs(settings->output_file);
  ofs << "YVALUE" << "=" << ((double) records.load() * 1000 / (end - startTime)) << std::endl;
  ofs.close();
  LOG(INFO)<< "Total records = " << records.load() << ", TPS = " << ((double) records.load() * 1000 / (end - startTime));
}}
}
