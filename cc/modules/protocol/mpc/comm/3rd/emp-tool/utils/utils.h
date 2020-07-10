#ifndef UTILS_H__
#define UTILS_H__
#include <string>
#include "emp-tool/utils/block.h"
#include <sstream>
#include <cstddef>//https://gcc.gnu.org/gcc-4.9/porting_to.html
#include "emp-tool/utils/prg.h"
#include <chrono>
#define macro_xstr(a) macro_str(a)
#define macro_str(a) #a

using std::string;
using std::chrono::time_point;
using std::chrono::high_resolution_clock;

namespace emp {
template<typename T>
void inline delete_array_null(T * ptr);

inline void error(const char * s, int line = 0, const char * file = nullptr);

template<class... Ts>
void run_function(void *function, const Ts&... args);

inline void parse_party_and_port(char ** arg, int argc, int * party, int * port);

// Timing related
inline time_point<high_resolution_clock> clock_start();
inline double time_from(const time_point<high_resolution_clock>& s);

//block conversions
block bool_to128(const bool * data);
void int64_to_bool(bool * data, uint64_t input, int length);

#include "emp-tool/utils/utils.hpp"
}
#endif// UTILS_H__
