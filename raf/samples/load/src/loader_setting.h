/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#pragma once

namespace idgs {
namespace client {
struct LoaderSettings {
  LoaderSettings() :
      standalone(0), data_path("."), client_cfg_file("conf/client.conf"), mapper_cfg_file("conf/tpch_file_mapper.conf"), thread_count(
          1), output_file("tpch-tcptps.txt") {

  }
  /// true, standalone; false, incluster
  int standalone;
  /// data file path
  std::string data_path;
  /// client config file
  std::string client_cfg_file;
  /// mapper config between store and load file
  std::string mapper_cfg_file;
  int thread_count; /// use can define thread count
  /// output file
  std::string output_file;
};
}
}

