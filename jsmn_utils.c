/*
 * cc-common
 *
 * Alistair King, CAIDA, UC San Diego
 * corsaro-info@caida.org
 *
 * Copyright (C) 2015 The Regents of the University of California.
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
#include "config.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "jsmn_utils.h"

int jsmn_isnull(const char *json, jsmntok_t *tok)
{
  if (tok->type == JSMN_PRIMITIVE &&
      strncmp("null", json + tok->start, tok->end - tok->start) == 0) {
    return 1;
  }
  return 0;
}

int jsmn_streq(const char *json, jsmntok_t *tok, const char *s)
{
  if (tok->type == JSMN_STRING &&
      (int) strlen(s) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 1;
  }
  return 0;
}

jsmntok_t *jsmn_skip(jsmntok_t *tok)
{
  int i;
  jsmntok_t *s;
  switch(tok->type)
    {
    case JSMN_PRIMITIVE:
    case JSMN_STRING:
      JSMN_NEXT(tok);
      break;
    case JSMN_OBJECT:
    case JSMN_ARRAY:
      s = tok;
      JSMN_NEXT(tok); // move onto first key
      for(i=0; i<s->size; i++) {
        tok = jsmn_skip(tok);
        if (s->type == JSMN_OBJECT) {
          tok = jsmn_skip(tok);
        }
      }
    default:
      assert(0);
    }

  return tok;
}

void jsmn_strcpy(char *dest, jsmntok_t *tok, const char *json)
{
  memcpy(dest, json + tok->start, tok->end  - tok->start);
  dest[tok->end - tok->start] = '\0';
}

int jsmn_strtoul(unsigned long *dest, const char *json, jsmntok_t *tok)
{
  char intbuf[20];
  char *endptr = NULL;
  assert(tok->end - tok->start < 20);
  jsmn_strcpy(intbuf, tok, json);
  *dest = strtoul(intbuf, &endptr, 10);
  if (*endptr != '\0') {
    return -1;
  }
  return 0;
}

int jsmn_strtod(double *dest, const char *json, jsmntok_t *tok)
{
  char intbuf[128];
  char *endptr = NULL;
  assert(tok->end - tok->start < 128);
  jsmn_strcpy(intbuf, tok, json);
  *dest = strtod(intbuf, &endptr);
  if (*endptr != '\0') {
    return -1;
  }
  return 0;
}
