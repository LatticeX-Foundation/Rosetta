#ifndef D_AUTH_HELPER_H__
#define D_AUTH_HELPER_H__

#include "emp-tool/emp-tool.h"

template<typename IO>
class DoubAuthHelper {
public:
	int party;
	IO *io;
	Hash hash;
	block delta_f2;
	__uint128_t delta_fp;

	DoubAuthHelper(int party, IO *io) {
		this->party = party;
		this->io = io;
	}

	void set_delta(block delta_f2, __uint128_t delta_fp) {
		this->delta_f2 = delta_f2;
		this->delta_fp = delta_fp;
	}

	/* --------------------- reveal and return value ----------------------*/

	void open_check_send(uint64_t *val, __uint128_t* dat_fp, int len) {
		uint64_t *mac = new uint64_t[len];
		for(int i = 0; i < len; ++i) {
			val[i] = _mm_extract_epi64((block)dat_fp[i], 1);
			mac[i] = _mm_extract_epi64((block)dat_fp[i], 0);
		}
		hash.put(mac, len * sizeof(uint64_t));
		io->send_data(val, len * sizeof(uint64_t));
		//io->flush();
		delete[] mac;
	}

	void open_check_recv(uint64_t *val, __uint128_t* dat_fp, int len) {
		io->recv_data(val, len * sizeof(uint64_t));
		uint64_t *mac = new uint64_t[len];
		for(int i = 0; i < len; ++i) {
			mac[i] = mult_mod(val[i], (uint64_t)delta_fp);
			mac[i] = add_mod((uint64_t)dat_fp[i], mac[i]);
		}
		hash.put(mac, len * sizeof(uint64_t));
		delete[] mac;
	}

	void open_check_send(uint64_t *val, Integer *dat_f2, int len) {
		/*int bit_len = dat_f2[0].size();
		for(int i = 0; i < len; ++i) {
			std::bitset<64> comp_val = 0;
			for(int j = 0; j < bit_len; ++j) {
				comp_val.set(j, getLSB(dat_f2[i].bits[j].bit));
				hash.put_block(&(dat_f2[i].bits[j].bit), 1);
			}
			val[i] = comp_val.to_ullong();
		}
		io->send_data(val, len * sizeof(uint64_t));
		io->flush();*/
		for(int i = 0; i < len; ++i)
			val[i] = dat_f2[i].reveal<uint64_t>(PUBLIC);
	}

	void open_check_recv(uint64_t *val, Integer *dat_f2, int len) {
		/*io->recv_data(val, len * sizeof(uint64_t));
		int bit_len = dat_f2[0].size();
		block auth_f2;
		for(int i = 0; i < len; ++i) {
			std::bitset<64> bs(val[i]);
			for(int j = 0; j < bit_len; ++j) {
				if(bs[j]) auth_f2 = dat_f2[i].bits[j].bit ^ delta_f2;
				else auth_f2 = dat_f2[i].bits[j].bit;
				hash.put_block(&auth_f2, 1);
			}
		}*/
		for(int i = 0; i < len; ++i)
			val[i] = dat_f2[i].reveal<uint64_t>(PUBLIC);
	}

	/* --------------------- open and check ----------------------*/

	void open_check_send(Integer *dat_f2, __uint128_t* dat_fp, int len) {
		uint64_t *val = new uint64_t[len];
		for(int i = 0; i < len; ++i) {
			val[i] = _mm_extract_epi64((block)dat_fp[i], 1);
			uint64_t mac = _mm_extract_epi64((block)dat_fp[i], 0);
			hash.put(&mac, sizeof(uint64_t));
		}
		io->send_data(val, len*sizeof(uint64_t));
		io->flush();
		int bit_len = dat_f2[0].size();
		for(int i = 0; i < len; ++i) {
			/*std::bitset<64> comp_val = 0;
			for(int j = 0; j < bit_len; ++j) {
				comp_val.set(j, getLSB(dat_f2[i].bits[j].bit));
				hash.put_block(&(dat_f2[i].bits[j].bit), 1);
			}*/
			hash.put_block((block*)dat_f2[i].bits.data(), bit_len);
			//val[len+i] = comp_val.to_ullong();
			//dat_f2[i].reveal<uint64_t>(PUBLIC);
		}
		delete[] val;
	}

	void open_check_recv(Integer *dat_f2, __uint128_t* dat_fp, int len) {
		uint64_t *val = new uint64_t[len];
		io->recv_data(val, len*sizeof(uint64_t));
		//if(memcmp(val, val+len, len*sizeof(uint64_t)) != 0)
			//error("different value in f2 and fp");
		for(int i = 0; i < len; ++i) {
			uint64_t tmp = mult_mod(val[i], (uint64_t)delta_fp);
			tmp = add_mod((uint64_t)dat_fp[i], tmp);
			hash.put(&tmp, sizeof(uint64_t));
		}
		int bit_len = dat_f2[0].size();
		block *auth_f2 = new block[bit_len];
		for(int i = 0; i < len; ++i) {
			std::bitset<64> bs(val[i]);
			for(int j = 0; j < bit_len; ++j) {
				if(bs[j]) auth_f2[j] = dat_f2[i].bits[j].bit ^ delta_f2;
				else auth_f2[j] = dat_f2[i].bits[j].bit;
			}
			hash.put_block(auth_f2, bit_len);
			//val[len+i] = dat_f2[i].reveal<uint64_t>(PUBLIC);
		}
		//if(memcmp(val, val+len, len*sizeof(uint64_t)) != 0)
			//error("different value in f2 and fp");
		delete[] val;
	}

	/* --------------------- finalize check ----------------------*/

	bool equality_check(Hash *h) {
		block digest[2];
		h->digest((char*)digest);
		h->reset();
		if(party == ALICE) {
			io->send_data(digest, 2*sizeof(block));
			io->flush();
			return true;
		} else {
			block recv[2];
			io->recv_data(recv, 2*sizeof(block));
			return cmpBlock(digest, recv, 2);
		}
	}

	bool triple_equality_check() {
		return equality_check(&hash);
	}

};
#endif
