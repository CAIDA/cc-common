#
# Copyright (C) 2012 The Regents of the University of California.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

SUBDIRS = libpatricia libinterval3 libcsv libjsmn

AM_CPPFLAGS = -I$(top_srcdir) \
	-I$(top_srcdir)/common/libpatricia \
	-I$(top_srcdir)/common/libpinterval3 \
	-I$(top_srcdir)/common/libcsv \
	-I$(top_srcdir)/common/libjsmn

noinst_LTLIBRARIES = libcccommon.la

CONDITIONAL_LIBS=

libcccommon_la_SOURCES = \
	utils.c 	\
	utils.h 	\
	ip_utils.c 	\
	ip_utils.h 	\
	jsmn_utils.h 	\
	jsmn_utils.c 	\
	parse_cmd.c 	\
	parse_cmd.h 	\
	khash.h 	\
	klist.h 	\
	ksort.h

libcccommon_la_LIBADD = $(top_builddir)/common/libpatricia/libpatricia.la \
			$(top_builddir)/common/libinterval3/libinterval3.la \
			$(top_builddir)/common/libcsv/libcsv.la \
			$(top_builddir)/common/libjsmn/libjsmn.la \
			$(CONDITIONAL_LIBS)

ACLOCAL_AMFLAGS = -I m4

CLEANFILES = *~
