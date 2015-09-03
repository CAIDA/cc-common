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

#ifndef __AKQ_H
#define __AKQ_H

#include "config.h"
#ifdef HAVE_PTHREAD

#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>

/** @file
 *
 * @brief Single producer, single consumer lock-free queue implementation. Based
 *  on a description from http://www.drdobbs.com/parallel/210604448
 *
 * @author Alistair King
 *
 */

/*
  Example usage.
  A queue of at most 1000 int elements with a separate producer thread

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "akq.h"
AKQ_INIT(intq, int, 1000)

static void *producer_thread(void *user)
{
  akq_t(intq) *q = (akq_t(intq)*) user;
  int i;
  assert(q);

  fprintf(stderr, "producing some things\n");
  for(i=0; i<10000; i++) {
    fprintf(stderr, "pushing %d\n", i);
    assert(akq_push(intq, q, i) == 0);
    usleep(999);
  }

  return NULL;
}

int main(int argc, char **argv)
{
  pthread_t producer;
  int i;
  int val;
  akq_t(intq) *q = akq_create(intq);
  assert(q);
  pthread_create(&producer, NULL, producer_thread, q);

  for(i = 0; i < 10000; i++) {
    val = akq_shift(intq, q);
    fprintf(stderr, "got %d\n", i);
    usleep(2000);
  };

  akq_destroy(intq, q);

  pthread_join(producer, NULL);
}
*/

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

#define __AKQ_TYPES(name, akqval_t)             \
  typedef struct akq_##name##_node {            \
    akqval_t value;                             \
    struct akq_##name##_node *next;             \
  } akq_##name##_node_t;                        \
  typedef struct {                              \
    akq_##name##_node_t *first;                 \
    akq_##name##_node_t *divider;               \
    akq_##name##_node_t *last;                  \
    uint32_t size;                              \
    /** @todo consider adding a pool of unused nodes */ \
  } akq_##name##_t;

#define __AKQ_PROTOTYPES(name, SCOPE, akqval_t)                         \
  SCOPE akq_##name##_t *akq_##name##_create();                          \
  SCOPE void akq_##name##_destroy(akq_##name##_t *q);                   \
  SCOPE int akq_##name##_push(akq_##name##_t *q, akqval_t val);         \
  SCOPE akqval_t akq_##name##_shift(akq_##name##_t *q);                 \
  SCOPE akq_##name##_node_t *__akq_##name##_node_create();              \
  SCOPE void __akq_##name##_node_destroy(akq_##name##_node_t *node);

#define __AKQ_IMPL(name, SCOPE, akqval_t, maxsize)                      \
  SCOPE akq_##name##_t *akq_##name##_create()                           \
  {                                                                     \
    akq_##name##_t *q = malloc(sizeof(akq_##name##_t));                 \
    if (q == NULL) { return NULL; }                                     \
    q->first = q->divider = q->last = __akq_##name##_node_create();     \
    if (q->first == NULL) { akq_##name##_destroy(q); return NULL; }     \
    q->size = 0;                                                        \
    return q;                                                           \
  }                                                                     \
  SCOPE void akq_##name##_destroy(akq_##name##_t *q)                    \
  {                                                                     \
    akq_##name##_node_t *tmp;                                           \
    while (q->first != NULL) {                                          \
      tmp = q->first;                                                   \
      q->first = tmp->next;                                             \
      __akq_##name##_node_destroy(tmp);                                 \
    }                                                                   \
    free(q);                                                            \
  }                                                                     \
  SCOPE int akq_##name##_push(akq_##name##_t *q, akqval_t val)          \
  {                                                                     \
    akq_##name##_node_t *node;                                          \
    /* don't write too much data */                                     \
    if (q->size >= maxsize) {                                           \
      do { pthread_yield(); } while (q->size >= maxsize * 3/4);         \
    }                                                                   \
    __sync_fetch_and_add(&q->size, 1); /* increment queue size */       \
    if ((node = __akq_##name##_node_create()) == NULL) {                \
      return -1;                                                        \
    }                                                                   \
    node->value = (val);                                                \
    q->last->next = node;                                               \
    if (!__sync_bool_compare_and_swap(&q->last, q->last, q->last->next)) { \
      return -1; /* failed to write to last */                          \
    }                                                                   \
    while (q->first != q->divider) { /* Recover used nodes */           \
      node = q->first;                                                  \
      if (!__sync_bool_compare_and_swap(&q->first, q->first,            \
                                        q->first->next)) {              \
        return -1; /* failed to write to first */                       \
      }                                                                 \
      __akq_##name##_node_destroy(node);                                \
    }                                                                   \
    return 0;                                                           \
  }                                                                     \
  SCOPE akqval_t akq_##name##_shift(akq_##name##_t *q)                  \
  {                                                                     \
    akq_##name##_node_t *node;                                          \
    while (q->divider == q->last) { /* queue is empty */                \
      pthread_yield();                                                  \
    }                                                                   \
    assert(q->divider != q->last);                                      \
    node = q->divider->next;                                            \
    if (!__sync_bool_compare_and_swap(&q->divider, q->divider,          \
                                      q->divider->next)) {              \
      assert(0); /* failed to write to divider */                       \
    }                                                                   \
    __sync_fetch_and_sub(&q->size, 1);                                  \
    return node->value;                                                 \
  }                                                                     \
  SCOPE akq_##name##_node_t *__akq_##name##_node_create()               \
  {                                                                     \
    akq_##name##_node_t *node;                                          \
    if ((node = malloc(sizeof(akq_##name##_node_t))) == NULL) {         \
      return NULL;                                                      \
    }                                                                   \
    node->next = NULL;                                                  \
    return node;                                                        \
  }                                                                     \
  SCOPE void __akq_##name##_node_destroy(akq_##name##_node_t *node)     \
  {                                                                     \
    if (node == NULL) { return; }                                       \
    free(node);                                                         \
    /* user is responsible for freeing all values if needed */          \
  }

#define __AKQ_INIT(name, SCOPE, akqval_t, maxsize)      \
  __AKQ_TYPES(name, akqval_t)                           \
  __AKQ_PROTOTYPES(name, SCOPE, akqval_t)               \
  __AKQ_IMPL(name, SCOPE, akqval_t, maxsize)

#define AKQ_INIT(name, akqval_t, maxsize)                       \
  __AKQ_INIT(name, UNUSED static inline, akqval_t, maxsize)

/** Convenience macros */

/** Type of the queue
 *
 * @param name          Name of the queue [symbol]
 */
#define akq_t(name) akq_##name##_t

/** Create a new queue
 *
 * @param name          Name of the queue [symbol]
 * @return pointer to the created queue if successful, NULL otherwise
 */
#define akq_create(name) akq_##name##_create()

/** Destroy the given queue
 *
 * @param name          Name of the queue [symbol]
 * @param q             Pointer to the queue to destroy [akq_t(name)*]
 */
#define akq_destroy(name, q) akq_##name##_destroy(q)

/** Get the size of the given queue
 *
 * @param q             Pointer to the queue to get size of [akq_t(name)*]
 * @return number of elements in the queue
 */
#define akq_size(q) ((q)->size)

/** Push a value to the tail of the queue
 *
 * @param name          Name of the queue [symbol]
 * @param q             Pointer to the queue [akq_t(name)*]
 * @param val           Value to add to the queue [akqval_t]
 * @return 0 if the value was added successfully, -1 otherwise
 */
#define akq_push(name, q, val) akq_##name##_push(q, val)

/** Shift a value from the head of the queue
 *
 * @param name          Name of the queue [symbol]
 * @param q             Pointer to the queue [akq_t(name)*]
 * @return value shifted from the queue [akqval_t]
 */
#define akq_shift(name, q) akq_##name##_shift(q)

#endif /* HAVE_PTHREAD */

#endif /* __AKQ_H */
