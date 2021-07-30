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
#include "cc/modules/common/include/utils/str_type_convert.h"
#include "cc/modules/common/include/utils/perf_stats.h"
#include "cc/modules/common/include/utils/helper.h"

namespace rosetta {
// step1 define the global(static) timer variables
DEFINE_GLOBAL_TIMER_COUNTER(convert_string_to_double_timer)

// step2 define an atexit function(static)
DEFINE_AT_EXIT_FUNCTION_BEG()
DEFINE_AT_EXIT_FUNCTION_BODY(convert_string_to_double_timer)
DEFINE_AT_EXIT_FUNCTION_END()

// step3 use ELAPSED_STATISTIC_BEG/ELAPSED_STATISTIC_END in the program
} // namespace rosetta

namespace rosetta {
namespace convert {
template void to_binary_str<double>(const double&, std::string&);
template void to_binary_str<mpc_t>(const mpc_t&, std::string&);
template void to_binary_str<double>(const std::vector<double>&, std::vector<std::string>&);
template void to_binary_str<mpc_t> (const std::vector<mpc_t>&,  std::vector<std::string>&);

// const std::string&
template double from_binary_str<double>(const std::string&);
template mpc_t from_binary_str<mpc_t>(const std::string&);
// const std::string&, T&
template void from_binary_str<double>(const std::string&, double&);
template void from_binary_str<mpc_t>(const std::string&, mpc_t&);
// std::vector<std::string>&, std::vector<T>&
template void from_binary_str<double>(const std::vector<std::string>&, std::vector<double>&);
template void from_binary_str<mpc_t>(const std::vector<std::string>&, std::vector<mpc_t>&);
// std::vector<std::string>&
template std::vector<double> from_binary_str<double>(const std::vector<std::string>&);
template std::vector<mpc_t> from_binary_str<mpc_t>(const std::vector<std::string>&);

vector<double> from_double_str(const vector<string>& s) {
  ELAPSED_STATISTIC_BEG(convert_string_to_double_timer);
  vector<double> t(s.size());
  for (int i = 0; i < s.size(); i++) {
    // t[i] = std::stod(s[i]);
    t[i] = to_double(s[i].c_str());
  }
  ELAPSED_STATISTIC_END(convert_string_to_double_timer);
  return t;
}

void from_double_str(const vector<string>& s, vector<double>& t) {
  ELAPSED_STATISTIC_BEG(convert_string_to_double_timer);
  t.resize(s.size());
  for (int i = 0; i < s.size(); i++) {
    // t[i] = std::stod(s[i]);
    t[i] = to_double(s[i].c_str());
  }
  ELAPSED_STATISTIC_END(convert_string_to_double_timer);
}

vector<int64_t> from_int_str(const vector<string>& s) {
  vector<int64_t> t(s.size());
  for (int i = 0; i < s.size(); i++)
    t[i] = std::stoll(s[i]);

  return t;
}

void from_int_str(const vector<string>& s, vector<int64_t>& t) {
  t.resize(s.size());
  for (int i = 0; i < s.size(); i++)
    t[i] = std::stoll(s[i]);
}

} // namespace convert
} // namespace rosetta
