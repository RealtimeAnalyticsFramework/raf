#
# Makefile for Intel Data Grid server
#
SUBDIRS = . src

# protobuf rules	
PROTO_INPUTS = \
	src/idgs/store/pb/store_config.proto \
	src/idgs/store/pb/store_service.proto \
	src/idgs/store/pb/store_file_mapper.proto \
	src/idgs/sync/pb/data_sync.proto 

PROTO_OUTPUTS = \
	src/idgs/store/pb/store_config.pb.cc \
	src/idgs/store/pb/store_config.pb.h \
	src/idgs/store/pb/store_service.pb.cc \
	src/idgs/store/pb/store_service.pb.h \
	src/idgs/store/pb/store_file_mapper.pb.cc \
	src/idgs/store/pb/store_file_mapper.pb.h \
	src/idgs/sync/pb/data_sync.pb.cc \
	src/idgs/sync/pb/data_sync.pb.h

$(PROTO_OUTPUTS) : $(PROTO_INPUTS)
	protoc -I@srcdir@/src -I@top_srcdir@/framework/src --cpp_out=@srcdir@/src $^
	
TEST_PROTO_INPUTS = test/customer.proto
TEST_PROTO_OUTPUTS = \
  test/customer.pb.cc \
  test/customer.pb.h

$(TEST_PROTO_OUTPUTS) : $(TEST_PROTO_INPUTS)
	protoc -I@srcdir@/test --cpp_out=@srcdir@/test $^

	
CLEANFILES = $(PROTO_OUTPUTS) $(TEST_PROTO_OUTPUTS)

# datastore library
lib_LTLIBRARIES=libidgsdatastore.la
libidgsdatastore_la_SOURCES = \
	$(PROTO_OUTPUTS) \
	src/idgs/store/config_parser.cpp \
	src/idgs/store/data_store.cpp \
	src/idgs/store/datastore_const.cpp \
	src/idgs/store/store.cpp \
	src/idgs/store/base_store.cpp \
	src/idgs/store/partitioned_store.cpp \
	src/idgs/store/replicated_store.cpp \
	src/idgs/store/store_config.cpp \
	src/idgs/store/store_module.cpp \
	src/idgs/store/schema/metadata_helper.cpp \
	src/idgs/store/schema/store_schema.cpp \
	src/idgs/store/schema/store_schema_actor.cpp \
	src/idgs/store/storage/aggregator_actor.cpp \
	src/idgs/store/storage/data_store_actor.cpp \
	src/idgs/store/listener/backup_store_listener.cpp \
	src/idgs/store/listener/listener_context.cpp \
	src/idgs/store/listener/listener_manager.cpp \
	src/idgs/store/listener/store_listener.cpp \
	src/idgs/store/listener/store_listener_factory.cpp \
	src/idgs/sync/data_sync_listener.cpp \
	src/idgs/sync/migration_target_actor.cpp \
	src/idgs/sync/store_migration_target_actor.cpp \
	src/idgs/sync/migration_source_actor.cpp \
	src/idgs/sync/store_migration_source_actor.cpp \
	src/idgs/sync/sync_target_actor.cpp \
	src/idgs/sync/store_sync_target_actor.cpp \
	src/idgs/sync/sync_source_actor.cpp \
	src/idgs/sync/store_sync_source_actor.cpp

	
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
       @IDGS_LDADD@ \
       -lgtest -lgtest_main 
 
AM_LDFLAGS = -module -avoid-version @IDGS_LDFLAGS@
# the macro IDGS_CXXFLAGS is defined in configure.ac
AM_CXXFLAGS= @IDGS_CXXFLAGS@

