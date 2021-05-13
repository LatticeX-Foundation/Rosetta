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
#include "subprocess.h"
#include "net_helper.h"

#include <vector>
#include <cassert>
#include <thread>
#include <mutex>
using namespace std;

static void run_px(int parties, int party) {
  string logfile = "log/" + string(__FILENAME__) + "-" + to_string(party) + ".log";
  cout << "logfile:" << logfile << endl;

  Logger::Get().log_to_stdout(false);
  Logger::Get().set_filename(logfile);

  vector<string> ips;
  for (int i = 0; i < parties; i++) {
    ips.push_back("127.0.0.1");
  }

  // init io
  shared_ptr<BasicIO> io = nullptr;
  netutil::enable_ssl_socket(true);
  if (netutil::is_enable_ssl_socket()) {
    io = make_shared<SSLParallelNetIO>(parties, party, 1, 22222, ips);
  } else {
    io = make_shared<ParallelNetIO>(parties, party, 1, 11111, ips);
  }
  io->init();
  io->sync();

  switch (party) {
    case 0: {
      sleep(rand() % 3);
      io->sync();
      break;
    }
    case 1: {
      sleep(rand() % 5);
      io->sync();
      break;
    }
    case 2: {
      sleep(rand() % 7);
      io->sync();
      break;
    }
    default:
      break;
  }

  cout << "sync 1 ok" << endl;
  io->sync_with(msg_id_t("kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk"));
  sleep(rand() % 3);
}

int main(int argc, char* argv[]) {
  vector<pid_t> pids;
  if (get_subprocess(pids) == 0) {
    run_px(3, 0);
    return 0;
  }
  if (get_subprocess(pids) == 0) {
    run_px(3, 1);
    return 0;
  }
  if (get_subprocess(pids) == 0) {
    run_px(3, 2);
    return 0;
  }

  wait_subprocess(pids);
  return 0;
}
