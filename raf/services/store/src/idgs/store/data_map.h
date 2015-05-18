
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <unordered_map>

#include <tbb/spin_rw_mutex.h>
// #include <tbb/scalable_allocator.h>
#include <btree_map.h> // google btree_map

#include "idgs/store/comparer.h"
#include "idgs/store/pb/store_service.pb.h"

namespace idgs {
namespace store {

typedef std::function<
    void(const StoreKey<google::protobuf::Message>& key, StoreValue<google::protobuf::Message>& value)> StoreEntryFunc;


/// Data access interface. <br>
/// Access data with ordered/unordered map. <br>
class StoreMap {
public:

  class Iterator {
  public:
    virtual ~Iterator() {}
    virtual const StoreKey<google::protobuf::Message>& key() = 0;
    virtual const StoreValue<google::protobuf::Message>& value() = 0;
    virtual void next() = 0;
    virtual bool hasNext() = 0;
  };

  virtual ~StoreMap() {};

  /// @brief  Set data to data map.
  /// @param  key The key of data to store.
  /// @param  value The value of data to store.
  /// @return Status code of result.
  virtual ResultCode set(const StoreKey<google::protobuf::Message>& key,
      StoreValue<google::protobuf::Message>& value) = 0;

  /// @brief  Get data by key from data map.
  /// @param  key The key of data.
  /// @param  value Return value of data from data map.
  /// @return Status code of result.
  virtual ResultCode get(const StoreKey<google::protobuf::Message>& key,
      StoreValue<google::protobuf::Message>& value) = 0;

  /// @brief  Remove data by key from data map.
  /// @param  key The key of data.
  /// @param  value The removed data for return.
  /// @return Status code of result.
  virtual ResultCode remove(const StoreKey<google::protobuf::Message>& key,
      StoreValue<google::protobuf::Message>& value) = 0;

  /// @brief  Get the size of data in data map.
  /// @return The size of data.
  virtual size_t size() = 0;

  /// @brief  Clear all data.
  /// @return Status code of result.
  virtual ResultCode clear() = 0;

  virtual void foreach(StoreEntryFunc fn) = 0;

  virtual std::shared_ptr<StoreMap> snapshot() = 0;

  virtual std::shared_ptr<Iterator> iterator() = 0;

protected:
  tbb::spin_rw_mutex mutex;

};
// DataMap

template <typename M>
class BasicDataMap : public StoreMap {
private:
  typedef decltype(M().begin()) MI;
  class BasicIterator : public Iterator {
  public:
    BasicIterator(MI& begin, MI& end) {
      it = begin;
      itend = end;
    }

    virtual ~BasicIterator() {
    }

    virtual const StoreKey<google::protobuf::Message>& key() override {
      return it->first;
    }

    virtual const StoreValue<google::protobuf::Message>& value() override {
      return it->second;
    }

    virtual void next() override {
      if (hasNext()) {
        ++ it;
      }
    }

    virtual bool hasNext() override {
      return it != itend;
    }

  private:
    MI it;
    MI itend;
  };

public:
  ~BasicDataMap() {
    clear();
  }

  ResultCode set(const StoreKey<google::protobuf::Message>& key, StoreValue<google::protobuf::Message>& value) {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
    if (map.find(key) == map.end()) {
      map[key] = value;
    } else {
      DVLOG_FIRST_N(5, 20) << "insert duplicate data will be covered, key: " << key->DebugString();
      map[key].swap(value);
    }

    return RC_SUCCESS;
  }

  ResultCode get(const StoreKey<google::protobuf::Message>& key, StoreValue<google::protobuf::Message>& value) {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
    if (map.find(key) == map.end()) {
      return RC_DATA_NOT_FOUND;
    }

    value = map[key];

    return RC_SUCCESS;
  }

  ResultCode remove(const StoreKey<google::protobuf::Message>& key, StoreValue<google::protobuf::Message>& value) {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
    if (map.find(key) == map.end()) {
      return RC_DATA_NOT_FOUND;
    }

    value = map[key];
    map.erase(key);
    return RC_SUCCESS;
  }

  size_t size() {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
    return map.size();
  }

  ResultCode clear() {
    DVLOG(5) << "Clear store map";
    tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
    map.clear();
    return RC_SUCCESS;
  }

  void foreach(StoreEntryFunc fn) {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
    auto it = map.begin();
    auto end = map.end();
    for (; it != end; ++it) {
      fn(it->first, it->second);
    }
  }

  virtual std::shared_ptr<StoreMap> snapshot() override {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
    std::shared_ptr<BasicDataMap<M>> result = std::make_shared<BasicDataMap<M> >();
    result->map = this->map;
    return result;
  }


  virtual std::shared_ptr<Iterator> iterator() {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
    auto begin = map.begin();
    auto end = map.end();
    return std::make_shared<BasicIterator>(begin, end);
  }

private:
  M map;
};

//typedef std::map<StoreKey<google::protobuf::Message>, StoreValue<google::protobuf::Message>, idgs::store::less,
//    tbb::scalable_allocator<std::pair<const StoreKey<google::protobuf::Message>, StoreValue<google::protobuf::Message> > > > TREEMAP;
//
//typedef std::unordered_map<StoreKey<google::protobuf::Message>, StoreValue<google::protobuf::Message>,
//    idgs::store::hash_code, idgs::store::equals_to,
//    tbb::scalable_allocator<std::pair<const StoreKey<google::protobuf::Message>, StoreValue<google::protobuf::Message> > > > HASHMAP;
//
//typedef btree::btree_map<StoreKey<google::protobuf::Message>, StoreValue<google::protobuf::Message>, idgs::store::less,
//    tbb::scalable_allocator<std::pair<const StoreKey<google::protobuf::Message>, StoreValue<google::protobuf::Message> > > > BTREEMAP;

typedef std::map<StoreKey<google::protobuf::Message>, StoreValue<google::protobuf::Message>, idgs::store::less > TREEMAP;

typedef std::unordered_map<StoreKey<google::protobuf::Message>, StoreValue<google::protobuf::Message>,
    idgs::store::hash_code, idgs::store::equals_to> HASHMAP;

typedef btree::btree_map<StoreKey<google::protobuf::Message>, StoreValue<google::protobuf::Message>, idgs::store::less> BTREEMAP;

// typedef BasicDataMap<TREEMAP> TreeMap;
typedef BasicDataMap<BTREEMAP> TreeMap;
typedef BasicDataMap<HASHMAP> HashMap;


}// namespace store
} // namespace idgs

