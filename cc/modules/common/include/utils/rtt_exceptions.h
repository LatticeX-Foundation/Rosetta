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

#include <exception>
#include <sstream>
#include <string>
using namespace std;

#define make_general_exception(cls)                                   \
  class cls##_exp : public exception {                                \
    string msg_ = string("exception ") + string(#cls);                \
                                                                      \
   public:                                                            \
    cls##_exp(string s = "") {                                        \
      if (!s.empty())                                                 \
        msg_ = msg_ + " - " + s;                                      \
    }                                                                 \
    virtual const char* what() const throw() { return msg_.c_str(); } \
  }

make_general_exception(invalid_params);
make_general_exception(invalid_length);
make_general_exception(invalid_argument);
make_general_exception(openfile);
make_general_exception(init);
make_general_exception(init_comm);
make_general_exception(load_and_sharing);
make_general_exception(select_model);
make_general_exception(train);
make_general_exception(save_model);
make_general_exception(load_model);
make_general_exception(prediction);
make_general_exception(cuckoo_hash_fail);
make_general_exception(sslutil);
make_general_exception(socket);
make_general_exception(ssl_socket);
make_general_exception(socket_recv);
make_general_exception(socket_send);
make_general_exception(other);
