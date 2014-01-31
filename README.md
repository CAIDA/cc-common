cc-common
=========

A collection of helper code for C projects.

This code should not be distributed by itself. It is designed to be integrated
into a larger project which itself contains autoconf and automake scripts to
drive the build process.

Usage
-----

cc-common is designed to be included in projects as a git submodule.

To use cc-common, loosely do the following:

1. Add cc-common as a git submodule
  - `git submodule add git://github.com/alistairking/cc-common.git common`
  - See http://git-scm.com/book/en/Git-Tools-Submodules for more info about
    working with git submodules.
2. Update the *AC_CONFIG_FILES* macro in *configure.ac* to include:
`common/Makefile`, `common/libpatricia/Makefile`, and
`common/libcsv/Makefile`. For example:

        AC_CONFIG_FILES([Makefile lib/Makefile
                         tools/Makefile
                         common/Makefile
                         common/libpatricia/Makefile
                         common/libcsv/Makefile
                        ])

3. In the main *Makefile.am* file for your library (usually */lib/Makefile.am*),
add `$(top_builddir)/common/libcccommon.la` to the LIBADD variable. For example:

        libsomething_la_LIBADD = $(top_builddir)/common/libcccommon.la

4. To include header files directly, you may need to add to the *AM_CPPFLAGS*
variable in the appropriate *Makefile.am*. For example:

        AM_CPPFLAGS = -I$(top_srcdir) \
                      -I$(top_srcdir)/common/ \
                      -I$(top_srcdir)/common/libpatricia \
                      -I$(top_srcdir)/common/libcsv

5. Run *autoreconf* and *configure* and you should be all set.

Copyright
---------

All code has been written by Alistair King <alistair@caida.org> and is released
under the terms of the GPL (see the COPYING file of the including project),
**except**:

 - libpatricia
   - Adapted from the
     [Net::Patricia](http://search.cpan.org/~plonka/Net-Patricia-1.014/Patricia.am)
     perl module by Dave Plonka, see [copyright notice](libpatricia/copyright).

 - libcsv
   - Adapted from [libcsv](http://libcsv.sourceforge.net)
   - The original library can be obtained from http://libcsv.sourceforge.net
   - Provided under the LGPL. See included
     [copyright notice](libcsv/COPYING.LESSER).

 - parse_cmd.[ch]
   - Developed by [WAND](http://www.wand.net.nz) and used with permission.

 - utils.c
   - *timeval_subtract* code from
     http://www.gnu.org/software/libc/manual/html_node/Elapsed-Time.html
   - *bytes_htons*, *bytes_htonl*, *gettimeofday_wrap*, *malloc_zero* from
     [scamper](http://www.caida.org/tools/measurement/scamper/)
