/*
 * ntholl and htonll macros from
 *   http://www.codeproject.com/KB/cpp/endianness.aspx
 *
 * All other code Copyright (C) 2012 The Regents of the University of
 * California.
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

#ifndef __UTILS_H
#define __UTILS_H

#include "config.h"

#include <inttypes.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif

/** @file
 *
 * @brief Header file for common utility functions
 *
 * @author Alistair King
 *
 */

/** Randomize values in an array (in place)
 * See https://en.wikipedia.org/wiki/Fisher-Yates_shuffle
 * Requires: stdlib.h
 *
 * @param arr_t         Type of array values
 * @param arr           Pointer to the array to randomize
 * @param len           Number of elements in the array
 */
#define array_shuffle_fy(arr_t, arr, len)       \
  do {                                          \
    arr_t k;                                    \
    int i, r;                                   \
    for (i=(len)-1; i > 0; i--) {               \
      r = rand() % (i+1);                       \
      k = (arr)[i];                             \
      (arr)[i] = (arr)[r];                      \
      (arr)[r] = k;                             \
    }                                           \
  }                                             \
  while (0)

/** Internal to the STR macro */
#define XSTR(a) #a

/** Stringify a macro value */
#define STR(a) XSTR(a)

/** Count the number of elements in an arbitrary array */
#define ARR_CNT(a) (sizeof(a) / sizeof(a[0]))

/* ntholl and htonll macros from
   http://www.codeproject.com/KB/cpp/endianness.aspx */
/** Byte-swap a 64-bit integer */
#ifndef ntohll
#define ntohll(x) (((uint64_t)(ntohl((int)((x << 32) >> 32))) << 32) |	\
		   (uint32_t)ntohl(((int)(x >> 32))))
#endif

/** Byte-swap a 64-bit integer */
#ifndef htonll
#define htonll(x) ntohll(x)
#endif


/** Convert a host ordered short to a network ordered byte array
 *
 * @param[out] bytes        The converted byte array
 * @param u16               The host-ordered short
 */
void bytes_htons(uint8_t *bytes, uint16_t u16);

/** Convert a host ordered long to a network ordered byte array
 *
 * @param[out] bytes        The converted byte array
 * @param u32               The host-ordered long
 */
void bytes_htonl(uint8_t *bytes, uint32_t u32);

/** Convert a host ordered long-long (64 bit) to a network ordered byte array
 *
 * @param[out] bytes        The converted byte array
 * @param u64               The host-ordered long-long (64 bit)
 */
void bytes_htonll(uint8_t *bytes, uint64_t u64);

/** Convenience function to get the current time of day
 *
 * @param[out] tv           A pointer to a timeval containing the time of day
 */
void gettimeofday_wrap(struct timeval *tv);

/** Convenience function to get the current unix epoch time in milliseconds
 *
 * @return the current number of milliseconds since the unix epoch
 */
uint64_t epoch_msec();

/** Convenience function to get the current unix epoch time in sec
 *
 * @return the current number of seconds since the unix epoch
 */
uint32_t epoch_sec();

/** Allocate memory and set it to zero
 *
 * @param size              The size of memory to allocate
 * @return a pointer to zeroed memory if successful, NULL if an error occurs
 */
#define malloc_zero(size) calloc(1, size)

/** Find the delta between two timevals
 *
 * @param[out] result      A pointer to a timeval containing the delta
 * @param x                The start timeval
 * @param y                The end timeval
 * @return 1 if the result is negative, 0 otherwise
 */
int timeval_subtract (struct timeval *result,
		      const struct timeval *x, const struct timeval *y);

/** Remove a newline from the given string
* @param line              A pointer to the string to be chomped
*
* @note This function replaces the first occurance of '\n' with '\0'.
* Therefore it is only useful for strings where the newline is at the end.
* e.g. those returned by fgets (or similar)
*/
void chomp(char *line);

/** Parse timestamp string into seconds and nano seconds
 * @param buf     timestamp string buffer
 * @param len     timestamp string buffer length
 * @param sec     unsigned integer for seconds part of the timestamp
 * @param usec    unsigned integer for microsecond part of the timestamp
 *
 * @return 0 if no errors, negative values if errors occur
 */
int strntotime(const char* buf, size_t len, uint32_t *sec, uint32_t* usec);

#endif /* __UTILS_H */
