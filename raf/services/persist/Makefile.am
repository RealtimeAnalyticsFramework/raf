#
# Makefile for Intel Data Grid server
#

####################auto gen hbase thrift cpp code.####################
AUTO_GENERATED_FILE_DIR = @srcdir@/src/idgs/persist/thrift	

HBASE_THRIFT_INPUT = @srcdir@/src/idgs/persist/thrift/Hbase.thrift

HBASE_THRIFT_OUTPUTS_NEEDED = \
	src/idgs/persist/thrift/Hbase_constants.cpp \
	src/idgs/persist/thrift/Hbase_constants.h \
	src/idgs/persist/thrift/Hbase.cpp \
	src/idgs/persist/thrift/Hbase.h \
	src/idgs/persist/thrift/Hbase_types.cpp \
	src/idgs/persist/thrift/Hbase_types.h
	
HBASE_THRIFT_OUTPUTS = \
	$(HBASE_THRIFT_OUTPUTS_NEEDED) \
	src/idgs/persist/thrift/Hbase_server.skeleton.cpp

$(HBASE_THRIFT_OUTPUTS) : $(HBASE_THRIFT_INPUT)
	thrift --gen cpp -out $(AUTO_GENERATED_FILE_DIR) $(HBASE_THRIFT_INPUT)
####################auto gen hbase thrift cpp code.####################
	
CLEANFILES = $(HBASE_THRIFT_OUTPUTS)

# library
lib_LTLIBRARIES=libidgspersist.la
libidgspersist_la_SOURCES = \
	$(HBASE_THRIFT_OUTPUTS_NEEDED) \
	src/idgs/persist/hbase_client_pool.cpp	

AM_DEFAULT_SOURCE_EXT = .cpp
		
LDADD =  \
       -lidgsdatastore \
       -lidgsframework \
       @IDGS_LDADD@ \
       -lgtest -lgtest_main -ltbb -lthrift
       
AM_LDFLAGS = -module -avoid-version @IDGS_LDFLAGS@
# the macro IDGS_CXXFLAGS is defined in configure.ac
AM_CXXFLAGS= @IDGS_CXXFLAGS@