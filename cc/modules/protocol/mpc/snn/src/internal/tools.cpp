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
/*
 * 	BMR_BGW_aux.cpp
 *
 *      Author: Aner Ben-Efraim, Satyanarayana
 *
 * 	year: 2016
 *
 */

#include "cc/modules/common/include/utils/rtt_logger.h"
#include "cc/modules/common/include/utils/helper.h"
#include "cc/modules/common/include/utils/rtt_exceptions.h"
#include "cc/modules/protocol/mpc/snn/src/internal/tools.h"
#include "cc/modules/protocol/mpc/snn/src/internal/snn_helper.h"
#include <bitset>
#include <mutex>
#include <cassert>
#include <stdint.h>
using namespace std;
#define NANOSECONDS_PER_SEC 1E9

// For time measurements
clock_t tStart;
struct timespec requestStart, requestEnd;
bool alreadyMeasuringTime = false;

int roundComplexitySend = 0;
int roundComplexityRecv = 0;
bool alreadyMeasuringRounds = false;

/* Here are the global variables, precomputed once in order to save time*/
// powers[x*deg+(i-1)]=x^i, i.e., POW(SETX(X),i). Notice the (i-1), due to
// POW(x,0) not saved (always 1...)
__m128i* powers;
// the coefficients used in the reconstruction
__m128i* baseReduc;
__m128i* baseRecon;
// allocation of memory for polynomial coefficients
__m128i* polyCoef;
// one in the field
const __m128i ONE = SET_ONE;
// zero in the field
const __m128i ZERO = SET_ZERO;
// the constant used for BGW step by this party
__m128i BGWconst;

__m128i* sharesTest;

void gfmul(__m128i a, __m128i b, __m128i* res) {
  __m128i tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10, tmp11, tmp12;
  __m128i XMMMASK = _mm_setr_epi32(0xffffffff, 0x0, 0x0, 0x0);
  tmp3 = _mm_clmulepi64_si128(a, b, 0x00);
  tmp6 = _mm_clmulepi64_si128(a, b, 0x11);
  tmp4 = _mm_shuffle_epi32(a, 78);
  tmp5 = _mm_shuffle_epi32(b, 78);
  tmp4 = _mm_xor_si128(tmp4, a);
  tmp5 = _mm_xor_si128(tmp5, b);
  tmp4 = _mm_clmulepi64_si128(tmp4, tmp5, 0x00);
  tmp4 = _mm_xor_si128(tmp4, tmp3);
  tmp4 = _mm_xor_si128(tmp4, tmp6);
  tmp5 = _mm_slli_si128(tmp4, 8);
  tmp4 = _mm_srli_si128(tmp4, 8);
  tmp3 = _mm_xor_si128(tmp3, tmp5);
  tmp6 = _mm_xor_si128(tmp6, tmp4);
  tmp7 = _mm_srli_epi32(tmp6, 31);
  tmp8 = _mm_srli_epi32(tmp6, 30);
  tmp9 = _mm_srli_epi32(tmp6, 25);
  tmp7 = _mm_xor_si128(tmp7, tmp8);
  tmp7 = _mm_xor_si128(tmp7, tmp9);
  tmp8 = _mm_shuffle_epi32(tmp7, 147);

  tmp7 = _mm_and_si128(XMMMASK, tmp8);
  tmp8 = _mm_andnot_si128(XMMMASK, tmp8);
  tmp3 = _mm_xor_si128(tmp3, tmp8);
  tmp6 = _mm_xor_si128(tmp6, tmp7);
  tmp10 = _mm_slli_epi32(tmp6, 1);
  tmp3 = _mm_xor_si128(tmp3, tmp10);
  tmp11 = _mm_slli_epi32(tmp6, 2);
  tmp3 = _mm_xor_si128(tmp3, tmp11);
  tmp12 = _mm_slli_epi32(tmp6, 7);
  tmp3 = _mm_xor_si128(tmp3, tmp12);

  *res = _mm_xor_si128(tmp3, tmp6);
}

// this function works correctly only if all the upper half of b is zeros
void gfmulHalfZeros(__m128i a, __m128i b, __m128i* res) {
  __m128i tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10, tmp11, tmp12;
  __m128i XMMMASK = _mm_setr_epi32(0xffffffff, 0x0, 0x0, 0x0);
  tmp3 = _mm_clmulepi64_si128(a, b, 0x00);
  tmp4 = _mm_shuffle_epi32(a, 78);
  tmp5 = _mm_shuffle_epi32(b, 78);
  tmp4 = _mm_xor_si128(tmp4, a);
  tmp5 = _mm_xor_si128(tmp5, b);
  tmp4 = _mm_clmulepi64_si128(tmp4, tmp5, 0x00);
  tmp4 = _mm_xor_si128(tmp4, tmp3);
  tmp5 = _mm_slli_si128(tmp4, 8);
  tmp4 = _mm_srli_si128(tmp4, 8);
  tmp3 = _mm_xor_si128(tmp3, tmp5);
  tmp6 = tmp4;
  tmp7 = _mm_srli_epi32(tmp6, 31);
  tmp8 = _mm_srli_epi32(tmp6, 30);
  tmp9 = _mm_srli_epi32(tmp6, 25);
  tmp7 = _mm_xor_si128(tmp7, tmp8);
  tmp7 = _mm_xor_si128(tmp7, tmp9);
  tmp8 = _mm_shuffle_epi32(tmp7, 147);

  tmp8 = _mm_andnot_si128(XMMMASK, tmp8);
  tmp3 = _mm_xor_si128(tmp3, tmp8);
  tmp10 = _mm_slli_epi32(tmp6, 1);
  tmp3 = _mm_xor_si128(tmp3, tmp10);
  tmp11 = _mm_slli_epi32(tmp6, 2);
  tmp3 = _mm_xor_si128(tmp3, tmp11);
  tmp12 = _mm_slli_epi32(tmp6, 7);
  tmp3 = _mm_xor_si128(tmp3, tmp12);
  *res = _mm_xor_si128(tmp3, tmp6);
}

// multiplies a and b
__m128i gfmul(__m128i a, __m128i b) {
  __m128i ans;
  gfmul(a, b, &ans);
  return ans;
}

// This function works correctly only if all the upper half of b is zeros
__m128i gfmulHalfZeros(__m128i a, __m128i b) {
  __m128i ans;
  gfmulHalfZeros(a, b, &ans);
  return ans;
}

__m128i gfpow(__m128i x, int deg) {
  __m128i ans = ONE;
  // TODO: improve this
  for (int i = 0; i < deg; i++)
    ans = MUL(ans, x);

  return ans;
}

__m128i fastgfpow(__m128i x, int deg) {
  __m128i ans = ONE;
  if (deg == 0)
    return ans;
  else if (deg == 1)
    return x;
  else {
    int hdeg = deg / 2;
    __m128i temp1 = fastgfpow(x, hdeg);
    __m128i temp2 = fastgfpow(x, deg - hdeg);
    ans = MUL(temp1, temp2);
  }
  return ans;
}

__m128i square(__m128i x) {
  return POW(x, 2);
}

__m128i inverse(__m128i x) {
  __m128i powers[128];
  powers[0] = x;
  __m128i ans = ONE;
  for (int i = 1; i < 128; i++) {
    powers[i] = SQ(powers[i - 1]);
    ans = MUL(ans, powers[i]);
  }
  return ans;
}

string _sha256hash_(char* input, int length) {
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256_CTX sha256;
  SHA256_Init(&sha256);
  SHA256_Update(&sha256, input, length);
  SHA256_Final(hash, &sha256);
  string output;
  output.resize(SHA256_DIGEST_LENGTH);
  for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    output[i] = hash[i];
  return output;
}

string sha256hash(char* input, int length) {
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256_CTX sha256;
  SHA256_Init(&sha256);
  SHA256_Update(&sha256, input, length);
  SHA256_Final(hash, &sha256);
  char outputBuffer[65];
  for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
    sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
  }
  outputBuffer[64] = 0;
  return outputBuffer;
}

void printError(string error) {
  log_error << error ;
  throw other_exp("printError error:" + error);
}

string __m128i_toHex(__m128i var) {
  static char const alphabet[] = "0123456789abcdef";
  // 	output += alphabet[((int)s[i]+128)/16];
  // 	output += alphabet[((int)s[i]+128)%16];
  // }

  char* val = (char*)&var;
  stringstream ss;
  for (unsigned int i = 0; i < 16; i++) {
    // if ((int)val[i] + 128 < 16)
    // ss << '0';
    // ss << std::hex << (int)val[i] + 128;

    ss << alphabet[((int)val[i] + 128) / 16];
    ss << alphabet[((int)val[i] + 128) % 16];
  }
  return ss.str();
}

string toHex(string s) {
  static char const alphabet[] = "0123456789abcdef";
  string output;
  for (int i = 0; i < s.length(); i++) {
    output += alphabet[((int)s[i] + 128) / 16];
    output += alphabet[((int)s[i] + 128) % 16];
  }
  return output;
}

string __m128i_toString(__m128i var) {
  char* val = (char*)&var;
  stringstream ss;
  for (unsigned int i = 0; i < 16; i++)
    ss << val[i];
  return ss.str();
}

__m128i stringTo__m128i(string str) {
  if (str.length() != 16)
    log_error << "Error: Length of input to stringTo__m128i is " << str.length() ;

  __m128i output;
  char* val = (char*)&output;
  for (int i = 0; i < 16; i++)
    val[i] = str[i];
  return output;
}

unsigned int charValue(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  }
  if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }
  if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }
  return -1;
}

double diff(timespec start, timespec end) {
  timespec temp;

  if ((end.tv_nsec - start.tv_nsec) < 0) {
    temp.tv_sec = end.tv_sec - start.tv_sec - 1;
    temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
  } else {
    temp.tv_sec = end.tv_sec - start.tv_sec;
    temp.tv_nsec = end.tv_nsec - start.tv_nsec;
  }
  return temp.tv_sec + (double)temp.tv_nsec / NANOSECONDS_PER_SEC;
}

string convertBooltoChars(bool* input, int length) {
  stringstream output;
  int count = 0;
  for (int i = 0; i < ceil((float)length / 8); i++) {
    int ch = 0;
    for (int c = 0; c < 8; c++) {
      ch *= 2;
      if (count < length)
        ch += input[count++];
    }
    output << (char)ch;
  }
  return output.str();
}

string convertCharsToString(char* input, int size) {
  stringstream ss;
  for (int i = 0; i < size; i++)
    ss << input[i];
  return ss.str();
}

void print(__m128i* arr, int size) {
  for (int i = 0; i < size; i++) {
    print128_num(arr[i]);
  }
}

void print128_num(__m128i var) {
  uint16_t* val = (uint16_t*)&var;
  printf(
    "Numerical:%i %i %i %i %i %i %i %i\n", val[0], val[1], val[2], val[3], val[4], val[5], val[6],
    val[7]);
}

void print_usage(const char* bin) {
  cout << "Usage: " << bin
       << " MPC_TYPE PARTY_NUM IP_ADDR_FILE AES_SEED_INDEP AES_SEED_COMMON "
          "TRAINING_DATA TRAINING_LABELS TESTING_DATA TESTING_LABELS"
       << endl;
  cout << endl;
  cout << "Required Arguments:\n";
  cout << "MPC_TYPE			Type of MPC (STANDALONE, 3PC or 4PC)\n";
  cout << "PARTY_NUM			Party Identifier (0,1,2,3 or 4 "
          "(standalone))\n";
  cout << "IP_ADDR_FILE		\tIP Address file\n";
  cout << "AES_SEED_INDEP		\tAES seed file independent\n";
  cout << "AES_SEED_COMMON	\t \tAES seed file common (for shared keys)\n";
  cout << "TRAINING_DATA		\tTraining data file\n";
  cout << "TRAINING_LABELS	\t \tTraining labels file\n";
  cout << "TESTING_DATA		\tTesting data file\n";
  cout << "TESTING_LABELS		\tTesting labels file\n";
  cout << endl;
  cout << "Report bugs to swagh@princeton.edu" << endl;
  throw other_exp("print_usage!");
}

void start_time() {
  if (alreadyMeasuringTime) {
    log_error << "Nested timing measurements" ;
    throw other_exp("Nested timing measurements!");
  }

  tStart = clock();
  clock_gettime(CLOCK_REALTIME, &requestStart);
  alreadyMeasuringTime = true;
}

void end_time(string str) {
  if (!alreadyMeasuringTime) {
    log_error << "start_time() never called" ;
    throw other_exp("start_time() never called!");
  }

  clock_gettime(CLOCK_REALTIME, &requestEnd);
  log_info << "------------------------------------" ;
  log_info << "Wall Clock time for " << str << ": " << diff(requestStart, requestEnd) << " sec\n";
  log_info << "CPU time for " << str << ": " << (double)(clock() - tStart) / CLOCKS_PER_SEC
           << " sec\n";
  log_info << "------------------------------------" ;
  alreadyMeasuringTime = false;
}

void start_rounds() {
  if (alreadyMeasuringRounds) {
    log_error << "Nested round measurements" ;
    throw other_exp("Nested round measurements!");
  }

  roundComplexitySend = 0;
  roundComplexityRecv = 0;
  alreadyMeasuringRounds = true;
}

void end_rounds(string str) {
  if (!alreadyMeasuringTime) {
    log_error << "start_rounds() never called" ;
    throw other_exp("start_rounds() never called!");
  }

  log_info << "------------------------------------" ;
  log_info << "Send Round Complexity of " << str << ": " << roundComplexitySend ;
  log_info << "Recv Round Complexity of " << str << ": " << roundComplexityRecv ;
  log_info << "------------------------------------" ;
  alreadyMeasuringRounds = false;
}

void print_myType(mpc_t var, string message, string type, int float_precision) {
  if (type == "BITS")
    log_info << message << ": " << bitset<64>(var) ;
  else if (type == "FLOAT")
    log_info << message << ": " << (static_cast<int64_t>(var)) / (float)(1 << float_precision)
             ;
  else if (type == "SIGNED")
    log_info << message << ": " << static_cast<int64_t>(var) ;
  else if (type == "UNSIGNED")
    log_info << message << ": " << to_readable_dec(var) ;
}

void print_linear(mpc_t var, string type, int float_precision) {
  if (type == "BITS")
    log_info << bitset<64>(var) << " ";
  else if (type == "FLOAT")
    log_info << (static_cast<int64_t>(var)) / (float)(1 << float_precision) << " ";
  else if (type == "SIGNED")
    log_info << static_cast<int64_t>(var) << " ";
  else if (type == "UNSIGNED")
    log_info << to_readable_dec(var) << " ";
}

void checkOverflow(
  const vector<mpc_t>& a, const vector<mpc_t>& b, size_t rows, size_t common_dim, size_t columns,
  size_t transpose_a, size_t transpose_b) {
#if 0
  Matrix<mpc_t, Dynamic, Dynamic, RowMajor> eigen_a(rows, common_dim);
  Matrix<mpc_t, Dynamic, Dynamic, RowMajor> eigen_b(common_dim, columns);

  for (size_t i = 0; i < rows; ++i) {
    for (size_t j = 0; j < common_dim; ++j) {
      if (transpose_a)
        eigen_a(i, j) = a[j * rows + i];
      else
        eigen_a(i, j) = a[i * common_dim + j];
    }
  }

  for (size_t i = 0; i < common_dim; ++i) {
    for (size_t j = 0; j < columns; ++j) {
      if (transpose_b)
        eigen_b(i, j) = b[j * common_dim + i];
      else
        eigen_b(i, j) = b[i * columns + j];
    }
  }

  mpc_t maxVal = std::numeric_limits<mpc_t>::max();
  mpc_t sumVal = 0;

  for (size_t rii = 0; rii < rows; rii++) {
    for (size_t cii = 0; cii < columns; cii++) {
      for (size_t com = 0; com < common_dim; com++) {
        mpc_t temp_a = eigen_a(rii, com);
        mpc_t temp_b = eigen_b(com, cii);
        mpc_t temp_c = 0;
        bool a_sign = temp_a >> (8 * sizeof(mpc_t) - 1);
        bool b_sign = temp_b >> (8 * sizeof(mpc_t) - 1);
        if (a_sign || b_sign) {
          // if (__builtin_smulll_overflow(temp_a, temp_b, &temp_c))
          // {
          // 	assert(0 && "Multiplication overflow!!!");
          // }
        } else if (__builtin_umul_overflow(temp_a, temp_b, (unsigned long long*)&temp_c)) {
          assert(0 && "Multiplication overflow!!!");
          throw other_exp("Multiplication overflow!!!");
        }

        bool c_sign = temp_c >> (8 * sizeof(mpc_t) - 1);
        bool sum_sign = sumVal >> (8 * sizeof(mpc_t) - 1);
        if (c_sign || sum_sign) {
          // if (__builtin_saddll_overflow(sumVal, temp_c, &sumVal))
          // {
          // 	// assert(0 && "Addition overflow!!!");
          // }
        } else if (__builtin_uadd_overflow(sumVal, temp_c, (unsigned long long*)&sumVal)) {
          assert(0 && "Addition overflow!!!");
          throw other_exp("Addition overflow!!!");
        }
      }
    }
  }
#endif
}

void sigmoidSA(const vector<mpc_t>& input, vector<mpc_t>& output, size_t rows, size_t cols, int float_precision) {
  for (size_t i = 0; i < rows; ++i) {
    for (size_t j = 0; j < cols; ++j) {
      size_t index = i * cols + j;
      auto x = MpcTypeToFloat(input[index], float_precision);
      output[index] = FloatToMpcType(1.0 / (1.0 + exp(-x)), float_precision);
    }
  }
}


mpc_t divideMyTypeSA(mpc_t a, mpc_t b, int float_precision/*=FLOAT_PRECISION_DEFAULT*/) {
  // assert((sizeof(double) == sizeof(mpc_t)) && "sizeof(double) !=
  // sizeof(mpc_t)");
  assert((b != 0) && "Cannot divide by 0");
  return FloatToMpcType((double)((signed_mpc_t)a) / (double)((signed_mpc_t)b), float_precision);
}

mpc_t dividePlainSA(mpc_t a, int b) {
  assert((b != 0) && "Cannot divide by 0");
  return static_cast<mpc_t>(static_cast<signed_mpc_t>(a) / static_cast<signed_mpc_t>(b));
}

void dividePlainSA(vector<mpc_t>& vec, int divisor) {
  // assert((sizeof(double) == sizeof(mpc_t)) && "sizeof(double) !=
  // sizeof(mpc_t)");
  assert((divisor != 0) && "Cannot divide by 0");
  for (int i = 0; i < vec.size(); ++i)
    vec[i] = (mpc_t)((double)((signed_mpc_t)vec[i]) / (double)((signed_mpc_t)divisor));
}

mpc_t multiplyMyTypesSA(mpc_t a, mpc_t b, int shift) {
  mpc_t ret;
  ret = static_cast<mpc_t>(
    (static_cast<signed_mpc_t>(a) * static_cast<signed_mpc_t>(b)) / (1 << shift));
  return ret;
}

size_t partner(size_t party) {
  size_t ret(99);

  switch (party) {
    case PARTY_A:
      ret = PARTY_C;
      break;
    case PARTY_B:
      ret = PARTY_D;
      break;
    case PARTY_C:
      ret = PARTY_A;
      break;
    case PARTY_D:
      ret = PARTY_B;
      break;
    default:
      // error
      throw other_exp("wrong partner: " + to_string(party));
  }
  return ret;
}

size_t adversary(size_t party) {
  size_t ret(99);

  switch (party) {
    case PARTY_A:
      ret = PARTY_B;
      break;
    case PARTY_B:
      ret = PARTY_A;
      break;
    case PARTY_C:
      ret = PARTY_D;
      break;
    case PARTY_D:
      ret = PARTY_C;
      break;
    default:
      // error
      throw other_exp("wrong adversary: " + to_string(party));
  }
  return ret;
}

small_mpc_t subtractModPrime(small_mpc_t a, small_mpc_t b) {
  if (b == 0)
    return a;
  else {
    b = (PRIME_NUMBER - b);
    return kAdditionModPrime[a][b];
  }
}

void wrapAround(
  const vector<mpc_t>& a, const vector<mpc_t>& b, vector<small_mpc_t>& c, size_t size) {
  for (size_t i = 0; i < size; ++i)
    c[i] = wrapAround(a[i], b[i]);
}

void XORVectors(
  const vector<small_mpc_t>& a, const vector<small_mpc_t>& b, vector<small_mpc_t>& c, size_t size) {
  for (size_t i = 0; i < size; ++i)
    c[i] = a[i] ^ b[i];
}

void umul64wide(uint64_t a, uint64_t b, uint64_t* hi, uint64_t* lo) {
  uint64_t a_lo = (uint64_t)(uint32_t)a;
  uint64_t a_hi = a >> 32;
  uint64_t b_lo = (uint64_t)(uint32_t)b;
  uint64_t b_hi = b >> 32;

  uint64_t p0 = a_lo * b_lo;
  uint64_t p1 = a_lo * b_hi;
  uint64_t p2 = a_hi * b_lo;
  uint64_t p3 = a_hi * b_hi;

  uint32_t cy = (uint32_t)(((p0 >> 32) + (uint32_t)p1 + (uint32_t)p2) >> 32);

  *lo = p0 + (p1 << 32) + (p2 << 32);
  *hi = p3 + (p1 >> 32) + (p2 >> 32) + cy;
}

/* compute (a*b) >> s, for s in [0,63] */
// This piece has correctness issues when both the multiplicands are large
// numbers
uint64_t mulshift(uint64_t a, uint64_t b, int s) {
  uint64_t res;
  uint64_t hi = 0;
  uint64_t lo = 0;
  umul64wide(a, b, &hi, &lo);
  if (s) {
    res = ((uint64_t)hi << (64 - s)) | ((uint64_t)lo >> s);
  } else {
    res = lo;
  }
  return res;
}

/* compute (a*b) >> s, for s in [0,63] Assembly code for Intel C/C++ compiler */
int64_t mulshift_assembly(int64_t a, int64_t b, int s) {
  int64_t res;
  __asm__(
    "movq  %1, %%rax;\n\t" // rax = a
    "movl  %3, %%ecx;\n\t" // ecx = s
    "imulq %2;\n\t" // rdx:rax = a * b
    "shrdq %%cl, %%rdx, %%rax;\n\t" // rax = int64_t (rdx:rax >> s)
    "movq  %%rax, %0;\n\t" // res = rax
    : "=rm"(res)
    : "rm"(a), "rm"(b), "rm"(s)
    : "%rax", "%rdx", "%ecx");
  return res;
}

mpc_t multiplyMyTypes(mpc_t a, mpc_t b, size_t shift) {
  if (CPP_ASSEMBLY)
    return (mpc_t)mulshift_assembly((int64_t)a, (int64_t)b, shift);
  else {
    log_warn << "This multiplication function has bugs" ;
    return mulshift(a, b, shift);
  }
}

void log_print(string str) {
#if (MPC_LOG_DEBUG)
  TDEB("{}", str.data());
#else
  TINFO("{}", str.data());
#endif
}

void error(string str) {
  log_error << "Error: " << str ;
  throw other_exp("error Error: " + str);
}

void notYet() {
  throw other_exp("rosetta mpc don't support four sides now!");
}

void convolutionReshape(
  const vector<mpc_t>& vec, vector<mpc_t>& vecShaped, size_t ih, size_t iw, size_t C, size_t B,
  size_t fh, size_t fw, size_t sy, size_t sx) {
  size_t p_range = (ih - fh + 1);
  size_t q_range = (iw - fw + 1);
  size_t size_activation = ih * iw * C * B;
  size_t size_shaped = ((((iw - fw) / sx) + 1) * (((ih - fh) / sy) + 1) * B) * (fw * fh * C);

  // assert((sx == 1 and sy == 1) && "Stride not implemented in generality");
  assert(fw >= sx and fh >= sy && "Check implementation");
  // assert((iw - fw)%sx == 0 && "Check implementations for this unmet
  // condition");
  assert((ih - fh) % sy == 0 && "Check implementations for this unmet condition");
  // assert(vec.size() == size_activation && "Dimension issue with
  // convolutionReshape"); assert(vecShaped.size() == size_shaped && "Dimension
  // issue with convolutionReshape");

  size_t loc = 0, counter = 0;
  for (size_t i = 0; i < B; ++i)
    for (size_t j = 0; j < p_range; j += sy)
      for (size_t k = 0; k < q_range; k += sx)
        for (size_t l = 0; l < C; ++l) {
          loc = i * iw * ih * C + l * iw * ih + j * iw + k;
          for (size_t a = 0; a < fh; ++a)
            for (size_t b = 0; b < fw; ++b) {
              size_t real_loc = loc + a * iw + b;
              assert(real_loc < vec.size());
              vecShaped[counter++] = vec[real_loc];
            }
        }
}

void maxPoolReshape(
  const vector<mpc_t>& vec, vector<mpc_t>& vecShaped, size_t ih, size_t iw, size_t D, size_t B,
  size_t fh, size_t fw, size_t sy, size_t sx) {
  assert(fw >= sx and fh >= sy && "Check implementation");
  assert((iw - fw) % sx == 0 && "Check implementations for this unmet condition");
  assert((ih - fh) % sy == 0 && "Check implementations for this unmet condition");
  assert(vec.size() == vecShaped.size() && "Dimension issue with convolutionReshape");

  size_t loc = 0, counter = 0;
  for (size_t i = 0; i < B; ++i)
    for (size_t j = 0; j < D; ++j)
      for (size_t k = 0; k < ih - fh + 1; k += sy)
        for (size_t l = 0; l < iw - fw + 1; l += sx) {
          loc = i * iw * ih * D + j * iw * ih + k * iw + l;
          for (size_t a = 0; a < fh; ++a)
            for (size_t b = 0; b < fw; ++b)
              vecShaped[counter++] = vec[loc + a * iw + b];
        }
}

void start_m() {
  start_time();
  //start_communication();
}

void end_m(string str) {
  end_time(str);
  //pause_communication();
  //end_communication(str);
}

void compress_C2(const vector<mpc_t>& src_C2, vector<small_mpc_t>& dest_C2) {
  const size_t CompUnitSizes = (sizeof(mpc_t) - 1);
  size_t srcSizes = src_C2.size();
  for (size_t i = 0; i < srcSizes; i++) {
    for (size_t pos = CompUnitSizes; pos >= 1; pos--) {
      dest_C2.push_back(src_C2[i] >> (pos * sizeof(small_mpc_t) * 8));
    }
  }

  assert(dest_C2.size() == srcSizes * CompUnitSizes);
}

void restore_C2(const vector<small_mpc_t>& src_C2, vector<mpc_t>& dest_C2) {
  const size_t CompUnitSizes = (sizeof(mpc_t) - 1);
  size_t srcSizes = src_C2.size();
  assert(srcSizes % CompUnitSizes == 0);
  for (size_t i = 0; i < srcSizes; i += CompUnitSizes) {
    mpc_t sum = 0;
    for (size_t pos = 0; pos < CompUnitSizes; pos++) {
      mpc_t temp = src_C2[i + pos];
      sum += (temp << (CompUnitSizes - pos) * sizeof(small_mpc_t) * 8);
    }
    dest_C2.push_back(sum);
  }
}
