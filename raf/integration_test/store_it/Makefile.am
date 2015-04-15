#
# Makefile for Intel Data Grid server
#
PROTO_INPUTS = \
	@top_srcdir@/conf/tpch.proto 

PROTO_OUTPUTS = \
	tpch.pb.cc \
	tpch.pb.h 

$(PROTO_OUTPUTS) : $(PROTO_INPUTS)
	protoc -I@top_srcdir@/conf/ --cpp_out=@srcdir@/. $^
	
CLEANFILES = $(PROTO_OUTPUTS)


itest_PROGRAMS= \
	it_store_test_client_actor \
	it_partition_store_insert_test \
	it_partition_store_get_test \
	it_partition_store_update_test \
	it_partition_store_delete_test \
	it_replicated_store_test_insert \
	it_replicated_store_test_get \
	it_truncate_store_test \
	it_store_data_not_found_test \
	it_store_schema_test

it_store_test_client_actor_SOURCES = \
  $(PROTO_OUTPUTS) \
	store_test_client_actor.cpp
	
it_partition_store_insert_test_SOURCES = \
  $(PROTO_OUTPUTS) \
	partition_store_insert_test.cpp
	
it_partition_store_get_test_SOURCES = \
  $(PROTO_OUTPUTS) \
	partition_store_get_test.cpp
	
it_partition_store_update_test_SOURCES = \
  $(PROTO_OUTPUTS) \
	partition_store_update_test.cpp
	
it_partition_store_delete_test_SOURCES = \
  $(PROTO_OUTPUTS) \
	partition_store_delete_test.cpp

it_replicated_store_test_insert_SOURCES = \
  $(PROTO_OUTPUTS) \
	replicated_store_test_insert.cpp

it_replicated_store_test_get_SOURCES = \
  $(PROTO_OUTPUTS) \
	replicated_store_test_get.cpp

it_truncate_store_test_SOURCES = \
  $(PROTO_OUTPUTS) \
	truncate_store_test.cpp

it_store_data_not_found_test_SOURCES = \
  $(PROTO_OUTPUTS) \
	store_data_not_found_test.cpp

it_store_schema_test_SOURCES = \
	store_schema_test.cpp

LDADD=  \
       -lidgsclient \
       -lidgsrddng \
       -lidgsdatastore \
       -lidgsframework \
       -lgtest -lgtest_main \
       @IDGS_LDADD@

AM_LDFLAGS = @IDGS_LDFLAGS@
       
AM_CXXFLAGS = @IDGS_CXXFLAGS@

