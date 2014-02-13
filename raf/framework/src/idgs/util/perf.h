
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

//
// set/get current thread name
/// @see pthread_setname_np, pthread_getname_np — set/get the name of a thread
//
#if defined(__unix__)
#include <sys/prctl.h>
/// @param name  can be up to 16 bytes long, and should be null-terminated if it contains fewer bytes.
inline void set_thread_name(const char* name) {
  prctl(PR_SET_NAME, name, 0L, 0L, 0);
}
/// @param name  can be up to 16 bytes long, and should be null-terminated if it contains fewer bytes.
inline void get_thread_name(const char* name) {
  prctl(PR_GET_NAME, name, 0L, 0L, 0);
}
#else // defined(__unix__)
inline void set_thread_name(const char* name) {
}
inline void get_thread_name(const char* name) {
}
#endif // defined(__unix__)
//
// Macro likely and unlikely can be used to improve branch predication.
// e.g.
// if (unlikely(sys_error)) { }
//
#ifdef __GNUC__
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)
#else
#define likely(x)       (x)
#define unlikely(x)     (x)
#endif
