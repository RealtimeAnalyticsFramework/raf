#
# Makefile for tpch/ssb server side
#

# protobuf rules
PROTO_INPUTS = \
  	@srcdir@/src/idgs/tpc/pb/tpc_crud.proto \
  	@srcdir@/src/idgs/tpc/pb/tpc_rdd_action.proto

PROTO_OUTPUTS = \
	src/idgs/tpc/pb/tpc_crud.pb.cc \
	src/idgs/tpc/pb/tpc_crud.pb.h \
	src/idgs/tpc/pb/tpc_rdd_action.pb.cc \
	src/idgs/tpc/pb/tpc_rdd_action.pb.h

$(PROTO_OUTPUTS) : $(PROTO_INPUTS)
	protoc -I@srcdir@/src/idgs/tpc/pb --cpp_out=@srcdir@/src/idgs/tpc/pb $^
	
TPC_FILES = \
	$(PROTO_OUTPUTS) \
	src/idgs/tpc/tpc_svc_module.cpp \
	src/idgs/tpc/actor/line_crud_actor.cpp \
	src/idgs/tpc/transform/tpch_Q6_transformer.cpp \
	src/idgs/tpc/transform/ssb_Q1_transformer.cpp \
	src/idgs/tpc/action/tpch_Q6_action.cpp \
	src/idgs/tpc/action/ssb_Q1_action.cpp \
	src/idgs/tpc/action/partition_count_action.cpp

lib_LTLIBRARIES=libtpc.la
libtpc_la_SOURCES = \
	$(TPC_FILES)

itest_PROGRAMS = \
  tpch_Q6 \
  tpch_Q6_raw_data_result \
  ssb_Q1_1 \
  ssb_Q1_1_raw_data_result \
  partition_count_action_it \
  ssb_q1_1_join_transformer \
  tpch_Q6_1

tpch_Q6_SOURCES = \
  test/idgs_tpch_Q6.cpp
  
tpch_Q6_1_SOURCES = \
  test/idgs_tpch_Q6_1.cpp
  
tpch_Q6_raw_data_result_SOURCES = \
  test/tpch_Q6_raw_data_result.cpp

ssb_Q1_1_SOURCES = \
  test/idgs_ssb_Q1_1.cpp

ssb_Q1_1_raw_data_result_SOURCES = \
  test/ssb_Q1_1_raw_data_result.cpp

partition_count_action_it_SOURCES = \
  test/partition_count_action_test.cpp

ssb_q1_1_join_transformer_SOURCES = \
  test/ssb_q1_1_join_transformer_test.cpp
  
LDADD = @IDGS_LDFLAGS@ \
	-ltpc \
	-lidgsclient \
	-lidgsadmin \
	-lidgsrdd \
	-lidgsdatastore \
	-lidgsframework \
	-lgtest -lgtest_main -ltbb

CLEANFILES = $(PROTO_OUTPUTS) 

# the macro IDGS_CXXFLAGS is defined in configure.ac
AM_CXXFLAGS= @IDGS_CXXFLAGS@ -I@srcdir@/src -I@srcdir@/test
AM_LDFLAGS = -module -avoid-version @IDGS_LDFLAGS@ -pthread
