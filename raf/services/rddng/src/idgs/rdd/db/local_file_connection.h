
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/rdd/db/base_connection.h"

namespace idgs {
namespace rdd {
namespace db {

class LocalFileConnection: public BaseConnection {
public:
  LocalFileConnection();
  virtual ~LocalFileConnection();

  void setSplitter(const std::string& splitter);

  virtual const std::string& name();

  virtual BaseConnection* clone() const override;

  virtual void init(const std::map<std::string, std::string>& params) override;

  virtual idgs::ResultCode connect() override;

  virtual idgs::ResultCode disconnect() override;

  virtual idgs::ResultCode insert(const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) override;

  virtual idgs::ResultCode get(const idgs::actor::PbMessagePtr& key, idgs::actor::PbMessagePtr& value) override;

  virtual idgs::ResultCode commit() override;

  virtual idgs::ResultCode rollback() override;

private:
  std::string fileFullName;
  std::string content;
  std::string splitter;
  std::string partition;
};

} // db
} // rdd
} // idgs
