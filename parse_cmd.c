/*
 * Copyright (c) 2004-2009 The University of Waikato, Hamilton, New Zealand.
 * Authors: Daniel Lawson
 *          Shane Alcock
 *          Perry Lorier
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

/*
 * 2016-04-20:
 * Alistair updated to support nested quoted strings. The implementation is not
 * perhaps the most efficient, but this function is hardly
 * performance-critical. Basically it works by only considering a quoted word
 * complete when it finds a quote that is **not** preceded by a backslash. It
 * then deletes one backslash that precedes each quotation.
 * I also "fixed" the formatting of the code.
 */

#include <string.h>

static void skip_white(char **buf)
{
  while (**buf==' ') {
    (*buf)++;
  }
}

/* Get the next word in a line
 *
 * Returns
 *  NULL : End of line
 *  other: Pointer to a word
 *
 * Side effects:
 *  updates *buf
 *  modified *buf
 *
 * ' foo bar baz' => 'foo' 'bar baz'
 * ' "foo bar" baz' => 'foo bar' ' baz'
 * ' foo "bar \"baz baz\""' => 'foo' 'bar "baz baz"'
 */
static char * split_cmd(char **buf)
{
  char *ret = 0;

  skip_white(buf);

  if (**buf=='"') { /* Quoted */
    (*buf)++;
    ret=*buf;

    while (**buf && (**buf!='"' || *((*buf)-1)=='\\')) {
      if (**buf=='"' && *((*buf)-1)=='\\') { /* Nested quote */
        memmove((*buf)-1, *buf, strlen(*buf));
      } else {
        (*buf)++;
      }
    }

    if (**buf) {
      **buf='\0';
      (*buf)++;
    }
  } else {
    ret=*buf;

    while (**buf && **buf!=' ') {
      (*buf)++;
    }

    if (**buf) {
      **buf='\0';
      (*buf)++;
    }
  }
  return ret;
}

/* Split a command line up into parc,parv
 * using command line rules
 */
void parse_cmd(char *buf, int *parc, char *parv[], int MAXTOKENS,
	       const char *command_name)
{
  int i=0;
  parv[0] = (char*) command_name;
  *parc=1;

  while(*buf) {
    parv[(*parc)++] = split_cmd(&buf);
    if (*parc>(MAXTOKENS-1)) {
      parv[*parc]=buf;
      break;
    }
  }
  for (i = *parc; i<MAXTOKENS; i++) {
    parv[i]="";
  }
}





