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
#include <algorithm>
#include <mutex>
#include <fstream>
using namespace std;

#include "cc/modules/protocol/public/protocol_manager.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_util.h"
#include "cc/modules/protocol/mpc/snn/include/snn_opsets.h"
using np_str_t = std::array<char, 33>; // at most 33 bytes

class DataSet {
  vector<int> data_owner_;
  bool p2_owns_data_ = false;
  int label_owner_ = -1;
  int dataset_type_ = -1;
  int partyid = -1;
  // -1, have not checked; 0, have checked, but not ok; 1, have checked, but ok
  int args_checked_ok_ = -1;

  bool p0_has_data = false;
  bool p1_has_data = false;
  bool p2_has_data = false;

  enum DatasetType {
    SampleAligned = 1,
    FeatureAligned = 2,
  };

 public:
  DataSet(const vector<int>& data_owner, int label_owner, int dataset_type)
      : data_owner_(data_owner), label_owner_(label_owner), dataset_type_(dataset_type) {
    log_debug << "DataSet, p2 owns data:" << data_owner.size() << ", dataset type:" << dataset_type_
              << endl;
    std::sort(data_owner_.begin(), data_owner_.end());
    std::unique(data_owner_.begin(), data_owner_.end());
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
      throw runtime_error("In Dataset, no protocol have activated!");
    }
    partyid = rosetta::ProtocolManager::Instance()->GetProtocol()->GetPartyId();
    // cout << "-args_checked_ok_:" << args_checked_ok_ << endl;
    __check_args();
    // cout << "+args_checked_ok_:" << args_checked_ok_ << endl;
    if (args_checked_ok_ == 0) {
      //! @todo, print more error messages.
      throw invalid_argument("Invalid_argument!");
    }
  }

  void __check_args() {
    if (args_checked_ok_ != -1)
      return;

    // locally check
    int local_check_ok = 1;
    /// owner \in {0,1,2}
    for (auto& owner : data_owner_) {
      if ((owner < 0) || (owner > 2)) {
        local_check_ok = 0;
      }
      if (owner == 0) {
        p0_has_data = true;
      } else if (owner == 1) {
        p1_has_data = true;
      } else if (owner == 2) {
        p2_has_data = true;
      }
    }
    if (data_owner_.size() == 0) {
      local_check_ok = 0;
    }

    /// label_owner \in {0,1,2}
    if ((label_owner_ < -1) || (label_owner_ > 2)) {
      local_check_ok = 0;
    }

    /// dataset_type \in {1,2}
    if ((dataset_type_ < 1) || (dataset_type_ > 2)) {
      local_check_ok = 0;
    }

    auto ops = rosetta::ProtocolManager::Instance()->GetProtocol()->GetOps("__check_args");
    auto netio = rosetta::ProtocolManager::Instance()->GetProtocol()->GetNetHandler();
    {
      // 1. check "locally check" is ok
      int a, b, c;
      a = b = c = local_check_ok;
      sync_d(netio, a, b, c);

      if (!((a == b) && (b == c))) { // at least one of Pi is invalid (if not ok)
        args_checked_ok_ = 0;
        return;
      }

      if (a == 0) { // all Pi is invalid (if a == b == c == 0)
        args_checked_ok_ = 0;
        return;
      }
    }

    {
      // 2. check owner size
      int a, b, c;
      a = b = c = data_owner_.size();
      sync_d(netio, a, b, c);

      if (!((a == b) && (b == c))) { // at least one of Pi is not equal to other(s) (if not ok)
        args_checked_ok_ = 0;
        return;
      }
    }

    {
      // 3. check all arguments
      vector<int> a, b, c;
      a = data_owner_;
      a.push_back(dataset_type_);
      a.push_back(label_owner_);
      b = c = a;
      sync_d(netio, a, b, c);

      for (int i = 0; i < a.size(); i++) {
        if (!((a[i] == b[i]) &&
              (b[i] == c[i]))) { // at least one of Pi is not equal to other(s) (if not ok)
          args_checked_ok_ = 0;
          return;
        }
      }
    }

    args_checked_ok_ = 1;
  }

  // csv inputs(X)
  py::array_t<np_str_t> private_dataset_input_2d_X(const py::array_t<double>& input) {
    log_debug << "DataSet, private_dataset_input_2d_X." << endl;
    auto ops =
      rosetta::ProtocolManager::Instance()->GetProtocol()->GetOps("private_dataset_input_2d_X");
    auto netio = rosetta::ProtocolManager::Instance()->GetProtocol()->GetNetHandler();

    ////////////////////////////////
    py::buffer_info buf = input.request();
    ssize_t ndim = buf.ndim;
    ssize_t size = buf.size;

    {
      // check ndim
      // check size
      int ndimA, ndimB, ndimC;
      //int sizeA, sizeB, sizeC;
      ndimA = ndimB = ndimC = ndim;
      //sizeA = sizeB = sizeC = size;
      sync_d(netio, ndimA, ndimB, ndimC);
      if (p0_has_data) {
        if (ndimA != 2) {
          throw runtime_error("X p0_has_data, but ndimA[" + to_string(ndimA) + "] != 2");
        }
      }
      if (p1_has_data) {
        if (ndimB != 2) {
          throw runtime_error("X p1_has_data, but ndimB[" + to_string(ndimB) + "] != 2");
        }
      }
      if (p2_has_data) {
        if (ndimC != 2) {
          throw runtime_error("X p2_has_data, but ndimC[" + to_string(ndimC) + "] != 2");
        }
      }
    }

    //! @todo optimize[yl]
    if (dataset_type_ == DatasetType::SampleAligned) {
      // initialize
      vector<double> valuesX;
      int n, d, dA, dB, dC;
      n = dA = dB = dC = d = 0;
      if (
        ((partyid == PARTY_A) && p0_has_data) || ((partyid == PARTY_B) && p1_has_data) ||
        ((partyid == PARTY_C) && p2_has_data)) {
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
        sync_n_and_check_x(netio, n);
        sync_d(netio, dA, dB, dC);

        d = 0;
        if (p0_has_data)
          d += dA;
        else
          dA = 0;
        if (p1_has_data)
          d += dB;
        else
          dB = 0;
        if (p2_has_data)
          d += dC;
        else
          dC = 0;

        log_info << "shape:(n,d) --> "
                 << "(" << n << ", " << d << " = dA:" << dA << " + dB:" << dB << " + dC:" << dC
                 << ")" << endl;
      }

      // get sharings
      vector<string> SSA(n * dA);
      vector<string> SSB(n * dB);
      vector<string> SSC(n * dC);
      if (p0_has_data) {
        vector<double> valuesXX;
        if (partyid == PARTY_A)
          valuesXX = valuesX;
        else
          valuesXX.resize(n * dA);
        ops->PrivateInput(PARTY_A, valuesXX, SSA);
      }
      if (p1_has_data) {
        vector<double> valuesXX;
        if (partyid == PARTY_B)
          valuesXX = valuesX;
        else
          valuesXX.resize(n * dB);
        ops->PrivateInput(PARTY_B, valuesXX, SSB);
      }
      if (p2_has_data) {
        vector<double> valuesXX;
        if (partyid == PARTY_C)
          valuesXX = valuesX;
        else
          valuesXX.resize(n * dC);
        ops->PrivateInput(PARTY_C, valuesXX, SSC);
      }

      // set result. combine partA,partB,partC for each party
      auto result = py::array_t<np_str_t>(n * d);
      py::buffer_info out = result.request();
      np_str_t* pout = reinterpret_cast<np_str_t*>(out.ptr);
      memset((char*)pout, 0, n * d * sizeof(np_str_t));
      {
        result.resize({n, d});
        auto res = result.mutable_unchecked<2>();
        // partA
        if (p0_has_data) {
          for (int i = 0; i < n; i++) {
            for (int j = 0; j < dA; j++) {
              std::memcpy((char*)res(i, j).data(), SSA[i * dA + j].data(), SSA[i * dA + j].size());
            }
          }
        }
        // partB
        if (p1_has_data) {
          for (int i = 0; i < n; i++) {
            for (int j = 0; j < dB; j++) {
              std::memcpy(
                (char*)res(i, j + dA).data(), SSB[i * dB + j].data(), SSB[i * dB + j].size());
            }
          }
        }
        if (p2_has_data) {
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

    //! @todo optimize[yl]
    if (dataset_type_ == DatasetType::FeatureAligned) {
      // initialize
      vector<double> valuesX;
      int n, d, nA, nB, nC;
      n = nA = nB = nC = d = 0;
      if (
        ((partyid == PARTY_A) && p0_has_data) || ((partyid == PARTY_B) && p1_has_data) ||
        ((partyid == PARTY_C) && p2_has_data)) {
        n = input.shape()[0];
        d = input.shape()[1];
        nA = nB = nC = n;

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
        sync_n_and_check_x(netio, d);
        sync_d(netio, nA, nB, nC);

        n = 0;
        if (p0_has_data)
          n += nA;
        else
          nA = 0;
        if (p1_has_data)
          n += nB;
        else
          nB = 0;
        if (p2_has_data)
          n += nC;
        else
          nC = 0;

        log_info << "shape:(n,d) --> "
                 << "(" << n << " = nA:" << nA << " + nB:" << nB << " + nC:" << nC << ", " << d
                 << ")" << endl;
      }

      // get sharings
      vector<string> SSA(nA * d);
      vector<string> SSB(nB * d);
      vector<string> SSC(nC * d);
      if (p0_has_data) {
        vector<double> valuesXX;
        if (partyid == PARTY_A)
          valuesXX = valuesX;
        else
          valuesXX.resize(nA * d);
        ops->PrivateInput(PARTY_A, valuesXX, SSA);
      }
      if (p1_has_data) {
        vector<double> valuesXX;
        if (partyid == PARTY_B)
          valuesXX = valuesX;
        else
          valuesXX.resize(nB * d);
        ops->PrivateInput(PARTY_B, valuesXX, SSB);
      }
      if (p2_has_data) {
        vector<double> valuesXX;
        if (partyid == PARTY_C)
          valuesXX = valuesX;
        else
          valuesXX.resize(nC * d);
        ops->PrivateInput(PARTY_C, valuesXX, SSC);
      }

      // set result. combine partA,partB,partC for each party
      auto result = py::array_t<np_str_t>(n * d);
      py::buffer_info out = result.request();
      np_str_t* pout = reinterpret_cast<np_str_t*>(out.ptr);
      memset((char*)pout, 0, n * d * sizeof(np_str_t));
      {
        result.resize({n, d});
        auto res = result.mutable_unchecked<2>();
        // partA
        if (p0_has_data) {
          for (int i = 0; i < nA; i++) {
            for (int j = 0; j < d; j++) {
              std::memcpy((char*)res(i, j).data(), SSA[i * d + j].data(), SSA[i * d + j].size());
            }
          }
        }
        // partB
        if (p1_has_data) {
          for (int i = 0; i < nB; i++) {
            for (int j = 0; j < d; j++) {
              std::memcpy(
                (char*)res(i + nA, j).data(), SSB[i * d + j].data(), SSB[i * d + j].size());
            }
          }
        }
        if (p2_has_data) {
          // partC
          for (int i = 0; i < nC; i++) {
            for (int j = 0; j < d; j++) {
              std::memcpy(
                (char*)res(i + nA + nB, j).data(), SSC[i * d + j].data(), SSC[i * d + j].size());
            }
          }
        }
      }

      netio->sync_with(msg_id_t("private_dataset_input_2d_X"));
      return result;
    }

    return py::array_t<np_str_t>();
  }

  // csv inputs(Y)
  py::array_t<np_str_t> private_dataset_input_2d_Y(const py::array_t<double>& input) {
    log_debug << "DataSet, private_dataset_input_2d_Y." << endl;
    auto ops =
      rosetta::ProtocolManager::Instance()->GetProtocol()->GetOps("private_dataset_input_2d_Y");
    auto netio = rosetta::ProtocolManager::Instance()->GetProtocol()->GetNetHandler();
    ////////////////////////////////////
    py::buffer_info buf = input.request();
    ssize_t ndim = buf.ndim;
    ssize_t size = buf.size;

    // check
    if (dataset_type_ == DatasetType::SampleAligned) {
      // only label_owner has label
      if ((label_owner_ < 0) || (label_owner_ > 2)) {
        throw runtime_error("invalid label_owner under DatasetType::SampleAligned!");
        // auto result = py::array_t<np_str_t>();
        // return result;
      }
    } else if (dataset_type_ == DatasetType::FeatureAligned) {
      // ignore label_owner_
    } else if (label_owner_ == -1) {
      // ignore label
      auto result = py::array_t<np_str_t>();
      return result;
    } else {
      // nothing
      auto result = py::array_t<np_str_t>();
      return result;
    }

    // check ndim
    // check size
    int ndimA, ndimB, ndimC;
    //int sizeA, sizeB, sizeC;
    ndimA = ndimB = ndimC = ndim;
    //sizeA = sizeB = sizeC = size;
    sync_d(netio, ndimA, ndimB, ndimC);

    //! @todo optimize[yl]
    if (dataset_type_ == DatasetType::SampleAligned) {
      {
        if (label_owner_ == PARTY_A) {
          if (ndimA != 2) {
            throw runtime_error("Y p0_has_data, but ndimA[" + to_string(ndimA) + "] != 2");
          }
        }
        if (label_owner_ == PARTY_B) {
          if (ndimB != 2) {
            throw runtime_error("Y p1_has_data, but ndimB[" + to_string(ndimB) + "] != 2");
          }
        }
        if (label_owner_ == PARTY_C) {
          if (ndimC != 2) {
            throw runtime_error("Y p2_has_data, but ndimC[" + to_string(ndimC) + "] != 2");
          }
        }
      }

      vector<double> valuesY;
      int n = 1, d = 1;

      // initialize
      if (label_owner_ == partyid) {
        n = input.shape()[0];
        //d = input.shape()[1];

        valuesY.resize(size);
        auto inp = input.unchecked<2>();
        for (int i = 0; i < n; i++) {
          valuesY[i] = inp(i, 0);
        }
      }

      // sync n
      sync_n_and_check_y(netio, n);
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

    //! @todo optimize[yl]
    if (dataset_type_ == DatasetType::FeatureAligned) {
      {
        if (p0_has_data) {
          if (ndimA != 2) {
            throw runtime_error("Y p0_has_data, but ndimA[" + to_string(ndimA) + "] != 2");
          }
        }
        if (p1_has_data) {
          if (ndimB != 2) {
            throw runtime_error("Y p1_has_data, but ndimB[" + to_string(ndimB) + "] != 2");
          }
        }
        if (p2_has_data) {
          if (ndimC != 2) {
            throw runtime_error("Y p2_has_data, but ndimC[" + to_string(ndimC) + "] != 2");
          }
        }
      }

      // initialize
      vector<double> valuesY;
      int n, d, nA, nB, nC;
      n = nA = nB = nC = d = 1;
      if (
        ((partyid == PARTY_A) && p0_has_data) || ((partyid == PARTY_B) && p1_has_data) ||
        ((partyid == PARTY_C) && p2_has_data)) {
        //d = input.shape()[1];
        n = input.shape()[0];
        nA = nB = nC = n;

        valuesY.resize(size);
        auto inp = input.unchecked<2>();
        for (int i = 0; i < n; i++) {
          valuesY[i] = inp(i, 0);
        }
      }

      // sync shape (n, d) and valid check
      {
        sync_d(netio, nA, nB, nC);

        n = 0;
        if (p0_has_data)
          n += nA;
        else
          nA = 0;
        if (p1_has_data)
          n += nB;
        else
          nB = 0;
        if (p2_has_data)
          n += nC;
        else
          nC = 0;

        log_info << "shape:(n,d) --> "
                 << "(" << n << " = nA:" << nA << " + nB:" << nB << " + nC:" << nC << ", " << d
                 << ")" << endl;
      }

      // get sharings
      vector<string> SSA(nA * d);
      vector<string> SSB(nB * d);
      vector<string> SSC(nC * d);
      if (p0_has_data) {
        vector<double> valuesYY;
        if (partyid == PARTY_A)
          valuesYY = valuesY;
        else
          valuesYY.resize(nA * d);
        ops->PrivateInput(PARTY_A, valuesYY, SSA);
      }
      if (p1_has_data) {
        vector<double> valuesYY;
        if (partyid == PARTY_B)
          valuesYY = valuesY;
        else
          valuesYY.resize(nB * d);
        ops->PrivateInput(PARTY_B, valuesYY, SSB);
      }
      if (p2_has_data) {
        vector<double> valuesYY;
        if (partyid == PARTY_C)
          valuesYY = valuesY;
        else
          valuesYY.resize(nC * d);
        ops->PrivateInput(PARTY_C, valuesYY, SSC);
      }

      // set result. combine partA,partB,partC for each party
      auto result = py::array_t<np_str_t>(n * d);
      py::buffer_info out = result.request();
      np_str_t* pout = reinterpret_cast<np_str_t*>(out.ptr);
      memset((char*)pout, 0, n * d * sizeof(np_str_t));
      // because of d ==== 1, so reduce the second dim
      {
        result.resize({n, d});
        auto res = result.mutable_unchecked<2>();
        // partA
        if (p0_has_data) {
          for (int i = 0; i < nA; i++) {
            std::memcpy((char*)res(i, 0).data(), SSA[i].data(), SSA[i].size());
          }
        }
        // partB
        if (p1_has_data) {
          for (int i = 0; i < nB; i++) {
            std::memcpy((char*)res(i + nA, 0).data(), SSB[i].data(), SSB[i].size());
          }
        }
        // partC
        if (p2_has_data) {
          for (int i = 0; i < nC; i++) {
            std::memcpy((char*)res(i + nA + nB, 0).data(), SSC[i].data(), SSC[i].size());
          }
        }
      }

      netio->sync_with(msg_id_t("private_dataset_input_2d_Y"));
      return result;
    }

    return py::array_t<np_str_t>();
  }

 private:
  /**
   * sync the n (the number of labels(Y) of the label owner) to the two others
   * only for input Y
   * only for DatasetType::SampleAligned
   */
  void sync_n_and_check_y(shared_ptr<NET_IO>& netio, int& n) {
    msg_id_t msgid("sync_n_and_check_y"); // temp

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
  }

  /**
   * only for input X
   */
  void sync_n_and_check_x(shared_ptr<NET_IO>& netio, int& n) {
    msg_id_t msgid("sync_n_and_check_x"); // temp

    //! @todo optimize[yl]
    if (!p2_has_data) {
      if (p0_has_data) {
        if (partyid == PARTY_A) {
          netio->send(PARTY_C, (const char*)&n, sizeof(int), msgid);
        } else if (partyid == PARTY_C) {
          netio->recv(PARTY_A, (char*)&n, sizeof(int), msgid);
        }
      } else if (p1_has_data) {
        if (partyid == PARTY_B) {
          netio->send(PARTY_C, (const char*)&n, sizeof(int), msgid);
        } else if (partyid == PARTY_C) {
          netio->recv(PARTY_B, (char*)&n, sizeof(int), msgid);
        }
      }
    }

    if (!p1_has_data) {
      if (p0_has_data) {
        if (partyid == PARTY_A) {
          netio->send(PARTY_B, (const char*)&n, sizeof(int), msgid);
        } else if (partyid == PARTY_B) {
          netio->recv(PARTY_A, (char*)&n, sizeof(int), msgid);
        }
      } else if (p2_has_data) {
        if (partyid == PARTY_C) {
          netio->send(PARTY_B, (const char*)&n, sizeof(int), msgid);
        } else if (partyid == PARTY_B) {
          netio->recv(PARTY_C, (char*)&n, sizeof(int), msgid);
        }
      }
    }

    if (!p0_has_data) {
      if (p1_has_data) {
        if (partyid == PARTY_B) {
          netio->send(PARTY_A, (const char*)&n, sizeof(int), msgid);
        } else if (partyid == PARTY_A) {
          netio->recv(PARTY_B, (char*)&n, sizeof(int), msgid);
        }
      } else if (p2_has_data) {
        if (partyid == PARTY_C) {
          netio->send(PARTY_A, (const char*)&n, sizeof(int), msgid);
        } else if (partyid == PARTY_A) {
          netio->recv(PARTY_C, (char*)&n, sizeof(int), msgid);
        }
      }
    }

    // valid check
    int n1, n2, n3;
    n1 = n2 = n3 = n;
    sync_d(netio, n1, n2, n3);
    if (!(n1 == n2 && n2 == n3)) {
      log_error << "check n failed. n1:" << n1 << ",n2:" << n2 << ",n3:" << n3 << endl;
      throw runtime_error("!(n1 == n2 && n2 == n3)");
    }
  }

  void sync_d(shared_ptr<NET_IO>& netio, int& dA, int& dB, int& dC) {
    vector<int> dAs{dA}, dBs{dB}, dCs{dC};
    sync_d(netio, dAs, dBs, dCs);
    dA = dAs[0];
    dB = dBs[0];
    dC = dCs[0];
  }

  // in the order of C-A-B
  void sync_d(shared_ptr<NET_IO>& netio, vector<int>& dA, vector<int>& dB, vector<int>& dC) {
    msg_id_t msgid("sync_d vector"); // temp
    if (partyid == PARTY_A) {
      netio->recv(PARTY_C, dC, dC.size(), msgid);

      netio->send(PARTY_B, dA, dA.size(), msgid);
      netio->send(PARTY_C, dA, dA.size(), msgid);

      netio->recv(PARTY_B, dB, dB.size(), msgid);
    } else if (partyid == PARTY_B) {
      netio->recv(PARTY_C, dC, dC.size(), msgid);
      netio->recv(PARTY_A, dA, dA.size(), msgid);

      netio->send(PARTY_A, dB, dB.size(), msgid);
      netio->send(PARTY_C, dB, dB.size(), msgid);
    } else if (partyid == PARTY_C) {
      netio->send(PARTY_A, dC, dC.size(), msgid);
      netio->send(PARTY_B, dC, dC.size(), msgid);

      netio->recv(PARTY_A, dA, dA.size(), msgid);
      netio->recv(PARTY_B, dB, dB.size(), msgid);
    }
  }
};
