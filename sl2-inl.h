#pragma once

#include "gf2p127-inl.h"
#include <stdio.h>
#include <string.h>

typedef gf2p127_t sl2_t[2][2];
typedef gf2p127_t (*psl2_t)[2];

#define sl2_elem_get(x, idx1, idx2) (_mm_loadu_si128(&(x)[(idx1)][(idx2)]))
#define sl2_elem_set(x, idx1, idx2, v) (_mm_storeu_si128(&(x)[(idx1)][(idx2)], (v)))

static inline
_Bool sl2_valid(sl2_t a) {
  gf2p127_t det = gf2p127_add(gf2p127_mul(sl2_elem_get(a, 0, 0), sl2_elem_get(a, 1, 1)),
                              gf2p127_mul(sl2_elem_get(a, 0, 1), sl2_elem_get(a, 1, 0)));
  return _mm_extract_epi64(det, 0) == 1 &&
         _mm_extract_epi64(det, 1) == 0 &&
         gf2p127_valid(sl2_elem_get(a, 0, 0)) &&
         gf2p127_valid(sl2_elem_get(a, 0, 1)) &&
         gf2p127_valid(sl2_elem_get(a, 1, 0)) &&
         gf2p127_valid(sl2_elem_get(a, 1, 1));
}

static inline
_Bool sl2_eq(sl2_t a, sl2_t b) {
  return gf2p127_eq(sl2_elem_get(a, 0, 0), sl2_elem_get(b, 0, 0)) &&
         gf2p127_eq(sl2_elem_get(a, 0, 1), sl2_elem_get(b, 0, 1)) &&
         gf2p127_eq(sl2_elem_get(a, 1, 0), sl2_elem_get(b, 1, 0)) &&
         gf2p127_eq(sl2_elem_get(a, 1, 1), sl2_elem_get(b, 1, 1));
}

static inline
int sl2_cmp(sl2_t a, sl2_t b) {
  uint64_t *ua = (uint64_t *)a;
  uint64_t *ub = (uint64_t *)b;
  int i;
  for (i = 0; i < sizeof(sl2_t) / sizeof(uint64_t); i++) {
    if (ua[i] == ub[i]) {
      continue;
    }
    if (ua[i] > ub[i]) {
      return 1;
    }
    if (ua[i] < ub[i]) {
      return -1;
    }
  }
  return 0;
}

static inline
void sl2_copy(sl2_t dst, sl2_t src) {
  memcpy(dst, src, sizeof(sl2_t));
}

#ifdef __AVX2__
static inline
void sl2_mul_bit_left_x2(gf2p127x2_t *b00b01, gf2p127x2_t *b10b11, gf2p127_t bits) {
  // A: {00 = 10, 01 = 01, 10 = 01, 11 = 00}
  // B: {00 = 10, 01 = 11, 10 = 01, 11 = 01}
  gf2p127x2_t bb = _mm256_broadcastsi128_si256(bits);
  gf2p127x2_t masked = _mm256_and_si256(*b10b11, bb);
  gf2p127x2_t maskedadd = _mm256_xor_si256(*b00b01, masked);
  gf2p127x2_t mul = gf2p127x2_mul_10(maskedadd);
  gf2p127x2_t muladd = _mm256_xor_si256(*b10b11, mul);
  *b10b11 = maskedadd;
  *b00b01 = muladd;
}
#endif

static inline
void sl2_mul_bit_left(gf2p127_t *b00, gf2p127_t *b01, gf2p127_t *b10, gf2p127_t *b11, gf2p127_t bits) {
  // A: {00 = 10, 01 = 01, 10 = 01, 11 = 00}
  // B: {00 = 10, 01 = 11, 10 = 01, 11 = 01}
  gf2p127_t b10_ = *b10;
  gf2p127_t b11_ = *b11;
  *b10 = gf2p127_add(*b00, _mm_and_si128(*b10, bits));
  *b11 = gf2p127_add(*b01, _mm_and_si128(*b11, bits));
  *b00 = gf2p127_add(b10_, gf2p127_mul_10(*b10));
  *b01 = gf2p127_add(b11_, gf2p127_mul_10(*b11));
}

#ifdef __AVX2__
static inline
void sl2_mul_bits_left_x2(gf2p127x2_t *b00b01, gf2p127x2_t *b10b11, unsigned char byte) {
  sl2_mul_bit_left_x2(b00b01, b10b11, _mm_load_si128(&minmax[(byte >> 0) & 1]));
  sl2_mul_bit_left_x2(b00b01, b10b11, _mm_load_si128(&minmax[(byte >> 1) & 1]));
  sl2_mul_bit_left_x2(b00b01, b10b11, _mm_load_si128(&minmax[(byte >> 2) & 1]));
  sl2_mul_bit_left_x2(b00b01, b10b11, _mm_load_si128(&minmax[(byte >> 3) & 1]));
  sl2_mul_bit_left_x2(b00b01, b10b11, _mm_load_si128(&minmax[(byte >> 4) & 1]));
  sl2_mul_bit_left_x2(b00b01, b10b11, _mm_load_si128(&minmax[(byte >> 5) & 1]));
  sl2_mul_bit_left_x2(b00b01, b10b11, _mm_load_si128(&minmax[(byte >> 6) & 1]));
  sl2_mul_bit_left_x2(b00b01, b10b11, _mm_load_si128(&minmax[(byte >> 7) & 1]));
}
#endif

static inline
void sl2_mul_bits_left(gf2p127_t *b00, gf2p127_t *b01, gf2p127_t *b10, gf2p127_t *b11, unsigned char byte) {
  sl2_mul_bit_left(b00, b01, b10, b11, _mm_load_si128(&minmax[(byte >> 0) & 1]));
  sl2_mul_bit_left(b00, b01, b10, b11, _mm_load_si128(&minmax[(byte >> 1) & 1]));
  sl2_mul_bit_left(b00, b01, b10, b11, _mm_load_si128(&minmax[(byte >> 2) & 1]));
  sl2_mul_bit_left(b00, b01, b10, b11, _mm_load_si128(&minmax[(byte >> 3) & 1]));
  sl2_mul_bit_left(b00, b01, b10, b11, _mm_load_si128(&minmax[(byte >> 4) & 1]));
  sl2_mul_bit_left(b00, b01, b10, b11, _mm_load_si128(&minmax[(byte >> 5) & 1]));
  sl2_mul_bit_left(b00, b01, b10, b11, _mm_load_si128(&minmax[(byte >> 6) & 1]));
  sl2_mul_bit_left(b00, b01, b10, b11, _mm_load_si128(&minmax[(byte >> 7) & 1]));
}

#ifdef __AVX2__
static inline
void sl2_mul_buf_left(sl2_t b, unsigned char *buf, size_t n) {
  gf2p127x2_t b00b01 = _mm256_loadu2_m128i(&b[0][0], &b[0][1]);
  gf2p127x2_t b10b11 = _mm256_loadu2_m128i(&b[1][0], &b[1][1]);
  size_t i;
  for (i = n; i > 0; i--) {
    sl2_mul_bits_left_x2(&b00b01, &b10b11, buf[i - 1]);
  }
  _mm256_storeu2_m128i(&b[0][0], &b[0][1], b00b01);
  _mm256_storeu2_m128i(&b[1][0], &b[1][1], b10b11);
}
#else
static inline
void sl2_mul_buf_left(sl2_t b, unsigned char *buf, size_t n) {
  gf2p127_t b00 = sl2_elem_get(b, 0, 0);
  gf2p127_t b01 = sl2_elem_get(b, 0, 1);
  gf2p127_t b10 = sl2_elem_get(b, 1, 0);
  gf2p127_t b11 = sl2_elem_get(b, 1, 1);
  size_t i;
  for (i = n; i > 0; i--) {
    sl2_mul_bits_left(&b00, &b01, &b10, &b11, buf[i - 1]);
  }
  sl2_elem_set(b, 0, 0, b00);
  sl2_elem_set(b, 0, 1, b01);
  sl2_elem_set(b, 1, 0, b10);
  sl2_elem_set(b, 1, 1, b11);
}
#endif

#ifdef __AVX2__
static inline
void sl2_mul_bit_right_x2(gf2p127x2_t *a00a10, gf2p127x2_t *a01a11, gf2p127_t bits) {
  // A: {00 = 10, 01 = 01, 10 = 01, 11 = 00}
  // B: {00 = 10, 01 = 11, 10 = 01, 11 = 01}
  gf2p127x2_t bb = _mm256_broadcastsi128_si256(bits);
  gf2p127x2_t mul = gf2p127x2_mul_10(*a00a10);
  gf2p127x2_t muladd = _mm256_xor_si256(mul, *a01a11);
  gf2p127x2_t masked = _mm256_and_si256(muladd, bb);
  gf2p127x2_t maskedadd = _mm256_xor_si256(masked, *a00a10);
  *a01a11 = maskedadd;
  *a00a10 = muladd;
}
#endif

static inline
void sl2_mul_bit_right(gf2p127_t *a00, gf2p127_t *a01, gf2p127_t *a10, gf2p127_t *a11, gf2p127_t bits) {
  // A: {00 = 10, 01 = 01, 10 = 01, 11 = 00}
  // B: {00 = 10, 01 = 11, 10 = 01, 11 = 01}
  gf2p127_t a00_ = *a00;
  gf2p127_t a10_ = *a10;
  *a00 = gf2p127_add(gf2p127_mul_10(*a00), *a01);
  *a10 = gf2p127_add(gf2p127_mul_10(*a10), *a11);
  *a01 = gf2p127_add(a00_, _mm_and_si128(*a00, bits));
  *a11 = gf2p127_add(a10_, _mm_and_si128(*a10, bits));
}

#ifdef __AVX2__
static inline
void sl2_mul_bits_right_x2(gf2p127x2_t *a00a01, gf2p127x2_t *a10a11, unsigned char byte) {
  sl2_mul_bit_right_x2(a00a01, a10a11, _mm_load_si128(&minmax[(byte >> 7) & 1]));
  sl2_mul_bit_right_x2(a00a01, a10a11, _mm_load_si128(&minmax[(byte >> 6) & 1]));
  sl2_mul_bit_right_x2(a00a01, a10a11, _mm_load_si128(&minmax[(byte >> 5) & 1]));
  sl2_mul_bit_right_x2(a00a01, a10a11, _mm_load_si128(&minmax[(byte >> 4) & 1]));
  sl2_mul_bit_right_x2(a00a01, a10a11, _mm_load_si128(&minmax[(byte >> 3) & 1]));
  sl2_mul_bit_right_x2(a00a01, a10a11, _mm_load_si128(&minmax[(byte >> 2) & 1]));
  sl2_mul_bit_right_x2(a00a01, a10a11, _mm_load_si128(&minmax[(byte >> 1) & 1]));
  sl2_mul_bit_right_x2(a00a01, a10a11, _mm_load_si128(&minmax[(byte >> 0) & 1]));
}
#endif

static inline
void sl2_mul_bits_right(gf2p127_t *a00, gf2p127_t *a01, gf2p127_t *a10, gf2p127_t *a11, unsigned char byte) {
  sl2_mul_bit_right(a00, a01, a10, a11, _mm_load_si128(&minmax[(byte >> 7) & 1]));
  sl2_mul_bit_right(a00, a01, a10, a11, _mm_load_si128(&minmax[(byte >> 6) & 1]));
  sl2_mul_bit_right(a00, a01, a10, a11, _mm_load_si128(&minmax[(byte >> 5) & 1]));
  sl2_mul_bit_right(a00, a01, a10, a11, _mm_load_si128(&minmax[(byte >> 4) & 1]));
  sl2_mul_bit_right(a00, a01, a10, a11, _mm_load_si128(&minmax[(byte >> 3) & 1]));
  sl2_mul_bit_right(a00, a01, a10, a11, _mm_load_si128(&minmax[(byte >> 2) & 1]));
  sl2_mul_bit_right(a00, a01, a10, a11, _mm_load_si128(&minmax[(byte >> 1) & 1]));
  sl2_mul_bit_right(a00, a01, a10, a11, _mm_load_si128(&minmax[(byte >> 0) & 1]));
}

#ifdef __AVX2__
static inline
void sl2_mul_buf_right(sl2_t a, unsigned char *buf, size_t n) {
  size_t i;
  gf2p127x2_t a00a10 = _mm256_loadu2_m128i(&a[0][0], &a[1][0]);
  gf2p127x2_t a01a11 = _mm256_loadu2_m128i(&a[0][1], &a[1][1]);

  for (i = 0; i < n; i++) {
    sl2_mul_bits_right_x2(&a00a10, &a01a11, buf[i]);
  }
  _mm256_storeu2_m128i(&a[0][1], &a[1][1], a01a11);
  _mm256_storeu2_m128i(&a[0][0], &a[1][0], a00a10);
}
#else
static inline
void sl2_mul_buf_right(sl2_t a, unsigned char *buf, size_t n) {
  gf2p127_t a00 = sl2_elem_get(a, 0, 0);
  gf2p127_t a01 = sl2_elem_get(a, 0, 1);
  gf2p127_t a10 = sl2_elem_get(a, 1, 0);
  gf2p127_t a11 = sl2_elem_get(a, 1, 1);
  size_t i;
  for (i = 0; i < n; i++) {
    sl2_mul_bits_right(&a00, &a01, &a10, &a11, buf[i]);
  }
  sl2_elem_set(a, 0, 0, a00);
  sl2_elem_set(a, 0, 1, a01);
  sl2_elem_set(a, 1, 0, a10);
  sl2_elem_set(a, 1, 1, a11);
}
#endif

static inline
void sl2_mul(sl2_t c, sl2_t a, sl2_t b) {
  // Strassen algorithm
  gf2p127_t m0, m1, m2, m3, m4, m5, m6;
  m0 = gf2p127_mul(gf2p127_add(sl2_elem_get(a, 0, 0), sl2_elem_get(a, 1, 1)),
                   gf2p127_add(sl2_elem_get(b, 0, 0), sl2_elem_get(b, 1, 1)));
  m1 = gf2p127_mul(gf2p127_add(sl2_elem_get(a, 1, 0), sl2_elem_get(a, 1, 1)), sl2_elem_get(b, 0, 0));
  m2 = gf2p127_mul(sl2_elem_get(a, 0, 0), gf2p127_add(sl2_elem_get(b, 0, 1), sl2_elem_get(b, 1, 1)));
  m3 = gf2p127_mul(sl2_elem_get(a, 1, 1), gf2p127_add(sl2_elem_get(b, 1, 0), sl2_elem_get(b, 0, 0)));
  m4 = gf2p127_mul(gf2p127_add(sl2_elem_get(a, 0, 0), sl2_elem_get(a, 0, 1)), sl2_elem_get(b, 1, 1));
  m5 = gf2p127_mul(gf2p127_add(sl2_elem_get(a, 1, 0), sl2_elem_get(a, 0, 0)),
                   gf2p127_add(sl2_elem_get(b, 0, 0), sl2_elem_get(b, 0, 1)));
  m6 = gf2p127_mul(gf2p127_add(sl2_elem_get(a, 0, 1), sl2_elem_get(a, 1, 1)),
                   gf2p127_add(sl2_elem_get(b, 1, 0), sl2_elem_get(b, 1, 1)));
  sl2_elem_set(c, 0, 0, gf2p127_add(gf2p127_add(m0, m3), gf2p127_add(m4, m6)));
  sl2_elem_set(c, 0, 1, gf2p127_add(m2, m4));
  sl2_elem_set(c, 1, 0, gf2p127_add(m1, m3));
  sl2_elem_set(c, 1, 1, gf2p127_add(gf2p127_add(m0, m1), gf2p127_add(m2, m5)));
}

static inline
void sl2_mul_optimized(sl2_t c, sl2_t a, sl2_t b) {
  gf2p127_t t0, t1, t2, t3, t4, t5;
  t0 = gf2p127_add(sl2_elem_get(a, 1, 0), sl2_elem_get(a, 1, 1));
  t1 = gf2p127_add(sl2_elem_get(a, 1, 1), sl2_elem_get(a, 0, 1));
  t2 = gf2p127_add(sl2_elem_get(a, 1, 1), sl2_elem_get(a, 0, 0));
  t3 = gf2p127_add(sl2_elem_get(b, 1, 1), sl2_elem_get(b, 0, 0));
  t4 = gf2p127_add(sl2_elem_get(b, 1, 0), sl2_elem_get(b, 1, 1));
  t5 = gf2p127_add(sl2_elem_get(b, 1, 1), sl2_elem_get(b, 0, 1));
  
  gf2p127_t M0, M1, M2, M3, M4, M5, M6;
  M0 = gf2p127_mul(sl2_elem_get(a, 0, 0), sl2_elem_get(b, 0, 0));
  M1 = gf2p127_mul(sl2_elem_get(a, 0, 1), sl2_elem_get(b, 1, 0));
  M2 = gf2p127_mul(sl2_elem_get(a, 1, 0), t3);
  M3 = gf2p127_mul(sl2_elem_get(a, 1, 1), sl2_elem_get(b, 1, 1));
  M4 = gf2p127_mul(t0, t4);
  M5 = gf2p127_mul(t1, t5);
  M6 = gf2p127_mul(t2, sl2_elem_get(b, 0, 1));

  sl2_elem_set(c, 0, 0, gf2p127_add(M0, M1));
  sl2_elem_set(c, 0, 1, gf2p127_add(M4, M6));
  sl2_elem_set(c, 1, 0, gf2p127_add(M2, M5));
  sl2_elem_set(c, 1, 1, gf2p127_add(gf2p127_add(M4, M5), gf2p127_add(M1, M3)));

  // gf2p127_hex(new_c00, sl2_elem_get(c, 0, 0));
  // gf2p127_hex(new_c01, sl2_elem_get(c, 0, 1));
  // gf2p127_hex(new_c10, sl2_elem_get(c, 1, 0));
  // gf2p127_hex(new_c11, sl2_elem_get(c, 1, 1));

  // gf2p127_t m0, m1, m2, m3, m4, m5, m6;
  // m0 = gf2p127_mul(gf2p127_add(sl2_elem_get(a, 0, 0), sl2_elem_get(a, 1, 1)),
  //                  gf2p127_add(sl2_elem_get(b, 0, 0), sl2_elem_get(b, 1, 1)));
  // m1 = gf2p127_mul(gf2p127_add(sl2_elem_get(a, 1, 0), sl2_elem_get(a, 1, 1)), sl2_elem_get(b, 0, 0));
  // m2 = gf2p127_mul(sl2_elem_get(a, 0, 0), gf2p127_add(sl2_elem_get(b, 0, 1), sl2_elem_get(b, 1, 1)));
  // m3 = gf2p127_mul(sl2_elem_get(a, 1, 1), gf2p127_add(sl2_elem_get(b, 1, 0), sl2_elem_get(b, 0, 0)));
  // m4 = gf2p127_mul(gf2p127_add(sl2_elem_get(a, 0, 0), sl2_elem_get(a, 0, 1)), sl2_elem_get(b, 1, 1));
  // m5 = gf2p127_mul(gf2p127_add(sl2_elem_get(a, 1, 0), sl2_elem_get(a, 0, 0)),
  //                  gf2p127_add(sl2_elem_get(b, 0, 0), sl2_elem_get(b, 0, 1)));
  // m6 = gf2p127_mul(gf2p127_add(sl2_elem_get(a, 0, 1), sl2_elem_get(a, 1, 1)),
  //                  gf2p127_add(sl2_elem_get(b, 1, 0), sl2_elem_get(b, 1, 1)));

  // gf2p127_hex(old_c00, gf2p127_add(gf2p127_add(m0, m3), gf2p127_add(m4, m6)));
  // gf2p127_hex(old_c01, gf2p127_add(m2, m4));
  // gf2p127_hex(old_c10, gf2p127_add(m1, m3));
  // gf2p127_hex(old_c11, gf2p127_add(gf2p127_add(m0, m1), gf2p127_add(m2, m5)));

  // if (strcmp(old_c00, new_c00) != 0)
  //   printf("old_c00: %s, new_c00: %s\n", old_c00, new_c00);
  // if (strcmp(old_c01, new_c01) != 0)
  //   printf("old_c01: %s, new_c01: %s\n", old_c01, new_c01);
  // if (strcmp(old_c10, new_c10) != 0)
  //   printf("old_c10: %s, new_c10: %s\n", old_c10, new_c10);
  // if (strcmp(old_c11, new_c11) != 0)
  //   printf("old_c11: %s, new_c11: %s\n", old_c11, new_c11);
  // printf("====================================\n");

  // sl2_elem_get(c, 0, 0) = gf2p127_add(gf2p127_add(m0, m3), gf2p127_add(m4, m6));
  // sl2_elem_get(c, 0, 1) = gf2p127_add(m2, m4);
  // sl2_elem_get(c, 1, 0) = gf2p127_add(m1, m3);
  // sl2_elem_get(c, 1, 1) = gf2p127_add(gf2p127_add(m0, m1), gf2p127_add(m2, m5));
}


static inline
void sl2_mul_byte_left(sl2_t b, unsigned char byte, sl2_t m[256]) {
  sl2_mul(m[byte], b, b);
}

static inline
void sl2_mul_byte_right(sl2_t a, unsigned char byte, sl2_t m[256]) {
  sl2_mul(a, m[byte], a);
}

static inline
void sl2_init(sl2_t m[2]) {
  sl2_elem_set(m[0], 0, 0, gf2p127_from_int(2));
  sl2_elem_set(m[0], 0, 1, gf2p127_from_int(1));
  sl2_elem_set(m[0], 1, 0, gf2p127_from_int(1));
  sl2_elem_set(m[0], 1, 1, gf2p127_from_int(0));
  sl2_elem_set(m[1], 0, 0, gf2p127_from_int(2));
  sl2_elem_set(m[1], 0, 1, gf2p127_from_int(3));
  sl2_elem_set(m[1], 1, 0, gf2p127_from_int(1));
  sl2_elem_set(m[1], 1, 1, gf2p127_from_int(1));
}

static inline
void sl2_unit(sl2_t a) {
  sl2_elem_set(a, 0, 0, gf2p127_from_int(1));
  sl2_elem_set(a, 0, 1, gf2p127_from_int(0));
  sl2_elem_set(a, 1, 0, gf2p127_from_int(0));
  sl2_elem_set(a, 1, 1, gf2p127_from_int(1));
}

static inline
char *sl2_hex(char *buf, sl2_t a) {
  gf2p127_hex(&buf[0],  sl2_elem_get(a, 0, 0));
  gf2p127_hex(&buf[32], sl2_elem_get(a, 0, 1));
  gf2p127_hex(&buf[64], sl2_elem_get(a, 1, 0));
  gf2p127_hex(&buf[96], sl2_elem_get(a, 1, 1));
  return buf;
}

static const unsigned char b64[] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

static const unsigned char unb64[256] = {
  ['A'] =  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,
          13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
  ['a'] = 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
          39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
  ['0'] = 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
  ['-'] = 62, ['_'] = 63
};

static inline
void sl2_serialize(sl2_t m, unsigned char buf[86]) {

  int i, j;
  unsigned char a, b, c, *data = (unsigned char *)m;

  for (i = j = 0; i <= 64 - 3; i += 3, j += 4) {
    a = data[i + 0];
    b = data[i + 1];
    c = data[i + 2];
    buf[j + 0] = b64[a >> 2];
    buf[j + 1] = b64[((0x03 & a) << 4) + (b >> 4)];
    buf[j + 2] = b64[((0x0f & b) << 2) + (c >> 6)];
    buf[j + 3] = b64[0x3f & c];
  }

  buf[84] = b64[data[i] >> 2];
  buf[85] = b64[(0x3 & data[i]) << 4];

}

static inline
void sl2_unserialize(sl2_t m, unsigned char buf[86]) {

  int i, j;
  unsigned char a, b, c, d, *data = (unsigned char *)m;

  for (i = j = 0; i <= 86 - 4; i += 4, j += 3) {
    a = unb64[buf[i + 0]];
    b = unb64[buf[i + 1]];
    c = unb64[buf[i + 2]];
    d = unb64[buf[i + 3]];
    data[j + 0] = (a << 2) | (b >> 4);
    data[j + 1] = (b << 4) | (c >> 2);
    data[j + 2] = (c << 6) | (d);
  }

  data[63] = (unb64[buf[i]] << 2) | (unb64[buf[i + 1]] >> 4);

}
