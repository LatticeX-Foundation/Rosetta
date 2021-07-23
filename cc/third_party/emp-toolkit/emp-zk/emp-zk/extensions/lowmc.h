/*
 *
 * C++ implementation of LowMC block cipher family
 * https://github.com/LowMC/lowmc
 *
 */

#ifndef _EMP_LOWMC_H__
#define _EMP_LOWMC_H__

#include "emp-zk/emp-zk-bool/emp-zk-bool.h"

const unsigned numofboxes = 15;    // Number of Sboxes
const unsigned blocksize = 64;   // Block size in bits
const unsigned keysize = 128; // Key size in bits
const unsigned rounds = 11; // Number of rounds

const unsigned identitysize = blocksize - 3*numofboxes;
                  // Size of the identity part in the Sbox layer

class ZKLowMC {
public:
	ZKLowMC(bool *key_in) {
		init(key_in);
		keyschedule();
	}

	~ZKLowMC() {
		if(zero_bit_vec != nullptr) delete[] zero_bit_vec;
		if(LinMatrices_loc != nullptr) delete[] LinMatrices_loc;
		if(roundconstants_loc != nullptr) delete[] roundconstants_loc;
		if(key_loc != nullptr) delete[] key_loc;
		if(KeyMatrices_loc != nullptr) delete[] KeyMatrices_loc;
		if(roundkeys_loc != nullptr) delete[] roundkeys_loc;
		//if(LinMatrices != nullptr) delete[] LinMatrices;
		if(roundconstants != nullptr) delete[] roundconstants;
		if(key != nullptr) delete[] key;
		//if(KeyMatrices != nullptr) delete[] KeyMatrices;
		if(roundkeys != nullptr) delete[] roundkeys;
	}

	void encrypt(Bit* ctx, const Bit* msg, int nblocks) {
		for(int i = 0; i < nblocks; ++i)
			encrypt_block(ctx+i*blocksize, msg+i*blocksize);
	}

	void encrypt(bool* ctx, const bool* msg, int nblocks) {
		for(int i = 0; i < nblocks; ++i)
			encrypt_block(ctx+i*blocksize, msg+i*blocksize);
	}

	void encrypt_block(Bit* ctx, const Bit* msg) {
		ZKBitVecXor(ctx, msg, roundkeys, blocksize);
		for(unsigned r = 1; r <= rounds; ++r) {
			Substitution(ctx, ctx);
			MultiplyWithGF2Matrix(ctx, LinMatrices_loc+(r-1)*blocksize*blocksize, ctx);
			ZKBitVecXor(ctx, ctx, roundconstants+(r-1)*blocksize, blocksize);
			ZKBitVecXor(ctx, ctx, roundkeys+r*blocksize, blocksize);
		}
	}

	void encrypt_block(bool* ctx, const bool* msg) {
		ZKBitVecXor(ctx, msg, roundkeys_loc, blocksize);
		for(unsigned r = 1; r <= rounds; ++r) {
			Substitution(ctx, ctx);
			MultiplyWithGF2Matrix(ctx, LinMatrices_loc+(r-1)*blocksize*blocksize, ctx);
			ZKBitVecXor(ctx, ctx, roundconstants_loc+(r-1)*blocksize, blocksize);
			ZKBitVecXor(ctx, ctx, roundkeys_loc+r*blocksize, blocksize);
		}
	}

private:
	const std::vector<unsigned> Sbox =
	{0x00, 0x01, 0x03, 0x06, 0x07, 0x04, 0x05, 0x02};

	PRG prg;

	block choice[2];
	bool *zero_bit_vec = nullptr;

	bool *LinMatrices_loc = nullptr;
	bool *roundconstants_loc = nullptr;
	bool *key_loc = nullptr;
	bool *KeyMatrices_loc = nullptr;
	bool *roundkeys_loc = nullptr;

	//Bit* LinMatrices = nullptr; 	// r*n*n
	Bit* roundconstants = nullptr; 	// r*n
	Bit* key = nullptr; 		// k
	//Bit* KeyMatrices = nullptr; 	// r*n*k
	Bit* roundkeys = nullptr; 	// r*n

	template<typename T>
	void ZKBitVecXor(T *out, const T *a, const T *b, int len) {
		for(int i = 0; i < len; ++i)
			out[i] = a[i] ^ b[i];
	}

	template<typename T>
	void Substitution(T* out, const T* in) {
		const T *pt = in;
		T *pto = out;
		T a, b, c;
		for(unsigned i = 0; i < numofboxes; ++i, pt+=3, pto+=3) {
			a = pt[0] ^ (pt[1] & pt[2]);
			b = pt[0] ^ (pt[1] ^ (pt[0] & pt[2]));
			c = pt[0] ^ (pt[1] ^ (pt[2] ^ (pt[0] & pt[1])));
			pto[0] = a;
			pto[1] = b;
			pto[2] = c;
		}
		memcpy(pto, pt, identitysize*sizeof(T));
	}

	void MultiplyWithGF2Matrix(bool *out, const bool *matrix, const bool *in) {
		const bool *pt = matrix;
		bool *buf = new bool[blocksize];
		for(unsigned i = 0; i < blocksize; ++i)
			buf[i] = pt[i] & in[0];
		pt += blocksize;
		for(unsigned i = 1; i < blocksize; ++i, pt+=blocksize) {
			for(unsigned j = 0; j < blocksize; ++j)
				buf[j] = buf[j] ^ (pt[j] & in[i]);
		}
		memcpy(out, buf, blocksize*sizeof(bool));
		delete[] buf;
	}

	void MultiplyWithGF2Matrix(Bit *out, const bool *matrix, const Bit *in) {
		const bool *pt = matrix;
		Bit *buf = new Bit[blocksize];
		choice[1] = in[0].bit;
		for(unsigned i = 0; i < blocksize; ++i)
			buf[i].bit = choice[pt[i]];
		pt += blocksize;
		for(unsigned i = 1; i < blocksize; ++i, pt+=blocksize) {
			choice[1] = in[i].bit;
			for(unsigned j = 0; j < blocksize; ++j) {
				buf[j].bit = buf[j].bit ^ choice[pt[j]];
			}
		}
		memcpy(out, buf, blocksize*sizeof(Bit));
		delete[] buf;
	}

	// old
	/*void MultiplyWithGF2Matrix(Bit *out, const bool *matrix, const Bit *in) {
		const bool *pt = matrix;
		Bit *buf = new Bit[blocksize];
		for(unsigned i = 0; i < blocksize; ++i, pt+=blocksize) {
			choice[1] = in[0].bit;
			buf[i].bit = choice[pt[0]];
			for(unsigned j = 1; j < blocksize; ++j) {
				choice[1] = in[j].bit;
				buf[i].bit = buf[i].bit ^ choice[pt[j]];
			}
		}
		memcpy(out, buf, blocksize*sizeof(Bit));
		delete[] buf;
	}*/

	void MultiplyWithGF2Matrix_Key(bool *out, const bool *matrix, const bool *k) {
		const bool *pt = matrix;
		for(unsigned i = 0; i < blocksize; ++i, pt+=keysize) {
			out[i] = pt[0] & k[0];
			for(unsigned j = 1; j < keysize; ++j)
				out[i] = out[i] ^ (pt[j] & k[j]);
		}
	}

	void MultiplyWithGF2Matrix_Key(Bit *out, const bool *matrix, const Bit *k) {
		const bool *pt = matrix;
		for(unsigned i = 0; i < blocksize; ++i, pt+=keysize) {
			choice[1] = k[0].bit;
			out[i].bit = choice[pt[0]];
			for(unsigned j = 1; j < keysize; ++j) {
				choice[1] = k[j].bit;
				out[i].bit = out[i].bit ^ choice[pt[j]];
			}
		}
	}

	void keyschedule() {
		roundkeys = new Bit[(rounds+1)*blocksize];
		roundkeys_loc = new bool[(rounds+1)*blocksize];
		for(unsigned r = 0; r <= rounds; ++r) {
			MultiplyWithGF2Matrix_Key(roundkeys+r*blocksize,
					KeyMatrices_loc+r*blocksize*keysize, key);
			MultiplyWithGF2Matrix_Key(roundkeys_loc+r*blocksize,
					KeyMatrices_loc+r*blocksize*keysize, key_loc);
		}
	}

	void init(bool *key_b) {
		prg.reseed(&zero_block);
		unsigned len = 0;

		// key: k
		key = new Bit[keysize];
		key_loc = new bool[keysize];
		memcpy(key_loc, key_b, keysize*sizeof(bool));
		ProtocolExecution::prot_exec->feed((block*)key, ALICE, key_loc, keysize);

		// LinMatrices: r * n * n
		//random_bit_vec_gen(LinMatrices, LinMatrices_loc, rounds*blocksize*blocksize);
		len = rounds*blocksize*blocksize;
		//LinMatrices = new Bit[len];
		LinMatrices_loc = new bool[len];
		prg.random_bool(LinMatrices_loc, len);
		//ProtocolExecution::prot_exec->feed((block*)LinMatrices, PUBLIC, LinMatrices_loc, len);

		// roundconstants: r * n
		//random_bit_vec_gen(roundconstants, roundconstants_loc, rounds * blocksize);
		len = rounds*blocksize;
		roundconstants = new Bit[len];
		roundconstants_loc = new bool[len];
		prg.random_bool(roundconstants_loc, len);
		ProtocolExecution::prot_exec->feed((block*)roundconstants, PUBLIC, roundconstants_loc, len);

		// keyMatrices: r * n * k
		//random_bit_vec_gen(KeyMatrices, KeyMatrices_loc, rounds * blocksize * keysize);
		len = (rounds+1)*blocksize*keysize;
		//KeyMatrices = new Bit[len];
		KeyMatrices_loc = new bool[len];
		prg.random_bool(KeyMatrices_loc, len);
		//ProtocolExecution::prot_exec->feed((block*)KeyMatrices, PUBLIC, KeyMatrices_loc, len);

		zero_bit_vec = new bool[blocksize];
		memset(zero_bit_vec, 0, blocksize*sizeof(bool));
		choice[0] = zero_block;
	}

	void random_bit_vec_gen(Bit *out, bool *out_loc, int len) {
		out_loc = new bool[len];
		out = new Bit[len];
		prg.random_bool(out_loc, len);
		ProtocolExecution::prot_exec->feed((block*)out, PUBLIC, out_loc, len);
	}
};

#endif
