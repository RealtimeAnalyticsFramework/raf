#
# Makefile for Intel Data Grid server
#
SUBDIRS = . src

PROTO_INPUTS = \
	src/idgs/pb/rpc_message.proto \
	src/idgs/pb/primitive_type.proto \
	src/idgs/pb/cluster_config.proto \
	src/idgs/pb/cluster_event.proto \
	src/idgs/pb/expr.proto 
	

PROTO_OUTPUTS = \
	src/idgs/pb/rpc_message.pb.cc \
	src/idgs/pb/rpc_message.pb.h \
	src/idgs/pb/primitive_type.pb.cc \
	src/idgs/pb/primitive_type.pb.h \
	src/idgs/pb/cluster_config.pb.cc \
	src/idgs/pb/cluster_config.pb.h \
	src/idgs/pb/cluster_event.pb.cc \
	src/idgs/pb/cluster_event.pb.h \
	src/idgs/pb/expr.pb.cc \
	src/idgs/pb/expr.pb.h

$(PROTO_OUTPUTS) : $(PROTO_INPUTS)
	protoc -I@srcdir@/src --cpp_out=@srcdir@/src $^
	


ACTOR_SRCFILES = \
	src/idgs/actor/actor.cpp \
	src/idgs/actor/thread_model_stl.cpp \
	src/idgs/actor/stateless_actor.cpp \
	src/idgs/actor/actor_framework.cpp \
	src/idgs/actor/scheduled_future.cpp \
	src/idgs/actor/scheduler.cpp \
	src/idgs/actor/stateful_actor.cpp \
	src/idgs/actor/actor_worker.cpp \
	src/idgs/actor/actor_descriptor.cpp \
	src/idgs/actor/actor_message.cpp \
	src/idgs/actor/actor_descriptor_mgr.cpp
	
NET_SRCFILES = \
	src/idgs/net/async_tcp_server.cpp \
	src/idgs/net/stateful_tcp_actor.cpp \
	src/idgs/net/member_endpoint.cpp \
	src/idgs/net/async_udp_server.cpp \
	src/idgs/net/network_model_asio.cpp \
	src/idgs/net/rpc_member_listener.cpp \
	src/idgs/net/inner_tcp_connection.cpp \
	src/idgs/net/network_statistics.cpp \
	src/idgs/net/network_interface.cpp \
	src/idgs/net/resend_scheduler.cpp \
	src/idgs/net/inner_tcp_server.cpp

CLUSTER_SRCFILES = \
	src/idgs/cluster/cluster_cfg_parser.cpp \
	src/idgs/cluster/partition_wrapper.cpp \
	src/idgs/cluster/partitiontable_balance_verifier.cpp \
	src/idgs/cluster/member_wrapper.cpp \
	src/idgs/cluster/partitiontable_mgr.cpp \
	src/idgs/cluster/membershiptable_mgr.cpp \
	src/idgs/cluster/corosync_adapter.cpp \
	src/idgs/cluster/corosync_cpg.cpp \
	src/idgs/cluster/naive_partition_balancer.cpp \
	src/idgs/cluster/cluster_const.cpp

PB_UTIL_SRCFILES = \
	src/protobuf/pbvariant.cpp \
	src/protobuf/type_composer.cpp \
	src/protobuf/hash_code.cpp \
	src/protobuf/msg_comparer.cpp \
	src/protobuf/json_message.cpp \
	src/protobuf/message_helper.cpp

EXPR_FILES=\
  src/idgs/expr/expression_factory.cpp \
  src/idgs/expr/expression.cpp \
  src/idgs/expr/expression_context.cpp \
  src/idgs/expr/parsed_field_extractor.cpp \
  src/idgs/expr/const_expr.cpp \
  src/idgs/expr/logical_expr.cpp \
  src/idgs/expr/arithmetic_expr.cpp \
  src/idgs/expr/compare_expr.cpp \
  src/idgs/expr/string_expr.cpp \
  src/idgs/expr/branch_expr.cpp
  
HTTP_SERVER_FILES =\
  src/idgs/httpserver/http_request_parser.cpp \
  src/idgs/httpserver/http_response.cpp \
  src/idgs/httpserver/http_mime_types.cpp \
  src/idgs/httpserver/http_connection.cpp \
  src/idgs/httpserver/http_server.cpp \
  src/idgs/httpserver/http_servlet.cpp
  

	
# framework library
lib_LTLIBRARIES=libidgsframework.la
libidgsframework_la_SOURCES= \
  $(PROTO_OUTPUTS) \
	$(PB_UTIL_SRCFILES) \
	$(ACTOR_SRCFILES) \
	$(NET_SRCFILES) \
	$(CLUSTER_SRCFILES) \
	$(EXPR_FILES) \
	$(HTTP_SERVER_FILES) \
	src/idgs/util/utillity.cpp \
	src/idgs/util/enum_def.cpp \
	src/idgs/util/gcc_backtrace.cpp \
	src/idgs/result_code.cpp \
	src/idgs/application.cpp \
	src/idgs/signal_handler.cpp
	
bin_PROGRAMS=idgs
idgs_SOURCES = src/idgs/main.cpp 
		
conf_DATA = conf/cluster.conf

TEST_PROTO_INPUTS = test/employee.proto
TEST_PROTO_OUTPUTS = \
  test/employee.pb.cc \
  test/employee.pb.h

$(TEST_PROTO_OUTPUTS) : $(TEST_PROTO_INPUTS)
	protoc -I@srcdir@/test --cpp_out=@srcdir@/test $^

CLEANFILES = $(TEST_PROTO_OUTPUTS) $(PROTO_OUTPUTS) 

utest_PROGRAMS =  \
    test/idgs/rpc/actor_framework_test \
	test/idgs/rpc/thread_model_test \
	test/idgs/rpc/scheduler_test \
	test/idgs/rpc/actor_handler_test \
    test/backtrace_test \
    test/backtrace_sys_test \
	test/idgs/cluster/cluster_cfg_parser_test \
	test/idgs/cluster/member_wrapper_test \
	test/idgs/cluster/membershiptable_mgr_test \
	test/idgs/cluster/partition_wrapper_test \
	test/idgs/cluster/partition_table_mgr_test \
	test/idgs/cluster/cluster_framewok_test \
	test/idgs/cluster/cluster_cfg_parser_env_test \
	test/idgs/cluster/partitiontable_balance_verifier_test \
	test/idgs/boot/actor_descriptor_dump_test \
	test/idgs/boot/module_test \
	test/hashcode_test \
	test/type_composer_test \
	test/singleton_test \
	test/cloneable_test \
	test/result_code_test \
	test/glog_test  \
	test/comparer_test \
	test/proto_json_test \
	test/message_helper_test \
	test/pbvariant_test \
	test/timer_test \
	test/lamda_test \
	test/enum_test \
	test/pb_serdes_test \
	test/pb_dynamic_test \
	test/network_interface_test

test/hashcode_test.cpp:$(TEST_PROTO_OUTPUTS)

AM_DEFAULT_SOURCE_EXT = .cpp

AM_LDFLAGS=-avoid-version @IDGS_LDFLAGS@
LDADD = \
      -lidgsframework \
      -lgtest -lgtest_main 

idgs_LDADD = \
      -lidgsframework \
      -ltbb \
      -ldl

# the macro IDGS_CXXFLAGS is defined in configure.ac
AM_CXXFLAGS= @IDGS_CXXFLAGS@ 
