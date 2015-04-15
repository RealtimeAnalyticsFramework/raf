
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "gtest/gtest.h"
#include <fstream>
#include "idgs/util/utillity.h"

using namespace std;

double getTpchRawResult(const string& filename) {
  ifstream fs(filename);
  int32_t len = 1024;
  double sum = 0;
  while (!fs.eof()) {
    char line[len];
    fs.getline(line, len - 1);
    string cmd(line);
    if (cmd.size() == 0) {
      continue;
    }

    vector<string> strs;
    idgs::str::split(cmd, "|", strs);

    double price, discount, quantity;
    idgs::sys::convert<double>(strs[4], quantity);
    idgs::sys::convert<double>(strs[5], price);
    idgs::sys::convert<double>(strs[6], discount);

    if (quantity >= 24) {
      continue;
    }

    if (discount < 0.05 || discount > 0.07) {
      continue;
    }

    if (strs[10] < "1994-01-01" || strs[10] >= "1995-01-01") {
      continue;
    }

    sum = sum + (price * discount);
  }

  fs.close();
  return sum;
}

TEST(tpch_Q6, raw_data_result) {
  string filename;
  char* tpch_home = getenv("TPCH_HOME");
  if (tpch_home) {
    string tpchHome(tpch_home);
    filename = tpchHome + "/dbgen/lineitem.tbl";
  } else {
    filename = "/tmp/tpch_it/dbgen/lineitem.tbl";
  }

  double sum = getTpchRawResult(filename);

  LOG(INFO) << "=========== TPCH Q6 RAW DATA RESULT ===========";
  LOG(INFO) << "select sum(l_extendedprice * l_discount)";
  LOG(INFO) << "  from lineitem";
  LOG(INFO) << " where l_shipdate >= '1994-01-01'";
  LOG(INFO) << "   and l_shipdate < '1995-01-01'";
  LOG(INFO) << "   and l_discount between 0.05 and 0.07";
  LOG(INFO) << "   and l_quantity < 24";
  LOG(INFO) << "";
  LOG(INFO) << "sql result of raw data is : " << std::fixed << sum;
  LOG(INFO) << "===============================================";
}
