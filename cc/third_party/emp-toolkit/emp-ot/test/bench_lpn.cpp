#include <iostream>
#include "emp-tool/emp-tool.h"
#include "emp-ot/emp-ot.h"
#include <stdlib.h>
using namespace std;
using namespace emp;

int main(int argc, char** argv) {
	PRG prg;
	int k, n;
	if (argc >= 3) {
		k = atoi(argv[1]);
		n = atoi(argv[2]);
	} else {
		k = 11;
		n = 20;
	}
	block seed;
	block * kk = new block[1<<k];
	block * nn = new block[1<<n];
	prg.random_block(&seed, 1);
	prg.random_block(kk, 1<<k);
	prg.random_block(nn, 1<<n);

	ThreadPool * pool = new ThreadPool(4);
	for (int kkk = 10; kkk < k; ++kkk) {	
		auto t1 = clock_start();
		for (int ttt = 0; ttt < 20; ttt++) {
			LpnF2<NetIO, 10> lpn(ALICE, 1<<n, 1<<kkk, pool, nullptr, pool->size());
			lpn.bench(nn, kk);
			kk[0] = nn[0];
		}
		cout << n<<"\t"<<kkk<<"\t";
		cout << time_from(t1)/20<<"\t"<<time_from(t1)/20*1000.0/(1<<n)<<endl;
	}
	cout << nn[0] <<endl;
}
