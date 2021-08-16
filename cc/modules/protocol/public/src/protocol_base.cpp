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
#include "cc/modules/protocol/public/include/protocol_base.h"

#include <stdexcept>
#include <string>
#include <iostream>

using namespace std;

namespace rosetta {

int ProtocolBase::Init() {
  cout << "calling ProtocolBase::Init" << endl;
  THROW_NOT_IMPL_FN(__func__);
  return -1;
}

int ProtocolBase::Uninit() {
  cout << "calling ProtocolBase::Uninit" << endl;
  THROW_NOT_IMPL_FN(__func__);
  return -1;
}

void ProtocolBase::SetSaverModel(const SaverModel& model) {
  std::lock_guard<std::mutex> lock(status_mtx_);
  context_->SAVER_MODEL = model;
}

void ProtocolBase::SetRestoreModel(const RestoreModel& model) {
  std::lock_guard<std::mutex> lock(status_mtx_);
  context_->RESTORE_MODEL = model;
}

void ProtocolBase::SetFloatPrecision(int float_precision) {
  std::lock_guard<std::mutex> lock(status_mtx_);
  context_->FLOAT_PRECISION = float_precision;
}

} // namespace rosetta
