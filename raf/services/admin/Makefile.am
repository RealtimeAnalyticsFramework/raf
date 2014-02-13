#
# Makefile for Intel Data Grid server
#

# protobuf rules	

SUBDIRS = . src

	
PROTO_INPUTS = \
	src/idgs/admin/pb/admin_svc.proto

PROTO_OUTPUTS = \
  src/idgs/admin/pb/admin_svc.pb.cc \
  src/idgs/admin/pb/admin_svc.pb.h

$(PROTO_OUTPUTS) : $(PROTO_INPUTS)
	protoc -I@srcdir@/src/ -I@top_srcdir@/framework/src -I@srcdir@/../rdd/src -I@srcdir@/../store/src --cpp_out=@srcdir@/src/  $^
	
CLEANFILES = $(PROTO_OUTPUTS)

lib_LTLIBRARIES=libidgsadmin.la
libidgsadmin_la_SOURCES = \
  $(PROTO_OUTPUTS) \
  src/idgs/admin/actor/admin_service_actor.cpp \
  src/idgs/admin/admin_module.cpp \
  src/idgs/admin/admin_server.cpp \
  src/idgs/admin/attribute_operation.cpp \
  src/idgs/admin/cluster_admin.cpp \
  src/idgs/admin/manageable_node.cpp \
  src/idgs/admin/rdd_admin.cpp \
  src/idgs/admin/net_admin.cpp

utest_PROGRAMS = \
  test/attribute_operation_test
  
AM_DEFAULT_SOURCE_EXT = .cpp

LDADD =  \
       -lgtest_main -lgtest \
       -lidgsadmin \
       -lidgsrdd \
       -lidgsdatastore \
       -lidgsframework \
       -ltbb
       
AM_CXXFLAGS= @IDGS_CXXFLAGS@
AM_LDFLAGS = -module -avoid-version @IDGS_LDFLAGS@ -pthread
