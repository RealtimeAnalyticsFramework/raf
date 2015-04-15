#
# Makefile for Intel Data Grid server
#
	
itest_PROGRAMS= \
	it_persist2hbase

it_persist2hbase_SOURCES = \
	persist2hbase_it_test.cpp

LDADD= \
    -lidgspersist \
    -lidgsdatastore \
    -lidgsframework \
    -lgtest -lgtest_main \
    @IDGS_LDADD@

AM_LDFLAGS = @IDGS_LDFLAGS@
       
AM_CXXFLAGS = @IDGS_CXXFLAGS@

