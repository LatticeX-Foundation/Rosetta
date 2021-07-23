#ifndef FP_AUTH_HELPER_H__
#define FP_AUTH_HELPER_H__

#include "emp-tool/emp-tool.h"

#define MAC_CHECK_BUFFER_SZ 8192

template<typename IO>
class FpAuthHelper {
public:
	int party;
	IO *io;
	Hash hash;

	FpAuthHelper(int party, IO *io) {
		this->party = party;
		this->io = io;
	}

	/* --------------------- MAC check zero ----------------------*/

	void store(uint64_t& mac) {
		hash.put(&mac, sizeof(uint64_t));
	}

	void flush() {
		char dig[emp::Hash::DIGEST_SIZE];
		hash.digest(dig);
		if(party == ALICE) {
			io->send_data(dig, emp::Hash::DIGEST_SIZE);
			io->flush();
		} else {
			char dig_recv[emp::Hash::DIGEST_SIZE];
			io->recv_data(dig_recv, emp::Hash::DIGEST_SIZE);
			if(!cmpBlock((block*)dig, (block*)dig_recv, emp::Hash::DIGEST_SIZE/16)) {
				error("Fp auth_helper check mac fails");
			}
		}
	}
};
#endif
