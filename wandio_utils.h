/*
 * cc-common
 *
 * Alistair King, CAIDA, UC San Diego
 * corsaro-info@caida.org
 *
 * bytes_htons, bytes_htonl, gettimeofday_wrap, malloc_zero from scamper
 *   http://www.caida.org/tools/measurement/scamper/
 *
 * timeval_subtract code from
 *   http://www.gnu.org/software/libc/manual/html_node/Elapsed-Time.html
 *
 * Copyright (C) 2012 The Regents of the University of California.
 *
 * This file is part of cc-common.
 *
 * cc-common is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cc-common is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cc-common.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __WANDIO_UTILS_H
#define __WANDIO_UTILS_H

#include <wandio.h>

/** Read a line from the given wandio file pointer
 *
 * @param file          The wandio file to read from
 * @param buffer        The buffer to read into
 * @param len           The maximum number of bytes to read
 * @param chomp         Should the newline be removed
 * @return the number of bytes actually read
 */
off_t wandio_fgets(io_t *file, void *buffer, off_t len, int chomp);

/** Attempt to detect desired compression for an output file based on file name
 *
 * @param filename      The filename to test for a compression extension
 * @return a wandio compression type suitable for use with wandio_wcreate
 */
int wandio_detect_compression_type(const char *filename);

/** Print a string to a wandio file
 *
 * @param file          The file to write to
 * @param format        The format string to write
 * @param args          The arguments to the format string
 * @return The amount of data written, or -1 if an error occurs
 *
 * The arguments for this function are the same as those for vprintf(3). See the
 * vprintf(3) manpage for more details.
 */
off_t wandio_vprintf(iow_t *file, const char *format, va_list args);

/** Print a string to a wandio
 *
 * @param file          The file to write to
 * @param format        The format string to write
 * @param ...           The arguments to the format string
 * @return The amount of data written, or -1 if an error occurs
 *
 * The arguments for this function are the same as those for printf(3). See the
 * printf(3) manpage for more details.
 */
off_t wandio_printf(iow_t *file, const char *format, ...);

#endif /* __WANDIO_UTILS_H */
