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

#ifndef __IP_UTILS_H
#define __IP_UTILS_H

/** Represents a IPv4 prefix
 * e.g. 192.168.0.0/16
 */
typedef struct ip_prefix 
{
  /** The address component of the prefix */
  uint32_t addr;
  /** The length of the prefix mask */
  uint8_t masklen;
} ip_prefix_t;

/** An element in a linked list of IP prefixes */
typedef struct ip_prefix_list
{
  /** The prefix that this element represents */
  ip_prefix_t prefix;
  /** The next prefix in the list */
  struct ip_prefix_list *next;
} ip_prefix_list_t;


/** 
 * Set a bit in an IP address to a given value
 *
 * @param addr      The address to set the bit in
 * @param bitno     The bit index to set
 * @param val       The value (0 or 1) to set the bit to
 * @return the address with the corresponding bit set to the given value
 * @note MSB is bit 1, LSB is bit 32
 */
uint32_t ip_set_bit(uint32_t addr, int bitno, int val);

/** 
 * Compute netmask address given a prefix bit length
 *
 * @param masklen   The mask length to calculate the netmask address for
 * @return the network mask for the given bit length
 */
uint32_t ip_netmask(int masklen);

/**
 * Compute broadcast address given address and prefix
 *
 * @param addr      The address to calculate the broadcast address for
 * @param masklen   The mask length to apply to the address
 * @return the broadcast address for the given prefix
 */
uint32_t ip_broadcast_addr(uint32_t addr, int masklen);

/**
 * Compute network address given address and prefix
 *
 * @param addr      The address to calculate the network address for
 * @param masklen   The mask length to apply to the address
 * @return the network address for the given prefix
 */
uint32_t ip_network_addr(uint32_t addr, int masklen);

/**
 * Compute the minimal list of prefixes which this range represents
 *
 * @param lower         The lower bound of the range
 * @param upper         The upper bound of the range
 * @param[out] pfx_list A linked list of prefixes
 * @return 0 if the list is successfully built, -1 if an error occurs
 *
 * @note the pfx_list returned MUST be free'd using ip_prefix_list_free
 */
int ip_range_to_prefix(ip_prefix_t lower, ip_prefix_t upper, 
		       ip_prefix_list_t **pfx_list);

/** 
 * Free a list of prefixes as returned by ip_range_to_prefix
 *
 * @param pfx_list      The list to free
 */
void ip_prefix_list_free(ip_prefix_list_t *pfx_list);

#endif /* __IP_UTILS_H */
