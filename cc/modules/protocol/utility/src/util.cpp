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
#include "cc/modules/protocol/utility/include/util.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_helper.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <chrono>
#include <random>
#include <mutex>

using namespace std;

namespace rosetta {

static std::mutex s_cout_mutex;
static std::streambuf* cout_buf = nullptr;
static std::ofstream of;
static bool redirect_io = false;

// redirect stdout to external specified log file
void redirect_stdout(const std::string& logfile, const std::string& taskid/*=""*/) {
  std::lock_guard<std::mutex> lck(s_cout_mutex);
  if (cout_buf)
    return;
  
  cout_buf = cout.rdbuf();
  of.open(logfile);
  streambuf* fileBuf = of.rdbuf();
  cout.rdbuf(fileBuf);
  redirect_io = true;
}

void restore_stdout() {
  std::lock_guard<std::mutex> lck(s_cout_mutex);
  if (!redirect_io)
    return;
  // return;////////// fix me
  of.flush();
  of.close();
  cout.rdbuf(cout_buf);
}
} // namespace rosetta
