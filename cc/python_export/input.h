
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

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
namespace py = pybind11;

#include <cstdio>
#include <string>
#include <vector>
#include <iostream>
#include <mutex>
#include <fstream>

#include "cc/modules/protocol/public/protocol_manager.h"
using np_str_t = std::array<char, 33>; // at most 33 bytes

/**
 * Not thread-safe
 */
class Input {
  std::string pri_input_msg_id = "cc/Player This msg id for global PrivateInput.";

 public:
  Input() {}

  void public_input() {}

  // np_str_t private_input(int party_id, double d) {
  //   vector<std::string> outd(1);
  //   auto ops = rosetta::ProtocolManager::Instance()->GetProtocol()->GetOps(pri_input_msg_id);
  //   ops->PrivateInput(party_id, {d}, outd);

  //   auto result = py::array_t<np_str_t>(1);
  //   py::buffer_info out = result.request();
  //   np_str_t* pout = reinterpret_cast<np_str_t*>(out.ptr);
  //   memset((char*)pout, 0, sizeof(np_str_t));
  //   std::memcpy((char*)pout->data(), outd[0].data(), outd[0].size());
  //   return result.at(0);
  // }

  py::array_t<np_str_t> private_input(int party_id, const py::array_t<double>& input) {
    py::buffer_info buf = input.request();
    ssize_t ndim = buf.ndim;
    ssize_t size = buf.size;
    //cout << "input size:" << size << endl;
    //cout << "input ndim:" << ndim << endl;
    auto result = py::array_t<np_str_t>(size);
    py::buffer_info out = result.request();
    np_str_t* pout = reinterpret_cast<np_str_t*>(out.ptr);
    memset((char*)pout, 0, size * sizeof(np_str_t));
    result.resize(buf.shape);

    auto ops = rosetta::ProtocolManager::Instance()->GetProtocol()->GetOps(pri_input_msg_id);

    vector<double> vd(size, 0);
    vector<std::string> vs(size);
    if (ndim == 1) {
      double* pbuf = (double*)buf.ptr;
      for (int i = 0; i < input.shape()[0]; i++) {
        vd[i] = pbuf[i];
      }

      ops->PrivateInput(party_id, vd, vs);

      for (int i = 0; i < input.shape()[0]; i++) {
        std::memcpy((char*)pout->data(), vs[i].data(), vs[i].size());
        pout++;
      }
    } else if (ndim == 2) {
      auto inp = input.unchecked<2>();
      auto res = result.mutable_unchecked<2>();
      int J = input.shape()[1];
      for (int i = 0; i < input.shape()[0]; i++) {
        for (int j = 0; j < input.shape()[1]; j++) {
          vd[i * J + j] = inp(i, j);
        }
      }

      ops->PrivateInput(party_id, vd, vs);

      for (int i = 0; i < input.shape()[0]; i++) {
        for (int j = 0; j < input.shape()[1]; j++) {
          int idx = i * J + j;
          std::memcpy((char*)res(i, j).data(), vs[idx].data(), vs[idx].size());
        }
      }
    } else if (ndim == 3) {
      auto inp = input.unchecked<3>();
      auto res = result.mutable_unchecked<3>();
      int J = input.shape()[1];
      int K = input.shape()[2];
      for (int i = 0; i < input.shape()[0]; i++) {
        for (int j = 0; j < input.shape()[1]; j++) {
          for (int k = 0; k < input.shape()[2]; k++) {
            vd[i * J * K + j * K + k] = inp(i, j, k);
          }
        }
      }

      ops->PrivateInput(party_id, vd, vs);

      for (int i = 0; i < input.shape()[0]; i++) {
        for (int j = 0; j < input.shape()[1]; j++) {
          for (int k = 0; k < input.shape()[2]; k++) {
            int idx = i * J * K + j * K + k;
            std::memcpy((char*)res(i, j, k).data(), vs[idx].data(), vs[idx].size());
          }
        }
      }
    } else {
      // not cope now
    }
    return result;
  }
};
