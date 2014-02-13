
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

double getTpchRawResult(const string& orderFile, const string& dateFile) {
  map<string, string> date;
  int32_t len = 1024;
  ifstream fsDate(dateFile);
  while (!fsDate.eof()) {
    char line[len];
    fsDate.getline(line, len - 1);
    string cmd(line);
    if (cmd.size() == 0) {
      continue;
    }

    vector<string> strs;
    idgs::str::split(cmd, "|", strs);

    if (strs[4] == "1992") {
      date[strs[0]] = strs[0];
    }
  }

  fsDate.close();

  ifstream fsOrder(orderFile);
  double sum = 0;
  while (!fsOrder.eof()) {
    char line[len];
    fsOrder.getline(line, len - 1);
    string cmd(line);
    if (cmd.size() == 0) {
      continue;
    }

    vector<string> strs;
    idgs::str::split(cmd, "|", strs);

    string orderdate = strs[5];
    double price, discount, quantity;
    idgs::sys::convert<double>(strs[8], quantity);
    idgs::sys::convert<double>(strs[9], price);
    idgs::sys::convert<double>(strs[11], discount);

    if (quantity >= 25) {
      continue;
    }

    if (discount < 1 || discount > 3) {
      continue;
    }

    if (date.find(orderdate) == date.end()) {
      continue;
    }

    sum = sum + (price * discount);
  }

  fsOrder.close();
  return sum;
}

TEST(ssb_Q1_1, raw_data_result) {
  string orderFile, dateFile;
  char* ssb_home = getenv("SSB_HOME");
  if (ssb_home) {
    string ssbHome(ssb_home);
    orderFile = ssbHome + "/ssb-dbgen-master/lineorder.tbl";
    dateFile = ssbHome + "/ssb-dbgen-master/date.tbl";
  } else {
    orderFile = "/tmp/ssb_it/ssb-dbgen-master/lineorder.tbl";
    dateFile = "/tmp/ssb_it/ssb-dbgen-master/date.tbl";
  }

  double sum = getTpchRawResult(orderFile, dateFile);

  LOG(INFO) << "=========== SSB Q1.1 RAW DATA RESULT ===========";
  LOG(INFO) << "select sum(lo_extendedprice * lo_discount)";
  LOG(INFO) << "  from lineorder, date";
  LOG(INFO) << " where lo_orderdate = d_datekey";
  LOG(INFO) << "   and d_year < '1992'";
  LOG(INFO) << "   and lo_discount between 1 and 3";
  LOG(INFO) << "   and l_quantity < 25";
  LOG(INFO) << "";
  LOG(INFO) << "sql result of raw data is : " << std::fixed << sum;
  LOG(INFO) << "===============================================";
}
