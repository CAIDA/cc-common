#
# interval tree
# 
# Vasco Asturiano, CAIDA, UC San Diego
# corsaro-info@caida.org
# 
# Copyright (C) 2014 The Regents of the University of California.
# 
# This file is part of cc-common.
# 
# cc-common is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# cc-common is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with cc-common.  If not, see <http://www.gnu.org/licenses/>.
# 

SUBDIRS = rb_tree

AM_CPPFLAGS = -I$(top_srcdir)/common/libinterval3/rb_tree

noinst_LTLIBRARIES = libinterval3.la

libinterval3_la_SOURCES = \
	interval_tree.c interval_tree.h

libinterval3_la_LIBADD = $(top_builddir)/common/libinterval3/rb_tree/librbtree.la \
			$(CONDITIONAL_LIBS)

ACLOCAL_AMFLAGS = -I m4

CLEANFILES = *~
