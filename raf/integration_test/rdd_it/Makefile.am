#
# Makefile for Intel Data Grid server
#

# protobuf rules
itest_PROGRAMS= \
	it_store_delegate_rdd_test \
	it_filter_rdd_test \
	it_rdd_name_test \
	it_union_transformer_test \
	it_reducebykey_transformer_test \
	it_hash_join_transformer_test \
	it_lookup_action_test \
	it_collect_action_test \
	it_collect_action_test_big_data \
	it_count_action_test \
	it_sum_action_test \
	it_export_action_test \
	it_top_n_action_test \
	it_if_expression_test
	

it_store_delegate_rdd_test_SOURCES = \
	store_delegate_rdd_test.cpp
	
it_filter_rdd_test_SOURCES = \
	filter_rdd_test.cpp

it_rdd_name_test_SOURCES = \
	rdd_name_test.cpp

it_union_transformer_test_SOURCES = \
	union_transformer_test.cpp
		
it_reducebykey_transformer_test_SOURCES = \
	reducebykey_transformer_test.cpp

it_hash_join_transformer_test_SOURCES = \
	hash_join_transformer_test.cpp
	
it_lookup_action_test_SOURCES = \
	lookup_action_test.cpp

it_collect_action_test_SOURCES = \
	collect_action_test.cpp

it_collect_action_test_big_data_SOURCES = \
	collect_action_test_big_data.cpp

it_count_action_test_SOURCES = \
	count_action_test.cpp

it_sum_action_test_SOURCES = \
	sum_action_test.cpp

it_export_action_test_SOURCES = \
	export_action_test.cpp

it_top_n_action_test_SOURCES = \
	top_n_action_test.cpp

it_if_expression_test_SOURCES = \
	if_expression_test.cpp
		
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

