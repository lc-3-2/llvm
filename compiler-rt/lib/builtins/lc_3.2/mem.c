//===-- mem.c - Implement memory functions --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// The compiler is allowed to generate calls to `memset`, `memcpy`, and
// `memmove`. Thus, we provide implementations for them here. They are declared
// weak so that they can be overridden by the C library if provided.
//
// At the moment, we don't optimize by using word operations. As such, these can
// be a factor of four slower than they should be.
//
//===----------------------------------------------------------------------===//

#include <stddef.h>
#include <stdint.h>

__attribute__((weak)) void *memset(void *s, int c, size_t n) {

  // Save the original pointer so we can return it later
  // Also cast the current pointer to char
  void *ret = s;
  char *cur = (char *)s;

  // Do the set
  for (; n != 0; cur++, n--)
    *cur = c;

  // Return the saved pointer
  return ret;
}

__attribute__((weak)) void *memcpy(void *restrict dest,
                                   const void *restrict src, size_t n) {

  // Save the original destination so we can return it later
  // Also cast both pointers to char
  void *ret = dest;
  char *d = (char *)dest;
  const char *s = (const char *)src;

  // Do the copy
  for (; n != 0; d++, s++, n--)
    *d = *s;

  return ret;
}

__attribute__((weak)) void *memmove(void *dest, const void *src, size_t n) {

  // Save the original destination so we can return it later
  // Also cast both pointers to char
  void *ret = dest;
  char *d = (char *)dest;
  const char *s = (const char *)src;

  // Determine the direction of the copy
  // If the destination is to the left, we copy left-to-right
  // Otherwise, right-to-left
  if ((intptr_t)dest <= (intptr_t)src) {
    // ->
    for (; n != 0; d++, s++, n--)
      *d = *s;

  } else {
    // <-
    d += n - 1;
    s += n - 1;
    for (; n != 0; d--, s--, n--)
      *d = *s;
  }

  return ret;
}
