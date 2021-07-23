#ifndef EMP_ZK_BOOL_TRIPLE_AUTH_H__
#define EMP_ZK_BOOL_TRIPLE_AUTH_H__

#include "emp-tool/emp-tool.h"

#define MAC_CHECK_BUFFER_SZ 8192

template<typename IO>
class TripleAuth { public:
	int party;
	Hash hash;
	block choice[2];
	vector<block> tmp;
	IO * io;
	TripleAuth(int party, IO * io) {
		this->io = io;
		this->party = party;
		choice[0] = zero_block;
	}

	void set_delta(block delta) {
		choice[1] = delta;
	}
	
	void prv_check(bool * b, const block * mac, int length) {
		hash.put_block(mac, length);
	}

	void ver_check(bool * b, const block * key, size_t length) {
		if(tmp.size() < length)
			tmp.resize(length);
		for(size_t i = 0; i < length; ++i)
			tmp[i] = key[i] ^ choice[b[i]];
		hash.put_block(tmp.data(), length);
	}

	bool finalize() {
		char digest[emp::Hash::DIGEST_SIZE];
		hash.digest(digest);
		if(party == ALICE) {
			io->send_data(digest, emp::Hash::DIGEST_SIZE);
			io->flush();
			return true;
		} else {
			char digest2[emp::Hash::DIGEST_SIZE];
			io->recv_data(digest2, emp::Hash::DIGEST_SIZE);
			return memcmp(digest, digest2, emp::Hash::DIGEST_SIZE)==0;
		}
	}	
};
#endif
