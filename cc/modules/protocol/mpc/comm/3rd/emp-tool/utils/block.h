#ifndef UTIL_BLOCK_H__
#define UTIL_BLOCK_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <wmmintrin.h>
#include <assert.h>
#include <iomanip>

namespace emp {

using block = __m128i;

inline bool getLSB(const block & x) {
	return (x[0] & 1) == 1;
}

__attribute__((target("sse2")))
inline block makeBlock(uint64_t high, uint64_t low) {
	return _mm_set_epi64x(high, low);
}

inline block sigma(block a) {
	return _mm_shuffle_epi32(a, 78) ^ (a & makeBlock(0xFFFFFFFFFFFFFFFF, 0x00));
}

const block zero_block = makeBlock(0, 0);
const block all_one_block = makeBlock(0xFFFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFFFF);

__attribute__((target("sse2")))
inline block set_bit(const block & a, int i) {
	if(i < 64)
		return _mm_or_si128(makeBlock(0L, 1ULL<<i), a);
	else
		return _mm_or_si128(makeBlock(1ULL<<(i-64), 0), a);
}

std::ostream& operator<<(std::ostream& out, const block& blk)
{
	out << std::hex;
	uint64_t* data = (uint64_t*)&blk;

	out << std::setw(16) << std::setfill('0') << data[1]
		<< std::setw(16) << std::setfill('0') << data[0];

	out << std::dec << std::setw(0);
	return out;
}


inline void xorBlocks_arr(block* res, const block* x, const block* y, int nblocks) {
	const block * dest = nblocks+x;
	for (; x != dest;) {
		*(res++) = *(x++) ^ *(y++);
	}
}
inline void xorBlocks_arr(block* res, const block* x, block y, int nblocks) {
	const block * dest = nblocks+x;
	for (; x != dest;) 
		*(res++) =  *(x++) ^ y;
}

__attribute__((target("sse4")))
inline bool cmpBlock(const block * x, const block * y, int nblocks) {
	const block * dest = nblocks+x;
	for (; x != dest;) {
		__m128i vcmp = _mm_xor_si128(*(x++), *(y++));
		if(!_mm_testz_si128(vcmp, vcmp))
			return false;
	}
	return true;
}

//Modified from
//https://mischasan.wordpress.com/2011/10/03/the-full-sse2-bit-matrix-transpose-routine/
// with inner most loops changed to _mm_set_epi8 and _mm_set_epi16
#define INP(x, y) inp[(x)*ncols / 8 + (y) / 8]
#define OUT(x, y) out[(y)*nrows / 8 + (x) / 8]

__attribute__((target("sse2")))
inline void sse_trans(uint8_t *out, uint8_t const *inp, uint64_t nrows,
               uint64_t ncols) {
  uint64_t rr, cc;
  int i, h;
  union {
    __m128i x;
    uint8_t b[16];
  } tmp;
  __m128i vec;
  assert(nrows % 8 == 0 && ncols % 8 == 0);

  // Do the main body in 16x8 blocks:
  for (rr = 0; rr <= nrows - 16; rr += 16) {
    for (cc = 0; cc < ncols; cc += 8) {
      vec = _mm_set_epi8(INP(rr + 15, cc), INP(rr + 14, cc), INP(rr + 13, cc),
                         INP(rr + 12, cc), INP(rr + 11, cc), INP(rr + 10, cc),
                         INP(rr + 9, cc), INP(rr + 8, cc), INP(rr + 7, cc),
                         INP(rr + 6, cc), INP(rr + 5, cc), INP(rr + 4, cc),
                         INP(rr + 3, cc), INP(rr + 2, cc), INP(rr + 1, cc),
                         INP(rr + 0, cc));
      for (i = 8; --i >= 0; vec = _mm_slli_epi64(vec, 1))
        *(uint16_t *)&OUT(rr, cc + i) = _mm_movemask_epi8(vec);
    }
  }
  if (rr == nrows)
    return;

  // The remainder is a block of 8x(16n+8) bits (n may be 0).
  //  Do a PAIR of 8x8 blocks in each step:
  if ((ncols % 8 == 0 && ncols % 16 != 0) ||
      (nrows % 8 == 0 && nrows % 16 != 0)) {
    // The fancy optimizations in the else-branch don't work if the above if-condition
    // holds, so we use the simpler non-simd variant for that case.
    for (cc = 0; cc <= ncols - 16; cc += 16) {
      for (i = 0; i < 8; ++i) {
        tmp.b[i] = h = *(uint16_t const *)&INP(rr + i, cc);
        tmp.b[i + 8] = h >> 8;
      }
      for (i = 8; --i >= 0; tmp.x = _mm_slli_epi64(tmp.x, 1)) {
        OUT(rr, cc + i) = h = _mm_movemask_epi8(tmp.x);
        OUT(rr, cc + i + 8) = h >> 8;
      }
    }
  } else {
    for (cc = 0; cc <= ncols - 16; cc += 16) {
      vec = _mm_set_epi16(*(uint16_t const *)&INP(rr + 7, cc),
                          *(uint16_t const *)&INP(rr + 6, cc),
                          *(uint16_t const *)&INP(rr + 5, cc),
                          *(uint16_t const *)&INP(rr + 4, cc),
                          *(uint16_t const *)&INP(rr + 3, cc),
                          *(uint16_t const *)&INP(rr + 2, cc),
                          *(uint16_t const *)&INP(rr + 1, cc),
                          *(uint16_t const *)&INP(rr + 0, cc));
      for (i = 8; --i >= 0; vec = _mm_slli_epi64(vec, 1)) {
        OUT(rr, cc + i) = h = _mm_movemask_epi8(vec);
        OUT(rr, cc + i + 8) = h >> 8;
      }
    }
  }
  if (cc == ncols)
    return;

  //  Do the remaining 8x8 block:
  for (i = 0; i < 8; ++i)
    tmp.b[i] = INP(rr + i, cc);
  for (i = 8; --i >= 0; tmp.x = _mm_slli_epi64(tmp.x, 1))
    OUT(rr, cc + i) = _mm_movemask_epi8(tmp.x);
}
}
#endif//UTIL_BLOCK_H__
