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
#include "communication.h"

#if USE_NETIO_WITH_MESSAGEID
rosetta::io::ParallelIO* g_io;
#else
rosetta::io::IO* g_io;
#endif

bool initialize_communication(CommOptions& opt) {
#if USE_NETIO_WITH_MESSAGEID
  g_io = new rosetta::io::ParallelIO(opt.parties, opt.party_id, 1, opt.base_port, opt.hosts);
#else
  g_io = new rosetta::io::IO(opt.parties, opt.party_id, 1, opt.base_port, opt.hosts);
#endif
  g_io->set_server_cert(opt.server_cert_);
  g_io->set_server_prikey(opt.server_prikey_, opt.server_prikey_password_);
  bool ret = g_io->init();
  if (!ret) {
    cerr << "init faild" << endl;
    throw;
  }

  msg_id_t msgid("this message id for synchronize P0/P1/P2 init");
  // the following time(0) will show the sync beg/end
  cout << "  " << __FUNCTION__ << " beg sync :" << time(0) << endl;
  synchronize(msgid);
  cout << "  " << __FUNCTION__ << " end sync :" << time(0) << endl;

  return ret;
}
void uninitialize_communication() {
  msg_id_t msgid("this message id for synchronize P0/P1/P2 uninit");
  // the following time(0) will show the sync beg/end
  cout << __FUNCTION__ << " beg sync :" << time(0) << endl;
  synchronize(msgid);
  cout << __FUNCTION__ << " end sync :" << time(0) << endl;
  sleep(1);
  g_io->close();
  delete g_io;
  g_io = nullptr;
}
void synchronize(const msg_id_t& msg_id) {
  g_io->sync_with(msg_id);
}

void synchronize(int length) {
#if USE_NETIO_WITH_MESSAGEID
  cerr << "error! please use synchronize(const msg_id_t& msg_id)" << endl;
  throw;
#endif
  g_io->sync();
}
void synchronize() {
  synchronize(1);
}
