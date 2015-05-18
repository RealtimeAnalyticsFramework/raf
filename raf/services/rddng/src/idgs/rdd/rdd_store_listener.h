/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#pragma once


#include "idgs/rdd/rdd_local.h"

namespace idgs {
namespace rdd {

class RddStoreListener : public idgs::store::StoreListener {
public:
  RddStoreListener();
  virtual ~RddStoreListener();

  virtual const std::string& getName() const override {
    static std::string name = RDD_STORE_LISTENER;
    return name;
  }

  /// the description of current listener
  virtual const std::string& getDescription() const override {
    static std::string description = "Receive store insert and append current data to RDD calculate.";
    return description;
  }

  virtual StoreListener* clone() const override {
    return new RddStoreListener();
  }

  virtual idgs::ResultCode init(std::map<std::string, std::string> props) override {
    return idgs::RC_SUCCESS;
  }

  /// @brief  send message to next listener manager
  /// @param  context context of listener
  /// @return result code of listener
  virtual idgs::store::ListenerResultCode insert(idgs::store::ListenerContext* context) override;

  void setRddLocal(const std::shared_ptr<RddLocal>& rddlocal);

  const std::shared_ptr<RddLocal>& getRddLocal() const;

private:
  std::shared_ptr<RddLocal> rddLocal;
};

} /* namespace rdd */
} /* namespace idgs */
