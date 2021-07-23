#ifndef __EMP_BITIO_H__
#define __EMP_BITIO_H__
#include "emp-tool/emp-tool.h"

namespace emp {
template<typename IO>
class BoolIO: public IOChannel<BoolIO<IO>> { public:
	IO * io;
	Hash hash;//modelled as RO
	bool * buf;
	int ptr;
	bool sender;
	vector<unsigned char> tmp_arr;
	BoolIO(IO * io, int sender) : io(io), sender(sender) {
		buf = new bool[NETWORK_BUFFER_SIZE2];
		if(sender)
			ptr = 0;
		else ptr = NETWORK_BUFFER_SIZE2;
	}
	~BoolIO() {
		this->flush();
		delete[] buf;
	}
	void flush() {
		if(sender) {
			if(ptr != 0) {
				bool data = true;
				while(ptr != 0)
					send_bit(data);
			}
		} else {
			ptr = NETWORK_BUFFER_SIZE2;
		}
		io->flush();
	}

	block get_hash_block() {
		block res[2];
		hash.digest((char *)res);
		return res[0];
	}
	
	void send_bit(bool data) {
		buf[ptr] = data;
		ptr++;
		if(ptr == NETWORK_BUFFER_SIZE2) {
			send_bool_raw(buf, NETWORK_BUFFER_SIZE2);
			ptr = 0;
		}
	}

	bool recv_bit() {
		if(ptr == NETWORK_BUFFER_SIZE2) {
			recv_bool_raw(buf, NETWORK_BUFFER_SIZE2);
			ptr = 0;
		}
		bool res = buf[ptr];
		ptr++;
		return res;
	}

	void send_data_internal(const void * data, int len) {
		if(ptr != 0) flush();
		io->send_data_internal(data, len);
	}

	void recv_data_internal(void * data, int len) {
		if(ptr != NETWORK_BUFFER_SIZE2) flush();
		io->recv_data_internal(data, len);
	}
	
	void send_bool_raw(const bool * data, int length) {
		if(tmp_arr.size() < (size_t)length/8)
			tmp_arr.resize(length/8);
		int cnt = 0;
		
		unsigned long long * data64 = (unsigned long long * )data;
		int i = 0;
		for(; i < length/8; ++i) {
			unsigned long long mask = 0x0101010101010101ULL;
			unsigned long long tmp = 0;
#if defined(__BMI2__)
			tmp = _pext_u64(data64[i], mask);
#else
			// https://github.com/Forceflow/libmorton/issues/6
			for (unsigned long long bb = 1; mask != 0; bb += bb) {
				if (data64[i] & mask & -mask) { tmp |= bb; }
				mask &= (mask - 1);
			}
#endif
			tmp_arr[cnt] = tmp;
			cnt++;
		}
		hash.put(tmp_arr.data(), cnt);
		io->send_data(tmp_arr.data(), cnt);

		if (8*i != length) {
			hash.put(data + 8*i, length - 8*i);
			io->send_data(data + 8*i, length - 8*i);
		}
	}
	void recv_bool_raw(bool * data, int length) {
		if(tmp_arr.size() < (size_t)length/8)
			tmp_arr.resize(length/8);

		int cnt = 0;
		io->recv_data(tmp_arr.data(), length/8);
		hash.put(tmp_arr.data(), length/8);

		unsigned long long * data64 = (unsigned long long *) data;
		int i = 0;
		for(; i < length/8; ++i) {
			unsigned long long mask = 0x0101010101010101ULL;
			unsigned long long tmp = tmp_arr[cnt];
			cnt++;
#if defined(__BMI2__)
			data64[i] = _pdep_u64(tmp, mask);
#else
			data64[i] = 0;
			for (unsigned long long bb = 1; mask != 0; bb += bb) {
				if (tmp & bb) {data64[i] |= mask & (-mask); }
				mask &= (mask - 1);
			}
#endif
		}
		if (8*i != length) {
			io->recv_data(data + 8*i, length - 8*i);
			hash.put(data + 8*i, length - 8*i);
		}
	}
};
}
#endif// __UNIDIRIO_H__
