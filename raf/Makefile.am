# Root Makefile.
SUBDIRS= . framework services client front_end webroot samples integration_test 

install-exec-hook:
	cd @srcdir@/webroot; ./gen-makefile.sh; cd - ; 



bin_SCRIPTS = setenv.sh

conf_DATA = \
  conf/cluster.conf \
  conf/sync_cluster.conf \
  conf/client.conf \
  conf/client_8800.conf \
  conf/tpch.proto \
  conf/ssb.proto \
  conf/data_store.conf \
  conf/sync_data_store.conf \
  conf/ssb_file_mapper.conf \
  conf/tpch_file_mapper.conf


