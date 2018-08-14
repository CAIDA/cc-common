/*
 * Copyright (C) 2012 The Regents of the University of California.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __WANDIO_UTILS_H
#define __WANDIO_UTILS_H

#include "config.h" /* < configure.ac must use AC_SYS_LARGEFILE */
#include <stdarg.h>
#include <wandio.h>

/**
 * Generic read call-back function pointer
 */
typedef int64_t (read_cb_t)(void* file, void* buffer, int64_t len);

/** Generic readline function
 *
 * @param file          The wandio file to read from
 * @param buffer        The buffer to read into
 * @param len           The maximum number of bytes to read
 * @param chomp         Should the newline be removed
 * @return the number of bytes actually read
 */
int64_t generic_fgets(void *file, void *buffer, off_t len, int chomp, read_cb_t *read_cb);

/** Read a line from the given wandio file pointer
 *
 * @param file          The wandio file to read from
 * @param buffer        The buffer to read into
 * @param len           The maximum number of bytes to read
 * @param chomp         Should the newline be removed
 * @return the number of bytes actually read
 */
int64_t wandio_fgets(io_t *file, void *buffer, int64_t len, int chomp);

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
