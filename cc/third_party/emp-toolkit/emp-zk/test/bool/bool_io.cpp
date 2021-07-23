#include<iostream>
#include "emp-tool/emp-tool.h"
#include <emp-zk/emp-zk.h>
using namespace emp;
using namespace std;
const int LL = 1024*1024*100+10;
int main(int argc, char** argv) {
	int party, port;
	parse_party_and_port(argv, &party, &port);
	NetIO * netio = new NetIO(party == ALICE?nullptr:"127.0.0.1",port, false);
	BoolIO<NetIO> * io = new BoolIO<NetIO>(netio, party == ALICE);
	bool * data = new bool[LL];
	bool * data2 = new bool[LL];

	PRG prg(fix_key);
	prg.random_bool(data, LL);
	auto t1 = clock_start();
	if(party == ALICE) {
		for(int i = 0; i < LL; ++i)
			io->send_bit(data[i]);
	} else {
		for(int i = 0; i < LL; ++i) 
			data[i] = io->recv_bit();
	}
	io->flush();
	cout << time_from(t1)/LL*1000<<"\n";
	if(party == BOB) {
		PRG prg2(fix_key);
		prg2.random_bool(data2, LL);
		if(memcmp(data, data2, LL)!=0)
			cout<<"wrong!\n";
		else
			cout<<"fine!\n";

	}	
	delete netio;
	delete io;
	return 0;
}
