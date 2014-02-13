#
# Makefile for Intel Data Grid server
#
PROTO_INPUTS = \
	@top_srcdir@/services/store/test/proto/tpch.proto 

PROTO_OUTPUTS = \
	tpch.pb.cc \
	tpch.pb.h 

$(PROTO_OUTPUTS) : $(PROTO_INPUTS)
	protoc -I@top_srcdir@/services/store/test/proto/ --cpp_out=@srcdir@/. $^
	
CLEANFILES = $(PROTO_OUTPUTS)


itest_PROGRAMS= \
	it_store_test_client_actor \
	it_partition_store_test_insert \
	it_partition_store_test_get \
	it_replicated_store_test_insert \
	it_replicated_store_test_get \
	it_clear_data_test \
	it_store_data_not_found_test

it_store_test_client_actor_SOURCES = \
  $(PROTO_OUTPUTS) \
	store_test_client_actor.cpp
	
it_partition_store_test_insert_SOURCES = \
  $(PROTO_OUTPUTS) \
	partition_store_test_insert.cpp
	
it_partition_store_test_get_SOURCES = \
  $(PROTO_OUTPUTS) \
	partition_store_test_get.cpp

it_replicated_store_test_insert_SOURCES = \
  $(PROTO_OUTPUTS) \
	replicated_store_test_insert.cpp

it_replicated_store_test_get_SOURCES = \
  $(PROTO_OUTPUTS) \
	replicated_store_test_get.cpp

it_clear_data_test_SOURCES = \
  $(PROTO_OUTPUTS) \
	clear_store_test.cpp

it_store_data_not_found_test_SOURCES = \
  $(PROTO_OUTPUTS) \
	store_data_not_found_test.cpp

LDADD=  \
       -lidgsclient \
       -lidgsadmin \
       -lidgsrdd \
       -lidgsdatastore \
       -lidgsframework \
       -lgtest -lgtest_main \
       -ltbb -lpthread

AM_LDFLAGS = @IDGS_LDFLAGS@
       
AM_CXXFLAGS = @IDGS_CXXFLAGS@

