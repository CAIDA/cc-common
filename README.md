cc-common
=========

A collection of helper code for C projects.

This code should not be distributed by itself. It is designed to be integrated
into a larger project which itself contains autoconf and automake scripts to
drive the build process.

All code has been written by Alistair King <alistair@caida.org> and is released
under the terms of the GPL (see the COPYING file of the including project),
*except*:

 - libpatricia
   - Adapted from the Net::Patricia perl module by Dave Plonka, see
     [copyright notice](libpatrica/copyright).

 - libcsv
   - Adapted from [libcsv](http://libcsv.sourceforge.net)
   - The original library can be obtained from http://libcsv.sourceforge.net
   - Provided under the LGPL. See included
     [copyright notice](libcsv/COPYING.LESSER).

 - parse_cmd.[ch]
   - Developed by [WAND](http://www.wand.net.nz) and used with permission.

 - utils.c
   - _timeval_subtract_ code from
     http://www.gnu.org/software/libc/manual/html_node/Elapsed-Time.html
   - _bytes_htons_, _bytes_htonl_, _gettimeofday_wrap_, _malloc_zero_ from
     [scamper](http://www.caida.org/tools/measurement/scamper/)
