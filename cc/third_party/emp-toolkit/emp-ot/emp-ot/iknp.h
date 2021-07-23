#ifndef EMP_IKNP_H__
#define EMP_IKNP_H__
#include "emp-ot/cot.h"
#include "emp-ot/co.h"

namespace emp {

/*
 * IKNP OT Extension
 * [REF] Implementation of "Extending oblivious transfers efficiently"
 * https://www.iacr.org/archive/crypto2003/27290145/27290145.pdf
 *
 * [REF] With optimization of "More Efficient Oblivious Transfer and Extensions for Faster Secure Computation"
 * https://eprint.iacr.org/2013/552.pdf
 * [REF] With optimization of "Better Concrete Security for Half-Gates Garbling (in the Multi-Instance Setting)"
 * https://eprint.iacr.org/2019/1168.pdf
 */
template<typename T>
class IKNP: public COT<T> { public:
	using COT<T>::io;
	using COT<T>::Delta;

	OTCO<T> * base_ot = nullptr;
	bool setup = false, *extended_r = nullptr;

	const static int64_t block_size = 1024*2;
	block local_out[block_size];
	bool s[128], local_r[256];
	PRG prg, G0[128], G1[128];
	bool malicious = false;
	block k0[128], k1[128]; 
	IKNP(T * io, bool malicious = false): malicious(malicious) {
		this->io = io;
	}
	~IKNP() {
		delete_array_null(extended_r);
	}

	void setup_send(const bool* in_s = nullptr, block * in_k0 = nullptr) {
		setup = true;
		if(in_s == nullptr)
			prg.random_bool(s, 128);
		else 
			memcpy(s, in_s, 128);
		
		if(in_k0 != nullptr) {
			memcpy(k0, in_k0, 128*sizeof(block));
		} else {
			this->base_ot = new OTCO<T>(io);
			base_ot->recv(k0, s, 128);
			delete base_ot;
		}
		for(int64_t i = 0; i < 128; ++i)
			G0[i].reseed(&k0[i]);

		Delta = bool_to_block(s);
	}

	void setup_recv(block * in_k0 = nullptr, block * in_k1 =nullptr) {
		setup = true;
		if(in_k0 !=nullptr) {
			memcpy(k0, in_k0, 128*sizeof(block));
			memcpy(k1, in_k1, 128*sizeof(block));
		} else {
			this->base_ot = new OTCO<T>(io);
			prg.random_block(k0, 128);
			prg.random_block(k1, 128);
			base_ot->send(k0, k1, 128);
			delete base_ot;
		}
		for(int64_t i = 0; i < 128; ++i) {
			G0[i].reseed(&k0[i]);
			G1[i].reseed(&k1[i]);
		}
	}
	void send_pre(block * out, int64_t length) {
		if(not setup)
			setup_send();
		int64_t j = 0;
		for (; j < length/block_size; ++j)
			send_pre_block(out + j*block_size, block_size);
		int64_t remain = length % block_size;
		if (remain > 0) {
			send_pre_block(local_out, remain);
			memcpy(out+j*block_size, local_out, sizeof(block)*remain);
		}
		if(malicious)
			send_pre_block(local_out, 256);
	}

	void send_pre_block(block * out, int64_t len) {
		block t[block_size];
		block tmp[block_size];
		int64_t local_block_size = (len+127)/128*128;
		io->recv_block(tmp, local_block_size);
		for(int64_t i = 0; i < 128; ++i) {
			G0[i].random_data(t+(i*block_size/128), local_block_size/8);
			if (s[i])
				xorBlocks_arr(t+(i*block_size/128), t+(i*block_size/128), tmp+(i*local_block_size/128), local_block_size/128);
		}
		sse_trans((uint8_t *)(out), (uint8_t*)t, 128, block_size);
	}

	void recv_pre(block * out, const bool* r, int64_t length) {
		if(not setup)
			setup_recv();

		block *block_r = new block[(length+127)/128];
		for(int64_t i = 0; i < length/128; ++i)
			block_r[i] = bool_to_block(r+i*128);
		if (length%128 != 0) {
			bool tmp_bool_array[128];
			memset(tmp_bool_array, 0, 128);
			int64_t start_point = (length / 128)*128;
			memcpy(tmp_bool_array, r+start_point, length % 128);
			block_r[length/128] = bool_to_block(tmp_bool_array);
		}
		
		int64_t j = 0;
		for (; j < length/block_size; ++j)
			recv_pre_block(out+j*block_size, block_r + (j*block_size/128), block_size);
		int64_t remain = length % block_size;
		if (remain > 0) {
			recv_pre_block(local_out, block_r + (j*block_size/128), remain);
			memcpy(out+j*block_size, local_out, sizeof(block)*remain);
		}
		if(malicious) {
			block local_r_block[2]; 
			prg.random_bool(local_r, 256);
			local_r_block[0] = bool_to_block(local_r);
			local_r_block[1] = bool_to_block(local_r + 128);
			recv_pre_block(local_out, local_r_block, 256);
		}
		delete[] block_r;
	}
	void recv_pre_block(block * out, block * r, int64_t len) {
		block t[block_size];
		block tmp[block_size];
		int64_t local_block_size = (len+127)/128 * 128;
		for(int64_t i = 0; i < 128; ++i) {
			G0[i].random_data(t+(i*block_size/128), local_block_size/8);
			G1[i].random_data(tmp, local_block_size/8);
			xorBlocks_arr(tmp, t+(i*block_size/128), tmp, local_block_size/128);
			xorBlocks_arr(tmp, r, tmp, local_block_size/128);
			io->send_data(tmp, local_block_size/8);
		}

		sse_trans((uint8_t *)(out), (uint8_t*)t, 128, block_size);
	}

	void send_cot(block * data, int64_t length) override{
		send_pre(data, length);

		if(malicious)
			if(!send_check(data, length))
				error("OT Extension check failed");
	}
	void recv_cot(block* data, const bool * b, int64_t length) override {
		recv_pre(data, b, length);
		if(malicious)
			recv_check(data, b, length);
	}

/*
 *
 * [REF] Implementation of "Actively Secure OT Extension with Optimal Overhead"
 * https://eprint.iacr.org/2015/546.pdf
 */
	bool send_check(block * out, int64_t length) {
		block seed2, x, t[2], q[2], tmp[2];
		block chi[block_size];
		q[0] = q[1] = makeBlock(0, 0);
		io->recv_block(&seed2, 1);
		io->flush();

		for(int64_t i = 0; i < length/block_size; ++i) {
			uni_hash_coeff_gen<block_size>(chi, seed2);
			vector_inn_prdt_sum_no_red<block_size>(tmp, chi, out+i*block_size);
			q[0] = q[0] ^ tmp[0];
			q[1] = q[1] ^ tmp[1];
		}
		int64_t remain = length % block_size;
		if(remain != 0) {
			uni_hash_coeff_gen<block_size>(chi, seed2);
			vector_inn_prdt_sum_no_red(tmp, chi, out + length - remain, remain);
			q[0] = q[0] ^ tmp[0];
			q[1] = q[1] ^ tmp[1];
		}
		{
			uni_hash_coeff_gen<256>(chi, seed2);
			vector_inn_prdt_sum_no_red<256>(tmp, chi, local_out);
			q[0] = q[0] ^ tmp[0];
			q[1] = q[1] ^ tmp[1];
		}

		io->recv_block(&x, 1);
		io->recv_block(t, 2);
		mul128(x, Delta, tmp, tmp+1);
		q[0] = q[0] ^ tmp[0];
		q[1] = q[1] ^ tmp[1];

		return cmpBlock(q, t, 2);	
	}
	void recv_check(block * out, const bool* r, int64_t length) {
		block select[2] = {zero_block, all_one_block};
		block seed2, x = makeBlock(0,0), t[2], tmp[2];
		prg.random_block(&seed2,1);
		io->send_block(&seed2, 1);
		io->flush();
		block chi[block_size];
		t[0] = t[1] = makeBlock(0, 0);

		for(int64_t i = 0; i < length/block_size; ++i) {
			uni_hash_coeff_gen<block_size>(chi, seed2);
			vector_inn_prdt_sum_no_red<block_size>(tmp, chi, out+i*block_size);
			t[0] = t[0] ^ tmp[0];
			t[1] = t[1] ^ tmp[1];
			for(int64_t j = 0; j < block_size; ++j) 
				x = x ^ (chi[j] & select[r[i*block_size+j]]);
		}
		int64_t remain = length % block_size;
		if(remain != 0) {
			uni_hash_coeff_gen<block_size>(chi, seed2);
			vector_inn_prdt_sum_no_red(tmp, chi, out+length - remain, remain);
			t[0] = t[0] ^ tmp[0];
			t[1] = t[1] ^ tmp[1];
			for(int64_t j = 0; j < remain; ++j)
				x = x ^ (chi[j] & select[r[length - remain + j]]);
		}
		
		{
			uni_hash_coeff_gen<256>(chi, seed2);
			vector_inn_prdt_sum_no_red<256>(tmp, chi, local_out);
			t[0] = t[0] ^ tmp[0];
			t[1] = t[1] ^ tmp[1];
			for(int64_t j = 0; j < 256; ++j)
				x = x ^ (chi[j] & select[local_r[j]]);
		}

		io->send_block(&x, 1);
		io->send_block(t, 2);
	}
};

}//namespace
#endif
