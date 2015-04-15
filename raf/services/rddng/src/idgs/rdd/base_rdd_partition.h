
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/actor/stateful_actor.h"

#include "idgs/rdd/transform/transformer_context.h"

namespace idgs {
namespace rdd {

class RddLocal;

typedef std::function<void(const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value)> RddEntryFunc;
typedef std::function<void(const idgs::actor::PbMessagePtr& key, const std::vector<idgs::actor::PbMessagePtr>& value)> RddGroupEntryFunc;

/// The interface of RDD partition.
class BaseRddPartition: public idgs::actor::StatefulActor {
public:

  /// @brief Constructor
  BaseRddPartition();

  /// @brief Destructor
  virtual ~BaseRddPartition();

  void transform();
  void transform(const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value);

  /// @brief  Get the value of data by key.
  /// @param  key Key of data.
  /// @return The value of data.
  virtual idgs::actor::PbMessagePtr get(const idgs::actor::PbMessagePtr& key) const = 0;

  /// @brief  Get the value of data by key.
  /// @param  key Key of data.
  /// @return The value of data.
  virtual std::vector<idgs::actor::PbMessagePtr> getValue(const idgs::actor::PbMessagePtr& key) const = 0;

  /// @brief  Whether key is in RDD data.
  /// @param  key Key of data.
  /// @return True or false.
  virtual bool containKey(const idgs::actor::PbMessagePtr& key) const = 0;

  /// @brief  Loop the data of RDD.
  /// @param  fn A function to loop data.
  virtual void foreach(RddEntryFunc fn) const = 0;

  /// @brief  Loop the data of RDD.
  /// @param  fn A function to loop data.
  virtual void foreachGroup(RddGroupEntryFunc fn) const = 0;

  /// @brief  Whether data of RDD is empty.
  /// @return True or false.
  virtual bool empty() const = 0;

  /// @brief  Get the key data size of RDD.
  /// @return The data size of RDD.
  virtual size_t keySize() const = 0;

  /// @brief  Get the value data size of RDD.
  /// @return The data size of RDD.
  virtual size_t valueSize() const = 0;

  uint32_t getPartition() const;

  const std::string& getRddName() const;
  const std::string& getPartitionName() const;

  void setRddLocal(RddLocal* rddlocal);

  RddLocal* getRddLocal();

  const idgs::actor::PbMessagePtr& getKeyTemplate() const;

  const idgs::actor::PbMessagePtr& getValueTemplate() const;

  void addTransformerContext(const transform::TransformerContext& ctx);

  void setAggrTransformerContext(transform::TransformerContext& ctx);

protected:
  // partition id
  uint32_t partition;

  // name of RDD
  std::string rddName;

  /// @todo remove
  // name of partition, "RDD NAME_OF_RDD[PARTITION_ID];
  std::string partitionName;

  // information of RDD
  RddLocal* rddLocal;

  // context from downstream RDD, include message and operator
  std::vector<transform::TransformerContext> transformerContexts;

  // context from downstream RDD, include message and operator of first upstream
  transform::TransformerContext aggrCtx;

private:

};
} // namespace rdd
} // namespace idgs 
