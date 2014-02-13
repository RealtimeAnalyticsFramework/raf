# $Header: /var/cvsroot/idgs/samples/load/Makefile.am,v 1.20 2013/11/07 07:35:05 tinglou Exp $
# Makefile for load tpch
#

LOAD_FILES = \
	src/loader.cpp \
	src/incluster_loader.cpp \
	src/tcp_loader.cpp \
	src/standalone_loader.cpp \
	src/batch_line_loader.cpp \
	src/loader_factory.cpp

conf_DATA = conf/client.conf conf/ssb_file_mapper.conf conf/tpch_file_mapper.conf
	
bin_PROGRAMS = load

load_SOURCES = \
  $(LOAD_FILES) \
	src/load_main.cpp


utest_PROGRAMS = ut_load
ut_load_SOURCES = \
  $(LOAD_FILES) \
  test/load_test.cpp
  
  
LDADD = @IDGS_LDFLAGS@ \
	-ltpc \
	-lidgsclient \
	-lidgsadmin \
	-lidgsrdd \
	-lidgsdatastore \
	-lidgsframework \
	-lgtest -lgtest_main -ltbb


# the macro IDGS_CXXFLAGS is defined in configure.ac
AM_CXXFLAGS= @IDGS_CXXFLAGS@ -I@srcdir@/src -I@srcdir@/test
