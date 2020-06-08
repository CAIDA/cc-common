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
under a BSD license, **except**:

 - [libcsv](libcsv/)
   - Adapted from [libcsv](http://libcsv.sourceforge.net)
   - The original library can be obtained from http://libcsv.sourceforge.net
   - Provided under the LGPL. See included
     [copyright notice](libcsv/COPYING.LESSER).

 - [libinterval3](libinterval3/)
   - Adapted from
     [rb_tree](http://web.mit.edu/~emin/www.old/source_code/red_black_tree/index.html)
     implementation. See the included
     [license file](libinterval3/rb_tree/LICENSE).
   - Interval tree code authored by Vasco Asturiano and Copyright The Regents of
     the University of California and released under a BSD license.

 - [libjsmn](libjsmn/)
   - See the [libjsmn website](http://zserge.com/jsmn.html)
   - Released under an [MIT license](libjsmn/LICENSE)

 - [libpatricia](libpatricia/)
   - Adapted from the
     [Net::Patricia](https://metacpan.org/release/Net-Patricia) 1.22
     perl module by Dave Plonka, see [copyright notice](libpatricia/copyright).

 - [khash.h](khash.h), [klist.h](klist.h), [ksort.h](ksort.h)
   - Provided under an MIT license. See license notice at the top of the
     respective files.

 - [parse_cmd.c](parse_cmd.c) and [parse_cmd.h](parse_cmd.h)
   - Developed by [WAND](http://www.wand.net.nz) and released under a BSD
     license with permission from the author.

 - [utils.c](utils.c)
   - *timeval_subtract* code from
     http://www.gnu.org/software/libc/manual/html_node/Elapsed-Time.html
   - *bytes_htons*, *bytes_htonl*, *gettimeofday_wrap*, *malloc_zero* from
     [scamper](http://www.caida.org/tools/measurement/scamper/) and re-released
     under a BSD license with permission from the author.
