
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include <gtest/gtest.h>
#include "idgs/result_code.h"

using namespace std;
using namespace idgs;

TEST(result_code, toString) {
  string msg = dumpErrorDescription();
  cerr << "###############################" << endl;
  cerr << msg << endl;
  cerr << "###############################" << endl;

}

TEST(result_code, check) {
  string empty("");
  for (int i = 0; i < RC_MAX; ++i) {
    ASSERT_NE(getErrorDescription((ResultCode)i), empty) << "No description for result code: " << ((ResultCode)i);
  }
}

TEST(result_code, getMessage) {
  string msg;
  msg = getErrorDescription(RC_SUCCESS);
  cerr << RC_SUCCESS << ":" << msg << endl;

  msg = getErrorDescription(RC_ERROR);
  cerr << RC_ERROR << ":" << msg << endl;

  msg = getErrorDescription(RC_TIMEOUT);
  cerr << RC_TIMEOUT << ":" << msg << endl;

  msg = getErrorDescription(RC_ACTOR_NOT_FOUND);
  cerr << RC_ACTOR_NOT_FOUND << ":" << msg << endl;
}





