lib_LTLIBRARIES = libcborphine.la

AM_CFLAGS       = -iquote $(top_srcdir)/include
include_HEADERS = $(top_srcdir)/include/cbor.h \
                  $(top_srcdir)/include/pstdint.h

libcborphine_la_LDFLAGS = -version-info 0:1:0

libcborphine_la_SOURCES = src/cbor.c