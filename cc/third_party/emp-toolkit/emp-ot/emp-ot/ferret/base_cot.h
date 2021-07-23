#ifndef COT_H__
#define COT_H__

#include "emp-ot/ferret/preot.h"

template<typename IO>
class BaseCot { public:
   int party;
	block one, minusone;
	block ot_delta;
	IO *io;
	IKNP<IO> *iknp;
	bool malicious = false;

	BaseCot(int party, IO *io, bool malicious = false) {
		this->party = party;
		this->io = io;
		this->malicious = malicious;
		iknp = new IKNP<IO>(io, malicious);
		minusone = makeBlock(0xFFFFFFFFFFFFFFFFLL,0xFFFFFFFFFFFFFFFELL);
		one = makeBlock(0x0LL, 0x1LL);
	}

	void cot_gen_pre(block deltain) {
		if (this->party == ALICE) {
			this->ot_delta = deltain;
			bool delta_bool[128];
			block_to_bool(delta_bool, ot_delta);
			iknp->setup_send(delta_bool);
		} else {
			iknp->setup_recv();
		}
	}

	void cot_gen_pre() {
		if (this->party == ALICE) {
			PRG prg;
			prg.random_block(&ot_delta, 1);
			ot_delta = ot_delta & minusone;
			ot_delta = ot_delta ^ one;
			bool delta_bool[128];
			block_to_bool(delta_bool, ot_delta);
			iknp->setup_send(delta_bool);
		} else {
			iknp->setup_recv();
		}
	}

	void cot_gen(block *ot_data, int64_t size) {
		if (this->party == ALICE) {
			iknp->send_cot(ot_data, size);
			io->flush();
			for(int64_t i = 0; i < size; ++i)
				ot_data[i] = ot_data[i] & minusone;
		} else {
			PRG prg;
			bool *pre_bool_ini = new bool[size];
			prg.random_bool(pre_bool_ini, size);
			iknp->recv_cot(ot_data, pre_bool_ini, size);
			block ch[2];
			ch[0] = zero_block;
			ch[1] = makeBlock(0, 1);
			for(int64_t i = 0; i < size; ++i)
				ot_data[i] = 
						(ot_data[i] & minusone) ^ ch[pre_bool_ini[i]];
			delete[] pre_bool_ini;
		}
	}

	void cot_gen(OTPre<IO> *pre_ot, int64_t size) {
		block *ot_data = new block[size];
		if (this->party == ALICE) {
			iknp->send_cot(ot_data, size);
			io->flush();
			for(int64_t i = 0; i < size; ++i)
				ot_data[i] = ot_data[i] & minusone;
			pre_ot->send_pre(ot_data, ot_delta);
		} else {
			PRG prg;
			bool *pre_bool_ini = new bool[size];
			prg.random_bool(pre_bool_ini, size);
			iknp->recv_cot(ot_data, pre_bool_ini, size);
			block ch[2];
			ch[0] = zero_block;
			ch[1] = makeBlock(0, 1);
			for(int64_t i = 0; i < size; ++i)
				ot_data[i] = 
						(ot_data[i] & minusone) ^ ch[pre_bool_ini[i]];
			pre_ot->recv_pre(ot_data, pre_bool_ini);
			delete[] pre_bool_ini;
		}
		delete[] ot_data;
	}

	// debug
	bool check_cot(block *data, int64_t len) {
		if(party == ALICE) {
			io->send_block(&ot_delta, 1);
			io->send_block(data, len); 
			io->flush();
			return true;
		} else {
			block * tmp = new block[len];
			block ch[2];
			io->recv_block(ch+1, 1);
			ch[0] = zero_block;
			io->recv_block(tmp, len);
			for(int64_t i = 0; i < len; ++i)
				tmp[i] = tmp[i] ^ ch[getLSB(data[i])];
			bool res = cmpBlock(tmp, data, len);
			delete[] tmp;
			return res;
		}
	}
};

#endif
