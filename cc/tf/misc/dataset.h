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
using namespace std;

#include "input.h"
#include "player.h"

class DataSet {
  const Player& player_;
  Input input_;
  bool p2_owns_data_;
  int label_owner_;
  int dataset_type_;

 public:
  DataSet(const Player& player, bool p2_owns_data, int label_owner, int dataset_type)
      : player_(player),
        input_(player_),
        p2_owns_data_(p2_owns_data),
        label_owner_(label_owner),
        dataset_type_(dataset_type) {
    cout << "player have " << string(player_.inited ? "inited" : "not inited")
         << " with id:" << player_.id << ", p2 owns data:" << p2_owns_data
         << ", dataset type:" << dataset_type_ << endl;
  }

 public:
  py::array_t<double> private_input_x(const py::array_t<double>& input) {
    return private_dataset_input_2d_X(input);
  }
  py::array_t<double> private_input_y(const py::array_t<double>& input) {
    return private_dataset_input_2d_Y(input);
  }

 private:
  // csv inputs(X)
  py::array_t<double> private_dataset_input_2d_X(const py::array_t<double>& input) {
    auto op_base = GetMpcOpWithKey(OpBase, "op_base_msg_id private_dataset_input_2d_X");
    auto op_priv = GetMpcOpWithKey(PrivateInput, op_base->msg_id());

    py::buffer_info buf = input.request();
    ssize_t ndim = buf.ndim;
    ssize_t size = buf.size;
    cout << "input size:" << size << endl;
    cout << "input ndim:" << ndim << endl;

    vector<mpc_t> SSA, SSB, SSC; // secert sharing
    vector<double> valuesX;
    int n, d, dA, dB, dC;
    n = dA = dB = dC = d = 1;
    if (PRIMARY || p2_owns_data_) {
      assert(ndim == 2);
      assert(size > 0);

      n = input.shape()[0];
      d = input.shape()[1];
      dA = dB = dC = d;
      //
      valuesX.resize(size);
      auto inp = input.unchecked<2>();
      for (int i = 0; i < n; i++) {
        for (int j = 0; j < d; j++) {
          valuesX[i * d + j] = inp(i, j);
        }
      }
    }

    // sync shape (n, d) and valid check
    {
      sync_n_and_check(op_base, n);
      sync_d(op_base, dA, dB, dC);
      d = dA + dB;
      if (p2_owns_data_)
        d += dC;
      cout << "shape:(n,d) --> "
           << "(" << n << ", " << d << " = dA:" << dA << " + dB:" << dB
           << string(p2_owns_data_ ? " + dC:" + to_string(dC) : "") << ")" << endl;
    }

    // get sharings
    SSA.resize(n * dA);
    SSB.resize(n * dB);
    SSC.resize(n * dC);
    if (PRIMARY) {
      op_priv->Run(PARTY_A, valuesX, SSA); // no communications
      op_priv->Run(PARTY_B, valuesX, SSB); // no communications
    }
    if (p2_owns_data_) {
      op_priv->Run(PARTY_C, valuesX, SSC); // communications is SSC.size() * sizeof(SSC[0])
    }

    vector<double> SSAd(n * dA), SSBd(n * dB), SSCd(n * dC);
    tf_convert_mpctype_to_double(SSA, SSAd);
    tf_convert_mpctype_to_double(SSB, SSBd);
    tf_convert_mpctype_to_double(SSC, SSCd);

    // combine partA,partB(,partC) for each party
    auto result = py::array_t<double>(n * d);
    {
      result.resize({n, d});
      auto res = result.mutable_unchecked<2>();
      if (PRIMARY) {
        // partA
        for (int i = 0; i < n; i++) {
          for (int j = 0; j < dA; j++) {
            res(i, j) = SSAd[i * dA + j];
          }
        }
        // partB
        for (int i = 0; i < n; i++) {
          for (int j = 0; j < dB; j++) {
            res(i, j + dA) = SSBd[i * dB + j];
          }
        }
        if (p2_owns_data_) {
          // partC
          for (int i = 0; i < n; i++) {
            for (int j = 0; j < dC; j++) {
              res(i, j + dA + dB) = SSCd[i * dC + j];
            }
          }
        }
      }
    }

    op_base->synchronize(op_base->msg_id());
    return result;
  }

  // csv inputs(Y)
  py::array_t<double> private_dataset_input_2d_Y(const py::array_t<double>& input) {
    auto op_base = GetMpcOpWithKey(OpBase, "op_base_msg_id private_dataset_input_2d_Y");
    auto op_priv = GetMpcOpWithKey(PrivateInput, op_base->msg_id());

    py::buffer_info buf = input.request();
    ssize_t ndim = buf.ndim;
    ssize_t size = buf.size;
    cout << "input size:" << size << endl;
    cout << "input ndim:" << ndim << endl;

    vector<mpc_t> SSY; // secert sharing
    vector<double> valuesY;
    int n = 1, d = 1;
    if (label_owner_ == player_.id) {
      assert(ndim == 2);
      assert(size > 0);

      n = input.shape()[0];
      d = input.shape()[1];
      assert(d == 1);

      valuesY.resize(size);
      auto inp = input.unchecked<2>();
      for (int i = 0; i < n; i++) {
        valuesY[i] = inp(i, 0);
      }
    }

    sync_n_and_check(op_base, n, true);

    // get sharings
    SSY.resize(n * d);
    if (PRIMARY) {
      op_priv->Run(label_owner_, valuesY, SSY); // no communications
    }

    vector<double> SSYd(n * d);
    tf_convert_mpctype_to_double(SSY, SSYd);

    auto result = py::array_t<double>(n * d);
    {
      result.resize({n, d});
      auto res = result.mutable_unchecked<2>();
      if (PRIMARY) {
        for (int i = 0; i < n; i++) {
          res(i, 0) = SSYd[i];
        }
      }
    }

    op_base->synchronize(op_base->msg_id());
    return result;
  }

 private:
  void sync_n_and_check(shared_ptr<rosetta::mpc::OpBase>& op_base, int& n, bool labels = false) {
    if (labels) {
      // send n from label_owner party to the two others
      if (player_.id == label_owner_) {
        for (int i = 0; i < 3; i++) {
          if (i != player_.id) {
            op_base->sendBuf(i, (const char*)&n, sizeof(int));
          }
        }
      } else {
        op_base->receiveBuf(label_owner_, (char*)&n, sizeof(int));
      }
    } else {
      if (!p2_owns_data_) {
        if (player_.id == PARTY_A) {
          op_base->sendBuf(PARTY_C, (const char*)&n, sizeof(int));
        } else if (player_.id == PARTY_C) {
          op_base->receiveBuf(PARTY_A, (char*)&n, sizeof(int));
        }
      }
      // valid check
      int n1, n2, n3;
      n1 = n2 = n3 = n;
      sync_d(op_base, n1, n2, n3);
      if (!(n1 == n2 && n2 == n3)) {
        cout << "check n failed. n1:" << n1 << ",n2:" << n2 << ",n3:" << n3 << endl;
        throw;
      }
    }
  }
  void sync_d(shared_ptr<rosetta::mpc::OpBase>& op_base, int& dA, int& dB, int& dC) {
    if (player_.id == PARTY_A) {
      op_base->receiveBuf(PARTY_C, (char*)&dC, sizeof(int));

      op_base->sendBuf(PARTY_B, (const char*)&dA, sizeof(int));
      op_base->sendBuf(PARTY_C, (const char*)&dA, sizeof(int));

      op_base->receiveBuf(PARTY_B, (char*)&dB, sizeof(int));
    } else if (player_.id == PARTY_B) {
      op_base->receiveBuf(PARTY_C, (char*)&dC, sizeof(int));
      op_base->receiveBuf(PARTY_A, (char*)&dA, sizeof(int));

      op_base->sendBuf(PARTY_A, (const char*)&dB, sizeof(int));
      op_base->sendBuf(PARTY_C, (const char*)&dB, sizeof(int));
    } else if (player_.id == PARTY_C) {
      op_base->sendBuf(PARTY_A, (const char*)&dC, sizeof(int));
      op_base->sendBuf(PARTY_B, (const char*)&dC, sizeof(int));

      op_base->receiveBuf(PARTY_A, (char*)&dA, sizeof(int));
      op_base->receiveBuf(PARTY_B, (char*)&dB, sizeof(int));
    }
  }
};
