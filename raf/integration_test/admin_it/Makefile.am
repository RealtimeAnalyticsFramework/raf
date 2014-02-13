#
# Makefile for Intel Data Grid server
#

# protobuf rules
itest_PROGRAMS= \
	it_admin_memberinfo_test \
	it_rdd_admin_test \
	it_net_admin_test
	
it_rdd_admin_test_SOURCES = \
    rdd_admin_test.cpp

it_admin_memberinfo_test_SOURCES = \
	admin_memberinfo_test.cpp
	
it_net_admin_test_SOURCES = \
	net_admin_test.cpp	
		
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

