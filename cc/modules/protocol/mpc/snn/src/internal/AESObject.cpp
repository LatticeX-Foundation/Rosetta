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
#include <cstring>
#include <iostream>
#include <fstream>
using namespace std;

#include "cc/modules/protocol/mpc/snn/src/internal/TedKrovetzAesNiWrapperC.h"
#include "cc/modules/protocol/mpc/snn/src/internal/AESObject.h"

#include "cc/modules/common/include/utils/rtt_logger.h"

AESObject::AESObject(const char* filename) {
  ifstream f(filename);
  string str{istreambuf_iterator<char>(f), istreambuf_iterator<char>()};
  f.close();

  log_info << "aes file:" << filename << " key:" << str << "\n";
  string fname(filename);
  if ((fname.find("null") == string::npos) && str.empty()) {
    log_error << filename << " is not exist!" ;
  }
  Init(str);
}
void AESObject::Init(const std::string& key) {
  int len = key.length();
  char* common_aes_key = new char[len + 1];
  memset(common_aes_key, 0, len + 1);
  strcpy(common_aes_key, key.c_str());
  AES_set_encrypt_key((unsigned char*)common_aes_key, 256, &aes_key);
  delete []common_aes_key;
}
__m128i AESObject::newRandomNumber() {
  rCounter++;
  if (rCounter % RANDOM_COMPUTE == 0) // generate more random seeds
  {
    __m128i tempSecComp[RANDOM_COMPUTE];
    for (int i = 0; i < RANDOM_COMPUTE; i++) {
      // not exactly counter mode - (rcounter+i,rcouter+i,rcounter+i,rcounter+i)
      tempSecComp[i] = _mm_set1_epi32(rCounter + i);
    }
    AES_ecb_encrypt_chunk_in_out(tempSecComp, pseudoRandomString, RANDOM_COMPUTE, &aes_key);
  }
  return pseudoRandomString[rCounter % RANDOM_COMPUTE];
}

mpc_t AESObject::get64Bits() {
  mpc_t ret;

#if ROSETTA_MPC_128
  random64BitNumber = newRandomNumber();
#else
  if (random64BitCounter == 0)
    random64BitNumber = newRandomNumber();
#endif

  uint64_t* temp = (uint64_t*)&random64BitNumber;

#if ROSETTA_MPC_128
  ret = (mpc_t(temp[0]) << 64) | (mpc_t(temp[1]));
#else
  uint8_t x = random64BitCounter % 2;
  switch (x) {
    case 0:
      ret = (mpc_t)temp[1];
      break;
    case 1:
      ret = (mpc_t)temp[0];
      break;
  }
#endif //ROSETTA_MPC_128

#if !ROSETTA_MPC_128
  random64BitCounter++;
  if (random64BitCounter == 2)
    random64BitCounter = 0;
#endif

  return ret;
}

small_mpc_t AESObject::get8Bits() {
  small_mpc_t ret;

  if (random8BitCounter == 0)
    random8BitNumber = newRandomNumber();

  uint8_t* temp = (uint8_t*)&random8BitNumber;
  ret = (small_mpc_t)temp[random8BitCounter];

  random8BitCounter++;
  if (random8BitCounter == 16)
    random8BitCounter = 0;

  return ret;
}

small_mpc_t AESObject::getBit() {
  small_mpc_t ret;
  __m128i temp;

  if (randomBitCounter == 0)
    randomBitNumber = newRandomNumber();

  uint8_t x = randomBitCounter % 8;
  switch (x) {
    case 0:
      temp = _mm_and_si128(randomBitNumber, BIT1);
      break;
    case 1:
      temp = _mm_and_si128(randomBitNumber, BIT2);
      break;
    case 2:
      temp = _mm_and_si128(randomBitNumber, BIT4);
      break;
    case 3:
      temp = _mm_and_si128(randomBitNumber, BIT8);
      break;
    case 4:
      temp = _mm_and_si128(randomBitNumber, BIT16);
      break;
    case 5:
      temp = _mm_and_si128(randomBitNumber, BIT32);
      break;
    case 6:
      temp = _mm_and_si128(randomBitNumber, BIT64);
      break;
    case 7:
      temp = _mm_and_si128(randomBitNumber, BIT128);
      break;
  }
  uint8_t* val = (uint8_t*)&temp;
  ret = (val[0] >> x);

  randomBitCounter++;
  if (randomBitCounter % 8 == 0)
    randomBitNumber = _mm_srli_si128(randomBitNumber, 1);

  if (randomBitCounter == 128)
    randomBitCounter = 0;

  return ret;
}

small_mpc_t AESObject::randModPrime() {
  small_mpc_t ret(0);

  do {
    ret = get8Bits();
  } while (ret >= BOUNDARY);

#if (PRIME_NUMBER == 127)

#if CPP_ASSEMBLY
  // this block by cyf, 2019
  __asm__(
    // reture value must push in al register
    "movb %0, %%al;\n\t"
    "movb %%al, %%dl;\n\t"
    "shr  $7, %%dl;\n\t" // ret >> 7
    "and  $127, %%al;\n\t" // ret & PRIME_NUMBER
    "add  %%dl, %%al;\n\t" // al is i = (ret & PRIME_NUMBER) + (ret >> 7)
    "movb %%al, %%dl;\n\t" // dl is also i
    "sub  $127, %%dl;\n\t" // dl is (i - PRIME_NUMBER)
    "cmpb $127, %%al;\n\t" // if (i >= PRIME_NUMBER)
    "jb label;\n\t"
    "movb %%dl, %%al;\n\t" // ret is (i - PRIME_NUMBER)
    "label:;\n\t"
    :
    : "r"(ret)
    : "%eax", "%ebx", "%ecx", "%edx");

  return (ret % PRIME_NUMBER);
#else
  small_mpc_t i = (ret & PRIME_NUMBER) + (ret >> 7);
  return (i >= PRIME_NUMBER) ? i - PRIME_NUMBER : i;
#endif

#else
  return (ret % PRIME_NUMBER);
#endif//(PRIME_NUMBER == 127)
}

small_mpc_t AESObject::randNonZeroModPrime() {
  small_mpc_t ret;
  do {
    ret = randModPrime();
  } while (ret == 0);

  return ret;
}

mpc_t AESObject::randModuloOdd() {
  mpc_t ret;
  do {
    ret = get64Bits();
  } while (ret == MINUS_ONE);
  return ret;
}

small_mpc_t AESObject::AES_random(int i) {
  small_mpc_t ret;
  do {
    ret = get8Bits();
  } while (ret >= ((256 / i) * i));

  return (ret % i);
}

void AESObject::AES_random_shuffle(vector<small_mpc_t>& vec, size_t begin_offset, size_t end_offset) {
  vector<small_mpc_t>::iterator it = vec.begin();
  auto first = it + begin_offset;
  auto last = it + end_offset;
  auto n = last - first;

  for (auto i = n - 1; i > 0; --i) {
    using std::swap;
    swap(first[i], first[AES_random(i + 1)]);
  }
}
