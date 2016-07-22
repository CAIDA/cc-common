/*
 * cc-common
 *
 * Alistair King, CAIDA, UC San Diego
 * corsaro-info@caida.org
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

#ifndef __AKARR_H
#define __AKARR_H

//#include "config.h"
#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

/** @file
 *
 * @brief A macro-based array management implementation. Especially helpful for
 * uses where the common case is a small (< 8) number of small (1-2 byte)
 * elements as it does some trickery with pointers to optimize memory use in
 * this case.
 *
 * @author Alistair King
 *
 */

/* Example usage:

#include "akarr.h"
#include <stdio.h>

AKARR_INIT(myarr, uint16_t, uint8_t);

int main(int argc, char **argv)
{
  // declare the array
  akarr_t(myarr) things;
  // initialize the array
  akarr_init(myarr, things);

  fprintf(stdout, "---- INSERTING VALUES ----\n");

  // add some values and watch the length/size
  int i;
  for (i=0; i<300; i++) {
    // add a value to the array
    int idx = akarr_append(myarr, things, i);
    if (idx < 0) {
      if (idx == AKARR_ERR_FULL) {
        fprintf(stdout, "Array full\n");
        break;
      } else {
        fprintf(stderr, "ERROR: akarr_append returned %d\n", idx);
        exit(-1);
      }
    }
    // how long is the array?
    akarr_len_t(myarr) len = akarr_len(things);
    // how many things can we store in it?
    akarr_len_t(myarr) max_len = akarr_capacity(myarr);
    // approx. how much memory is the array occupying (including the length)?
    size_t bytes = akarr_size(myarr, things);
    // and can we get back the value that we inserted?
    akarr_val_t(myarr) val = akarr_get(myarr, things, idx);

    fprintf(stdout, "idx: %d, val: %d, len: %d (max: %d) size: %d\n",
            idx, (int)val, (int)len, (int)max_len, (int)bytes);
  }

  fprintf(stdout, "---- DUMPING ARRAY ----\n");

  // looping over the array
  for (i = 0; i < akarr_len(things); i++) {
    fprintf(stdout, "idx: %d, val: %d\n", i, (int)akarr_get(myarr, things, i));
    akarr_set(myarr, things, i, 256-i);
  }

  for (i = 0; i < akarr_len(things); i++) {
    fprintf(stdout, "idx: %d, val: %d\n", i, (int)akarr_get(myarr, things, i));
  }

  akarr_clean(myarr, things);

  return 0;
}

END EXAMPLE
*/

/** Error codes */
typedef enum {
  /** Array is full (the length field has reached it's max value) */
  AKARR_ERR_FULL = -10,
  /** Malloc failed. Yikes! */
  AKARR_ERR_MALLOC = -11,
} akarr_err_t;

/* prevent warnings for unused, macro-generated functions */
#if __GNUC__ >= 3
#  ifndef UNUSED
#    define UNUSED  __attribute__((unused))
#  endif
#else
#  ifndef UNUSED
#    define UNUSED
#  endif
#endif

/* some helper macros */

/** How many values can be jammed directly into the pointer address ;) */
#define AKARR_IMM_STORAGE_CNT(akarr_val_t)      \
  (sizeof(akarr_val_t*)/sizeof(akarr_val_t))

#define AKARR_IMM_SHIFT(akarr_val_t, idx)               \
  (((sizeof(akarr_val_t*) - sizeof(akarr_val_t)) -      \
    ((idx) * sizeof(akarr_val_t))) * 8)

#define AKARR_IMM_MASK(akarr_val_t)             \
  (((uint64_t)1 << (sizeof(akarr_val_t)*8)) - 1)

#define AKARR_IMM_SET(akarr_val_t, dst, idx, val)                       \
  do {                                                                  \
    /* what is the left-shift for this index? */                        \
    int lshift = AKARR_IMM_SHIFT(akarr_val_t, (idx));                   \
    /* zero out the value currently at that location */                 \
    (dst) = (akarr_val_t*)                                              \
      ((uint64_t)(dst) & ~(AKARR_IMM_MASK(akarr_val_t)<<lshift));       \
    /* and now stick our value in */                                    \
    (dst) = (akarr_val_t*) ((uint64_t)(dst) | ((uint64_t)val << lshift)); \
  } while (0)

#define AKARR_IMM_GET(akarr_val_t, dst, idx)                            \
  ((akarr_val_t)(((uint64_t)(dst) >>                                    \
                  (AKARR_IMM_SHIFT(akarr_val_t, idx))) &                \
                 AKARR_IMM_MASK(akarr_val_t)))

#define AKARR_CAP(akarr_len_t) ((1<<(sizeof(akarr_len_t)*8))-1)

/** Is the array 'full'? */
#define AKARR_FULL(akarr_len_t, cnt) ((cnt) == AKARR_CAP(akarr_len_t))

/** Define a new structure type that contains a pointer to our type, and a
 * counter of the number of elements in the array.
 *
 * @todo add another type for arrays that can shrink
 */
#define __AKARR_TYPES(name, akarr_val_t, akarr_len_t)   \
  typedef akarr_val_t akarr_##name##_val_t;             \
  typedef akarr_len_t akarr_##name##_len_t;             \
  typedef struct akarr_##name##_t {                     \
    akarr_val_t *vals;                                  \
    akarr_len_t cnt;                                    \
  } __attribute__((packed)) akarr_##name##_t;

#define __AKARR_PROTOTYPES(name, SCOPE, akarr_val_t, akarr_len_t)       \
  SCOPE void akarr_##name##_init(akarr_##name##_t *arrp);               \
  SCOPE void akarr_##name##_clean(akarr_##name##_t *arrp);              \
  SCOPE int akarr_##name##_append(akarr_##name##_t *arrp, akarr_val_t val); \
  SCOPE void akarr_##name##_set(akarr_##name##_t *arrp, int idx, akarr_val_t val); \
  SCOPE int akarr_##name##_capacity();                                  \
  SCOPE int akarr_##name##_size(akarr_##name##_t *arrp);                \
  SCOPE akarr_val_t akarr_##name##_get(akarr_##name##_t *arrp, akarr_len_t idx);

#define __AKARR_IMPL(name, SCOPE, akarr_val_t, akarr_len_t)             \
  SCOPE void akarr_##name##_init(akarr_##name##_t *arrp)                \
  {                                                                     \
    /* check some assumptions */                                        \
    assert(sizeof(akarr_val_t*) <= sizeof(uint64_t));                   \
    assert(INT_MAX >= AKARR_CAP(akarr_val_t));                          \
    arrp->vals = NULL;                                                  \
    arrp->cnt = 0;                                                      \
  }                                                                     \
  SCOPE void akarr_##name##_clean(akarr_##name##_t *arrp)               \
  {                                                                     \
    if (arrp->cnt > AKARR_IMM_STORAGE_CNT(akarr_val_t)) {               \
      free(arrp->vals);                                                 \
    }                                                                   \
    arrp->cnt = 0;                                                      \
  }                                                                     \
  SCOPE int akarr_##name##_append(akarr_##name##_t *arrp, akarr_val_t val) \
  {                                                                     \
    /* first, do we have capacity for another value? */                 \
    if (AKARR_FULL(akarr_len_t, arrp->cnt)) {                           \
      return AKARR_ERR_FULL;                                            \
    }                                                                   \
    /* next, can we store it immediately? */                            \
    if (arrp->cnt < AKARR_IMM_STORAGE_CNT(akarr_val_t)) {               \
      AKARR_IMM_SET(akarr_val_t, arrp->vals, arrp->cnt, val);           \
      return arrp->cnt++;                                               \
    }                                                                   \
    /* is this the first time that we are storing a non-immediate value? */ \
    if (AKARR_IMM_STORAGE_CNT(akarr_val_t) == arrp->cnt) {              \
      /* first, allocate enough memory for arrp->cnt+1 */               \
      akarr_val_t *tmp;                                                 \
      if ((tmp = malloc(sizeof(akarr_val_t) * (arrp->cnt+1))) == NULL) { \
        return AKARR_ERR_MALLOC;                                        \
      }                                                                 \
      /* now copy the values into this array */                         \
      /* do it manually to avoid endianness issues */                   \
      /* todo: improve this */                                          \
      int i;                                                            \
      for (i=0; i < AKARR_IMM_STORAGE_CNT(akarr_val_t); i++) {          \
        tmp[i] = AKARR_IMM_GET(akarr_val_t, arrp->vals, i);             \
      }                                                                 \
      /* and then update the pointer */                                 \
      arrp->vals = tmp;                                                 \
    } else {                                                            \
      /* too bad, we're going need to realloc the array first :/ */     \
      if ((arrp->vals =                                                 \
           realloc(arrp->vals, sizeof(akarr_val_t) * (arrp->cnt+1))) == NULL) { \
        return AKARR_ERR_MALLOC;                                        \
      }                                                                 \
    }                                                                   \
    arrp->vals[arrp->cnt] = val;                                        \
    /* and return the index */                                          \
    return arrp->cnt++;                                                 \
  }                                                                     \
  SCOPE void akarr_##name##_set(akarr_##name##_t *arrp, int idx, akarr_val_t val)\
  {                                                                     \
    assert(idx < arrp->cnt);                                            \
    if (arrp->cnt < AKARR_IMM_STORAGE_CNT(akarr_val_t)) {               \
      AKARR_IMM_SET(akarr_val_t, arrp->vals, idx, val);                 \
    } else {                                                            \
      /* now add it to the array */                                     \
      arrp->vals[idx] = val;                                            \
    }                                                                   \
  }                                                                     \
  SCOPE int akarr_##name##_capacity()                                   \
  {                                                                     \
    return AKARR_CAP(akarr_len_t);                                      \
  }                                                                     \
  SCOPE int akarr_##name##_size(akarr_##name##_t *arrp)                 \
  {                                                                     \
    if (arrp->cnt <= AKARR_IMM_STORAGE_CNT(akarr_val_t)) {              \
      return sizeof(akarr_##name##_t);                                  \
    } else {                                                            \
      /* need to add the size of the referenced array too */            \
      return sizeof(akarr_##name##_t) + (sizeof(akarr_val_t) * arrp->cnt); \
    }                                                                   \
  }                                                                     \
  SCOPE akarr_val_t akarr_##name##_get(akarr_##name##_t *arrp, akarr_len_t idx)\
  {                                                                     \
    assert(idx < arrp->cnt);                                            \
    if (arrp->cnt <= AKARR_IMM_STORAGE_CNT(akarr_val_t)) {              \
      return AKARR_IMM_GET(akarr_val_t, arrp->vals, idx);               \
    } else {                                                            \
      return arrp->vals[idx];                                           \
    }                                                                   \
  }

#define AKARR_INIT(name, akarr_val_t, akarr_len_t)      \
  __AKARR_TYPES(name, akarr_val_t, akarr_len_t)         \
  __AKARR_PROTOTYPES(name, UNUSED static inline, akarr_val_t, akarr_len_t) \
  __AKARR_IMPL(name, UNUSED static inline, akarr_val_t, akarr_len_t)

/* Convenience macros */
#define akarr_t(name) akarr_##name##_t
#define akarr_len_t(name) akarr_##name##_len_t
#define akarr_val_t(name) akarr_##name##_val_t
#define akarr_init(name, arr) akarr_##name##_init(&arr)
#define akarr_clean(name, arr) akarr_##name##_clean(&arr)
#define akarr_append(name, arr, val) akarr_##name##_append(&arr, val)
#define akarr_set(name, arr, idx, val) akarr_##name##_set(&arr, idx, val)
#define akarr_len(arr) (arr.cnt)
#define akarr_capacity(name) akarr_##name##_capacity()
#define akarr_size(name, arr) akarr_##name##_size(&arr)
#define akarr_get(name, arr, idx) akarr_##name##_get(&arr, idx)

#endif /* __AKARR_H */
