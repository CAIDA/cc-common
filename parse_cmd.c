/*
 * This file is part of cc-common
 *
 * Copyright (c) 2004-2009 The University of Waikato, Hamilton, New Zealand.
 * Authors: Daniel Lawson
 *          Shane Alcock
 *          Perry Lorier
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
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





