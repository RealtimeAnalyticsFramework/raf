#
# Makefile for demo
#
bin_PROGRAMS=client
client_SOURCES = \
	src/client_main.cpp

LDADD = @IDGS_LDFLAGS@ \
	-lidgsclient \
	-lidgsadmin \
	-lidgsrdd \
	-lidgsdatastore \
	-lidgsframework \
	-ltbb


# the macro IDGS_CXXFLAGS is defined in configure.ac
AM_CXXFLAGS= @IDGS_CXXFLAGS@


