/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(__GNUC__) || defined(__clang__) $
#include "batch_line_loader.h"
#include "idgs/util/utillity.h"
#include <thread>
#include "idgs/tpc/pb/tpc_crud.pb.h"

using namespace std;
using namespace idgs::store::pb;
using namespace idgs::pb;
using namespace idgs::pb;
using namespace idgs::tpc::pb;

namespace idgs {
namespace client {
static const std::string& OP_CRUD_MAPPER = "linecrud_mapper_operation";
static const std::string& OP_CRUD_REQUEST = "linecrud_request_operation";
static const std::string& LINECRUD_ACTOR_ID = "linecrud_actor";

void BatchLineLoader::displayResponse(const ClientActorMessagePtr& clientActorMsg) {
  RawlineCrudResponse response;
  clientActorMsg->parsePayload(&response);
  /// every 1 thousand error print once
  LOG_IF_EVERY_N(INFO, response.total_error_resp_count() > 0, 1000)
  << "total lines: " << response.total_line_count() << ", total response: " << response.total_resp_count()
      << ", delta: " << (response.total_line_count() - response.total_resp_count()) << "total error: "
      << response.total_error_resp_count();
}

BatchLineLoader::BatchLineLoader() :
    isStarted(false), batch_insert_time_out(10), batch_size(50) {
  isParseLine = false; /// client no need to parse
  char* str_env_value = getenv("BATCH_INSERT_TIME_OUT");
  if (str_env_value) {
    batch_insert_time_out = atoi(str_env_value);
  }
  str_env_value = getenv("BATCH_INSERT_LINES");
  if (str_env_value) {
    batch_size = atoi(str_env_value);
  }
  LOG(INFO)<< std::string(typeid(*this).name());
}

BatchLineLoader::~BatchLineLoader() {

}

ClientActorMessagePtr BatchLineLoader::genBatchLineClientActorMsg(const std::string& store_name,
    const std::vector<std::string>& lines, uint32_t option) {
  ClientActorMessagePtr actorMsg = std::make_shared<ClientActorMessage>();
  std::shared_ptr<RawlineCrudRequest> pay_load = std::make_shared<RawlineCrudRequest>();
  actorMsg->setOperationName(OP_CRUD_REQUEST);
  pay_load->set_store_name(store_name);
  pay_load->set_type(CrudType::INSERT);
  for (std::string line : lines) {
    pay_load->add_lines(line);
  }
  pay_load->set_option(option);
  actorMsg->setSourceActorId("client_actor_id");
  actorMsg->setSourceMemberId(CLIENT_MEMBER);
  actorMsg->setDestActorId(LINECRUD_ACTOR_ID);
  actorMsg->setDestMemberId(ANY_MEMBER);
  actorMsg->setPayload(pay_load);
  return actorMsg;
}

void BatchLineLoader::sendBatchLines(std::vector<std::string>& lines, size_t file_index, int rrc) {
  size_t size = lines.size();
  write_lines.fetch_add(size);
  std::string file_name;
  try {
    file_name = all_load_files.at(file_index); /// the last file
  } catch (std::exception& e) {
    LOG(ERROR)<< e.what() << "file_index: " << file_index << ", rrc: " << rrc;
  }
  const std::string& store_name = file_store_map.at(file_name);
  ClientActorMessagePtr clientActorMsg = genBatchLineClientActorMsg(store_name, lines);
  lines.clear();
  ResultCode rc;
  ClientActorMessagePtr response = sendRecvMessage(clientActorMsg, &rc, batch_insert_time_out);
  if (rc == RC_SUCCESS) {
    displayResponse(response);
    ulong t = records.fetch_add(size);
    if ((t % LOG_PER_RECORDS) == 0 && t > 0) {
      unsigned long now = idgs::sys::getCurrentTime();
      RawlineCrudResponse response;
      clientActorMsg->parsePayload(&response);
      LOG(INFO)<< "total lines: "<< t << " tps: " << ((double)LOG_PER_RECORDS * 1000 / (now - lastTime)) << ", delta: " << (response.total_line_count() - response.total_resp_count());
      lastTime = now;
    }
  }
}

bool BatchLineLoader::loadTask(std::vector<std::string>& lines) {
  int total_file_size = all_load_files.size();
  int rrc = -1;
  size_t size = 0;
  size_t tmp_file_index_backup, tmp_file_index;
  std::string line;
  {
    // cppcheck-suppress unreadVariable
    std::lock_guard < std::mutex > lockGuard(lock);
    do {
      tmp_file_index_backup = file_index;
      rrc = readline(line);
      tmp_file_index = file_index;
      if (rrc > 1) { /// open file error, or no more file to read
        return false;
      } else if (rrc == 1) {
        break;
      }
      if (line.empty()) {
        // LOG(WARNING)<< "read an empty line, ignored";
        continue;
      }
      read_lines.fetch_add(1);
      if ((size = lines.size()) < batch_size) {
        lines.push_back(line);
      }
    } while ((size = lines.size()) < batch_size);
  } /// release lock
  if (rrc == 1) {
    if ((size = lines.size()) > 0) { /// send accumulated lines if exists
      LOG(INFO)<< "Accumulated " << size << " lines will be sent";
      if(tmp_file_index < total_file_size) {
        LOG(INFO) << "File changed, " << all_load_files.at(tmp_file_index_backup) << " ====> " << all_load_files.at(tmp_file_index);
      }
      sendBatchLines(lines, tmp_file_index_backup, rrc);
    }
  } else {
    /// up to batch size
    sendBatchLines(lines, tmp_file_index);
  }
  return true;
}

void BatchLineLoader::sendStoreFileMapperCfg(idgs::ResultCode* rc) {
  ClientActorMessagePtr actorMsg = std::make_shared<ClientActorMessage>();
  std::shared_ptr<StoreFileMapperConfig> pay_load = std::make_shared<StoreFileMapperConfig>(* mapper_config);
  actorMsg->setOperationName(OP_CRUD_MAPPER);
  actorMsg->setSourceActorId("client_actor_id");
  actorMsg->setSourceMemberId(CLIENT_MEMBER);
  actorMsg->setDestActorId(LINECRUD_ACTOR_ID);
  actorMsg->setDestMemberId(ANY_MEMBER);
  actorMsg->setPayload(pay_load);
  sendMessage(actorMsg, rc);
}

void BatchLineLoader::import() {
  lastTime = startTime = idgs::sys::getCurrentTime();
  idgs::ResultCode rc;
  /// send store file mapper
  sendStoreFileMapperCfg(&rc);
  if (rc != RC_OK) {
    LOG(ERROR)<< "send store file mapper error, cause by " << getErrorDescription(rc);
    return;
  }
  uint32_t sleep_time = 2;
  LOG(INFO)<< "Batch insert lines: " << batch_size << ", sleep " << sleep_time << "s, waiting server handled store file mapper config";
  sleep(sleep_time);

  vector<thread> threadPool;
  int THREAD_COUNT = settings->thread_count;
  threadPool.reserve(THREAD_COUNT);
  for (int i = 0; i < THREAD_COUNT; i++) {
    threadPool.push_back(thread([this]() {
      vector<string> lines;
      lines.reserve(batch_size);
      while(this->loadTask(lines)) {
        ;
      }
    }));
  }
  for (auto it = threadPool.begin(); it != threadPool.end(); ++it) {
    it->join();
  }
  threadPool.clear();
  unsigned long end = idgs::sys::getCurrentTime();
  std::ofstream ofs(settings->output_file);
  ofs << "YVALUE" << "=" << ((double) records.load() * 1000 / (end - startTime)) << std::endl;
  ofs.close();
  LOG(INFO)<< "Total response records = " << records.load() << ", TPS = " << ((double)records.load() * 1000 / (end - startTime));
  LOG(INFO)<< "Total read line(s): " << read_lines << ", total write line(s): " << write_lines;
}}
}
