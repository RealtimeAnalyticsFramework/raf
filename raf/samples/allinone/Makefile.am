#
# Makefile for allin1
# all modules are linked into main program
#

bin_PROGRAMS = idgs-aio
idgs_aio_SOURCES = \
  @top_srcdir@/framework/src/idgs/main.cpp
  
  
LDADD = @IDGS_LDFLAGS@ \
	-lidgsrdd \
	-lidgsdatastore \
	-lidgsframework \
	-ltbb -ldl


# the macro IDGS_CXXFLAGS is defined in configure.ac
AM_CXXFLAGS= @IDGS_CXXFLAGS@
