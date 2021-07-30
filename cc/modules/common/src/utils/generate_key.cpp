// ==============================================================================
// Copyright 2020 The LatticeX Foundation
// This file is part of the Rosetta library.
//
// The Rosetta library is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// The Rosetta library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with the Rosetta library. If not, see <http://www.gnu.org/licenses/>.
// ==============================================================================
#include "cc/modules/common/include/utils/generate_key.h"
#include "cc/modules/common/include/utils/rtt_logger.h"
#include "cc/modules/common/include/utils/rtt_exceptions.h"

#include <libgen.h>
#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
using namespace std;

#define STORE_AS_BINARY 0
#define PRF_KEY_BYTES 16

string gen_key_str(uint32_t seed) {
  std::random_device rd;
  std::mt19937 mt(rd());

  if (seed != 0)
    mt.seed(seed);

  unsigned char key[PRF_KEY_BYTES] = {0};
  for (size_t i = 0; i < PRF_KEY_BYTES; i++) {
    key[i] = mt() & 0xFF;
  }

  char const hex_chars[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                              '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

  std::string s;
  for (size_t i = 0; i < PRF_KEY_BYTES; i++) {
    s += hex_chars[(key[i] & 0xF0) >> 4];
    s += hex_chars[(key[i] & 0x0F) >> 0];
  }

  return s;
}

void gen_key_file(const string& filename, uint32_t seed) {
  mkdir("key", 0755);

  ofstream ofile(filename, ios::out);
  if (!ofile.is_open()) {
    log_error << "open [" << filename << "] failed!\n";
    throw openfile_exp("open [" + filename + "] failed!");
  }
  string s = gen_key_str(seed);

  ofile.write((const char*)s.c_str(), PRF_KEY_BYTES * 2);
  ofile.close();

  log_debug << "Generate " << filename << " OK.\n\n";
}
