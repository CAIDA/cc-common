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

static uint8_t ipvx_bytemask[] =
  { 0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff };

static uint8_t ipvx_not_bytemask[] =
  { 0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01, 0x00 };

void ipvx_first_addr(const ipvx_prefix_t *pfx, ipvx_prefix_t *addr)
{
  const uint8_t famsize = ipvx_family_size(pfx->family);
  addr->family = pfx->family;
  addr->masklen = famsize * 8;
  uint8_t *a = (uint8_t*)&addr->addr;
  const uint8_t *p = (const uint8_t*)&pfx->addr;
  unsigned i = pfx->masklen / 8;
  memcpy(a, p, i);
  if (i == famsize)
    return;
  // calculate partial byte
  a[i] = p[i] & ipvx_bytemask[pfx->masklen % 8];
  // set trailing bytes to 0
  memset(a+i+1, 0, famsize - i - 1);
}

void ipvx_last_addr(const ipvx_prefix_t *pfx, ipvx_prefix_t *addr)
{
  uint8_t famsize = ipvx_family_size(pfx->family);
  addr->family = pfx->family;
  addr->masklen = famsize * 8;
  uint8_t *a = (uint8_t*)&addr->addr;
  const uint8_t *p = (const uint8_t*)&pfx->addr;
  unsigned i = pfx->masklen / 8;
  memcpy(a, p, i);
  if (i == famsize)
    return;
  // calculate partial byte
  a[i] = p[i] | ipvx_not_bytemask[pfx->masklen % 8];
  // set trailing bytes to 1
  memset(a+i+1, 0xFF, famsize - i - 1);
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
#define chunk(type, offset, p) \
  (((const type*)(&(p)->addr))[i])
#define chunk_op(type, offset, a, op, b) \
  (chunk(type, offset, a) op chunk(type, offset, b))

  // number of bits we still need to test
  uint8_t nbits = a->masklen < b->masklen ? a->masklen : b->masklen;

  // offset, in units of the current chunk size
  int i = 0;

  // Jump into the search at the largest applicable chunk size, then
  // repeatedly reduce chunk size until we find an unequal byte.
  switch ((nbits + 7) / 8) {
#if 0 // If we knew the addresses were 64-bit-aligned, it would be more
      // efficient to use an uint64 test.
  case 16: case 15: case 14: case 13: case 12: case 11: case 10: case 9:
    if (nbits > 64) {
      if (chunk_op(uint64_t, i, a, ==, b)) { i++; nbits -= 64; }
      else { nbits = 64; }
    }
    i = i << 1; // convert to offset of a uint32
    // fall through

#else // Alas, we know only that they are 32-bit-aligned, so a pair of
      // 32-bit tests are more efficient (and safe).
  case 16: case 15: case 14: case 13:
    if (nbits > 32) {
      if (chunk_op(uint32_t, i, a, ==, b)) { i++; nbits -= 32; }
      else { nbits = 32; }
    }
    // fall through
  case 12: case 11: case 10: case 9:
    if (nbits > 32) {
      if (chunk_op(uint32_t, i, a, ==, b)) { i++; nbits -= 32; }
      else { nbits = 32; }
    }
    // fall through
#endif

  case 8: case 7: case 6: case 5:
    if (nbits > 32) {
      if (chunk_op(uint32_t, i, a, ==, b)) { i++; nbits -= 32; }
      else { nbits = 32; }
    }
    i = i << 1; // convert to offset of a uint16
    // fall through

  case 4: case 3:
    if (nbits > 16) {
      if (chunk_op(uint16_t, i, a, ==, b)) { i++; nbits -= 16; }
      else { nbits = 16; }
    }
    i = i << 1; // convert to offset of a uint8
    // fall through

  case 2:
    if (nbits > 8) {
      if (chunk_op(uint8_t, i, a, ==, b)) { i++; nbits -= 8; }
      else { nbits = 8; }
    }
  }

  // If there are no trailing bits, we're done
  if (nbits == 0)
    return i * 8;
  // find first unequal bit in the final partial byte
  uint8_t unequal_bits = chunk_op(uint8_t, i, a, ^, b);
  unequal_bits |= ipvx_not_bytemask[nbits]; // bits past masklen are "unequal"
  return i * 8 + clz8(unequal_bits);
}

void ipvx_normalize(ipvx_prefix_t *pfx)
{
  unsigned famsize = ipvx_family_size(pfx->family);
  uint8_t *x = (uint8_t*)&pfx->addr;
  unsigned i = (pfx->masklen + 7) / 8;
  // clear trailing bytes
  memset(x + i, 0, famsize - i);
  // if there's a partial byte, clear its trailing bits
  if (pfx->masklen % 8 != 0)
    x[i-1] &= ipvx_bytemask[pfx->masklen % 8];
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
  char dup[INET6_ADDRSTRLEN];
  char *p = strchr(str, '/');
  if (!p)
    return ipvx_pton_addr(str, pfx);
  if (p > str + sizeof(dup) - 1)
    return -1; // invalid addr
  strcpy(dup, str);
  dup[p - str] = '\0';
  p++;
  if (ipvx_pton_addr(dup, pfx) < 0)
    return -1; // invalid addr
  errno = 0;
  char *end = p;
  unsigned long masklen = strtoul(p, &end, 10);
  if (errno || *end || end == p || masklen > pfx->masklen)
    return -2; // invalid prefix len
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

/**
 * Recursively compute network addresses to cover range lo-hi
 *
 * Note: Worst case IPv4 scenario is when lo=0.0.0.1 and hi=255.255.255.254
 *       We then need 62 CIDR blocks to cover this interval, and 125 calls to
 *       split_range(). The maximum possible recursion depth is 32.
 */
static int split_range(const ipvx_prefix_t *pfx,
                       const ipvx_prefix_t *lo, int lo_bounded,
                       const ipvx_prefix_t *hi, int hi_bounded,
                       ipvx_prefix_list_t **pfx_list)
{
#if 0
  char bufpfx[INET6_ADDRSTRLEN+4];
  char buflo[INET6_ADDRSTRLEN+4];
  char bufhi[INET6_ADDRSTRLEN+4];
  fprintf(stderr, "%s(pfx=%s, lo=%s, hi=%s, ...)\n", __func__,
      ipvx_ntop_pfx(pfx, bufpfx), ipvx_ntop_pfx(lo, buflo), ipvx_ntop_pfx(hi, bufhi));
#endif
  // assert(ipvx_pfx_contains(pfx, lo));
  // assert(ipvx_pfx_contains(pfx, hi));
  ipvx_prefix_t subpfx, last;

  do {
    if ((lo_bounded || ipvx_addr_eq(lo, pfx)) &&
        (hi_bounded || (ipvx_last_addr(pfx, &last), ipvx_addr_eq(hi, &last)))) {
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

    if (pfx != &subpfx) {
      subpfx = *pfx;
    }
    subpfx.masklen++; // pfx's lower half

    if (!hi_bounded && ipvx_pfx_contains(&subpfx, hi)) {
      // both lo and hi are in lower half
      pfx = &subpfx;
      // tail recursion

    } else if (!lo_bounded && !ipvx_pfx_contains(&subpfx, lo)) {
      // both lo and hi are in upper half
      ipvx_set_bit(&subpfx, subpfx.masklen - 1); // pfx's upper half
      pfx = &subpfx;
      // tail recursion

    } else {
      // search lower half
      ipvx_last_addr(&subpfx, &last); // last addr in lower half
      if (split_range(&subpfx, lo, lo_bounded, &last, 1, pfx_list) < 0)
        return -1;

      // search upper half
      ipvx_prefix_t first;
      ipvx_set_bit(&subpfx, subpfx.masklen - 1); // pfx's upper half
      ipvx_first_addr(&subpfx, &first); // first addr in upper half
      lo = &first;
      pfx = &subpfx;
      lo_bounded = 1;
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
    ipvx_last_addr(lower, &lower_addr);
    lower = &lower_addr;
  } // else, lower already is its own first address

  if (upper->masklen < maxlen) {
    // get the last address of the upper prefix
    ipvx_last_addr(upper, &upper_addr);
    upper = &upper_addr;
  } // else, upper already is its own last address

  ipvx_prefix_t addr;
#if 0
  // Set starting addr to 0.0.0.0/0 or ::/0
  memset(&addr, 0, sizeof(addr));
  addr.family = lower->family;
#else
  // Set starting addr to longest common prefix
  addr.family = lower->family;
  addr.masklen = ipvx_equal_length(lower, upper);
  memcpy(&addr.addr, &lower->addr, (addr.masklen + 7) / 8);
  ipvx_normalize(&addr);
#endif

  *pfx_list = NULL;
  return split_range(&addr, lower, 0, upper, 0, pfx_list);
}

void ipvx_prefix_list_free(ipvx_prefix_list_t *pfx_list)
{
  while (pfx_list) {
    ipvx_prefix_list_t *temp = pfx_list;
    pfx_list = pfx_list->next;
    free(temp);
  }
}
