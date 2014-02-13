
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "corosync_cpg.h"


#include <thread>

namespace idgs {
namespace cluster {

using namespace std;

CorosyncCpg* CorosyncCpg::cpgFromHandle(cpg_handle_t handle) {
  void* cpg = 0;
  if (CS_OK != cpg_context_get(handle, &cpg)) {
    LOG(ERROR)<< "Cannot get CPG instance";
  }
  if (!cpg) {
    LOG(ERROR)<< "Cannot get CPG instance";
  }
  return reinterpret_cast<CorosyncCpg*>(cpg);
}

// Global callback functions.
void CorosyncCpg::globalDeliver(cpg_handle_t handle, const struct cpg_name *group, uint32_t nodeid, uint32_t pid,
    void* msg, size_t msg_len) {
  CorosyncCpg* cpg = cpgFromHandle(handle);
  if (cpg) {
    cpg->handler.deliver(handle, group, nodeid, pid, msg, msg_len);
  } else {
    LOG(INFO)<< __FUNCTION__ << "cpg closed";
  }
}

void CorosyncCpg::globalConfigChange(cpg_handle_t handle, const struct cpg_name *group,
    const struct cpg_address *members, size_t nMembers, const struct cpg_address *left, size_t nLeft,
    const struct cpg_address *joined, size_t nJoined) {
  CorosyncCpg* cpg = cpgFromHandle(handle);
  if (cpg) {
    cpg->handler.configChange(handle, group, members, nMembers, left, nLeft, joined, nJoined);
  } else {
    LOG(INFO)<< __FUNCTION__ << "cpg closed";
  }
}

/// @return >0 for FD, -1 means error.
int CorosyncCpg::getFd() {
  int fd = -1;
  cs_error_t cs;
  if (ready && handle != 0 && CS_OK != (cs = cpg_fd_get(handle, &fd))) {
    LOG(ERROR)<< errorStr(cs, "Cannot get CPG file descriptor");
    return -1;
  }
  return fd;
}

cs_error_t CorosyncCpg::init() {
  if (ready) {
    return CS_OK;
  }
  cpg_callbacks_t callbacks;
  ::memset(&callbacks, 0, sizeof(callbacks));
  callbacks.cpg_deliver_fn = &globalDeliver;
  callbacks.cpg_confchg_fn = &globalConfigChange;

  //    QPID_LOG(notice, "Initializing CPG");
  cs_error_t err = cpg_initialize(&handle, &callbacks);
  int retries = 6; // @todo: make this configurable.
  while (err == CS_ERR_TRY_AGAIN && --retries) {
    LOG(WARNING)<< "cpg initialize retry " << retries;
    std::chrono::microseconds dura(5);
    std::this_thread::sleep_for(dura);
    err = cpg_initialize(&handle, &callbacks);
  }
  if (err != CS_OK) {
    LOG(ERROR)<< "Failed to initialize CPG, Please type terminal command 'ps -ef|grep corosync' check if dependency CPG service is started";
    return err;
  }
  if (CS_OK != cpg_context_set(handle, this)) {
    LOG(ERROR)<< "Cannot set CPG context";
    return err;
  }

  ready = true;
  return CS_OK;
}

CorosyncCpg::CorosyncCpg(CorosyncCpgHandler& h) :
    handler(h), ready(false), handle(0) {

}

CorosyncCpg::~CorosyncCpg() {
  if (ready) {
    shutdown();
  }
}

cs_error_t CorosyncCpg::join(const std::string& name) {
  group = name;

  cs_error_t result;
  unsigned int snooze = 10;
  for (unsigned int nth_try = 0; nth_try < cpgRetries; ++nth_try) {
    if (CS_OK == (result = cpg_join(handle, &group))) {
      break;
    } else if (result == CS_ERR_TRY_AGAIN) {
      LOG(WARNING)<< "Retrying join";
      std::chrono::microseconds dura(snooze);
      std::this_thread::sleep_for(dura);
      snooze *= 10;
      snooze = (snooze <= maxCpgRetrySleep) ? snooze : maxCpgRetrySleep;
    }
    else break;  // Don't retry unless CPG tells us to.
  }
  return result;
}

cs_error_t CorosyncCpg::leave() {
  cs_error_t result;
  unsigned int snooze = 10;
  for (unsigned int nth_try = 0; nth_try < cpgRetries; ++nth_try) {
    if (CS_OK == (result = cpg_leave(handle, &group))) {
      break;
    } else if (result == CS_ERR_TRY_AGAIN) {
      LOG(WARNING)<< "Retrying leave";
      std::chrono::microseconds dura(snooze);
      std::this_thread::sleep_for(dura);
      snooze *= 10;
      snooze = (snooze <= maxCpgRetrySleep) ? snooze : maxCpgRetrySleep;
    }
    else break;  // Don't retry unless CPG tells us to.
  }
  return result;

}

void CorosyncCpg::shutdown() {
  if (!ready) {
    return;
  }
  ready = false;
  LOG(INFO)<< "shutdown corosync.";
  if (handle) {
    cs_error_t result;
    unsigned int snooze = 10;
    for (unsigned int nth_try = 0; nth_try < cpgRetries; ++nth_try) {
      if (CS_OK == (result = cpg_finalize(handle))) {
        break;
      } else if (result == CS_ERR_TRY_AGAIN) {
        LOG(WARNING)<< "Retrying finalize";
        std::chrono::microseconds dura(snooze);
        std::this_thread::sleep_for(dura);
        snooze *= 10;
        snooze = (snooze <= maxCpgRetrySleep) ? snooze : maxCpgRetrySleep;
      }
      else break;  // Don't retry unless CPG tells us to.
    }

    if (result != CS_OK) {
      LOG(ERROR)<< "cpg_finalize error, error code = " << result;
    }
  }
  handle = 0;
}

cs_error_t CorosyncCpg::multicast(const iovec* iov, int iovLen) {
  // Check for flow control
  cpg_flow_control_state_t flowState;
  cs_error_t result;
  if (CS_OK != (result = cpg_flow_control_state_get(handle, &flowState))) {
    LOG(ERROR)<< "Cannot get CPG flow control status.";
  }
  if (flowState == CPG_FLOW_CONTROL_ENABLED) {
    return result;
  }
  do {
    result = cpg_mcast_joined(handle, CPG_TYPE_AGREED, const_cast<iovec*>(iov), iovLen);
    if (result != CS_ERR_TRY_AGAIN) {
      if (CS_OK != result) {
        LOG(ERROR)<< "Can't multicast messsage to group: " << group.str();
      }
    }
  } while(result == CS_ERR_TRY_AGAIN);
  return result;
}


cs_error_t CorosyncCpg::dispatchOne() {
  return cpg_dispatch(handle, CS_DISPATCH_ONE);
}

cs_error_t CorosyncCpg::dispatchAll() {
  return cpg_dispatch(handle, CS_DISPATCH_ALL);
}

cs_error_t CorosyncCpg::dispatchBlocking() {
  return cpg_dispatch(handle, CS_DISPATCH_BLOCKING);
}

cs_error_t CorosyncCpg::dispatchOneNonBlocking() {
  return cpg_dispatch(handle, CS_DISPATCH_ONE_NONBLOCKING);
}

string CorosyncCpg::errorStr(cs_error_t err, const std::string& msg) {
  std::ostringstream os;
  os << msg << ": ";
  switch (err) {
  case CS_OK:
    os << "ok";
    break;
  case CS_ERR_LIBRARY:
    os << "library";
    break;
  case CS_ERR_TIMEOUT:
    os << "timeout";
    break;
  case CS_ERR_TRY_AGAIN:
    os << "try again";
    break;
  case CS_ERR_INVALID_PARAM:
    os << "invalid param";
    break;
  case CS_ERR_NO_MEMORY:
    os << "no memory";
    break;
  case CS_ERR_BAD_HANDLE:
    os << "bad handle";
    break;
  case CS_ERR_ACCESS:
    os << "access denied. You may need to set your group ID to 'ais'";
    break;
  case CS_ERR_NOT_EXIST:
    os << "not exist";
    break;
  case CS_ERR_EXIST:
    os << "exist";
    break;
  case CS_ERR_NOT_SUPPORTED:
    os << "not supported";
    break;
  case CS_ERR_SECURITY:
    os << "security";
    break;
  case CS_ERR_TOO_MANY_GROUPS:
    os << "too many groups";
    break;
  default:
    os << ": unknown cpg error " << err;
    break;
  };
  os << " (" << err << ")";
  return os.str();
}

CorosyncMemberId CorosyncCpg::self() const {
  unsigned int nodeid;
  if (CS_OK != (cpg_local_get(handle, &nodeid))) {
    LOG(ERROR)<< "Cannot get local CPG identity";
  }
  return CorosyncMemberId(nodeid, getpid());
}

namespace {
int byte(uint32_t value, int i) {
  return (value >> (i * 8)) & 0xff;
}
}

ostream& operator<<(ostream& out, const CorosyncMemberId& id) {
  uint32_t nodeId = id.getNode();
  if (nodeId) {
    out << byte(nodeId, 0) << "." << byte(nodeId, 1) << "." << byte(nodeId, 2) << "." << byte(nodeId, 3) << ":";
  }
  return out << id.getPid();
}

std::string CorosyncMemberId::str() const {
  char s[8];
  uint32_t x;
  x = htonl(nodeId);
  ::memcpy(s, &x, 4);
  x = htonl(pid);
  ::memcpy(s + 4, &x, 4);
  return std::string(s, 8);
}

std::string CorosyncMemberId::toString() const {
  std::stringstream ss;
  ss << "nodeId: " << nodeId << ", pid: " << pid << ends;
  return ss.str();
}

CorosyncMemberId::CorosyncMemberId(const std::string& s) {
  uint32_t x;
  memcpy(&x, &s[0], 4);
  nodeId = ntohl(x);
  memcpy(&x, &s[4], 4);
  pid = ntohl(x);
}
} // namespace cluster
} // namespace idgs
