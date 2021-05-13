
#include <iostream>
using namespace std;

#include "emp-tool/emp-tool.h"
using namespace emp;

// from emp-tool/tests
void testPRG() {
  PRG prg; //using a random seed

  block rand_block[3];
  prg.random_block(rand_block, 3);

  prg.reseed(&rand_block[1]); //reset the PRG with another seed

  int rand_ints[100];
  int a = 0;
  prg.random_data_unaligned(
    rand_ints + 1, sizeof(int) * 99); //when the array is not 128-bit-aligned

  emp::block blk;
  memcpy(&blk, fix_key, sizeof(blk));
  prg.reseed(&blk);
  for (long long length = 2; length <= 8192 * 8; length *= 2) {
    //long long times = 1024 * 1024 * 32 / length;
    long long times = 1024 * 1024 * 2 / length;
    block* data = new block[length + 1];
    char* data2 = (char*)data;
    auto start = clock_start();
    for (int i = 0; i < times; ++i) {
      prg.random_data(data2, length * 16);
      //prg.random_data_unaligned(data2+1, length*16);
    }
    double interval = time_from(start);
    delete[] data;
    cout << "EMP PRG speed with block size " << length << " :\t"
         << (length * times * 128) / (interval + 0.0) * 1e6 * 1e-9 << " Gbps\n";
  }
}

int main(int argc, char* argv[]) {
  testPRG();
  return 0;
}