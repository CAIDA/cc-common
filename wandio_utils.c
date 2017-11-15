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

#include "config.h" /* we need _GNU_SOURCE to be defined */

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <wandio.h>

#include "wandio_utils.h"

off_t wandio_fgets(io_t *file, void *buffer, off_t len, int chomp)
{
  assert(file != NULL);

  char cbuf;
  int rval;
  int i;
  int done = 0;

  if(buffer == NULL || len <= 0)
    {
      return 0;
    }

  for(i=0; !done && i < len-1; i++)
    {
      if((rval = wandio_read(file, &cbuf, 1)) < 0)
       {
         return rval;
       }
      if(rval == 0)
       {
         done = 1;
         i--;
       }
      else
       {
         ((char*)buffer)[i] = cbuf;
         if(cbuf == '\n')
           {
             if(chomp != 0)
               {
                 ((char*)buffer)[i] = '\0';
               }
             done = 1;
           }
       }
    }

  ((char*)buffer)[i] = '\0';
  return i;
}

#define WANDIO_ZLIB_SUFFIX ".gz"
#define WANDIO_BZ2_SUFFIX ".bz2"

int wandio_detect_compression_type(const char *filename)
{
  const char *ptr = filename;

  int len = strlen(filename);

  if(len >= strlen(WANDIO_ZLIB_SUFFIX))
    {
      /* check for a .gz extension */
      ptr += (len - strlen(WANDIO_ZLIB_SUFFIX));
      if(strcmp(ptr, WANDIO_ZLIB_SUFFIX) == 0)
       {
         return WANDIO_COMPRESS_ZLIB;
       }

      ptr = filename;
    }

  if(len >= strlen(WANDIO_BZ2_SUFFIX))
    {
      /* check for a .bz2 extension */
      ptr += (len - strlen(WANDIO_BZ2_SUFFIX));
      if(strcmp(ptr, WANDIO_BZ2_SUFFIX) == 0)
       {
         return WANDIO_COMPRESS_BZ2;
       }
    }

  /* this is a suffix we don't know. don't compress */
  return WANDIO_COMPRESS_NONE;
}

inline off_t wandio_vprintf(iow_t *file, const char *format, va_list args)
{
  assert(file != NULL);
  char *buf;
  size_t len;
  int ret;

  if ((ret = vasprintf(&buf, format, args)) < 0)
    return ret;
  len = strlen(buf);
  len = len == (unsigned)len ? (size_t)wandio_wwrite(file, buf,
                                                    (unsigned)len) : 0;
  free(buf);
  return len;
}

inline off_t wandio_printf(iow_t *file, const char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  return wandio_vprintf(file, format, ap);
  va_end(ap);
}
