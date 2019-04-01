/*
 * bytes_htons, bytes_htonl, gettimeofday_wrap from scamper
 *   http://www.caida.org/tools/measurement/scamper/
 * and re-released under a BSD license with permission from the author.
 *
 * timeval_subtract code from
 *   http://www.gnu.org/software/libc/manual/html_node/Elapsed-Time.html
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

#include "config.h"

#include <arpa/inet.h>
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif

#include "utils.h"

static uint32_t ten_to_the_power[10] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };

void bytes_htons(uint8_t *bytes, uint16_t u16)
{
  uint16_t tmp = htons(u16);
  memcpy(bytes, &tmp, 2);
  return;
}

void bytes_htonl(uint8_t *bytes, uint32_t u32)
{
  uint32_t tmp = htonl(u32);
  memcpy(bytes, &tmp, 4);
  return;
}

void bytes_htonll(uint8_t *bytes, uint64_t u64)
{
  uint64_t tmp = htonll(u64);
  memcpy(bytes, &tmp, 8);
  return;
}

uint64_t epoch_msec()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (uint64_t) ((uint64_t) tv.tv_sec * 1000 + (uint64_t) tv.tv_usec / 1000);
}

uint32_t epoch_sec()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (uint32_t) tv.tv_sec;
}

void gettimeofday_wrap(struct timeval *tv)
{
  struct timezone tz;
  gettimeofday(tv, &tz);
  return;
}

int timeval_subtract (struct timeval *result,
		      const struct timeval *a, const struct timeval *b)
{
  struct timeval y = *b;

  /* Perform the carry for the later subtraction by updating y. */
  if (a->tv_usec < b->tv_usec) {
    int nsec = (b->tv_usec - a->tv_usec) / 1000000 + 1;
    y.tv_usec -= 1000000 * nsec;
    y.tv_sec += nsec;
  }
  if (a->tv_usec - b->tv_usec > 1000000) {
    int nsec = (a->tv_usec - b->tv_usec) / 1000000;
    y.tv_usec += 1000000 * nsec;
    y.tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
  result->tv_sec = a->tv_sec - y.tv_sec;
  result->tv_usec = a->tv_usec - y.tv_usec;

  /* Return 1 if result is negative. */
  return a->tv_sec < y.tv_sec;
}

void chomp(char *line)
{
  char *newln;
  if((newln = strchr(line, '\n')) != NULL)
    {
      *newln = '\0';
    }
}

int parse_timestamp(char* buf, size_t len, uint32_t* seconds, uint32_t* nano_seconds){
  char * pch = memchr(buf, '.', len);
  char * t = buf;
  if(pch==NULL){
    printf("error!");
    return -1;
  }
  // assign seconds value
  *seconds = (uint32_t)strtoul(buf, &pch, 0);

  uint32_t nano = 0;
  for(int i=0; i<=8; i++){
    if(pch-buf+1+i+1>len){
      // going outside the buffer
      break;
    }
    int digit = *(pch+1+i)-'0';
    if(digit>9 || digit<0){
      return -2;
    }
    nano += digit * ten_to_the_power[8-i];
  }
  *nano_seconds = nano;

  return 0;
}
