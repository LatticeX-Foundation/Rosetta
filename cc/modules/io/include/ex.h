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
#include "net_io.h"

/**
 * This header for io tests and examples
 */
#define USE_SSL_SOCKET 1
namespace rosetta {
namespace io {
#if USE_SSL_SOCKET
typedef SSLServer Server;
typedef SSLClient Client;
typedef SSLNetIO IO;
typedef SSLParallelNetIO ParallelIO;
#else
typedef TCPServer Server;
typedef TCPClient Client;
typedef NetIO IO;
typedef ParallelNetIO ParallelIO;
#endif
} // namespace io
} // namespace rosetta

using namespace rosetta::io;

// helpers
template <typename T>
static inline void rand_vec(vector<T>& vec, int length) {
  vec.clear();
  vec.resize(length);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<T> dis(
    std::numeric_limits<T>::min(), std::numeric_limits<T>::max());

  for (int i = 0; i < length; i++) {
    vec[i] = dis(gen);
  }
}
