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
 * @author Ken Keys
 */

#include "config.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>

#include "ipvx_utils.h"

// byte-sized netmask
static inline uint8_t netmask8(int n) { return ~(0xFF >> n); }

// byte-sized hostmask
static inline uint8_t hostmask8(int n) { return 0xFF >> n; }

void ipvx_first_addr(const ipvx_prefix_t *pfx, ipvx_prefix_t *addr)
{
  const uint8_t famsize = ipvx_family_size(pfx->family);
  addr->family = pfx->family;
  addr->masklen = famsize * 8;
  unsigned i = pfx->masklen / 8;
  memcpy(&addr->addr, &pfx->addr, i);
  if (i == famsize)
    return;
  // calculate partial byte
  addr->addr._u8[i] = pfx->addr._u8[i] & netmask8(pfx->masklen % 8);
  // set trailing bytes to 0
  memset(addr->addr._u8+i+1, 0, famsize - i - 1);
}

ipvx_prefix_t *ipvx_last_addr(const ipvx_prefix_t *pfx, ipvx_prefix_t *addr)
{
  uint8_t famsize = ipvx_family_size(pfx->family);
  addr->family = pfx->family;
  addr->masklen = famsize * 8;
  unsigned i = pfx->masklen / 8;
  memcpy(&addr->addr, &pfx->addr, i);
  if (i == famsize)
    return addr;
  // calculate partial byte
  addr->addr._u8[i] = pfx->addr._u8[i] | hostmask8(pfx->masklen % 8);
  // set trailing bytes to 1
  memset(addr->addr._u8+i+1, 0xFF, famsize - i - 1);
  return addr;
}

// count leading zeros in a uint8_t
static inline int clz8(uint8_t x)
{
  if (!x) return 8;
  int n = 0;
#if 0
  // linear search: O(n)
  while ((x & (0x80 >> n)) == 0)
    n++;
#else
  // binary search: O(log n)
  if ((x & 0xF0) == 0) { n += 4; x = (uint8_t)(x << 4); }
  if ((x & 0xC0) == 0) { n += 2; x = (uint8_t)(x << 2); }
  if ((x & 0x80) == 0) { n += 1; }
#endif
  return n;
}

int ipvx_equal_length(const ipvx_prefix_t *a, const ipvx_prefix_t *b)
{
  // number of bits we still need to test
  uint8_t nbits = a->masklen < b->masklen ? a->masklen : b->masklen;

  // offset, in units of the current chunk size
  int i = 0;

  // Jump into the search at the largest applicable chunk size, then
  // repeatedly reduce chunk size until we find an unequal byte.
  switch ((nbits + 7) / 8) {
#if 0 // If we knew the addresses were 64-bit-aligned, it would be most
      // efficient to start with a uint64 test.
  case 16: case 15: case 14: case 13: case 12: case 11: case 10: case 9:
    if (nbits > 64) {
      if (a->addr.u64[i] == b->addr.u64[i]) { i++; nbits -= 64; }
      else { nbits = 64; }
    }
    i = i << 1; // convert to offset of a uint32
    // fall through

#else // Alas, we know only that they are 32-bit-aligned, so we start with
      // uint32 tests.
  case 16: case 15: case 14: case 13:
    if (nbits > 32) {
      if (a->addr._u32[i] == b->addr._u32[i]) { i++; nbits -= 32; }
      else { nbits = 32; }
    }
    // fall through
  case 12: case 11: case 10: case 9:
    if (nbits > 32) {
      if (a->addr._u32[i] == b->addr._u32[i]) { i++; nbits -= 32; }
      else { nbits = 32; }
    }
    // fall through
#endif

  case 8: case 7: case 6: case 5:
    if (nbits > 32) {
      if (a->addr._u32[i] == b->addr._u32[i]) { i++; nbits -= 32; }
      else { nbits = 32; }
    }
    i = i << 1; // convert to offset of a uint16
    // fall through

  case 4: case 3:
    if (nbits > 16) {
      if (a->addr._u16[i] == b->addr._u16[i]) { i++; nbits -= 16; }
      else { nbits = 16; }
    }
    i = i << 1; // convert to offset of a uint8
    // fall through

  case 2:
    if (nbits > 8) {
      if (a->addr._u8[i] == b->addr._u8[i]) { i++; nbits -= 8; }
      else { nbits = 8; }
    }
  }

  // If there are no trailing bits, we're done
  if (nbits == 0)
    return i * 8;
  // find first unequal bit in the final partial byte
  uint8_t unequal_bits = a->addr._u8[i] ^ b->addr._u8[i];
  unequal_bits |= hostmask8(nbits); // bits past masklen are "unequal"
  return i * 8 + clz8(unequal_bits);
}

void ipvx_normalize(ipvx_prefix_t *pfx)
{
  unsigned famsize = ipvx_family_size(pfx->family);
  unsigned i = (pfx->masklen + 7) / 8;
  // clear trailing bytes
  memset(&pfx->addr._u8[i], 0, famsize - i);
  // if there's a partial byte, clear its trailing bits
  if (pfx->masklen % 8 != 0)
    pfx->addr._u8[i-1] &= netmask8(pfx->masklen % 8);
}

int ipvx_pton_addr(const char *str, ipvx_prefix_t *pfx)
{
  if (inet_pton(AF_INET, str, &pfx->addr.v4) == 1) {
    pfx->family = AF_INET;
    pfx->masklen = sizeof(struct in_addr) * 8;
  } else if (inet_pton(AF_INET6, str, &pfx->addr.v6) == 1) {
    pfx->family = AF_INET6;
    pfx->masklen = sizeof(struct in6_addr) * 8;
  } else {
    return -1;
  }
  return 0;
}

int ipvx_pton_pfx(const char *str, ipvx_prefix_t *pfx)
{
  int rc;
  char dup[INET6_ADDRSTRLEN];
  char *p = strchr(str, '/');
  if (!p)
    return ipvx_pton_addr(str, pfx);
  if (p > str + sizeof(dup) - 1) // addr is too long
    return IPVX_ERR_INVALID_ADDR;
  strcpy(dup, str);
  dup[p - str] = '\0';
  p++;
  if ((rc = ipvx_pton_addr(dup, pfx)) < 0)
    return rc;
  errno = 0;
  char *end = p;
  unsigned long masklen = strtoul(p, &end, 10);
  if (errno || *end || end == p || masklen > pfx->masklen)
    return IPVX_ERR_INVALID_MASKLEN;
  pfx->masklen = (uint8_t)masklen;
  ipvx_normalize(pfx);
  return 0; // ok
}

const char *ipvx_ntop_addr(const ipvx_prefix_t *addr, char *buf)
{
  return inet_ntop(addr->family, &addr->addr, buf, INET6_ADDRSTRLEN);
}

const char *ipvx_ntop_pfx(const ipvx_prefix_t *pfx, char *buf)
{
  if (!inet_ntop(pfx->family, &pfx->addr, buf, INET6_ADDRSTRLEN))
    return NULL;
  char *p = buf + strlen(buf);
  snprintf(p, 5, "/%d", pfx->masklen);
  return buf;
}

// (nth bit of a) == (nth bit of b)
static inline int ipvx_bit_eq(const ipvx_prefix_t *a, const ipvx_prefix_t *b, int n)
{
  return !((a->addr._u8[n/8] ^ b->addr._u8[n/8]) & (0x80 >> (n%8)));
}

// Is addr the last addr in its prefix?
static int ipvx_addr_is_last_in_pfx(const ipvx_prefix_t *addr, uint8_t masklen)
{
  int i = masklen / 8;
  if (masklen % 8) {
    if ((addr->addr._u8[i] | netmask8(masklen % 8)) != 0xFF) {
      return 0;
    }
    i++;
  }
  int famsize = ipvx_family_size(addr->family);
  for (; i < famsize; i++) {
    if (addr->addr._u8[i] != 0xFF) {
      return 0;
    }
  }
  return 1;
}

/**
 * Recursively compute network addresses to cover range lo-hi
 *
 * If lo (or hi) is NULL, it is treated as if it were the first (or last)
 * possible address of pfx.  This allows us to skip some calculations for first
 * and last addresses.
 *
 * Note: Worst case IPv4 scenario is when lo=0.0.0.1 and hi=255.255.255.254
 *       We then need 62 CIDR blocks to cover this interval, and 125 calls to
 *       split_range(). The maximum possible recursion depth is 32.
 */
static int split_range(const ipvx_prefix_t *pfx,
                       const ipvx_prefix_t *lo,
                       const ipvx_prefix_t *hi,
                       ipvx_prefix_list_t **pfx_list)
{
  ipvx_prefix_t subpfx;

  do {
    if ((!lo || ipvx_addr_eq(lo, pfx)) &&
        (!hi || ipvx_addr_is_last_in_pfx(hi, pfx->masklen))) {
      // This pfx exactly matches [lo,hi]; add it to the list.
      ipvx_prefix_list_t *new_node;
      if (!(new_node = malloc(sizeof(ipvx_prefix_list_t)))) {
        return -1;
      }
      new_node->prefix = *pfx;
      new_node->next = *pfx_list;
      *pfx_list = new_node;
      return 0;
    }

    int bitnum = pfx->masklen;
    if (pfx != &subpfx) {
      subpfx = *pfx;
    }
    subpfx.masklen++; // pfx's lower half

    if (hi && ipvx_bit_eq(&subpfx, hi, bitnum)) {
      // hi is in lower half (and since lo < hi, so is lo)
      pfx = &subpfx;
      // tail recursion

    } else if (lo && !ipvx_bit_eq(&subpfx, lo, bitnum)) {
      // lo is in upper half (and since hi > lo, so is hi)
      ipvx_set_bit(&subpfx, subpfx.masklen - 1); // pfx's upper half
      pfx = &subpfx;
      // tail recursion

    } else {
      // search lower half
      if (split_range(&subpfx, lo, NULL, pfx_list) < 0)
        return -1;

      // search upper half
      ipvx_set_bit(&subpfx, subpfx.masklen - 1); // pfx's upper half
      lo = NULL;
      pfx = &subpfx;
      // tail recursion
    }
  } while (1);
}

int ipvx_range_to_prefix(const ipvx_prefix_t *lower, const ipvx_prefix_t *upper,
                       ipvx_prefix_list_t **pfx_list)
{
  const uint8_t maxlen = ipvx_family_size(lower->family) * 8;
  ipvx_prefix_t lower_addr, upper_addr;

  if (lower->masklen < maxlen) {
    // get the first address of the lower prefix
    ipvx_first_addr(lower, &lower_addr);
    lower = &lower_addr;
  } // else, lower already is its own first address

  if (upper->masklen < maxlen) {
    // get the last address of the upper prefix
    ipvx_last_addr(upper, &upper_addr);
    upper = &upper_addr;
  } // else, upper already is its own last address

  // Set starting pfx to longest common prefix
  ipvx_prefix_t pfx;
  pfx.family = lower->family;
  pfx.masklen = ipvx_equal_length(lower, upper);
  memcpy(&pfx.addr, &lower->addr, (pfx.masklen + 7) / 8);
  ipvx_normalize(&pfx);

  *pfx_list = NULL;
  return split_range(&pfx, lower, upper, pfx_list);
}

void ipvx_prefix_list_free(ipvx_prefix_list_t *pfx_list)
{
  while (pfx_list) {
    ipvx_prefix_list_t *temp = pfx_list;
    pfx_list = pfx_list->next;
    free(temp);
  }
}
