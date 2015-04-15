#
# Makefile for Intel Data Grid server
#

# protobuf rules	

SUBDIRS = . src

	
PROTO_INPUTS = \
	src/idgs/rdd/pb/rdd_common.proto \
	src/idgs/rdd/pb/rdd_svc.proto \
	src/idgs/rdd/pb/rdd_internal.proto \
	src/idgs/rdd/pb/rdd_transform.proto \
	src/idgs/rdd/pb/rdd_action.proto

PROTO_OUTPUTS = \
  src/idgs/rdd/pb/rdd_common.pb.cc \
  src/idgs/rdd/pb/rdd_common.pb.h \
  src/idgs/rdd/pb/rdd_svc.pb.cc \
  src/idgs/rdd/pb/rdd_svc.pb.h \
  src/idgs/rdd/pb/rdd_internal.pb.h \
  src/idgs/rdd/pb/rdd_internal.pb.cc \
  src/idgs/rdd/pb/rdd_transform.pb.h \
  src/idgs/rdd/pb/rdd_transform.pb.cc \
  src/idgs/rdd/pb/rdd_action.pb.h \
  src/idgs/rdd/pb/rdd_action.pb.cc  

$(PROTO_OUTPUTS) : $(PROTO_INPUTS)
	protoc -I@srcdir@/src/ -I@top_srcdir@/framework/src -I@srcdir@/../store/src --cpp_out=@srcdir@/src/  $^
	
CLEANFILES = $(PROTO_OUTPUTS)


lib_LTLIBRARIES=libidgsrddng.la
libidgsrddng_la_SOURCES = \
  $(PROTO_OUTPUTS) \
  src/idgs/rdd/rdd_module.cpp \
  src/idgs/rdd/rdd_service_actor.cpp \
  src/idgs/rdd/base_rdd_actor.cpp \
  src/idgs/rdd/base_rdd_partition.cpp \
  src/idgs/rdd/pair_rdd_actor.cpp \
  src/idgs/rdd/pair_rdd_partition.cpp \
  src/idgs/rdd/pair_store_delegate_rdd_actor.cpp \
  src/idgs/rdd/pair_store_delegate_rdd_partition.cpp \
  src/idgs/rdd/rdd_internal_service_actor.cpp \
  src/idgs/rdd/rdd_local.cpp \
  src/idgs/rdd/rdd_const.cpp \
  src/idgs/rdd/rdd_store_listener.cpp \
  src/idgs/rdd/rdd_member_event_listener.cpp \
  src/idgs/rdd/transform/transformer.cpp \
  src/idgs/rdd/transform/filter_transformer.cpp \
  src/idgs/rdd/transform/union_transformer.cpp \
  src/idgs/rdd/transform/group_transformer.cpp \
  src/idgs/rdd/transform/hash_join_transformer.cpp \
  src/idgs/rdd/transform/reducebykey_transformer.cpp \
  src/idgs/rdd/transform/reduce_transformer.cpp \
  src/idgs/rdd/transform/transformer_context.cpp \
  src/idgs/rdd/action/rdd_action.cpp \
  src/idgs/rdd/action/action.cpp \
  src/idgs/rdd/action/count_action.cpp \
  src/idgs/rdd/action/sum_action.cpp \
  src/idgs/rdd/action/lookup_action.cpp \
  src/idgs/rdd/action/collect_action.cpp \
  src/idgs/rdd/action/export_action.cpp \
  src/idgs/rdd/action/top_n_action.cpp \
  src/idgs/rdd/action/action_context.cpp \
  src/idgs/rdd/op/expr_operator.cpp \
  src/idgs/rdd/op/reduce_operator.cpp \
  src/idgs/rdd/op/join_operator.cpp \
  src/idgs/rdd/op/reduce_option.cpp \
  src/idgs/rdd/db/base_connection.cpp \
  src/idgs/rdd/db/local_file_connection.cpp
  
#src/idgs/rdd/rdd_snapshot.cpp  

utest_PROGRAMS = 

AM_DEFAULT_SOURCE_EXT = .cpp

LDADD =  \
       -lidgsrddng \
       -lidgsdatastore \
       -lidgsframework \
       @IDGS_LDADD@ \
       -lgtest_main -lgtest 

AM_CXXFLAGS= @IDGS_CXXFLAGS@
AM_LDFLAGS = -module -avoid-version @IDGS_LDFLAGS@
