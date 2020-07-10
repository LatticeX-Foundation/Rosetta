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

#include "cc/modules/protocol/public/protocol_manager.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_util.h"
#include "cc/modules/protocol/mpc/snn/include/snn_opsets.h"
using np_str_t = std::array<char, 33>; // at most 33 bytes

class DataSet {
  bool p2_owns_data_;
  int label_owner_;
  int dataset_type_;
  int partyid;

  enum SecureDatasetType {
    COMMON_N_V_SPLIT = 1,
    COMMON_F_H_SPLIT = 2,
  };

 public:
  DataSet(bool p2_owns_data, int label_owner, int dataset_type)
      : p2_owns_data_(p2_owns_data), label_owner_(label_owner), dataset_type_(dataset_type) {
    log_debug << "DataSet, p2 owns data:" << p2_owns_data << ", dataset type:" << dataset_type_
              << endl;
  }

 public:
  py::array_t<np_str_t> private_input_x(const py::array_t<double>& input) {
    __check();
    return private_dataset_input_2d_X(input);
  }
  py::array_t<np_str_t> private_input_y(const py::array_t<double>& input) {
    __check();
    return private_dataset_input_2d_Y(input);
  }

 private:
  void __check() {
    if (!rosetta::ProtocolManager::Instance()->IsActivated()) {
      cerr << "have not actived!" << endl;
      throw;
    }
    partyid = rosetta::ProtocolManager::Instance()->GetProtocol()->GetPartyId();
  }
  // csv inputs(X)
  py::array_t<np_str_t> private_dataset_input_2d_X(const py::array_t<double>& input) {
    cout << "DataSet, private_dataset_input_2d_X." << endl;
    auto ops =
      rosetta::ProtocolManager::Instance()->GetProtocol()->GetOps("private_dataset_input_2d_X");
    auto netio = rosetta::ProtocolManager::Instance()->GetProtocol()->GetNetHandler();

    ////////////////////////////////
    py::buffer_info buf = input.request();
    ssize_t ndim = buf.ndim;
    ssize_t size = buf.size;
    cout << "input size:" << size << endl;
    cout << "input ndim:" << ndim << endl;

    // initialize
    vector<double> valuesX;
    int n, d, dA, dB, dC;
    n = dA = dB = dC = d = 1;
    if ((partyid == PARTY_A) || (partyid == PARTY_B) || p2_owns_data_) {
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
      sync_n_and_check(netio, n);
      sync_d(netio, dA, dB, dC);
      d = dA + dB;
      if (p2_owns_data_)
        d += dC;
      cout << "shape:(n,d) --> "
           << "(" << n << ", " << d << " = dA:" << dA << " + dB:" << dB
           << string(p2_owns_data_ ? " + dC:" + to_string(dC) : "") << ")" << endl;
    }

    // get sharings
    vector<string> SSA(n * dA);
    vector<string> SSB(n * dB);
    vector<string> SSC(n * dC);
    //! @todo optimized here, or in PrivateInput
    {
      vector<double> valuesXX;
      if (partyid == PARTY_A)
        valuesXX = valuesX;
      else
        valuesXX.resize(n * dA);
      ops->PrivateInput(PARTY_A, valuesXX, SSA);
    }
    {
      vector<double> valuesXX;
      if (partyid == PARTY_B)
        valuesXX = valuesX;
      else
        valuesXX.resize(n * dB);
      ops->PrivateInput(PARTY_B, valuesXX, SSB);
    }
    if (p2_owns_data_) {
      vector<double> valuesXX;
      if (partyid == PARTY_C)
        valuesXX = valuesX;
      else
        valuesXX.resize(n * dC);
      ops->PrivateInput(PARTY_C, valuesXX, SSC);
    }

    // set result. combine partA,partB(,partC) for each party
    auto result = py::array_t<np_str_t>(n * d);
    py::buffer_info out = result.request();
    np_str_t* pout = reinterpret_cast<np_str_t*>(out.ptr);
    memset((char*)pout, 0, n * d * sizeof(np_str_t));
    {
      result.resize({n, d});
      auto res = result.mutable_unchecked<2>();
      // partA
      for (int i = 0; i < n; i++) {
        for (int j = 0; j < dA; j++) {
          std::memcpy((char*)res(i, j).data(), SSA[i * dA + j].data(), SSA[i * dA + j].size());
        }
      }
      // partB
      for (int i = 0; i < n; i++) {
        for (int j = 0; j < dB; j++) {
          std::memcpy((char*)res(i, j + dA).data(), SSB[i * dB + j].data(), SSB[i * dB + j].size());
        }
      }
      if (p2_owns_data_) {
        // partC
        for (int i = 0; i < n; i++) {
          for (int j = 0; j < dC; j++) {
            std::memcpy(
              (char*)res(i, j + dA + dB).data(), SSC[i * dC + j].data(), SSC[i * dC + j].size());
          }
        }
      }
    }

    netio->sync_with(msg_id_t("private_dataset_input_2d_X"));
    return result;
  }

  // csv inputs(Y)
  py::array_t<np_str_t> private_dataset_input_2d_Y(const py::array_t<double>& input) {
    cout << "DataSet, private_dataset_input_2d_Y." << endl;
    auto ops =
      rosetta::ProtocolManager::Instance()->GetProtocol()->GetOps("private_dataset_input_2d_Y");
    auto netio = rosetta::ProtocolManager::Instance()->GetProtocol()->GetNetHandler();
    ////////////////////////////////////
    py::buffer_info buf = input.request();
    ssize_t ndim = buf.ndim;
    ssize_t size = buf.size;
    cout << "input size:" << size << endl;
    cout << "input ndim:" << ndim << endl;

    vector<double> valuesY;
    int n = 1, d = 1;

    // initialize
    if (label_owner_ == partyid) {
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

    // sync n
    sync_n_and_check(netio, n, true);
    if (label_owner_ != partyid) {
      valuesY.resize(n * d, 0);
    }

    // get sharings
    vector<string> SSY(n * d); // secert sharing
    ops->PrivateInput(label_owner_, valuesY, SSY);

    // set result
    auto result = py::array_t<np_str_t>(n * d);
    py::buffer_info out = result.request();
    np_str_t* pout = reinterpret_cast<np_str_t*>(out.ptr);
    memset((char*)pout, 0, n * d * sizeof(np_str_t));
    {
      result.resize({n, d});
      auto res = result.mutable_unchecked<2>();
      for (int i = 0; i < n; i++) {
        std::memcpy((char*)res(i, 0).data(), SSY[i].data(), SSY[i].size());
      }
    }

    netio->sync_with(msg_id_t("private_dataset_input_2d_Y"));
    return result;
  }

 private:
  void sync_n_and_check(shared_ptr<NET_IO>& netio, int& n, bool labels = false) {
    msg_id_t msgid("sync_n_and_check"); // temp
    if (labels) {
      // send n from label_owner party to the two others
      if (partyid == label_owner_) {
        for (int i = 0; i < 3; i++) {
          if (i != partyid) {
            netio->send(i, (const char*)&n, sizeof(int), msgid);
          }
        }
      } else {
        netio->recv(label_owner_, (char*)&n, sizeof(int), msgid);
      }
    } else {
      if (!p2_owns_data_) {
        if (partyid == PARTY_A) {
          netio->send(PARTY_C, (const char*)&n, sizeof(int), msgid);
        } else if (partyid == PARTY_C) {
          netio->recv(PARTY_A, (char*)&n, sizeof(int), msgid);
        }
      }
      // valid check
      int n1, n2, n3;
      n1 = n2 = n3 = n;
      sync_d(netio, n1, n2, n3);
      if (!(n1 == n2 && n2 == n3)) {
        cerr << "check n failed. n1:" << n1 << ",n2:" << n2 << ",n3:" << n3 << endl;
        throw;
      }
    }
  }
  // in the order of C-A-B
  void sync_d(shared_ptr<NET_IO>& netio, int& dA, int& dB, int& dC) {
    msg_id_t msgid("sync_d"); // temp
    if (partyid == PARTY_A) {
      netio->recv(PARTY_C, (char*)&dC, sizeof(int), msgid);

      netio->send(PARTY_B, (const char*)&dA, sizeof(int), msgid);
      netio->send(PARTY_C, (const char*)&dA, sizeof(int), msgid);

      netio->recv(PARTY_B, (char*)&dB, sizeof(int), msgid);
    } else if (partyid == PARTY_B) {
      netio->recv(PARTY_C, (char*)&dC, sizeof(int), msgid);
      netio->recv(PARTY_A, (char*)&dA, sizeof(int), msgid);

      netio->send(PARTY_A, (const char*)&dB, sizeof(int), msgid);
      netio->send(PARTY_C, (const char*)&dB, sizeof(int), msgid);
    } else if (partyid == PARTY_C) {
      netio->send(PARTY_A, (const char*)&dC, sizeof(int), msgid);
      netio->send(PARTY_B, (const char*)&dC, sizeof(int), msgid);

      netio->recv(PARTY_A, (char*)&dA, sizeof(int), msgid);
      netio->recv(PARTY_B, (char*)&dB, sizeof(int), msgid);
    }
  }
};
