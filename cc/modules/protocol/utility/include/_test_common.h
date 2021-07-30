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

#include <thread>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string>
#include <time.h>
#include <condition_variable>
#include <mutex>
#include <iostream>
#include <atomic>
#include <algorithm>
#include <sstream>
#include <fstream>
using namespace std;

#include "cc/modules/common/include/utils/random_util.h"
#include "cc/modules/protocol/utility/include/_test_check_func.h"

#define TEST_TASK_NUM    4

#define PERFORMANCE_TEST 0
typedef void (*_mpc_run_func)(int);
static inline int _mpc_main(int argc, char* argv[], _mpc_run_func run) {
  int ret = 0;
  int status0 = -1;
  int status1 = -1;

  pid_t p0 = fork();
  if (p0 < 0) {
    cerr << "error in fork p0!" << endl;
    exit(1);
  }
  if (p0 == 0) {
    run(0); // P0
  } else {
    pid_t p1 = fork();
    if (p1 < 0) {
      cerr << "error in fork p1!" << endl;
      exit(1);
    }

    if (p1 == 0) {
      run(1); // P1
    } else {
      run(2); // P2

      waitpid(p0, &status0, 0);
      //printf("wait p0 status0 = %d\n", WEXITSTATUS(status0));
      waitpid(p1, &status1, 0);
      //printf("wait p1 status1 = %d\n", WEXITSTATUS(status1));
      printf("3 mock nodes run to the end.\n");
    }
  }

  return ret;
}

static inline int _zk_main(int argc, char* argv[], _mpc_run_func run) {
  signal(SIGCHLD,SIG_IGN);
  int ret = 0;
  int status0 = -1;
  int status1 = -1;

  pid_t p0 = fork();
  if (p0 < 0) {
    cerr << "error in fork p0!" << endl;
    exit(1);
  }
  if (p0 == 0) {
    run(0); // P0
  } else {
    run(1); // P1

    waitpid(p0, &status0, 0);
    printf("status0 = %d\n", WEXITSTATUS(status0));
  }

  return ret;
}


#define print_result 0
#if print_result
#define if_print_vec print_vec
#else
template <class T>
inline void if_print_vec(const vector<T>& a, int length = -1, string msg = "") {}
#endif

#define RUN_MPC_TEST(func) \
  int main(int argc, char* argv[]) { return _mpc_main(argc, argv, func); }

#define RUN_ZK_TEST(func) \
  int main(int argc, char* argv[]) { return _zk_main(argc, argv, func); }

static string get_file_name(const char* file) {
  string file_path(file);
  size_t pos = file_path.find_last_of('/');
  if (pos != string::npos)
    return file_path.substr(pos+1, file_path.size() - pos);
  else
    return file_path;
}

#if !defined(__FILENAME__)
#define __FILENAME__ (strrchr(__FILE__, '\\') ? std::strrchr(__FILE__, '\\') + 1 : __FILE__)
#endif
