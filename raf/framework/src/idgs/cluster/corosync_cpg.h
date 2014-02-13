
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once
#include <corosync/cpg.h>

#include "idgs/idgslogging.h"

namespace idgs {
namespace cluster {

struct CorosyncMemberId {
  CorosyncMemberId(uint64_t n = 0) : nodeId(n >> 32), pid(n & 0xffffffff) {
  }
  CorosyncMemberId(uint32_t node, uint32_t pid_) : nodeId(node), pid(pid_) {
  }

  CorosyncMemberId(const cpg_address& caddr) :  nodeId(caddr.nodeid), pid(caddr.pid) {
  }

  CorosyncMemberId(const std::string&); // Decode from string.

  uint32_t getNode() const {
    return nodeId;
  }
  uint32_t getPid() const {
    return pid;
  }
  operator uint64_t() const {
    return (uint64_t(nodeId) << 32ull) + pid;
  }

  // MemberId as byte string, network byte order. Not human readable.
  std::string str() const;

  // human readable.
  std::string toString() const;
private:
  uint32_t nodeId;  /// usually IPv4 address
  uint32_t pid;     /// process id
};

class CorosyncCpg {
public:

  struct CpgName: public cpg_name {
    CpgName() {
      length = 0;
    }
    CpgName(const char* s) {
      copy(s, strlen(s));
    }
    CpgName(const char* s, size_t n) {
      copy(s, n);
    }
    CpgName(const std::string& s) {
      copy(s.data(), s.size());
    }
    void copy(const char* s, size_t n) {
      assert(n < CPG_MAX_NAME_LENGTH);
      memcpy(value, s, n);
      length = n;
    }

    std::string str() const {
      return std::string(value, length);
    }
  };

  static std::string str(const cpg_name& n) {
    return std::string(n.value, n.length);
  }

  class CorosyncCpgHandler {
  public:
    virtual ~CorosyncCpgHandler() {
    }

    virtual void deliver(cpg_handle_t handle, const struct cpg_name *group, uint32_t nodeid, uint32_t pid, void* msg,
        int msg_len) = 0;

    virtual void configChange(cpg_handle_t handle, const struct cpg_name * group, const struct cpg_address * members,
        int nMembers, const struct cpg_address * left, int nLeft, const struct cpg_address * joined, int nJoined) = 0;
  };

  CorosyncCpg(CorosyncCpgHandler&);

  /**
   *  Initialize cpg service
   */
  cs_error_t init();

  ~CorosyncCpg();

  void shutdown();

  cs_error_t dispatchOne();
  cs_error_t dispatchAll();
  cs_error_t dispatchBlocking();
  cs_error_t dispatchOneNonBlocking();

  cs_error_t join(const std::string& group);
  cs_error_t leave();

  cs_error_t multicast(const iovec* iov, int iovLen);

  cpg_handle_t getHandle() const {
    return handle;
  }

  CpgName getGroup() const {
    return group;
  }

  CorosyncMemberId self() const;

  int getFd();

  static std::string errorStr(cs_error_t err, const std::string& msg);

private:

  static const unsigned int cpgRetries = 5;

  static const unsigned int maxCpgRetrySleep = 100000;

  static CorosyncCpg* cpgFromHandle(cpg_handle_t);

  static void globalDeliver(cpg_handle_t handle, const struct cpg_name *group, uint32_t nodeid, uint32_t pid, void* msg,
      size_t msg_len);

  static void globalConfigChange(cpg_handle_t handle, const struct cpg_name *group, const struct cpg_address *members,
      size_t nMembers, const struct cpg_address *left, size_t nLeft, const struct cpg_address *joined, size_t nJoined);

private:
  cpg_handle_t handle;
  CorosyncCpgHandler& handler;
  bool ready;
  CpgName group;
};

inline bool operator==(const cpg_name& a, const cpg_name& b) {
  return a.length == b.length && strncmp(a.value, b.value, a.length) == 0;
}
inline bool operator!=(const cpg_name& a, const cpg_name& b) {
  return !(a == b);
}

} // namespace cluster
} // namespace idg

