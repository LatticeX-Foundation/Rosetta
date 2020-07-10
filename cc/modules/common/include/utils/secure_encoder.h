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


// SecureText construction:
// [prefix-protcol]  +  [string-text]  +  [suffix-dtype]
// prefix-protcol: snn(SecureNN), helix, plain
// string-text: text content
// suffix-dtype: int32, int64, float32, float64, string
struct SecureText {
  const char SP='_';
  enum ProtType {
    PLAIN=0,
    SNN=1,
    HELIX=2
  };
  enum DataType {
    D_INT32=0,
    D_INT64=1,
    D_FLOAT32=2,
    D_FLOAT64=3,
    D_STRING=4
  };
  
  ProtType prot; //PLAIN, SNN, HELIX
  string content;//string text
  //DataType dtype;//D_INT32, D_INT64, D_FLOAT32, D_FLOAT64, D_STRING

  static bool check_prot(int protcol) {
    if (protcol < PLAIN || protcol > HELIX)
    {
      //NOT SUPPORT PROTCOL
      return false;
    }

    return true;
  }

  static bool check_dtype(int protcol) {
    if (protcol < D_INT32 || protcol > D_STRING)
    {
      //NOT SUPPORT DTYPE
      return false;
    }

    return true;
  }

  char prot_str() const {
    // prot:  from G, H, ..., P
    return 'G' + prot;
  }

  char dtype_str() const {
    // dtype: from Q, R, ..., Z
    return 'Q' + prot;
  }

  string to_string() const {
    string text;
    //prot:  from G, H, ..., P
    text.append(1, prot_str());
    text.append(1, SP);
    //string text
    text.append(content);
    //text.append(1, SP);
    //dtype: from Q, R, ..., Z
    //text.append(1, dtype_str());

    return text;
  }

  static bool is_secure_text(const string& text) {
    int prot = text[0] - 'G';
    if (text.size() >= 3 && text[1] == '_' && check_prot(prot))
    {
      return true;
    }
    else
      return false;
  }

  
  static int from_string(const string& text, SecureText& stext) {
    if (text.empty())
    {
      log_error << "null text";
      return -1;
    }

    smatch results;
    //check secure type
    // if (std::regex_match(text, results, std::regex("([G-P]{1})_([[:alnum:]]{2,})"))) {
    //   if (results.size() < 2) {
    //     cout << "decode string: " << text << "matched, but with illegal matches !" << endl;
    //     return -1;
    //   }
    //   log_debug << "=> secure text:  " << text;

    //   // prot
    //   int prot = text[0] - 'G';
    //   if (!check_prot(prot)) {
    //     cout << "protocol type not support: " << results[0] << endl;
    //     return -1;
    //   }
    //   stext.prot = static_cast<SecureText::ProtType>(SecureText::PLAIN + prot);
    //   stext.content = results[2];
    //   //text.dtype = dtype;
    // }
    if (is_secure_text(text))
    {
      stext.prot = static_cast<SecureText::ProtType>(SecureText::PLAIN + (text[0] - 'G'));
      stext.content = text.substr(2);
    }
    else
    {
      log_error << "bad secure text : " << text << endl;
      return -2;
    }

    return 0;
  }
};

// stateless decoder/encoder
namespace encoder {
  using rosetta::convert::SecureText;

  // support string type
  // 0. secure string: H_{secure_string}
  // 1. private input: xxxxxxx#
  // 2. variable input string or const attribute string: literal string (double) 
  // 3. unknown and not support
  enum SupportDecodeType{
    SECURE_STR = 0,
    PRIVATE_IN_STR = 1,
    DOUBLE_STR = 2,
    UNKNOWN_STR = 3
  };

  template <typename T>
  int encode(const T& t, SecureText::ProtType prot, string& encode_str)
  {
    SecureText text;
    if (!SecureText::check_prot(prot))
      return -1;

    text.prot = prot;
    rosetta::convert::to_hex_str_copy(&t, sizeof(T), text.content);
    
    encode_str = text.to_string();
    return 0;
  }

  template <typename T>
  int encode(const vector<T>& t, SecureText::ProtType prot, vector<string>& encode_strs)
  {
    if (!SecureText::check_prot(prot))
      return -1;

    int size = t.size();
    encode_strs.resize(size);

    SecureText text = SecureText();
    text.prot = prot;
    for (int i = 0; i < size; ++i)
    {
      rosetta::convert::to_hex_str_copy(&t[i], sizeof(T), text.content);
      encode_strs[i] = text.to_string();
      text.content.clear();
    }
    
    return 0;
  }

  template <>
  int encode<string>(const string& t, SecureText::ProtType prot, string& encode_str)
  {
    SecureText text;
    if (!SecureText::check_prot(prot))
      return -1;
    
    text.prot = prot;
    text.content = t;
    
    encode_str = text.to_string();
    return 0;
  }

  template <>
  int encode<string>(const vector<string>& t, SecureText::ProtType prot, vector<string>& encode_strs)
  {
    if (!SecureText::check_prot(prot))
      return -1;
    
    int size = t.size();
    encode_strs.resize(size);

    SecureText text = SecureText();
    text.prot = prot;
    for (int i = 0; i < size; ++i)
    {
      text.content = t[i];
      encode_strs[i] = text.to_string();
    }
    
    return 0; 
  }

  // decode input text Snn Type input message
  static int decode_secure(const string& text, SecureText& stext) {
    if (0 != SecureText::from_string(text, stext)) {
      cout << "decode: " << text << ", failed, please input match pattern: ([G-P]{1})_([:alnum:]{2,}) "
           << endl;
      return -1;
    }

    return 0;
  }

  static int decode_secure(const vector<string>& texts, vector<SecureText>& stexts) {
    int size = texts.size();
    if (size == 0)
    {
      log_error << "empty decode texts !" << endl;
      return -1;
    }

    stexts.clear();
    stexts.resize(size);
    for (int i = 0; i < size; ++i) {
      if (0 != SecureText::from_string(texts[i], stexts[i])) {
        cout << "decode:  " << texts[i] << ",  failed, please input match pattern: ([G-P]{1})_([[:alnum:]]{2,}) "
             << endl;
        stexts.clear();
        return -1;
      }
    }

    return 0;
  }

  static SupportDecodeType probe_string_type(const string& text) {
    SupportDecodeType string_type = UNKNOWN_STR; // 0: secure string, 1: private_input string, 2: double literal string
    if (SecureText::is_secure_text(text))//check secure string first
    {
      string_type = SECURE_STR;
    }
    else if (text.size() >= 2 && text[text.size()-1] == '#')//check private input string
    {
      string_type = PRIVATE_IN_STR;
    }
    else if ((('0' <= text[0] && '9' >= text[0]) || text[0] == '-' || text[0] == '+') && std::isdigit(text[text.size()-1])) // just do simple check
    {// if (regex_match(text, regex("^[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)$"))) //check double string
      string_type = DOUBLE_STR;
    }
    else{
      //default, unkonw string
    }

    return string_type;
  }

  static int decode(const string& text, string& stext) {
    if (text.empty())
    {
      log_error << "null text to decode" << endl;
      return -1;
    }

    SupportDecodeType string_type = probe_string_type(text); // 0: secure string, 1: private_input string, 2: double literal string
    if (string_type == UNKNOWN_STR)
    {
      log_error << "find unknown string type, not support: " << text << endl;
      return -1;
    }

    if (string_type == SECURE_STR)
    {
      SecureText st;
      if (0 != SecureText::from_string(text, st)) {
          log_error << "decode '" << text << "' failed, please input secure text"  << endl;
          return -1;
      }
      stext = st.content;
    }
    else if (string_type == PRIVATE_IN_STR)
    {
      if (text[text.size()-1] != '#')
      {
        log_error << "bad text: " << text << "please input private_input text"  << endl;
        return -1;
      }
      stext = text.substr(0, text.size()-1);
    }
    else if (string_type == DOUBLE_STR)
    {
      stext = text;
    }
    else{}

    return static_cast<int>(string_type);
  }

  static int decode(const vector<string>& texts, vector<string>& stexts) {
    int size = texts.size();
    if (size == 0)
    {
      log_error << "empty decode texts !" << endl;
      return -1;
    }

    // only test the first element for convient
    if (texts[0].empty())
    {
      log_error << "null string to decode" << endl;
      return -1;
    }

    SupportDecodeType string_type = probe_string_type(texts[0]); // 0: secure string, 1: private_input string, 2: double literal string
    if (string_type == UNKNOWN_STR)
    {
      log_error << "find unknown string type, not support: " << texts[0] << endl;
      return -1;
    }

    stexts.resize(size);
    if (string_type == SECURE_STR)
    {
      SecureText st;
      for (int i = 0; i < size; ++i) {
        if (0 != SecureText::from_string(texts[i], st)) {
          log_error << "decode '" << texts[i] << "' failed, please input secure text"  << endl;
          stexts.clear();
          return -1;
        }

        stexts[i] = st.content;
      }
    }
    else if (string_type == PRIVATE_IN_STR)
    {
      for (int i = 0; i < size; ++i) {
        if (texts[i][texts[i].size()-1] != '#')
        {
          log_error << "bad text: " << texts[i] << "please input private_input text"  << endl;
          stexts.clear();
          return -1;
        }
        stexts[i] = texts[i].substr(0, texts[i].size()-1);
      }
    }
    else if (string_type == DOUBLE_STR)
    {
      for (int i = 0; i < size; ++i) {
        stexts[i] = texts[i];
      }
    }
    else{}

    return static_cast<int>(string_type);
  }
}

} // namespace convert
} // namespace rosetta
