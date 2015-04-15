
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/rdd/action/action.h"
#include "idgs/rdd/db/base_connection.h"
#include "idgs/rdd/pb/rdd_action.pb.h"

namespace idgs {
namespace rdd {
namespace action {

/// Action of RDD.
/// To handle action which would export data with the given type.
class ExportAction: public Action {
public:

  /// @brief Constructor
  ExportAction();

  /// @brief Destructor
  virtual ~ExportAction();

  ExportAction(const ExportAction& other) = delete;
  ExportAction(ExportAction&& other) = delete;
  ExportAction& operator()(const ExportAction& other) = delete;
  ExportAction& operator()(ExportAction&& other) = delete;

  /// @brief  Handle action and save result.
  /// @param  ctx     Context of RDD action.
  /// @param  input   Input RDD for each partition.
  /// @return Status code of result.
  virtual pb::RddResultCode action(ActionContext* ctx, const idgs::rdd::BaseRddPartition* input) override;

  /// @brief  Aggregate all action result of each partition, and calculate total action result.
  /// @param  ctx     Context of RDD action.
  /// @return Status code of result.
  virtual pb::RddResultCode aggregate(ActionContext* ctx) override;

  db::BaseConnectionPtr getConnection(const std::string& name);

private:
  idgs::util::resource_manager<db::BaseConnectionPtr> conns;

};

} // namespace action
} // namespace rdd
} // namespace idgs 
