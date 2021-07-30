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
#include <cstdio>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <mutex>
#include <fstream>
#include <Python.h>
#include "cc/modules/common/include/utils/rtt_logger.h"
#include "cc/modules/iowrapper/include/io_manager.h"
#include "cc/modules/iowrapper/include/io_wrapper.h"
#include "cc/modules/protocol/public/include/protocol_manager.h"
#include "cc/modules/protocol/utility/include/util.h"

using namespace std;
namespace py = pybind11;
using np_str_t = std::array<char, 33>; // at most 33 bytes

class DataSet {
  vector<string> data_owner_;
  int owner_index_ = -1;
  string label_owner_ = "";
  int dataset_type_ = -1;
  string node_id_ = "";
  string task_id_ = "";
  vector<string> data_nodes_;
  // -1, have not checked; 0, have checked, but not ok; 1, have checked, but ok
  int args_checked_ok_ = -1;
  std::string args_check_errmsg;

  enum DatasetType {
    SampleAligned = 1,
    FeatureAligned = 2,
  };

 public:
  DataSet(const vector<string>& data_owner, const string& label_owner, int dataset_type, const string& task_id)
      : data_owner_(data_owner), label_owner_(label_owner), dataset_type_(dataset_type), task_id_(task_id) {
    std::sort(data_owner_.begin(), data_owner_.end());
    std::unique(data_owner_.begin(), data_owner_.end());

    stringstream ss;
    ss << "data owner(";
    for (auto& owner : data_owner_) {
      ss << owner << ",";
    }
    ss << "), label owner(" << label_owner_ << "), dataset_type(" << dataset_type_ << ")";
    if (!task_id_.empty()) {
      ss << ", task id(" << task_id_ << ")";
    }
    log_info << ss.str() ;
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
    if (!rosetta::ProtocolManager::Instance()->GetProtocol(task_id_)->IsInit()) {
      throw runtime_error("In Dataset, no protocol have activated!");
    }
    node_id_ = rosetta::IOManager::Instance()->GetIOWrapper(task_id_)->GetCurrentNodeId();
    for (int i = 0; i < data_owner_.size(); i++) {
      if (data_owner_[i] == node_id_) {
        owner_index_ = i;
      }
    }
    __check_args(args_check_errmsg);
    if (args_checked_ok_ == 0) {
      throw invalid_argument("Invalid_argument - " + args_check_errmsg);
    }
  }

  void __check_args(std::string& errmsg) {
    if (args_checked_ok_ != -1)
      return;

    // locally check
    int local_check_ok = 1;
    data_nodes_ = rosetta::IOManager::Instance()->GetIOWrapper(task_id_)->GetDataNodes();
    /// owner \in {0,1,2}
    errmsg = "locally check:";
    if (data_owner_.size() == 0) {
      errmsg = errmsg + " data owner size() == 0.";
      log_error << errmsg ;
      local_check_ok = 0;
    } else {
      for (auto& owner : data_owner_) {
        if (node_id_ == owner && std::find(data_nodes_.begin(), data_nodes_.end(), owner) == data_nodes_.end()) {
          errmsg = errmsg + " invalid data owner(" + owner + ").";
          log_error << errmsg;
          local_check_ok = 0;
        }
      }
    }

    /// label_owner \in {0,1,2}
    if (node_id_ == label_owner_ && std::find(data_nodes_.begin(), data_nodes_.end(), label_owner_) == data_nodes_.end()) {
      errmsg = " invalid label owner(" + label_owner_ + ").";
      log_error << errmsg ;
      local_check_ok = 0;
    }

    msg_id_t msg__check_args("__check_args");
    auto ops = rosetta::ProtocolManager::Instance()->GetProtocol(task_id_)->GetOps(msg__check_args);
    auto netio = rosetta::IOManager::Instance()->GetIOWrapper(task_id_);
    {
      // 1. check "locally check" is ok
      vector<int> checked_ok(data_owner_.size(), 0);
      log_debug << "owner index:" << owner_index_ << " local check ok:" << local_check_ok ;
      if (owner_index_ >= 0)
        checked_ok[owner_index_] = local_check_ok;
      sync_check(netio, data_owner_, checked_ok);

      for (int i = 0; i < checked_ok.size(); i++) {
        if (checked_ok[i] == 0) {
          log_error << "data owner " << data_owner_[i] << " check error" ;
          args_checked_ok_ = 0;
          return;
        } else {
          log_debug << "data owner " << data_owner_[i] << " check ok" ;
        }
      }
    }

    errmsg = "data owner size check:";
    {
      // 2. check owner size
      vector<int> data_owner_size(data_owner_.size(), 0);
      if (owner_index_ >= 0)
        data_owner_size[owner_index_] = data_owner_.size();
      sync_check(netio, data_owner_, data_owner_size);
      int owner_size = data_owner_size[0];
      for (int i = 1; i < data_owner_size.size(); i++) { // at least one of Pi is not equal to other(s) (if not ok)
        if (owner_size != data_owner_size[i]) {
          errmsg = errmsg + " invalid data owner size: " + data_owner_[0] + "(" + std::to_string(owner_size) + ") " + data_owner_[i] + "(" +
            std::to_string(data_owner_size[i]) + ").";
          args_checked_ok_ = 0;
          return;
        }
      }
    }

    errmsg = "all check:";
    {
      // 3. check all arguments
      vector<vector<string>> all_args(data_owner_.size(), vector<string>(data_owner_.size() + 2));
      all_args[0] = data_owner_;
      all_args[0][data_owner_.size() - 2] = std::to_string(dataset_type_);
      all_args[0][data_owner_.size() - 1] = label_owner_;
      for (int i = 1; i < all_args.size(); i++) {
        all_args[i] = all_args[0];
      }
      sync_check(netio, data_owner_, all_args);
      for (int i = 1; i < all_args.size(); i++) {
        for (int j = 0; j < all_args[0].size(); j++) {
          if (all_args[0][j] != all_args[i][j]) {
            errmsg = errmsg + " invalid in check all args[j=" + std::to_string(j) + ", 0(" + all_args[0][j] +"), " + std::to_string(i) + "(" + all_args[i][j] + ")]";
            args_checked_ok_ = 0;
            return;
          }
        }
      }
    }

    errmsg = "ok.";
    args_checked_ok_ = 1;
  }

  // csv inputs(X)
  py::array_t<np_str_t> private_dataset_input_2d_X(const py::array_t<double>& input) {
    log_debug << "DataSet, private_dataset_input_2d_X." ;
    msg_id_t msg__private_dataset_input_2d_X("private_dataset_input_2d_X");
    auto ops =
      rosetta::ProtocolManager::Instance()->GetProtocol(task_id_)->GetOps(msg__private_dataset_input_2d_X);
    auto netio = rosetta::IOManager::Instance()->GetIOWrapper(task_id_);

    ////////////////////////////////
    py::buffer_info buf = input.request();
    ssize_t ndim = buf.ndim;
    ssize_t size = buf.size;

    {
      // check ndim
      // check size
      vector<int> ndims(data_owner_.size(), 0);
      if (owner_index_ >= 0) {
        ndims[owner_index_] = ndim;
      }
      sync_check(netio, data_owner_, ndims);
      for (int i = 0; i < ndims.size(); i++) {
        if (ndims[i] != 2) {
          throw runtime_error(data_owner_[i] + " has data, but ndim[" + to_string(ndims[i]) + "] != 2");
        }
      }
    }

    //! @todo optimize[yl]
    if (dataset_type_ == DatasetType::SampleAligned) {
      // initialize
      vector<double> valuesX;
      int n, d;
      n  = d = 0;
      vector<int> dAll;
      vector<int> nAll;
      dAll.resize(data_owner_.size(), 0);
      nAll.resize(data_owner_.size(), 0);
      if (std::find(data_owner_.begin(), data_owner_.end(), node_id_) != data_owner_.end()) {
        n = input.shape()[0];
        d = input.shape()[1];

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
        for (int i = 0; i < data_owner_.size(); i++) {
          if (data_owner_[i] == node_id_) {
            dAll[i] = d;
            nAll[i] = n;
            break;
          }
        }
        sync_d(netio, data_owner_, dAll);
        sync_d(netio, data_owner_, nAll);

        d = 0;
        for (int i = 0; i < dAll.size(); i++) {
          d += dAll[i];
        }
        n = nAll[0];
        for (int i = 1; i < nAll.size(); i++) {
          if (n != nAll[i]) {
            log_error << "n is not the same" ;
          }
        }

        log_info << "shape:(n,d) --> "
                 << "(" << n << ", " << d << ")" ;
      }

      // get sharings
      vector<vector<string>> SS(data_owner_.size());
      for (int i = 0; i < SS.size(); i++) {
        SS[i].resize(n * dAll[i]);
      }

      Py_BEGIN_ALLOW_THREADS;
      for (int i = 0; i < data_owner_.size(); i++) {
        vector<double> valuesXX;
        if (data_owner_[i] == node_id_) {
          valuesXX = valuesX;
        } else {
          valuesXX.resize(n * dAll[i], 0);
        }
        log_debug << "valuesXX size:" << valuesXX.size() << "SS size:" << SS[i].size() ;
        ops->PrivateInput(data_owner_[i], valuesXX, SS[i]);
      }
      Py_END_ALLOW_THREADS;

      // set result. combine partA,partB,partC for each party
      auto result = py::array_t<np_str_t>(n * d);
      py::buffer_info out = result.request();
      np_str_t* pout = reinterpret_cast<np_str_t*>(out.ptr);
      memset((char*)pout, 0, n * d * sizeof(np_str_t));
      {
        result.resize({n, d});
        auto res = result.mutable_unchecked<2>();
        int offset = 0;
        for (int i = 0; i < data_owner_.size(); i++) {
          int tmp_d = dAll[i];
          for (int j = 0; j < n; j++) {
            for (int k = 0; k < tmp_d; k++) {
              memcpy((char*)res(j, k + offset).data(), SS[i][j * tmp_d + k].data(), SS[i][j * tmp_d + k].size());
            }
          }
          offset += tmp_d;
        }
      }

      msg_id_t msg_sync_with("private_dataset_input_2d_X");
      Py_BEGIN_ALLOW_THREADS;
      netio->sync_with(msg_sync_with);
      Py_END_ALLOW_THREADS;
      return result;
    }

    //! @todo optimize[yl]
    if (dataset_type_ == DatasetType::FeatureAligned) {
      // initialize
      vector<double> valuesX;
      int n, d;
      n  = d = 0;
      vector<int> dAll;
      vector<int> nAll;
      dAll.resize(data_owner_.size(), 0);
      nAll.resize(data_owner_.size(), 0);
      if (std::find(data_owner_.begin(), data_owner_.end(), node_id_) != data_owner_.end()) {
        n = input.shape()[0];
        d = input.shape()[1];

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
        for (int i = 0; i < data_owner_.size(); i++) {
          if (data_owner_[i] == node_id_) {
            dAll[i] = d;
            nAll[i] = n;
            log_info << "my shape: ("<< n << "," << d <<")" ;
            break;
          }
        }
        sync_d(netio, data_owner_, dAll);
        sync_d(netio, data_owner_, nAll);

        for (int i = 0; i < data_owner_.size(); i++) {
          log_info << data_owner_[i] << " shape:(" << nAll[i] << ", " << dAll[i] << ")" ;
        }

        n = 0;
        for (int i = 0; i < nAll.size(); i++) {
          n += nAll[i];
        }
        d = dAll[0];
        for (int i = 1; i < dAll.size(); i++) {
          if (d != dAll[i]) {
            log_error << "d is not the same" ;
          }
        }

        log_info << "shape:(n,d) --> "
                 << "(" << n << ", " << d << ")" ;
      }

      // get sharings
      vector<vector<string>> SS(data_owner_.size());
      for (int i = 0; i < SS.size(); i++) {
        SS[i].resize(nAll[i] * d);
      }

      Py_BEGIN_ALLOW_THREADS;
      for (int i = 0; i < data_owner_.size(); i++) {
        vector<double> valuesXX;
        if (data_owner_[i] == node_id_) {
          valuesXX = valuesX;
        } else {
          valuesXX.resize(nAll[i] * d, 0);
        }
        log_debug << "valuesXX size:" << valuesXX.size() << "SS size:" << SS[i].size() ;
        ops->PrivateInput(data_owner_[i], valuesXX, SS[i]);
      }
      Py_END_ALLOW_THREADS;

      // set result. combine partA,partB,partC for each party
      auto result = py::array_t<np_str_t>(n * d);
      py::buffer_info out = result.request();
      np_str_t* pout = reinterpret_cast<np_str_t*>(out.ptr);
      memset((char*)pout, 0, n * d * sizeof(np_str_t));
      {
        result.resize({n, d});
        auto res = result.mutable_unchecked<2>();
        int offset = 0;
        for (int i = 0; i < data_owner_.size(); i++) {
          int tmp_n = nAll[i];
          for (int j = 0; j < tmp_n; j++) {
            for (int k = 0; k < d; k++) {
              memcpy((char*)res(j + offset, k).data(), SS[i][j * d + k].data(), SS[i][j * d + k].size());
            }
          }
          offset += tmp_n;
        }
      }

      msg_id_t msg_sync_with("private_dataset_input_2d_X");
      Py_BEGIN_ALLOW_THREADS;
      netio->sync_with(msg_sync_with);
      Py_END_ALLOW_THREADS;
      return result;
    }

    return py::array_t<np_str_t>();
  }

  // csv inputs(Y)
  py::array_t<np_str_t> private_dataset_input_2d_Y(const py::array_t<double>& input) {
    log_debug << "DataSet, private_dataset_input_2d_Y." ;
    msg_id_t msg__private_dataset_input_2d_Y("private_dataset_input_2d_Y");
    auto ops =
      rosetta::ProtocolManager::Instance()->GetProtocol(task_id_)->GetOps(msg__private_dataset_input_2d_Y);
    auto netio = rosetta::ProtocolManager::Instance()->GetProtocol(task_id_)->GetNetHandler();
    ////////////////////////////////////
    py::buffer_info buf = input.request();
    ssize_t ndim = buf.ndim;
    ssize_t size = buf.size;

    // check
    if (dataset_type_ == DatasetType::SampleAligned) {
      // only label_owner has label
      if (std::find(data_nodes_.begin(), data_nodes_.end(), label_owner_) == data_nodes_.end()) {
        throw runtime_error("invalid label_owner under DatasetType::SampleAligned!");
        // auto result = py::array_t<np_str_t>();
        // return result;
      }
    } else if (dataset_type_ == DatasetType::FeatureAligned) {
      // ignore label_owner_
    } else if (label_owner_ == "") {
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
    int dim = 0;
    if (node_id_ == label_owner_)
      dim = ndim;
    sync_l(netio, dim);

    //! @todo optimize[yl]
    if (dataset_type_ == DatasetType::SampleAligned) {
      if (dim != 2) {
        throw runtime_error("Y " + label_owner_ + " has data, but ndim[" + to_string(dim) + "] != 2");
      }

      vector<double> valuesY;
      int n = 1, d = 1;

      // initialize
      if (label_owner_ == node_id_) {
        n = input.shape()[0];
        //d = input.shape()[1];

        valuesY.resize(size);
        auto inp = input.unchecked<2>();
        for (int i = 0; i < n; i++) {
          valuesY[i] = inp(i, 0);
        }
      }

      // sync n
      //sync_n_and_check_y(netio, n);
      sync_l(netio, n);
      if (label_owner_ != node_id_) {
        valuesY.resize(n * d, 0);
      }

      // get sharings
      vector<string> SSY(n * d); // secert sharing
      log_debug << "valuesY size:" << valuesY.size() << "SSY size:" << SSY.size() ;
      Py_BEGIN_ALLOW_THREADS;
      ops->PrivateInput(label_owner_, valuesY, SSY);
      Py_END_ALLOW_THREADS;

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

      msg_id_t msg_sync_with("private_dataset_input_2d_Y");
      Py_BEGIN_ALLOW_THREADS;
      netio->sync_with(msg_sync_with);
      Py_END_ALLOW_THREADS;
      return result;
    }

    //! @todo optimize[yl]
    if (dataset_type_ == DatasetType::FeatureAligned) {
      {
        // check ndim
        // check size
        vector<int> ndims(data_owner_.size(), 0);
        if (owner_index_ >= 0) {
          ndims[owner_index_] = ndim;
        }
        sync_check(netio, data_owner_, ndims);
        for (int i = 0; i < ndims.size(); i++) {
          if (ndims[i] != 2) {
            throw runtime_error(data_owner_[i] + " has data, but ndim[" + to_string(ndims[i]) + "] != 2");
          }
        }
      }

      // initialize
      vector<double> valuesY;
      int n, d;
      n  = d = 0;
      vector<int> dAll;
      vector<int> nAll;
      dAll.resize(data_owner_.size(), 0);
      nAll.resize(data_owner_.size(), 0);
      if (std::find(data_owner_.begin(), data_owner_.end(), node_id_) != data_owner_.end()) {
        n = input.shape()[0];
        d = input.shape()[1];

        //
        valuesY.resize(size);
        auto inp = input.unchecked<2>();
        for (int i = 0; i < n; i++) {
          for (int j = 0; j < d; j++) {
            valuesY[i * d + j] = inp(i, j);
          }
        }
      }

      // sync shape (n, d) and valid check
      {
        for (int i = 0; i < data_owner_.size(); i++) {
          if (data_owner_[i] == node_id_) {
            dAll[i] = d;
            nAll[i] = n;
            log_info << "my shape:(" << n << ", " << d << ")" ;
            break;
          }
        }
        sync_d(netio, data_owner_, dAll);
        sync_d(netio, data_owner_, nAll);

        for (int i = 0; i < data_owner_.size(); i++) {
          log_info << data_owner_[i] << " shape:(" << nAll[i] << ", " << dAll[i] << ")" ;
        }

        n = 0;
        for (int i = 0; i < nAll.size(); i++) {
          n += nAll[i];
        }
        d = dAll[0];
        for (int i = 1; i < dAll.size(); i++) {
          if (d != dAll[i]) {
            log_error << "d is not the same" ;
          }
        }

        log_info << "shape:(n,d) --> "
                 << "(" << n << ", " << d << ")" ;
      }

      // get sharings
      vector<vector<string>> SS(data_owner_.size());
      for (int i = 0; i < SS.size(); i++) {
        SS[i].resize(nAll[i] * d);
      }

      Py_BEGIN_ALLOW_THREADS;
      for (int i = 0; i < data_owner_.size(); i++) {
        vector<double> valuesYY;
        if (data_owner_[i] == node_id_) {
          valuesYY = valuesY;
        } else {
          valuesYY.resize(nAll[i] * d, 0);
        }
        log_debug << "valuesXX size:" << valuesYY.size() << "SS size:" << SS[i].size() ;
        ops->PrivateInput(data_owner_[i], valuesYY, SS[i]);
      }
      Py_END_ALLOW_THREADS;

      // set result. combine partA,partB,partC for each party
      auto result = py::array_t<np_str_t>(n * d);
      py::buffer_info out = result.request();
      np_str_t* pout = reinterpret_cast<np_str_t*>(out.ptr);
      memset((char*)pout, 0, n * d * sizeof(np_str_t));
      {
        result.resize({n, d});
        auto res = result.mutable_unchecked<2>();
        int offset = 0;
        for (int i = 0; i < data_owner_.size(); i++) {
          int tmp_n = nAll[i];
          for (int j = 0; j < tmp_n; j++) {
            for (int k = 0; k < d; k++) {
              memcpy((char*)res(j + offset, k).data(), SS[i][j * d + k].data(), SS[i][j * d + k].size());
            }
          }
          offset += tmp_n;
        }
      }

      msg_id_t msg_sync_with("private_dataset_input_2d_Y");
      Py_BEGIN_ALLOW_THREADS;
      netio->sync_with(msg_sync_with);
      Py_END_ALLOW_THREADS;
      return result;
    }

    return py::array_t<np_str_t>();
  }

 private:

  void sync_d(shared_ptr<NET_IO>& netio, const vector<string> &owners, vector<int>& d) {
    Py_BEGIN_ALLOW_THREADS;
    msg_id_t msgid("sync_d vector"); // temp
    vector<string> non_computation_nodes = netio->GetNonComputationNodes();
    string node_id = netio->GetCurrentNodeId();
    int partyNum =  netio->GetPartyId(node_id);
    for (int i = 0; i < owners.size(); i++) {
      //log_info << "owner:" << owners[i] << "node id: " << node_id_ ;
      string node_a = netio->GetNodeId(PARTY_A);
      string node_b = netio->GetNodeId(PARTY_B);
      string node_c = netio->GetNodeId(PARTY_C);
      if (owners[i] == node_id_) {
        if (node_id_ != node_a) {
          netio->send(node_a, (const char*)&d[i], sizeof(d[i]), msgid);
          //log_info << "send to " << node_a << " data:" << d[i] ;
        }
        if (node_id_ != node_b) {
          netio->send(node_b, (const char*)&d[i], sizeof(d[i]), msgid);
          //log_info << "send to " << node_b << " data:" << d[i] ;
        }
        if (node_id_ != node_c) {
          netio->send(node_c, (const char*)&d[i], sizeof(d[i]), msgid);
          //log_info << "send to " << node_c << " data:" << d[i] ;
        }
      } else if (PRIMARY || HELPER) {
        netio->recv(owners[i], (char*)&d[i], sizeof(d[i]), msgid);
       // log_info << "recv from  " << owners[i] << " data:" << d[i] ;
      } else if (std::find(non_computation_nodes.begin(), non_computation_nodes.end(), node_id_) != non_computation_nodes.end()) {
        netio->recv(node_c, (char*)&d[i], sizeof(d[i]), msgid);
        //log_info << "recv from  " << node_c << " data:" << d[i] ;
      }

      if (HELPER) {
        for (auto iter = non_computation_nodes.begin(); iter != non_computation_nodes.end(); iter++) {
          if (*iter != owners[i] && *iter != node_a && *iter != node_b && *iter != node_c) {
            netio->send(*iter, (const char*)&d[i], sizeof(d[i]), msgid);
            //log_info << "send to " << *iter << " data:" << d[i] ;
          }
        }
      }
    }
    Py_END_ALLOW_THREADS;
  }
  void sync_l(shared_ptr<NET_IO>& netio, vector<int>&d) {
    sync_d(netio, data_owner_, d);
  }
  void sync_l(shared_ptr<NET_IO>& netio, int& label) {
    vector<string> label_owner{label_owner_};
    vector<int> labels{label}; 
    sync_d(netio, label_owner, labels);
    label = labels[0];
  }
  template<class T>
  void sync_check(shared_ptr<NET_IO>& netio, const vector<string>& owners, vector<vector<T>> &d) {
    Py_BEGIN_ALLOW_THREADS;
    msg_id_t msgid("sync check for dataset");
    string node_c = netio->GetNodeId(PARTY_C);
    for (int i = 0; i < owners.size(); i++) {
      if (node_id_ == owners[i]) {
        if (node_id_ != node_c) {
          //log_debug << "send to " << node_c << " size:" << d[i].size() ;
          netio->send(node_c, d[i], d[i].size(), msgid);
        }
      }

      if (node_id_ == node_c) {
        if (node_id_ != owners[i]) {
          //log_debug << "recv from " << owners[i] << " size:" << d[i].size() ;
          netio->recv(owners[i], d[i], d[i].size(), msgid);
          //log_debug << "after recv from " << owners[i] << " size:" << d[i].size() ;
        }
        vector<string> nodes = netio->GetConnectedNodes();
        for (int j = 0; j < nodes.size(); j++) {
          if (nodes[j] != owners[i]) {
            //log_debug << "send to " << nodes[j] << " size:" << d[i].size() ;
            netio->send(nodes[j], d[i], d[i].size(), msgid);
          }
        }
      } else if (node_id_ != owners[i]) {
        //log_debug << "recv from " << node_c << " size:" << d[i].size() ;
        netio->recv(node_c, d[i], d[i].size(), msgid);
      }
    }
    Py_END_ALLOW_THREADS;
  }

  void sync_check(shared_ptr<NET_IO>& netio, const vector<string>& owners, vector<int> &d) {
    vector<vector<int>> ds(d.size());
    for (int i = 0; i < ds.size(); i++) {
      ds[i].push_back(d[i]);
    }
    sync_check(netio, owners, ds);
    for (int i = 0; i < ds.size(); i++) {
      d[i] = ds[i][0];
    }
  }
};
