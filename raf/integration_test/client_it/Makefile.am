#
# Makefile for Intel Data Grid server
#

# protobuf rules
itest_PROGRAMS= \
	it_client_pool_test \
	it_asynch_client_timeout_test \
	it_asynch_client_multi_threads_test

it_client_pool_test_SOURCES = \
	client_pool_it.cpp 
	
it_asynch_client_timeout_test_SOURCES = \
    asynch_client_it.cpp
    
it_asynch_client_multi_threads_test_SOURCES = \
    asynch_client_multi_threads_it.cpp
	
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

