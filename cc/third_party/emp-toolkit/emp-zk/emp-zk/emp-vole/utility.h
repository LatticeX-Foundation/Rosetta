#ifndef FP_UTILITY_H__
#define FP_UTILITY_H__
#include <emp-tool/emp-tool.h>
using namespace emp;
using namespace std;

#define MERSENNE_PRIME_EXP 61
#define FIELD_TYPE uint64_t
const static __uint128_t p = 2305843009213693951;
const static int r = 1;
const static __uint128_t pr = 2305843009213693951;
const static block prs = makeBlock(2305843009213693951ULL, 2305843009213693951ULL);
const static uint64_t PR = 2305843009213693951;
static __m128i PRs = makeBlock(PR, PR);

#if  defined(__x86_64__) && defined(__BMI2__)
inline uint64_t mul64(uint64_t a, uint64_t b, uint64_t * c) {
	return _mulx_u64((unsigned long long )a, (unsigned long long) b, (unsigned long long*)c);
}
//
#else
inline uint64_t mul64(uint64_t a, uint64_t b, uint64_t * c) {
	__uint128_t aa = a;
	__uint128_t bb = b;
	auto cc = aa*bb;
	*c = cc>>64;
	return (uint64_t)cc;
}
#endif

inline uint64_t mod_pre(__uint128_t x) {
	return (x & PR) + (x >> MERSENNE_PRIME_EXP);
}

inline uint64_t mod(uint64_t x) {
	uint64_t i = (x & PR) + (x >> MERSENNE_PRIME_EXP);
	return (i >= p) ? i - p : i;
}

template<typename T>
T mod(T k, T p) {
	T i = (k & p) + (k >> MERSENNE_PRIME_EXP);
	return (i >= p) ? i - p : i;
}

inline block vec_partial_mod(block i) {
	return _mm_sub_epi64(i, _mm_andnot_si128(_mm_cmpgt_epi64(prs,i), prs));
}

#ifdef __AVX512F__
const static __m512i PR8 = _mm512_set_epi64(PR, PR, PR, PR, PR, PR, PR, PR);
inline __m512i vec_partial_mod_bch4(__m512i i) {
	__m512i tmp;
	block *pt = (block*)(&tmp);
	block *pti = (block*)(&i);
	pt[0] = _mm_andnot_si128(_mm_cmpgt_epi64(prs, pti[0]), prs);
	pt[1] = _mm_andnot_si128(_mm_cmpgt_epi64(prs, pti[1]), prs);
	pt[2] = _mm_andnot_si128(_mm_cmpgt_epi64(prs, pti[2]), prs);
	pt[3] = _mm_andnot_si128(_mm_cmpgt_epi64(prs, pti[3]), prs);
	return _mm512_sub_epi64(i, tmp);
}
inline void mult_mod_bch4(block* res, block *a, uint64_t *b) {
	__m512i bs[2];
	uint64_t *is = (uint64_t*)bs;
	for(int i = 0; i < 4; ++i) {
		uint64_t H = _mm_extract_epi64(a[i], 1);
		uint64_t L = _mm_extract_epi64(a[i], 0);
		is[2*i+1] = mul64(H, b[i], (uint64_t*)is+8+2*i+1);
		is[2*i] = mul64(L, b[i], (uint64_t*)is+8+2*i);
	}
	__m512i t1 = bs[0] & PR8;
	__m512i t2 = _mm512_srli_epi64(bs[0], MERSENNE_PRIME_EXP) ^ _mm512_slli_epi64(bs[1], 64 - MERSENNE_PRIME_EXP);
	t1 = _mm512_add_epi64(t1, t2);
	t1 = vec_partial_mod_bch4(t1);
	block *pt = (block*)(&t1);
	for(int i = 0; i < 4; ++i)
		res[i] = pt[i];
}

#endif

inline block vec_mod(block i) {
	i = _mm_add_epi64((i & prs), _mm_srli_epi64(i, MERSENNE_PRIME_EXP));
	return vec_partial_mod(i);
}

inline block mult_mod(block a, uint64_t b) {
	uint64_t H = _mm_extract_epi64(a, 1);
	uint64_t L = _mm_extract_epi64(a, 0);
	block bs[2];
	uint64_t * is = (uint64_t*)(bs);
//	uint64_t h0, h1, l0, l1;
	is[1] = mul64(H, b, (uint64_t*)(is+3));
	is[0] = mul64(L, b, (uint64_t*)(is+2));
	//block Hb = _mm_set_epi64((__m64)h1, (__m64)l1); 
	//block Lb = _mm_set_epi64((__m64)h0, (__m64)l0);
	block t1 = bs[0] & prs;
	block t2 = _mm_srli_epi64(bs[0], MERSENNE_PRIME_EXP) ^ _mm_slli_epi64(bs[1], 64 - MERSENNE_PRIME_EXP);
	block res = _mm_add_epi64(t1, t2);
	return vec_partial_mod(res);
}

inline void mult_mod_bch2(block* res, block *a, uint64_t *b) {
	block bs[4];
	uint64_t *is = (uint64_t*)bs;
	for(int i = 0; i < 2; ++i) {
		uint64_t H = _mm_extract_epi64(a[i], 1);
		uint64_t L = _mm_extract_epi64(a[i], 0);
		is[2*i+1] = mul64(H, b[i], (uint64_t*)is+4+2*i+1);
		is[2*i] = mul64(L, b[i], (uint64_t*)is+4+2*i);
	}
	block t1[2], t2[2];
       	t1[0] = bs[0] & prs;
	t1[1] = bs[1] & prs;
	t2[0] = _mm_srli_epi64(bs[0], MERSENNE_PRIME_EXP) ^ _mm_slli_epi64(bs[2], 64 - MERSENNE_PRIME_EXP);
	t2[1] = _mm_srli_epi64(bs[1], MERSENNE_PRIME_EXP) ^ _mm_slli_epi64(bs[3], 64 - MERSENNE_PRIME_EXP);
	t1[0] = _mm_add_epi64(t1[0], t2[0]);
	t1[1] = _mm_add_epi64(t1[1], t2[1]);
	res[0] = vec_partial_mod(t1[0]);
	res[1] = vec_partial_mod(t1[1]);
}


inline uint64_t mult_mod(uint64_t a, uint64_t b) {
	uint64_t c = 0;
	uint64_t e = mul64(a, b, (uint64_t*)&c);
	uint64_t res =  (e & PR) + ( (e>>MERSENNE_PRIME_EXP) ^ (c<< (64-MERSENNE_PRIME_EXP)));
	return (res >= PR) ? (res - PR) : res;
}

inline void mult_mod_bch2(uint64_t *res, uint64_t *a, uint64_t *b) {
	block cb, eb;
	uint64_t *c = (uint64_t*)&cb;
	uint64_t *e = (uint64_t*)&eb;
	e[0] = mul64(a[0], b[0], (uint64_t*)c);
	e[1] = mul64(a[1], b[1], (uint64_t*)c+1);
	eb = _mm_add_epi64(eb&prs, _mm_srli_epi64(eb, MERSENNE_PRIME_EXP) ^ _mm_slli_epi64(cb, 64-MERSENNE_PRIME_EXP));
	eb = vec_partial_mod(eb);
	res[0] = e[0];
	res[1] = e[1];
} 

inline void mult_mod_bch4(uint64_t *res, uint64_t *a, uint64_t *b) {
	block cb[2];
	block eb[2];
	uint64_t *c = (uint64_t*)cb;
	uint64_t *e = (uint64_t*)eb;
	e[0] = mul64(a[0], b[0], (uint64_t*)c);
	e[1] = mul64(a[1], b[1], (uint64_t*)c+1);
	e[2] = mul64(a[2], b[2], (uint64_t*)c+2);
	e[3] = mul64(a[3], b[3], (uint64_t*)c+3);
	block *resb = (block*)res;
	resb[0] = _mm_add_epi64((eb[0] & prs), (_mm_srli_epi64(eb[0], MERSENNE_PRIME_EXP) ^ _mm_slli_epi64(cb[0], 64-MERSENNE_PRIME_EXP)));
	resb[1] = _mm_add_epi64((eb[1] & prs), (_mm_srli_epi64(eb[1], MERSENNE_PRIME_EXP) ^ _mm_slli_epi64(cb[1], 64-MERSENNE_PRIME_EXP)));
	resb[0] = vec_partial_mod(resb[0]);
	resb[1] = vec_partial_mod(resb[1]);
}

inline block add_mod(block a, block b) {
	block res = _mm_add_epi64(a, b);
	return vec_partial_mod(res);
}

inline block add_mod(block a, uint64_t b) {
	block res = _mm_add_epi64(a, _mm_set_epi64((__m64)b, (__m64)b));
	return vec_partial_mod(res);
}

inline uint64_t add_mod(uint64_t a, uint64_t b) {
	uint64_t res = a + b;
	return (res >= PR) ? (res - PR) : res;
}

inline void extract_fp(__uint128_t& x) {
	x = mod(_mm_extract_epi64((block)x, 0));
}

template<typename T>
void uni_hash_coeff_gen(T* coeff, T seed, int sz) {
	coeff[0] = seed;
	for(int i = 1; i < sz; ++i)
		coeff[i] = mult_mod(coeff[i-1], seed);
}

template<typename T>
T vector_inn_prdt_sum_red(const T *a, const T *b, int sz) {
	T r = (T)0;
	for(int i = 0; i < sz; ++i)
		r = add_mod(r, mult_mod(a[i], b[i]));
	return r;
}

template<typename S, typename T>
T vector_inn_prdt_sum_red(const S *a, const T *b, int sz) {
	T r = (T)0;
	for(int i = 0; i < sz; ++i)
		r = add_mod(r, mult_mod((T)a[i], b[i]));
	return r;
}
/*
void feq_send(NetIO *io, void* in, int nbytes) {
	Hash hash;
	block h = hash.hash_for_block(in, nbytes);
	io->send_data(&h, sizeof(block));
}

bool feq_recv(NetIO *io, void* in, int nbytes) {
	Hash hash;
	block h = hash.hash_for_block(in, nbytes);
	block r;
	io->recv_data(&r, sizeof(block));
	if(!cmpBlock(&r, &h, 1)) return false;
	else return true;
}
*/
#endif
