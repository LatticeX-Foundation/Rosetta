
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
using namespace std;

#include "cc/modules/protocol/public/include/protocol_manager.h"
using np_str_t = std::array<char, 33>; // at most 33 bytes

/**
 * Not thread-safe
 */
class Input {
 protected:
  virtual int _input_op(const string& node_id, const vector<double>& in_x, vector<std::string>& out_x) = 0;
  string task_id_;

 public:
  Input(const string& task_id) : task_id_(task_id) {}

  py::array_t<np_str_t> input(const string& node_id, const py::array_t<double>& input) {
    py::buffer_info buf = input.request();
    ssize_t ndim = buf.ndim;
    ssize_t size = buf.size;
    auto result = py::array_t<np_str_t>(size);
    py::buffer_info out = result.request();
    np_str_t* pout = reinterpret_cast<np_str_t*>(out.ptr);
    memset((char*)pout, 0, size * sizeof(np_str_t));
    result.resize(buf.shape);

    vector<double> vd(size, 0);
    vector<std::string> vs(size);
    if (ndim == 1) {
      double* pbuf = (double*)buf.ptr;
      for (int i = 0; i < input.shape()[0]; i++) {
        vd[i] = pbuf[i];
      }

      _input_op(node_id, vd, vs);

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

      _input_op(node_id, vd, vs);

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

      _input_op(node_id, vd, vs);

      for (int i = 0; i < input.shape()[0]; i++) {
        for (int j = 0; j < input.shape()[1]; j++) {
          for (int k = 0; k < input.shape()[2]; k++) {
            int idx = i * J * K + j * K + k;
            std::memcpy((char*)res(i, j, k).data(), vs[idx].data(), vs[idx].size());
          }
        }
      }
    } else if (ndim == 4) {
      auto inp = input.unchecked<4>();
      auto res = result.mutable_unchecked<4>();
      int J = input.shape()[1];
      int K = input.shape()[2];
      int L = input.shape()[3];
      for (int i = 0; i < input.shape()[0]; i++) {
        for (int j = 0; j < input.shape()[1]; j++) {
          for (int k = 0; k < input.shape()[2]; k++) {
            for (int l = 0; l < input.shape()[3]; l++) {
              vd[i * J * K * L + j * K * L + k * L + l] = inp(i, j, k, l);
            }
          }
        }
      }

      _input_op(node_id, vd, vs);

      for (int i = 0; i < input.shape()[0]; i++) {
        for (int j = 0; j < input.shape()[1]; j++) {
          for (int k = 0; k < input.shape()[2]; k++) {
            for (int l = 0; l < input.shape()[3]; l++) {
              int idx = i * J * K * L + j * K * L + k * L + l;
              std::memcpy((char*)res(i, j, k, l).data(), vs[idx].data(), vs[idx].size());
            }
          }
        }
      }
    } else {
      // not cope now
      cerr << "not supported [ndim == " << ndim << "] now" << endl;
    }
    return result;
  }
};

class PublicInput : public Input {
 public:
  PublicInput(const string& task_id) : Input(task_id) {}
  int _input_op(const string& node_id, const vector<double>& vd, vector<std::string>& vs) {
    msg_id_t msg__input_msg_id("cc This msg id for global PublicInput.");
    auto ops = rosetta::ProtocolManager::Instance()->GetProtocol(task_id_)->GetOps(msg__input_msg_id);
    Py_BEGIN_ALLOW_THREADS;
    ops->PublicInput(node_id, vd, vs);
    Py_END_ALLOW_THREADS;
    return 0;
  }
};

class PrivateInput : public Input {
 public:
  PrivateInput(const string& task_id) : Input(task_id) {}
  int _input_op(const string& node_id, const vector<double>& vd, vector<std::string>& vs) {
    msg_id_t msg__input_msg_id("cc This msg id for global PrivateInput.");
    auto ops = rosetta::ProtocolManager::Instance()->GetProtocol(task_id_)->GetOps(msg__input_msg_id);
    Py_BEGIN_ALLOW_THREADS;
    ops->PrivateInput(node_id, vd, vs);
    Py_END_ALLOW_THREADS;
    return 0;
  }
};
