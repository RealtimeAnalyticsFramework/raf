#
# Makefile for Intel Data Grid server
#
	
itest_PROGRAMS= \
	it_cluster

it_cluster_SOURCES = \
	cluster_it_test.cpp

LDADD= \
    -lidgsrddng \
    -lidgsdatastore \
    -lidgsframework \
    -lgtest -lgtest_main \
    @IDGS_LDADD@

AM_LDFLAGS = @IDGS_LDFLAGS@
       
AM_CXXFLAGS = @IDGS_CXXFLAGS@

