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
#include "cc/modules/common/include/utils/subprocess.h"

//#include <thread>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
//#include <string>
//#include <time.h>
//#include <condition_variable>
//#include <mutex>
#include <iostream>
//#include <atomic>
//#include <algorithm>
//#include <sstream>
//#include <fstream>
using namespace std;

static inline pid_t get_subprocess(vector<pid_t>& pids) {
  pid_t pid = fork();
  if (pid < 0) {
    cerr << "error in fork! pid:" << getpid() << endl;
    exit(1);
  }
  if (pid == 0) {
    cout << "in sub process, ppid:" << getppid() << ",pid:" << getpid() << endl;
  } else {
    pids.push_back(pid);
  }
  return pid;
}

static inline void wait_subprocess(const vector<pid_t>& pids) {
  for (int i = 0; i < pids.size(); ++i) {
    int status0 = -1;
    waitpid(pids[i], &status0, 0);
    cout << "pids[" << i << "]:" << pids[i] << ",status0 = " << WEXITSTATUS(status0) << endl;
  }
}

int main_example(int argc, char* argv[]) {
  int i = 0;
  vector<pid_t> pids;
  if (get_subprocess(pids) == 0) {
    cout << "ppid:" << getppid() << ",pid:" << getpid() << ",i:" << ++i << endl;
    return 0;
  }
  if (get_subprocess(pids) == 0) {
    cout << "ppid:" << getppid() << ",pid:" << getpid() << ",i:" << ++i << endl;
    return 0;
  }
  if (get_subprocess(pids) == 0) {
    cout << "ppid:" << getppid() << ",pid:" << getpid() << ",i:" << ++i << endl;
    return 0;
  }
  if (get_subprocess(pids) == 0) {
    cout << "ppid:" << getppid() << ",pid:" << getpid() << ",i:" << ++i << endl;
    return 0;
  }
  if (get_subprocess(pids) == 0) {
    cout << "ppid:" << getppid() << ",pid:" << getpid() << ",i:" << ++i << endl;
    return 0;
  }

  wait_subprocess(pids);

  return 0;
}
