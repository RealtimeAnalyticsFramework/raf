#
# Makefile for Intel Data Grid server
#


#bin_PROGRAMS=rpc_tcp_test_server
#rpc_tcp_test_server_SOURCES = \
#   rpc_it/tcp_test_server.cpp
itest_PROGRAMS= it_rpc_tcp_test_client \
                 it_rpc_tcp_test_client2 \
                 it_rpc_tcp_test_server \
                 it_rpc_tcp_test_server2 \
                 it_rpc_schedule_test_server \
                 it_rpc_schedule_test_client

it_rpc_tcp_test_client_SOURCES = \
	tcp_test.cpp
it_rpc_tcp_test_client2_SOURCES = \
	tcp_test2.cpp

it_rpc_tcp_test_server_SOURCES = \
	tcp_test_server.cpp

it_rpc_tcp_test_server2_SOURCES = \
	tcp_test_server2.cpp

it_rpc_schedule_test_client_SOURCES = \
	scheduler_test_client.cpp

it_rpc_schedule_test_server_SOURCES = \
	scheduler_test_server.cpp

LDADD=  \
       -lidgsclient \
       -lidgsadmin \
       -lidgsrdd \
       -lidgsdatastore \
       -lidgsframework \
       -lgtest -lgtest_main \
       -ltbb       

AM_LDFLAGS = @IDGS_LDFLAGS@
       
AM_CXXFLAGS = @IDGS_CXXFLAGS@

