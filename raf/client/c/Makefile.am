#
# Makefile for Intel Data Grid server
#
SUBDIRS = . src

# protobuf rules
CLIENT_PROTO_INPUTS = \
  src/idgs/client/pb/client_config.proto


CLIENT_PROTO_OUTPUTS = \
  src/idgs/client/pb/client_config.pb.cc \
  src/idgs/client/pb/client_config.pb.h

$(CLIENT_PROTO_OUTPUTS) : $(CLIENT_PROTO_INPUTS)
	protoc -I@top_srcdir@/services/admin/src -I@top_srcdir@/services/store/src -I@top_srcdir@/services/rdd/src -I@srcdir@/src -I@top_srcdir@/framework/src --cpp_out=@srcdir@/src $^
	
CLEANFILES = $(CLIENT_PROTO_OUTPUTS) 

# cluster library
lib_LTLIBRARIES = libidgsclient.la

libidgsclient_la_SOURCES= \
    $(CLIENT_PROTO_OUTPUTS) \
    src/idgs/client/command.cpp \
    src/idgs/client/command_parser.cpp \
    src/idgs/client/client.cpp \
    src/idgs/client/client_actor_message.cpp \
    src/idgs/client/tcp_synchronous_client.cpp \
    src/idgs/client/tcp_client.cpp \
    src/idgs/client/client_pool.cpp \
    src/idgs/client/tcp_asynch_client.cpp \
    src/idgs/client/client_const.cpp \
    src/idgs/client/rdd/rdd_client.cpp \
    src/idgs/client/admin/admin_client.cpp

utest_PROGRAMS=  \
  test/parser_test \
  test/expr_test

AM_DEFAULT_SOURCE_EXT = .cpp


LDADD =  \
      -lidgsclient \
      -lidgsadmin \
      -lidgsrdd \
      -lidgsdatastore \
      -lidgsframework \
      -lgtest -lgtest_main -ltbb 

# the macro IDGS_CXXFLAGS is defined in configure.ac
AM_CXXFLAGS = @IDGS_CXXFLAGS@ 
AM_LDFLAGS = -avoid-version @IDGS_LDFLAGS@


