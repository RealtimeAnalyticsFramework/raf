#
# Makefile for demo
#
bin_PROGRAMS = idgs-cli

idgs_cli_SOURCES = \
	src/client_main.cpp

LDADD = \
	-lidgsclient \
	-lidgsrddng \
	-lidgsdatastore \
	-lidgsframework \
	@IDGS_LDADD@

AM_LDFLAGS= @IDGS_LDFLAGS@

AM_CXXFLAGS= @IDGS_CXXFLAGS@


