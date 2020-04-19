
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

#include "mpc.h"
#include "player.h"

/**
 * Not thread-safe
 */
class Input {
  const Player& player_;

 public:
  Input(const Player& player) : player_(player) {}

  void public_input() {}

  double private_input(int party_id, double d) {
    if (!player_.inited) {
      cerr << "have not inited!" << endl;
      throw;
    }
    auto pri_input = player_.pri_input;

    mpc_t t = 0;
    pri_input->Run(party_id, d, t);
    double ss = MpcTypeToFloatBC(t);
    return ss;
  }

  py::array_t<double> private_input(int party_id, const py::array_t<double>& input) {
    if (!player_.inited) {
      cerr << "have not inited!" << endl;
      throw;
    }
    auto pri_input = player_.pri_input;

    py::buffer_info buf = input.request();
    ssize_t ndim = buf.ndim;
    ssize_t size = buf.size;
    cout << "input size:" << size << endl;
    cout << "input ndim:" << ndim << endl;
    auto result = py::array_t<double>(size);
    result.resize(buf.shape);

    vector<double> vd(size, 0);
    vector<mpc_t> vt(size, 0);
    if (ndim == 1) {
      py::buffer_info out = result.request();
      double* pbuf = (double*)buf.ptr;
      double* pout = (double*)out.ptr;
      for (int i = 0; i < input.shape()[0]; i++) {
        vd[i] = pbuf[i];
      }
      pri_input->Run(party_id, vd, vt);
      tf_convert_mpctype_to_double(vt, vd);
      for (int i = 0; i < input.shape()[0]; i++) {
        pout[i] = vd[i];
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
      pri_input->Run(party_id, vd, vt);
      tf_convert_mpctype_to_double(vt, vd);
      for (int i = 0; i < input.shape()[0]; i++) {
        for (int j = 0; j < input.shape()[1]; j++) {
          res(i, j) = vd[i * J + j];
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
      pri_input->Run(party_id, vd, vt);
      tf_convert_mpctype_to_double(vt, vd);
      for (int i = 0; i < input.shape()[0]; i++) {
        for (int j = 0; j < input.shape()[1]; j++) {
          for (int k = 0; k < input.shape()[2]; k++) {
            res(i, j, k) = vd[i * J * K + j * K + k];
          }
        }
      }
    } else {
      // not cope now
    }
    return result;
  }
};
