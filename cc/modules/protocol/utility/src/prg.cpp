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
#include "cc/modules/protocol/utility/include/prg.h"

#include "emp-tool/emp-tool.h"
using namespace emp;

namespace rosetta {

RttPRG::RttPRG() {
  prg_ = make_shared<emp::PRG>();
  reseed(kdefault);
}
RttPRG::RttPRG(const std::string& key) {
  prg_ = make_shared<emp::PRG>();
  reseed(key);
}

block RttPRG::newRandomBlocks() {
  if (counter_++ % BLOCK_COUNT == 0) {
    randomDatas((char*)data_, BLOCK_COUNT * sizeof(block));
  }
  return data_[counter_ % BLOCK_COUNT];
}

void RttPRG::randomDatas(void* data, int nbytes) { prg_->random_data(data, nbytes); }
uint64_t RttPRG::get64Bits() {
  uint64_t ret = 0;
  if (index64 == 0)
    block64 = newRandomBlocks();

  uint64_t* temp = (uint64_t*)&block64;
  ret = temp[index64];

  index64++;
  if (index64 == 2)
    index64 = 0;

  return ret;
}
uint8_t RttPRG::get8Bits() {
  uint8_t ret;

  if (index08 == 0)
    block08 = newRandomBlocks();

  uint8_t* temp = (uint8_t*)&block08;
  ret = temp[index08];

  index08++;
  if (index08 == 16)
    index08 = 0;

  return ret;
}
uint8_t RttPRG::getBit() {
  uint8_t ret;

  if (index01 == 0)
    block01 = newRandomBlocks();

  uint8_t* temp = (uint8_t*)&block01;
  ret = temp[index01 >> 3];
  ret = (ret >> (index01 & 7)) & 0x01;

  index01++;
  if (index01 == 128)
    index01 = 0;

  return ret;
}

void RttPRG::reseed(const void* key, uint64_t id) { prg_->reseed((const block*)key, id); }
void RttPRG::reseed(const std::string& key) { reseed((const void*)key.data()); }

} // namespace rosetta
