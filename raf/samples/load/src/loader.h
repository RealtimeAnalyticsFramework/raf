/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#pragma once

#include <atomic>
#include <fstream>
#include <mutex>

#include "idgs/store/pb/store_service.pb.h"
#include "idgs/store/store_config_wrapper.h"
#include "loader_setting.h"
#include "idgs/store/parsed_store_descriptor.h"

namespace idgs {
namespace client {

#define LOG_PER_RECORDS 50000

class Loader {
public:
  Loader() :
      startTime(0), lastTime(0), records(0), settings(NULL), file_index(0), mapper_config(
          new idgs::store::pb::StoreFileMapperConfig), isParseLine(true) {
  }

  virtual ~Loader() {
  }
  ;

  virtual idgs::ResultCode init(LoaderSettings* settings);

  virtual void config();

  /// import data
  virtual void import() = 0;

  /// parse line into protobuf message
  idgs::store::KeyValueMessagePair parseLine(const std::string& store_name, const std::string& line,
      idgs::ResultCode* rc);

protected:
  /// read line into string
  int readline(std::string& line);

  /// begin time
  unsigned long startTime;

  /// end time
  unsigned long lastTime;

  /// total handled lines
  std::atomic_ulong records;

  /// load mapper config
  idgs::ResultCode loadMapperConfig();

  /// load settings
  LoaderSettings* settings;

  /// lock read line
  std::mutex lock;

  /// load all_load_files
  std::vector<std::string> all_load_files;

  /// file index of all load files
  size_t file_index;

  /// current file stream
  std::ifstream curr_file_stream;

  /// store file mapper cfg
  idgs::store::pb::StoreFileMapperConfig* mapper_config;

  /// key: file_name, value: store_name
  std::map<std::string, std::string> file_store_map;

  /// key: store_name, value: struct store descriptor
  idgs::store::StoreDescriptorMap store_descriptor_cache;

  /// whether parse line
  bool isParseLine;

};
}
}
