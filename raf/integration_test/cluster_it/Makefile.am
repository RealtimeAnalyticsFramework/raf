#
# Makefile for Intel Data Grid server
#
	
itest_PROGRAMS= \
	it_cluster

it_cluster_SOURCES = \
	cluster_it_test.cpp

LDADD= \
    -lidgsadmin \
    -lidgsrdd \
    -lidgsdatastore \
    -lidgsframework \
   	-ltbb \
    -lgtest -lgtest_main \
   	-lpthread 

AM_LDFLAGS = @IDGS_LDFLAGS@
       
AM_CXXFLAGS = @IDGS_CXXFLAGS@

