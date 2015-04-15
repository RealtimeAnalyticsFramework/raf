#ifdef TSX_RW_MUTEX
#define TBB_PREVIEW_SPECULATIVE_SPIN_RW_MUTEX 1
#endif

// #include <cmath>
#include <thread>
//#include <random>
//#include <chrono>

#include <map>
#include <unordered_map>
#include <btree_map.h>
//#include <vector>

// #include <iostream>
#include <sstream>
#include <fstream>

#include <sys/prctl.h>

#include <tbb/mutex.h>
#include <tbb/null_mutex.h>
#include <tbb/spin_mutex.h>
#include <tbb/null_rw_mutex.h>
#include <tbb/queuing_mutex.h>
#include <tbb/spin_rw_mutex.h>
#include <tbb/queuing_rw_mutex.h>

// #include <glog/logging.h>

// SFINAE test
template <typename T>
class has_lock_read_method
{
    typedef char one;
    typedef long two;

    template <typename C> static one test( decltype(&C::lock_read) ) ;
    template <typename C> static two test(...);


public:
    enum { value = sizeof(test<T>(0)) == sizeof(char) };
};

// Culled by SFINAE if reserve does not exist or is not accessible
template <typename T>
constexpr auto has_reserve_method(int) -> decltype(&T::reserve, bool()) {
  return true;
}

// Used as fallback when SFINAE culls the template method
constexpr bool has_reserve_method(...) { return false; }

template <typename T, bool hasReserve>
struct Reserver{
  static void reserve(T& c, size_t size){
    c.reserve(size);
  }
};
template <typename T>
struct Reserver <T, false>{
  static void reserve(T& c, size_t size){
  }
};

template <typename MUTEX, bool rw>
struct scoped_lock_helper {
  scoped_lock_helper(MUTEX& m, bool w) :lock(m, w) {
  }
private:
  typename MUTEX::scoped_lock lock;
};

template <typename MUTEX>
struct scoped_lock_helper<MUTEX, false> {
  scoped_lock_helper(MUTEX& m, bool w) :lock(m) {
  }
private:
  typename MUTEX::scoped_lock lock;
};

struct BlankCodec {
  double encode(double v) const {
    return v;
  }
  double decode(double v) const {
    return v;
  }
};

struct ExpCodec {
  double encode(double v) const {
    return std::atan(std::exp(v));
  }
  double decode(double v) const {
    return std::log(std::tan(v));
  }
};

struct Record {
  double value;
  char placeholder[64 - sizeof(double)];
};

template <typename MAP, typename MUTEX, typename CODEC >
class LockedMap {
public:
  void reserve(size_t size) {
    Reserver<MAP, has_reserve_method(0)>::reserve(map, size);
  }
  void set(int key, double value) {
    scoped_lock_helper<MUTEX, has_lock_read_method<MUTEX>::value> lock(mutex, true);
    auto& v = map[key];
    v.value = value;
    v.value = codec.encode(v.value);
  }
  double get(int key) {
    scoped_lock_helper<MUTEX, has_lock_read_method<MUTEX>::value> lock(mutex, false);
    auto it = map.find(key);
    double temp;
    if (it == map.end()) {
      temp = static_cast<double>(key);
    } else {
      temp = it->second.value;
    }
    double result = codec.decode(temp);
    return result;
  }
private:
  MAP map;
  MUTEX mutex;
  CODEC codec;
};

class Radom {
public:
  Radom (unsigned int id) : random_engine(id){
  }
  unsigned int operator () () {
    return random_engine();
  }
private:
  std::mt19937 random_engine;
};

template<typename LM>
class WriteWorker {
public:
  WriteWorker(int id, int range_, LM& lm) : random(id), range(range_), lockedMap(lm) {
  }

  void run () {
    int key = random();
    lockedMap.set(key % range, static_cast<double>(key));
  }

private:
  Radom random;
  int range;
  LM& lockedMap;
};

template<typename LM>
class ReadWorker {
public:
  ReadWorker(int id, int range_, LM& lm): random(id), range(range_), lockedMap(lm) {
    sum = 0.0;
  }

  void run () {
    int key = random() % range;
    sum += lockedMap.get(key);
  }

  double getSum() {
    return sum;
  }
private:
  Radom random;
  int range;
  LM& lockedMap;
  double sum;
};


// Hashmap
typedef LockedMap<std::unordered_map<int, Record>, tbb::spin_rw_mutex, BlankCodec> spin_rw_mutex_blank_hashmap;
typedef LockedMap<std::map<int, Record>, tbb::null_mutex, BlankCodec> null_mutex_blank_treemap;
typedef LockedMap<std::map<int, Record>, tbb::null_rw_mutex, BlankCodec> null_rw_mutex_blank_treemap;
typedef LockedMap<btree::btree_map<int, Record>, tbb::mutex, BlankCodec> mutex_blank_btreemap;
typedef LockedMap<btree::btree_map<int, Record>, tbb::spin_mutex, BlankCodec> spin_mutex_blank_btreemap;
typedef LockedMap<btree::btree_map<int, Record>, tbb::queuing_mutex, BlankCodec> queuing_mutex_blank_btreemap;
typedef LockedMap<btree::btree_map<int, Record>, tbb::queuing_rw_mutex, BlankCodec> queuing_rw_mutex_blank_btreemap;
typedef LockedMap<btree::btree_map<int, Record>, tbb::speculative_spin_mutex, ExpCodec> speculative_spin_mutex_exp_btreemap;
// typedef LockedMap<btree::btree_map<int, Record>, tbb::speculative_spin_rw_mutex, BlankCodec> speculative_spin_rw_mutex_blank_btreemap;


#ifndef TMAP
#define TMAP  std::unordered_map
#endif
#ifndef TMUTEX
#define TMUTEX  tbb::spin_mutex
#endif
#ifndef TCODEC
#define TCODEC  BlankCodec
#endif

/*
 * Concatenate preprocessor tokens A and B without expanding macro definitions
 * (however, if invoked from a macro, macro arguments are expanded).
 */
#define PPCAT_NX(A, B) A ## B

/*
 * Concatenate preprocessor tokens A and B after macro-expanding them.
 */
#define PPCAT(A, B) PPCAT_NX(A, B)

/*
 * Turn A into a string literal without expanding macro definitions
 * (however, if invoked from a macro, macro arguments are expanded).
 */
#define STRINGIZE_NX(A) #A

/*
 * Turn A into a string literal after macro-expanding it.
 */
#define STRINGIZE(A) STRINGIZE_NX(A)

std::string generateFileName(unsigned int totalThreads, float rwratio) {
  std::stringstream ss;

  if(getenv("NUMACTL") && strlen(getenv("NUMACTL")) > 10) {
    ss << "n";
  }
  ss << 'c' << totalThreads;

  ss << "-";
  ss << STRINGIZE(TCODEC);

  ss << "-";
  std::string tmp = STRINGIZE(TMAP);
  tmp = tmp.substr(tmp.rfind(':') + 1);
  ss << tmp;

  ss << "-";
  ss << rwratio;

  ss << "-";
  tmp = STRINGIZE(TMUTEX);
  tmp = tmp.substr(tmp.rfind(':') + 1);
  ss << tmp;

  return ss.str();
}
static double g_sum = 0.0;
void runTest(unsigned int range, unsigned int loop, float rwratio) {
  // google::InstallFailureSignalHandler();

  typedef LockedMap<TMAP<int, Record>, TMUTEX, TCODEC> TLOCKED_MAP;
  typedef WriteWorker<TLOCKED_MAP> TW_WORKER;
  typedef ReadWorker<TLOCKED_MAP> TR_WORKER;

  unsigned int totalThread = std::thread::hardware_concurrency();
  char* temp = getenv("TOTAL_THREADS");
  if (temp) {
    totalThread = atoi(temp);
  }
  if (totalThread < 1) {
    return;
  }
  unsigned int writeThreads = static_cast<int>(static_cast<float>(totalThread) * rwratio);
  unsigned int readThreads = totalThread - writeThreads;
  std::cerr << generateFileName(totalThread, rwratio)
      << ", range:" << range
      << ", threads (total:" << totalThread << ", write: " << writeThreads << ", read: " << readThreads << ")"
      << std::endl;

  TLOCKED_MAP m;
  m.reserve(range * 2);
  std::vector<std::thread> threads;
  std::vector<long> times;
  threads.reserve(totalThread);
  times.resize(totalThread);
  for (unsigned int i = 0; i < writeThreads; ++i) {
    threads.push_back(std::thread(
        [i, range, &m, loop, &times] () {
      char name[128];
      snprintf(name, 127, "tsx_w_%02d", (int)i);
      prctl(PR_SET_NAME, name, 0L, 0L, 0);
      TW_WORKER w(i, range, m);
      std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
      for(int j = 0; j < loop; j++) {
        w.run();
      }
      std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
      times[i] = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    }));

  }
  for (unsigned int i = 0; i < readThreads; ++i) {
    threads.push_back(std::thread(
        [i, range, &m, loop, &times, writeThreads] () {
      char name[128];
      snprintf(name, 127, "tsx_r_%02d", (int)i);
      prctl(PR_SET_NAME, name, 0L, 0L, 0);
      TR_WORKER w(i, range, m);
      std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
      for(int j = 0; j < loop; j++) {
        w.run();
      }
      g_sum += w.getSum();
      std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
      times[writeThreads + i] = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    }));
  }
  for(auto& t : threads) {
    try {
      t.join();
    } catch (...) {
      std::cerr << "error" << std::endl;
    }
  }


  long totalTime = 0;
  long writetime = 0;
  long readtime = 0;
  for (unsigned int i = 0; i < writeThreads; ++i) {
    // std::cerr << ", " << times[i];
    totalTime += times[i];
    writetime += times[i];
  }
  for (unsigned int i = writeThreads; i < times.size(); ++i) {
    // std::cerr << ", " << times[i];
    totalTime += times[i];
    readtime += times[i];
  }
  // std::cerr << std::endl;

  totalTime /= times.size();
  writetime /= writeThreads;
  if(readThreads) {
    readtime /= readThreads;
  }
  std::cerr << "avg: " << totalTime << ", " << writetime << ", " << readtime << std::endl;
  std::ofstream ofs(generateFileName(totalThread, rwratio) + ".txt", std::ios_base::app);
  ofs << range << ", " << totalTime << ", " << writetime << ", " << readtime << std::endl;
  ofs.close();
}

void main_test() {
  unsigned int range = 2;
  unsigned int loop = 10000;
  float rwratio = 1.0;
  char* tmp = getenv("RANGE");
  if (tmp) {
    range = atoi(tmp);
  }
  tmp = getenv("LOOP");
  if (tmp) {
    loop = atoi(tmp);
  }
  tmp = getenv("RWRATIO");
  if (tmp) {
    rwratio = atof(tmp);
  }

  runTest(range, loop, rwratio);
}

#ifdef MAIN
int main() {
  main_test();
  return 0;
}
#else
#include <gtest/gtest.h>
TEST(TSX, test) {
  main_test();
}
#endif
