#ifndef _PRE_OT__
#define  _PRE_OT__
#include "emp-tool/emp-tool.h"
#include "emp-ot/emp-ot.h"
namespace emp {

template<typename IO>
class OTPre { public:
	IO* io;
	block * pre_data = nullptr;
	bool * bits = nullptr;
	int n;
	vector<block*> pointers;
	vector<const bool*> choices;
	vector<const block*> pointers0;
	vector<const block*> pointers1;

	CCRH ccrh;
	int length, count;
	block Delta;
	OTPre(IO* io, int length, int times) {
		this->io = io;
		this->length = length;
		n = length*times;
		pre_data = new block[2*n];
		bits = new bool[n];
		count = 0;
	}
	~OTPre() {
		if (pre_data != nullptr)
			delete[] pre_data;

		if (bits != nullptr)
			delete[] bits;
	}
	void send_pre(block * data, block in_Delta) {
		Delta = in_Delta;
		ccrh.Hn(pre_data, data, 0, n, pre_data+n);
		xorBlocks_arr(pre_data+n, data, Delta, n);
		ccrh.Hn(pre_data+n, pre_data+n, 0, n);
	}

	void recv_pre(block * data, bool * b) {
		memcpy(bits, b, n);
		ccrh.Hn(pre_data, data, 0, n);
	}

	void recv_pre(block * data) {
		for(int i = 0; i < n; ++i)
			bits[i] = getLSB(data[i]);
		ccrh.Hn(pre_data, data, 0, n);
	}

	void choices_sender() {
		io->recv_data(bits+count, length);
		count +=length;
	}

	void choices_recver(const bool * b) {
		for (int i = 0; i < length; ++i) {
			bits[count + i] = (b[i] != bits[count + i]);
		}
		io->send_data(bits+count, length);
		count +=length;
	}
	
	void reset() {
		count = 0;
	}

	void send(const block * m0, const  block * m1, int length, IO * io2, int s) {
		block pad[2];
		int k = s*length;
		for (int i = 0; i < length; ++i) {
			if (!bits[k]) {
				pad[0] = m0[i] ^ pre_data[k];
				pad[1] = m1[i] ^ pre_data[k+n];
			} else {
				pad[0] = m0[i] ^ pre_data[k+n];
				pad[1] = m1[i] ^ pre_data[k];
			}
			++k;
			io2->send_block(pad, 2);
		}
	//	count +=length;
	}

	void recv(block* data, const bool* b, int length, IO* io2, int s) {
		int k = s*length;
		block pad[2];
		for (int i = 0; i < length; ++i) {
			io2->recv_block(pad, 2);
			int ind = b[i] ? 1 : 0;
			data[i] = pre_data[k] ^ pad[ind];
			++k;
		}
	//	count += length;
	}
};
}
#endif// _PRE_OT__
