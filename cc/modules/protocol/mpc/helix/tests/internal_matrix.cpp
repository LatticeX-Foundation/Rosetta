#include "helix__test.h"

int main(int argc, char* argv[]) {
  int m = 2, n = 2, k = 2;
  vector<mpc_t> a(m * n, 1), b(n * k, 1), c(m * k);
  EigenMatMul(a, b, c, m, n, k, false, false);
  //print_vec(c, 32, "c");

  return 0;
}
