/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#pragma once
#if defined(_GNUC_) || defined(__clang__)
#include "idgs_gch.h"
#endif // GNUC_ $
#include "tcp_loader.h"

namespace idgs {
namespace client {

class BatchLineLoader: public TcpLoader {
public:
  BatchLineLoader();

  ~BatchLineLoader();

  void import();

  /// generator batch line client actor message
  ClientActorMessagePtr genBatchLineClientActorMsg(const std::string& store_name, const std::vector<std::string>& lines,
      uint32_t option = 0);

private:

  bool loadTask(std::vector<std::string>& lines);

  void sendBatchLines(std::vector<std::string>& lines, size_t curr_file_index, int rrc = 0);

  void sendStoreFileMapperCfg(idgs::ResultCode* rc);

  void displayResponse(const ClientActorMessagePtr& clientActorMsg);

  bool isStarted;

  int batch_insert_time_out;

  int batch_size;

  std::atomic_ulong read_lines;
  std::atomic_ulong write_lines;
};
}
}

