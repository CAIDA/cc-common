/*
 * interval tree
 *
 * Vasco Asturiano, CAIDA, UC San Diego
 * corsaro-info@caida.org
 *
 * Implementation of an Interval Tree algorithm using a Red-Black tree underlying structure.
 *
 * rb_tree lib (Red-Black tree) was extended to allow tree augmentation from
 *    http://web.mit.edu/~emin/www.old/source_code/red_black_tree/index.html
 *
 * Copyright (C) 2014 The Regents of the University of California.
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

#include <inttypes.h>

 /** @file
 *
 * @brief Header file that exposes the public interface of interval tree.
 *
 * @author Vasco Asturiano
 *
 */

 /**
 * @name Public Opaque Data Structures
 *
 * @{ */

/** Opaque struct holding an interval tree. Manipulation using functions below. */
 typedef struct interval_tree interval_tree_t;

/** @} */

/**
 * @name Public Data Structures
 *
 * @{ */

/** Structure which defines an interval record used to update and query the tree. */
typedef struct interval
{
  /** 32 bit unsigned int defining the start of the interval */
  uint32_t start;
  /** 32 bit unsigned int defining the start of the interval */
  uint32_t end;
  /** A pointer to the data attached to the node representing the interval.
  *   This attribute is only relevant for insertions and query results. On query inputs it is unused.
  */
  void *data;
} interval_t;

/** @} */

/** Initialize a new interval tree instance
 *
 * @return the interval tree instance created, NULL if an error occurs
 */
interval_tree_t *interval_tree_init();

/** Free an interval tree instance
 *
 * @param               The interval tree instance to free
 */
void interval_tree_free(interval_tree_t *this);

/** Insert a new interval into the tree 
 *
 * @param this          The interval tree instance
 * @param interval      The interval to insert
 * 
 * @return 0 if successful or -1 if a malloc'ing error occurred
 */
int interval_tree_add_interval(interval_tree_t *this, const interval_t *interval);

/** Get all the interval tree nodes that are completely covered by the queried interval 
 *
 * @param this          The interval tree instance
 * @param interval      The interval to query for
 * @param num_ips       A pointer to an int where to place the number of matched interval nodes returned
 * 
 * @return a pointer to an array of intervals representing the matched intervals. 
 * 		   It is not necessary to free this array, it will be automatically collected during interval_tree_free().
 */
interval_t** getContained(interval_tree_t *this, const interval_t *interval, int *num_matches);

/** Get all the interval tree nodes that completely cover the queried interval 
 *
 * @param this          The interval tree instance
 * @param interval      The interval to query for
 * @param num_ips       A pointer to an int where to place the number of matched interval nodes returned
 * 
 * @return a pointer to an array of intervals representing the matched intervals. 
 * 		   It is not necessary to free this array, it will be automatically collected during interval_tree_free().
 */
interval_t** getContaining(interval_tree_t *this, const interval_t *interval, int *num_matches);

/** Get all the interval tree nodes that cover, are covered by, or partial overlap the queried interval 
 *
 * @param this          The interval tree instance
 * @param interval      The interval to query for
 * @param num_ips       A pointer to an int where to place the number of matched interval nodes returned
 * 
 * @return a pointer to an array of intervals representing the matched intervals. 
 * 		   It is not necessary to free this array, it will be automatically collected during interval_tree_free().
 */
interval_t** getOverlapping(interval_tree_t *this, const interval_t *interval, int *num_matches);
