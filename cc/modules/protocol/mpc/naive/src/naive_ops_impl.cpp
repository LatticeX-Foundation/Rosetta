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

#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
// #include <any>

#include "cc/modules/protocol/mpc/naive/include/naive_ops_impl.h"
#include "cc/modules/common/include/utils/logger.h"
#include "cc/modules/protocol/utility/include/util.h"
#include "cc/modules/protocol/utility/include/config.h"
#include "cc/modules/io/include/net_io.h"

using namespace std;

namespace rosetta {

int NaiveOpsImpl::PrivateInput(int party_id, const vector<double>& in_x, vector<string>& out_x) {
  log_info << "calling NaiveOpsImpl::PrivateInput" << endl;
  int vec_size = in_x.size(); 
  out_x.clear();
  out_x.resize(vec_size);
  string my_role = op_config_map["PID"];

  // In this insecure naive protocol, we just half the input as local share.
  vector<double> half_share(vec_size, 0.0);
  for(auto i = 0; i < vec_size; ++i) {
    half_share[i] = in_x[i] / 2.0;
  }

  // In this inscure naive protocol, only private data from P0 or P1 is supported for now. 
  if (party_id == 2) {
    log_error << "Not supported yet!";
    return -1;   
  }
  msg_id_t msgid(_op_msg_id);
  if (my_role == "P0") {
    if (party_id == 0) {
      io->send(1, half_share, vec_size, msgid);
    } else if (party_id == 1) {
      io->recv(1, half_share, vec_size, msgid);
    }
  } else if (my_role == "P1") {
    if (party_id == 0) {
      io->recv(0, half_share, vec_size, msgid);
    } else if (party_id == 1) {
      io->send(0, half_share, vec_size, msgid);
    }
  }
  for(auto i = 0; i < vec_size; ++i) {
    out_x[i] = std::to_string(half_share[i]);
  }
  return 0;
}

int NaiveOpsImpl::Add(const vector<string>& a,
                      const vector<string>& b,
                      vector<string>& output,
                      const attr_type* attr_info) {
  log_info << "calling NaiveOpsImpl::Add" << endl;
  int vec_size = a.size();
  output.resize(vec_size);
  for (auto i = 0; i < vec_size; ++i) {
    output[i] = std::to_string(std::stof(a[i]) + std::stof(b[i]));
  } 
  return 0;
}

int NaiveOpsImpl::Sub(const vector<string>& a,
                      const vector<string>& b,
                      vector<string>& output,
                      const attr_type* attr_info) {
  log_info << "calling NaiveOpsImpl::Sub" << endl;
  int vec_size = a.size();
  output.resize(vec_size);
  for (auto i = 0; i < vec_size; ++i) {
    output[i] = std::to_string(std::stof(a[i]) - std::stof(b[i]));
  } 
  return 0;
}


int NaiveOpsImpl::Mul(const vector<string>& a,
                      const vector<string>& b,
                      vector<string>& output,
                      const attr_type* attr_info) {
  log_info << "calling NaiveOpsImpl::Mul" << endl;
  int vec_size = a.size();
  output.resize(vec_size);
  for (auto i = 0; i < vec_size; ++i) {
    output[i] = std::to_string((2 * std::stof(a[i])) * (2 * std::stof(b[i])) / 2.0);
  }
  return 0;
}

int NaiveOpsImpl::Reveal(const vector<string>& a,
                          vector<string>& output,
                          const attr_type* attr_info) {
  log_info << "calling NaiveOpsImpl::Reveal" << endl;
  int vec_size = a.size();
  output.resize(vec_size);
  for (auto i = 0; i < vec_size; ++i) {
    output[i] = std::to_string(2 * std::stof(a[i]));
  } 
  return 0;
}

} // namespace rosetta
