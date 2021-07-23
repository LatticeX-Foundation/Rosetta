#ifndef EMP_OTIDEAL_H__
#define EMP_OTIDEAL_H__
#include "emp-ot/cot.h"

namespace emp { 
template<typename T> 
class OTIdeal: public COT<T> { public:
	using COT<T>::io;
	using COT<T>::Delta;
	int64_t cnt = 0;
	PRG prg;
	OTIdeal(T * io, bool * delta = nullptr) {
		this->io = io;
		prg.reseed((const block *)fix_key);
		if (delta!= nullptr)
			Delta = bool_to_block(delta);
	}

	void send_cot(block* data, int64_t length) override {
		cnt+=length;
		prg.random_block(data);
	}

	void recv_cot(block* data, const bool* b, int64_t length) override {
		cnt+=length;
		prg.random_block(data);
		for(int i = 0; i < length; ++i)
			if(b[i])
				data[i] = data[i] ^ Delta;
	}
};
}//namespace
#endif// OT_IDEAL_H__
