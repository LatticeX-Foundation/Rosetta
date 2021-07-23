#ifndef FP_POLY_H__
#define FP_POLY_H__

#include "emp-tool/emp-tool.h"
#include "emp-zk/emp-zk-bool/emp-zk-bool.h"
#include "emp-zk/emp-zk-arith/ostriple.h"

template<typename IO>
class FpPolyProof {
public:
	static FpPolyProof<IO> *fppolyproof;
	int party;
	IO *io;
	uint64_t delta;
	int buffer_sz = 1024;
	uint64_t *buffer = nullptr;
	uint64_t *buffer1 = nullptr;
	FpOSTriple<IO> *ostriple;
	int num;

	FpPolyProof(int party, IO *io, FpOSTriple<IO> *ostriple) {
		this->party = party;
		this->io = io;
		this->ostriple = ostriple;
		if(party == ALICE) {
			buffer = new uint64_t[buffer_sz];
			buffer1 = new uint64_t[buffer_sz];
		} else {
			buffer = new uint64_t[buffer_sz];
			this->delta = LOW64(ostriple->delta);
		}
		num = 0;
	}

	~FpPolyProof() {
		batch_check();
		if(buffer != nullptr) delete[] buffer;
		if(buffer1 != nullptr) delete[] buffer1;
	}

	void batch_check() {
		if(num == 0) return;
		uint64_t seed;
		io->flush();
		uint64_t *chi = new uint64_t[num];
		__uint128_t ope_data;
		uint64_t check_sum[2];
		if(party == ALICE) {
			io->recv_data(&seed, sizeof(uint64_t));

			uni_hash_coeff_gen(chi, seed, num);

			check_sum[0] = vector_inn_prdt_sum_red(chi, buffer, num);
			check_sum[1] = vector_inn_prdt_sum_red(chi, buffer1, num);
			ostriple->vole->extend(&ope_data, 1);

			check_sum[0] = add_mod(check_sum[0], LOW64(ope_data));
			check_sum[1] = add_mod(check_sum[1], HIGH64(ope_data));
			io->send_data(check_sum, 2*sizeof(uint64_t));
		} else {
			PRG prg;
			prg.random_data(&seed, sizeof(uint64_t));
			seed = mod(seed);
			io->send_data(&seed, sizeof(uint64_t));

			uni_hash_coeff_gen(chi, seed, num);
			uint64_t B = vector_inn_prdt_sum_red(chi, buffer, num);
			ostriple->vole->extend(&ope_data, 1);
			B = add_mod(B, LOW64(ope_data));
			io->recv_data(check_sum, 2*sizeof(uint64_t));

			uint64_t tmp = mult_mod(check_sum[1], delta);
			tmp = add_mod(B, tmp);
			if(tmp != check_sum[0])
				CheatRecord::put("polynomial zkp fails");
		}
		num = 0;
		delete[] chi;
	}

	inline void zkp_poly_deg2(__uint128_t *polyx, __uint128_t* polyy, uint64_t *coeff, int len) {
		if(num >= buffer_sz) batch_check();

		if(party == ALICE) {
			uint64_t A0 = 0, A1 = 0;
			uint64_t w0, w1, m0, m1, tmp;
			for(int i = 0; i < len; ++i) {
				w0 = HIGH64(polyx[i]);
				m0 = LOW64(polyx[i]);
				w1 = HIGH64(polyy[i]);
				m1 = LOW64(polyy[i]);

				tmp = mult_mod(m0, m1);
				tmp = mult_mod(coeff[i+1], tmp);
				A0 = add_mod(A0, tmp);

				tmp = add_mod(mult_mod(m0, w1), mult_mod(m1, w0));
				tmp = mult_mod(coeff[i+1], tmp);
				A1 = add_mod(A1, tmp);
			}
			buffer[num] = A0;
			buffer1[num] = A1;
		} else {
			uint64_t B = 0;
			uint64_t tmp;
			for(int i = 0; i < len; ++i) {
				tmp = mult_mod(LOW64(polyx[i]), LOW64(polyy[i]));
				tmp = mult_mod(coeff[i+1], tmp);
				B = add_mod(B, tmp);
			}
			tmp = mult_mod(delta, delta);
			tmp = mult_mod(coeff[0], tmp);
			B = add_mod(B, tmp);
			buffer[num] = B;
		}
		num++;
	}

	inline void zkp_inner_prdt(__uint128_t *polyx, __uint128_t *polyy, uint64_t constant, int len) {
		if(num >= buffer_sz) batch_check();

		if(party == ALICE) {
			uint64_t A0 = 0, A1 = 0;
			uint64_t w0, w1, m0, m1, tmp;
			for(int i = 0; i < len; ++i) {
				w0 = HIGH64(polyx[i]);
				m0 = LOW64(polyx[i]);
				w1 = HIGH64(polyy[i]);
				m1 = LOW64(polyy[i]);

				tmp = mult_mod(m0, m1);
				A0 = add_mod(A0, tmp);

				tmp = add_mod(mult_mod(m0, w1), mult_mod(m1, w0));
				A1 = add_mod(A1, tmp);
			}
			buffer[num] = A0;
			buffer1[num] = A1;
		} else {
			uint64_t B = 0;
			uint64_t tmp;
			for(int i = 0; i < len; ++i) {
				tmp = mult_mod(LOW64(polyx[i]), LOW64(polyy[i]));
				B = add_mod(B, tmp);
			}
			tmp = mult_mod(delta, delta);
			tmp = mult_mod(constant, tmp);
			B = add_mod(B, tmp);
			buffer[num] = B;
		}
		num++;
	}

};
template<typename IO>
FpPolyProof<IO>* FpPolyProof<IO>::fppolyproof = nullptr;
#endif
