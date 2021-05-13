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
#include "cc/modules/io/include/internal/socket.h"

#if USE_LIBEVENT_AS_BACKEND
namespace rosetta {
namespace io {
static void logcb(int severity, const char* msg) {
  cerr << "severity: " << severity << ", msg:" << msg << endl;
  /* This callback does nothing. */
}

void enable_event_log() {
  return;
  // set debug/log callback
  event_set_log_callback(logcb);
  event_enable_debug_logging(EVENT_DBG_ALL);
}

void enable_threads() {
#ifdef _WIN32
  evthread_use_windows_threads();
#else
  evthread_use_pthreads();
#endif
}
} // namespace io
} // namespace rosetta
#endif