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
#pragma once

#include <string>
#include <regex>
#include <memory>
#include <vector>
#include "cc/modules/common/include/utils/str_type_convert.h"

using std::string;
using std::vector;

namespace rosetta {
namespace convert {

static bool is_secure_text(const string& text) {
  if (text.size() >= 2 && text[text.size() - 1] == '#')
    return true;
  else
    return false;
}
static bool is_binary_double(const string& text) {
  if (text.size() >= 2 && text[text.size() - 1] == '$')
    return true;
  return false;
}

// stateless decoder/encoder
namespace encoder {

// support string type
// 0. secure string: H_{secure_string}
// 1. variable input string or const attribute string: literal string (double)
// 2. unknown and not support
enum SupportDecodeType { SECURE_STR = 0, DOUBLE_STR, UNKNOWN_STR };

template <typename T>
int encode_to_secure(const T& t, string& encode_str) {
  rosetta::convert::to_binary_str(&t, sizeof(T), encode_str);
  return 0;
}

template <typename T>
int encode_to_secure(const vector<T>& t, vector<string>& secure_binary) {
  int size = t.size();
  secure_binary.resize(size);

  for (int i = 0; i < size; ++i) {
    rosetta::convert::to_binary_str(&t[i], sizeof(t[i]), secure_binary[i]);
    secure_binary[i].append(1, '#');
  }

  return 0;
}

// template <>
// int encode_to_secure<string>(const string& t, string& encode_str) {
//   encode_str = t;
//   return 0;
// }

// template <>
// int encode_to_secure<string>(const vector<string>& t, vector<string>& encode_strs) {
//   int size = t.size();
//   encode_strs.assign(t.begin(), t.end());
//   return 0;
// }

static SupportDecodeType probe_string_type(const string& text) {
  SupportDecodeType string_type = UNKNOWN_STR; // 0: secure string, 2: double literal string
  if (is_secure_text(text)) //check secure string first
  {
    return SECURE_STR;
  } else if (
    (('0' <= text[0] && '9' >= text[0]) || (text[0] == '-' || text[0] == '+')) &&
    std::isdigit(text[text.size() - 1])) // is float
  { // if (regex_match(text, regex("^[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)$"))) //check double string
    return DOUBLE_STR;
  } else {
    return string_type;
  }
}

static int decode_secure(
  const string& text,
  string& stext,
  SupportDecodeType string_type = UNKNOWN_STR) {
  if (text.empty()) {
    log_error << "null text to decode" ;
    return -1;
  }

  if (string_type == UNKNOWN_STR)
    string_type = probe_string_type(text);

  if (string_type == SECURE_STR) {
    stext = text.substr(0, text.size() - 1);
  } else if (string_type == DOUBLE_STR) {
    stext = text;
  } else {
    log_error << "find unknown string type, not support: " << text ;
    return -1;
  }

  return static_cast<int>(string_type);
}

static int decode_secure(const vector<string>& texts, vector<string>& stexts) {
  int size = texts.size();
  if (size == 0) {
    log_error << "empty decode texts !" ;
    return -1;
  }

  // only test the first element for convient
  if (texts[0].empty()) {
    log_error << "null string to decode" ;
    return -1;
  }

  SupportDecodeType string_type = probe_string_type(
    texts[0]); // 0: secure string, 1: private_input string, 2: double literal string
  if (string_type == UNKNOWN_STR) {
    log_error << "find unknown string type, not support: " << texts[0] ;
    return -1;
  }

  stexts.resize(size);
  if (string_type == SECURE_STR) {
    for (int i = 0; i < size; ++i) {
      if (0 != decode_secure(texts[i], stexts[i])) {
        log_error << "decode '" << texts[i] << "' failed, please input secure text" ;
        stexts.clear();
        return -1;
      }
    }
  } else // string_type == DOUBLE_STR
    stexts.assign(texts.begin(), texts.end());

  return static_cast<int>(string_type);
} //decode

} // namespace encoder

} // namespace convert
} // namespace rosetta
