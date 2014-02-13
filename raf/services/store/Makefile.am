#
# Makefile for Intel Data Grid server
#
SUBDIRS = . src 
# protobuf rules	
	
PROTO_INPUTS = \
	src/idgs/store/pb/store_config.proto \
	src/idgs/store/pb/store_service.proto \
	src/idgs/store/pb/store_internal.proto \
	src/idgs/store/pb/store_sync.proto \
	src/idgs/store/pb/store_file_mapper.proto \
	src/idgs/store/pb/metadata.proto 

PROTO_OUTPUTS = \
	src/idgs/store/pb/store_config.pb.cc \
	src/idgs/store/pb/store_config.pb.h \
	src/idgs/store/pb/store_service.pb.cc \
	src/idgs/store/pb/store_service.pb.h \
	src/idgs/store/pb/store_internal.pb.cc \
	src/idgs/store/pb/store_internal.pb.h \
	src/idgs/store/pb/store_sync.pb.cc \
	src/idgs/store/pb/store_sync.pb.h \
	src/idgs/store/pb/store_file_mapper.pb.cc \
	src/idgs/store/pb/store_file_mapper.pb.h \
	src/idgs/store/pb/metadata.pb.cc \
	src/idgs/store/pb/metadata.pb.h

$(PROTO_OUTPUTS) : $(PROTO_INPUTS)
	protoc -I@srcdir@/src -I@top_srcdir@/framework/src --cpp_out=@srcdir@/src $^
	
TEST_PROTO_INPUTS = test/customer.proto
TEST_PROTO_OUTPUTS = \
  test/customer.pb.cc \
  test/customer.pb.h

$(TEST_PROTO_OUTPUTS) : $(TEST_PROTO_INPUTS)
	protoc -I@srcdir@/test --cpp_out=@srcdir@/test $^

	
CLEANFILES = $(PROTO_OUTPUTS) $(TEST_PROTO_OUTPUTS)

conf_DATA = test/data_store.conf test/proto/ssb.proto test/proto/tpch.proto

# datastore library
lib_LTLIBRARIES=libidgsdatastore.la
libidgsdatastore_la_SOURCES = \
	$(PROTO_OUTPUTS) \
	src/idgs/store/store_config_wrapper.cpp \
	src/idgs/store/store_schema_wrapper.cpp \
	src/idgs/store/store_config_wrappers.cpp \
	src/idgs/store/data_store.cpp \
	src/idgs/store/config_parser.cpp \
	src/idgs/store/data_map.cpp \
	src/idgs/store/store.cpp \
	src/idgs/store/store_ptr.cpp \
	src/idgs/store/data_store_actor.cpp \
	src/idgs/store/datastore_listener.cpp \
	src/idgs/store/aggregator_actor.cpp \
	src/idgs/store/data_sync_actor.cpp \
	src/idgs/store/store_module.cpp \
	src/idgs/store/datastore_const.cpp \
	src/idgs/store/metadata_helper.cpp \
	src/idgs/store/store_listener.cpp \
	src/idgs/store/store_listener_factory.cpp \
	src/idgs/store/listener_manager.cpp
	
utest_PROGRAMS =  \
	test/datastore_test \
	test/store_ptr_test  \
	test/dynamic_proto_test \
	test/store_listener_test
	
test/datastore_test.cpp:$(TEST_PROTO_OUTPUTS)

AM_DEFAULT_SOURCE_EXT = .cpp
		
LDADD =  \
       -lidgsdatastore \
       -lidgsframework \
       -lgtest -lgtest_main -ltbb
       
AM_LDFLAGS = -module -avoid-version @IDGS_LDFLAGS@
# the macro IDGS_CXXFLAGS is defined in configure.ac
AM_CXXFLAGS= @IDGS_CXXFLAGS@

