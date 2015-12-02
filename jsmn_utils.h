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

#ifndef __JSMN_UTILS_H
#define __JSMN_UTILS_H

#include "jsmn.h"

#define JSMN_NEXT(tok) (tok)++

/** Helper macro to check for an assumed string value
 *
 * @param json          JSON string
 * @param tok           pointer to the current token
 * @param str           expected token string value
 *
 * Note: the err label must be defined in the calling code
 */
#define jsmn_str_assert(json, tok, str)           \
  do {                                            \
    if (json_strcmp(json, tok, str) != 0) {       \
      goto err;                                   \
    }                                             \
  } while(0)

/** Helper macro to check for an assumed string value
 *
 * @param tok           pointer to the current token
 * @param exptype       expected token type
 *
 * Note: the err label must be defined in the calling code
 */
#define jsmn_type_assert(tok, exptype)           \
  do {                                           \
    if (tok->type != exptype) {                  \
      goto err;                                  \
    }                                            \
  } while(0)

/** Check if the given token is a "null"
 *
 * @param json          JSON string
 * @param tok           pointer to the current token
 * @return 0 if the token is NOT null, 1 otherwise
 */
int jsmn_isnull(const char *json, jsmntok_t *tok);

/** Check if the given token is a string and the value matches the given string
 *
 * @param json          JSON string
 * @param tok           pointer to the current token
 * @param s             string to compare
 * @return 0 if the strings do not match, 1 otherwise
 */
int jsmn_streq(const char *json, jsmntok_t *tok, const char *s);

/** Advance the token over the current value
 *
 * @param tok           token to advance
 * @return pointer to the token after skipping the current value
 *
 * This function is useful for entirely skipping complex values (e.g. arrays,
 * objects), that do not need to be parsed.
 */
jsmntok_t *jsmn_skip(jsmntok_t *tok);

/** Copy the given string value to the given string buffer
 *
 * @param dest          string buffer to copy into
 * @param tok           pointer to the current token
 * @param json          JSON string
 *
 * The dest buffer must be large enough to hold (tok->end - tok->start)+1
 * characters.
 */
void jsmn_strcpy(char *dest, jsmntok_t *tok, const char *json);

/** Convert the given json primitive value into a unsigned long
 *
 * @param dest          pointer to an unsigned long
 * @param json          JSON string
 * @param tok           pointer to the current token
 * @return 0 if the value was parsed successfully, -1 otherwise
 */
int jsmn_strtoul(unsigned long *dest, const char *json, jsmntok_t *tok);

/** Convert the given json primitive value into a double
 *
 * @param dest          pointer to a double
 * @param json          JSON string
 * @param tok           pointer to the current token
 * @return 0 if the value was parsed successfully, -1 otherwise
 */
int jsmn_strtod(double *dest, const char *json, jsmntok_t *tok);

#endif /* __JSMN_UTILS_H */
