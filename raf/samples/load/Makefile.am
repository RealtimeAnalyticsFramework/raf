# $Header$
# Makefile for load tpch
#

LOAD_FILES = \
	src/loader.cpp \
	src/incluster_loader.cpp \
	src/tcp_loader.cpp \
	src/standalone_loader.cpp \
	src/batch_line_loader.cpp \
	src/loader_factory.cpp

bin_PROGRAMS = idgs-load

idgs_load_SOURCES = \
  $(LOAD_FILES) \
	src/load_main.cpp


utest_PROGRAMS = ut_load
ut_load_SOURCES = \
  $(LOAD_FILES) \
  test/load_test.cpp
  
  
LDADD =  \
	-lidgsclient \
	-ltpc \
	-lidgsrddng \
	-lidgsdatastore \
	-lidgsframework \
  @IDGS_LDADD@ \
	-lgtest -lgtest_main

idgs_load_LDADD = \
	-lidgsclient \
	-ltpc \
	-lidgsrddng \
	-lidgsdatastore \
	-lidgsframework \
  @IDGS_LDADD@

AM_LDFLAGS= @IDGS_LDFLAGS@
AM_CXXFLAGS= @IDGS_CXXFLAGS@ -I@srcdir@/src -I@srcdir@/test
