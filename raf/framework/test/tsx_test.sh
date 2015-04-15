#
# Build CPF version of TBB 
# make tbb_cpf=1
#



TMAP=std::unordered_map
TMUTEX=tbb::spin_rw_mutex
TCODEC=BlankCodec

export LOOP=10000000
export RWRATIO=1.0
TBB_LIB=`find $TBB_HOME/build -name "*preview_release"`
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$TBB_LIB
export LD_LIBRARY_PATH

build() {
  BUILD_CMD="g++ -std=c++11 -g -DTSX_RW_MUTEX -Ofast -DMAIN -DTMAP=$TMAP -DTMUTEX=$TMUTEX -D TCODEC=$TCODEC -I$TBB_HOME/include -L$TBB_LIB -o tsx_test tsx_test.cpp -ltbb_preview -lglog"
  `$BUILD_CMD`
  if [ $? -ne 0 ] ; then
    echo $BUILD_CMD
    exit $RC_CODE
  fi

}

#
# loop all RANGE
#
run_range_test() {
  for RANGE in $ALL_RANGES; do
    export RANGE
    ./tsx_test
    if [ $? -ne 0 ] ; then
      exit $RC_CODE
    fi
    sleep 1
  done
}

#
# build
# loop all RWRATIO call run_range_test
#
run_rwratio_test() {
  build
  for RWRATIO in $ALL_RWRATIO; do
    export RWRATIO
    run_range_test
  done
  rm tsx_test
}

#
# loop all MUTEX call run_rwratio_test
#
run_mutex_test() {
  for TMUTEX in $ALL_MUTEX; do
    export TMUTEX
    run_rwratio_test
  done
}

#
# loop all CODEC call run_mutex_test
#
run_codec_test() {
  TCODEC="BlankCodec"
  LOOP=`expr $BASE_LOOP \* 100`
  export LOOP
  run_mutex_test

  TCODEC="ExpCodec"
  LOOP=$BASE_LOOP
  export LOOP
  run_mutex_test

#  for TCODEC in $ALL_TCODEC; do
#    export TCODEC
#    run_mutex_test
#  done
}

#
# loop all MAP call run_codec_test
#
run_map_test() {
  for TMAP in $ALL_TMAP; do
    export TMAP
    run_codec_test
  done
}

export NUMACTL=""
ALL_RANGES="2 4 6 8 12 16 24 32 48 64 128 256 512 1024 2048 4096 8192 16384"
ALL_RWRATIO="1 0.5 0.33 0.25"
ALL_MUTEX="tbb::speculative_spin_rw_mutex tbb::spin_mutex tbb::mutex tbb::queuing_mutex tbb::speculative_spin_mutex tbb::spin_rw_mutex tbb::queuing_rw_mutex"
ALL_TCODEC="BlankCodec ExpCodec"
ALL_TMAP="std::unordered_map std::map btree::btree_map"
ALL_TMAP="std::unordered_map"

export BASE_LOOP=1000
rm -f *.txt

run_nonuma() {
  ALL_THREADS="90 60 40 30 18 15 8 6 4"
  for TOTAL_THREADS in $ALL_THREADS; do
    RD="c$TOTAL_THREADS"
    echo $RD
    export TOTAL_THREADS
    run_map_test
    mkdir $RD
    mv -f *.txt $RD
  done
}

run_numa() {
  export NUMACTL="numactl --membind=0 --cpunodebind=0 "
  
  ALL_THREADS="30 18 15 8 6 4"
  for TOTAL_THREADS in $ALL_THREADS; do
    RD="nc$TOTAL_THREADS"
    echo $RD
    export TOTAL_THREADS
    run_map_test
    mkdir $RD
    mv -f *.txt $RD
  done
}

run_baseline() {
  export NUMACTL="numactl --membind=0 --cpunodebind=0 "
  ALL_THREADS="1"
  ALL_RWRATIO="1"
  ALL_MUTEX="tbb::null_mutex"
  for TOTAL_THREADS in $ALL_THREADS; do
    RD="baseline"
    echo $RD
    export TOTAL_THREADS
    run_map_test
    mkdir $RD
    mv -f *.txt $RD
  done
}

#run_nonuma
#run_numa
run_baseline







