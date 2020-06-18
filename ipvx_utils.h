/*
 * Copyright (C) 2020 The Regents of the University of California.
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

/** @file
 *
 * @brief Utilities for IPv4 and IPv6 addresses
 *
 * @author Ken Keys
 */

#ifndef IPVX_UTILS_H
#define IPVX_UTILS_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

/// Represents an IPv4 or IPv6 prefix or address
typedef struct ipvx_prefix {
  /// Address family (AF_INET or AF_INET6)
  uint16_t family;
  /// The length of the prefix mask
  uint8_t masklen; // (for addresses, this space would be padding anyway)
  /// The address component of the prefix (in network order)
  union {
    struct in_addr v4;
    struct in6_addr v6;
    uint8_t _u8[16];
    uint16_t _u16[8];
    uint32_t _u32[4];
  } addr;
} ipvx_prefix_t;

/// An element in a linked list of IP prefixes
typedef struct ipvx_prefix_list {
  /// The prefix that this element represents
  ipvx_prefix_t prefix;
  /// The next prefix in the list
  struct ipvx_prefix_list *next;
} ipvx_prefix_list_t;

/// Return the size in bytes of an address with the given family.
#define ipvx_family_size(family) \
  ((family) == AF_INET6 ? sizeof(struct in6_addr) : sizeof(struct in_addr))

/**
 * Set a bit in an IPvX address to 1.
 *
 * @param pfx       Pointer to the address to be modified
 * @param n         The bit index to set (0 is MSB)
 */
static inline void ipvx_set_bit(ipvx_prefix_t *pfx, int n)
{
  pfx->addr._u8[n/8] |= (0x80 >> (n % 8));
}

/**
 * Set a bit in an IPvX address to 0.
 *
 * @param pfx       Pointer to the address to be modified
 * @param n         The bit index to clear (0 is MSB)
 */
static inline void ipvx_clear_bit(ipvx_prefix_t *pfx, int n)
{
  pfx->addr._u8[n/8] &= ~(0x80 >> (n % 8));
}

/**
 * Toggle a bit in an IPvX address.
 *
 * @param pfx       Pointer to the address to be modified
 * @param n         The bit index to toggle (0 is MSB)
 */
static inline void ipvx_toggle_bit(ipvx_prefix_t *pfx, int n)
{
  pfx->addr._u8[n/8] ^= (0x80 >> (n % 8));
}

/**
 * Make sure all bits of a prefix past the masklen are zero.
 *
 * @param pfx    Pointer to the prefix to normalize.
 */
void ipvx_normalize(ipvx_prefix_t *pfx);

/**
 * Parse an IPv4 or IPv6 address string.
 *
 * @param str      Pointer to the string to parse.
 * @param addr     Pointer to where to store the result.
 * @return 0 if successful, -1 for an invalid address string.
 *
 * This function automatically detects IPv4 or IPv6, and sets addr->family
 * and addr->masklen accordingly.
 */
int ipvx_pton_addr(const char *str, ipvx_prefix_t *addr);

/**
 * Parse an IPv4 or IPv6 prefix string.
 *
 * @param str      Pointer to the string to parse.
 * @param pfx      Pointer to where to store the result.
 * @return 0 if successful, -1 for an invalid address string, or -2 for an
 * invalid mask length.
 *
 * The prefix string is an address followed by an optional "/" and mask
 * length.  If the mask length is omitted, it defaults to the maximum allowed
 * by the address family.  The address will be normalized.
 */
int ipvx_pton_pfx(const char *str, ipvx_prefix_t *pfx);

/**
 * Format the address part of an ipvx_prefix_t as a string.
 *
 * @param addr     Pointer to the address.
 * @param buf      Pointer to a character buffer to store the result.
 * @return a pointer to buf if successful, or NULL on error.
 *
 * @p str must point to a buffer of at least INET6_ADDRSTRLEN bytes.
 * (or INET_ADDRSTRLEN, if you know addr->family == AF_INET).
 */
const char *ipvx_ntop_addr(const ipvx_prefix_t *addr, char *buf);

/**
 * Format an ipvx_prefix_t as a string.
 *
 * @param pfx      Pointer to the prefix.
 * @param buf      Pointer to a character buffer to store the result.
 * @return a pointer to buf if successful, or NULL on error.
 *
 * @p str must point to a buffer of at least INET6_ADDRSTRLEN+4 bytes.
 * (or INET_ADDRSTRLEN+3, if you know pfx->family == AF_INET).
 */
const char *ipvx_ntop_pfx(const ipvx_prefix_t *pfx, char *buf);

/**
 * Compute first address in given prefix
 *
 * @param pfx      Pointer to the pfx to calculate the address for
 * @param addr     Pointer to the address to store the result
 */
void ipvx_first_addr(const ipvx_prefix_t *pfx, ipvx_prefix_t *addr);

/**
 * Compute last address in given prefix
 *
 * @param pfx      Pointer to the pfx to calculate the address for
 * @param addr     Pointer to the address to store the result
 */
ipvx_prefix_t *ipvx_last_addr(const ipvx_prefix_t *pfx, ipvx_prefix_t *addr);

/**
 * Test two prefixes for equality
 *
 * @param a      Pointer to first prefix
 * @param b      Pointer to second prefix
 * @return 1 if prefixes are equal, 0 if they are not
 */
static inline int ipvx_pfx_eq(const ipvx_prefix_t *a, const ipvx_prefix_t *b)
{
  return (a->family == b->family) && (a->masklen == b->masklen) &&
    (memcmp(&a->addr, &b->addr, ipvx_family_size(a->family)) == 0);
}

/**
 * Test two addresses for equality
 *
 * @param a      Pointer to first address
 * @param b      Pointer to second address
 * @return 1 if addresses are equal, 0 if they are not
 *
 * The addresses are assumed to have the same family and the maximum mask
 * length.
 */
static inline int ipvx_addr_eq(const ipvx_prefix_t *a, const ipvx_prefix_t *b)
{
  return (memcmp(&a->addr, &b->addr, ipvx_family_size(a->family)) == 0);
}

static inline int ipvx_pfx_contains(const ipvx_prefix_t *parent, const ipvx_prefix_t *child)
{
  const uint8_t *p = parent->addr._u8;
  const uint8_t *c = child->addr._u8;
  const uint16_t m = parent->masklen;

  return (parent->masklen <= child->masklen) &&
    (memcmp(p, c, parent->masklen / 8) == 0) &&
    ((m % 8 == 0) || (p[m/8] == (c[m/8] & ~(0xFF >> (m % 8))))) ;
}

/**
 * Count number of leading bits that are equal in two prefixes.
 *
 * @param a      Pointer to first prefix
 * @param b      Pointer to second prefix
 * @return the number of leading equal bits
 *
 * @p a and @p b must have the same family.  Only bits up to the smaller of
 * the two masklens are counted.
 */
int ipvx_equal_length(const ipvx_prefix_t *a, const ipvx_prefix_t *b);

/**
 * Compute the minimal list of prefixes which this range represents
 *
 * @param lower         Pointer to the lower bound of the range
 * @param upper         Pointer to the upper bound of the range
 * @param[out] pfx_list A linked list of prefixes
 * @return 0 if the list is successfully built, -1 if an error occurs
 *
 * @note The pfx_list returned MUST be free'd using ipvx_prefix_list_free().
 */
int ipvx_range_to_prefix(const ipvx_prefix_t *lower, const ipvx_prefix_t *upper,
    ipvx_prefix_list_t **pfx_list);

/**
 * Free a list of prefixes as returned by ipvx_range_to_prefix()
 *
 * @param pfx_list      The list to free
 */
void ipvx_prefix_list_free(ipvx_prefix_list_t *pfx_list);

#endif // IPVX_UTILS_H
