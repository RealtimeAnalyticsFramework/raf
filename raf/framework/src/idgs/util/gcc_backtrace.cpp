
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
//
// http://gcc.gnu.org/bugzilla/show_bug.cgi?id=33903
// dump call stach when a exception is thrown, ONLY for G++
// override G++ ABI __cxa_throw
// link option: -ldl -rdynamic -Wl,--wrap,__cxa_throw

/// switch for unwind or backtrace
#if !defined(SYS_BACKTRACE)
#define USE_UNWIND
#endif // !defined(SYS_BACKTRACE)

#if defined(__GNUC__)
#include <cxxabi.h>
#include <sys/wait.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <tuple>
#include "idgs/util/utillity.h"

#if defined(USE_UNWIND)
#include <unwind.h>
#include <dlfcn.h>
#else // defined(USE_UNWIND)
#include <execinfo.h>
#endif // defined(USE_UNWIND)

#define PATH_MAX 4096 // defined in <linux/limits.h>

namespace idgs {
namespace util {

///
/// thread local storage: exception call stack.
///
static __thread std::string* g_exception_callstack = NULL;

///
/// get call stack of exception
///
std::string* get_expection_callstack() {
  return g_exception_callstack;
}

///
/// set call stack of exception
///
void set_expection_callstack(const std::string& s) {
  if (g_exception_callstack) {
    delete g_exception_callstack;
    g_exception_callstack = NULL;
  }
  g_exception_callstack = new std::string(s);
}


///
/// Wrap function for G++ abi
///
std::string demangle(const char* mangled_name) {
  size_t length = 256;
  char* buff = reinterpret_cast<char*>(malloc(length));
  int status;
  char* ret = abi::__cxa_demangle(mangled_name, buff, &length, &status);
  std::string name;
  if (status == 0) {
    buff = ret;
    name = buff;
  } else {
    name = mangled_name;
  }
  free(buff);
  return name;
}

std::string pointer2string(void* p) {
  std::stringstream ss;
  ss << p << std::ends;
  return ss.str();
}

std::string program_path() {
  std::string path;
  path.resize(PATH_MAX);
  ssize_t len = readlink("/proc/self/exe", const_cast<char*>(path.data()), PATH_MAX);

  if (len >= 0) {
    path.resize(len);
  } else {
    LOG(ERROR) << "failed to readlink /proc/self/exe: " << len;
  }
  return path;
}

/// Wrap function for addr2line
std::string addr2line(const char* exe, void* addr) {
  std::string result;
  int pipefd[2];
  pid_t cpid;

  if (pipe(pipefd) == -1) {
    LOG(ERROR) << "Failed to create pipe";
    result = std::string(exe) + " " + pointer2string(addr);
    return result;
  }

  cpid = fork();
  if (cpid == -1) {
    LOG(ERROR) << "Failed to create fork";
    result = std::string(exe) + " " + pointer2string(addr);
    return result;
  }

  if (cpid == 0) {
    close(pipefd[0]);
    if(dup2(pipefd[1], 1) == -1) {
      LOG(ERROR) << "Failed to dup2 stdout";
    }
    close(pipefd[1]);

    auto a = pointer2string(addr);
    // LOG(INFO) << "addr2line -siCfe " << exe << " " << a;
    execlp("addr2line", "addr2line", "-siCfe", exe, a.c_str(), NULL);
    exit(0);
  } else {
    close(pipefd[1]);
    char buf;
    std::stringstream ss;
    int lineno = 0;
    bool out_inlined = false;
    while (read(pipefd[0], &buf, 1) > 0) {
      if(buf == '\r') {

      } else if (buf == '\n') {
        ++lineno;
        if (lineno % 2) {
          ss << " at ";
        } else {
          ss << std::endl;
          out_inlined = true;
        }
      } else {
        if(out_inlined) {
          ss << " (inlined) ";
          out_inlined = false;
        }
        ss << buf;
      }
    }
    result = ss.str();
    result = idgs::str::trim(result);
    int status;
    waitpid(cpid, &status, 0);
    // LOG(INFO) << "status code: " << status;
    close(pipefd[0]);
  }
  return result;
}


#if defined(USE_UNWIND)

_Unwind_Reason_Code helper(struct _Unwind_Context* ctx, void* arg) {
  void* p = reinterpret_cast<void*>(_Unwind_GetIP(ctx));
  std::tuple<int, int, std::stringstream*>* tuple = reinterpret_cast<std::tuple<int, int, std::stringstream*>*>(arg);
  if (std::get < 1 > (*tuple) >= std::get < 0 > (*tuple)) {
    Dl_info info;
    if (dladdr(p, &info)) {
      const static std::string program = program_path();

      std::string s;
      if(program == info.dli_fname) {
        s = addr2line(info.dli_fname, p);
      } else {
        s = addr2line(info.dli_fname, (void*)(reinterpret_cast<long>(p) - reinterpret_cast<long>(info.dli_fbase)));
      }

      if(!s.empty() && s[0] != '?') {
        (*(std::get < 2 > (*tuple))) << "  @ " << s << std::endl;
      } else if (info.dli_saddr) {
        long d = reinterpret_cast<long>(p) - reinterpret_cast<long>(info.dli_saddr);
        (*(std::get < 2 > (*tuple))) << "  @ " << demangle(info.dli_sname) << "+0x" << std::hex << d << " at " << info.dli_fname << ' ' << p << ' '  << std::endl;
      } else {
        (*(std::get < 2 > (*tuple))) << "  @ " << " at " << info.dli_fname << ' ' << p << ' '  << std::endl;
      }

    }
  }
  std::get < 1 > (*tuple)++;
  return _URC_NO_REASON;
}

///
/// dump call stack trace - unwind
///
std::string stacktrace(int skip) {
  std::stringstream ss;
  std::tuple<int, int, std::stringstream*> arg = std::make_tuple<int, int, std::stringstream*>(0, 0, &ss);
  std::get < 0 > (arg) = skip;
  _Unwind_Backtrace(helper, reinterpret_cast<void*>(&arg));
  return std::get < 2 > (arg)->str();
}

#else // defined(USE_UNWIND)
///
/// dump call stack trace - backtrace
///
// format: fname(sname+off) [address]
// e.g.
//./a.out() [0x401215]
//./a.out(__wrap___cxa_throw+0x2b) [0x401382]
//./a.out(_Z3zoov+0x35) [0x4013ce]
//./a.out(_Z3barPFvvE+0x12) [0x401424]
//./a.out(_Z3foov+0xe) [0x401434]
//./a.out(main+0x9) [0x40143f]
///lib/x86_64-linux-gnu/libc.so.6(__libc_start_main+0xed) [0x7fef6bf6976d]
//./a.out() [0x401099]
std::string stacktrace(int skip) {
  std::stringstream ss;

  int j, nptrs;
#define SIZE 100
  void *buffer[SIZE];
  char **strings;

  nptrs = backtrace(buffer, SIZE);

  strings = backtrace_symbols(buffer, nptrs);
  if (strings == NULL) {
    return std::string();
  }

  for (j = skip; j < nptrs; j++) {
    ss << strings[j] << std::endl;
  }
  free(strings);

  return ss.str();
}

#endif // defined(USE_UNWIND)
}
 // namespace util
}// namespace idgs
#endif // defined(__GNUC__)

///
/// original G++ ABI throw
///
extern "C" void __real___cxa_throw(void* thrown_exception, std::type_info* tinfo, void (*dest)(void*))
  __attribute__(( noreturn ));

///
/// override G++ ABI throw
///
extern "C" void __wrap___cxa_throw(void* thrown_exception, std::type_info* tinfo, void (*dest)(void*)) {
#if defined(__GNUC__)
  std::stringstream ss;
  ss << "Call stack of exception " << idgs::util::demangle(tinfo->name()) << std::endl;
  ss << idgs::util::stacktrace(2);
  std::string s = ss.str();
  idgs::util::set_expection_callstack(s);
  // LOG(ERROR) << s;
#endif // defined(__GNUC__)

  __real___cxa_throw(thrown_exception, tinfo, dest);
}

//typedef void (*cxa_throw_type)(void *, void *, void (*) (void *));
//static cxa_throw_type orig_cxa_throw = 0;
//
//void load_orig_throw_code()
//{
//  orig_cxa_throw = (cxa_throw_type) dlsym(RTLD_NEXT, "__cxa_throw");
//}
//
//extern "C"
//void __cxa_throw (void *thrown_exception, void *pvtinfo, void (*dest)(void *)) {
////  if (orig_cxa_throw == 0) {
////    load_orig_throw_code();
////  }
////  orig_cxa_throw(thrown_exception, pvtinfo, dest);
//  __real___cxa_throw(thrown_exception, reinterpret_cast<std::type_info*>(pvtinfo), dest);
//}
//
