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
#include "cc/modules/io/include/internal/server.h"
#include "cc/modules/common/include/utils/helper.h"

int init_ssl_locking();
namespace rosetta {
namespace io {

bool SSLServer::init_ssl() {
  init_ssl_locking();

// int index = sid_ * 3 + sid_;
// #if USE_GMTASSL
//   int ret = gmtassl_init_ssl_ctx(
//     true, &ctx_, netutil::vgs_root[index], netutil::vgs_cert[index], netutil::vgs_prik[index],
//     netutil::vgs_enc_cert[index], netutil::vgs_enc_prik[index], "123456");
// #else // USE_OPENSSL
//   int ret = openssl_init_ssl_ctx(
//     true, &ctx_, netutil::vgs_root[index], netutil::vgs_cert[index], netutil::vgs_prik[index],
//     "123456");
// #endif
#if USE_GMTASSL
#else // USE_OPENSSL
  int ret = openssl_init_ssl_ctx(
    true, &ctx_, "certs/Root.crt", "certs/Server.crt", "certs/Server.key", "123456");
#endif

  if (ret != 0 || ctx_ == nullptr) {
    ERR_print_errors_fp(stderr);
    log_error << "SSLServer init_ssl_ctx ret != 0 || ctx_ == nullptr" << endl;
    //throw socket_exp("SSLServer init_ssl_ctx ret != 0 || ctx_ == nullptr");
    return false;
  }
  return true;
}
SSLServer::SSLServer() { is_ssl_socket_ = true; }

SSLServer::~SSLServer() {
  stop();

  if (ctx_ != nullptr) {
    SSL_CTX_free(ctx_);
    ctx_ = nullptr;
  }
}

} // namespace io
} // namespace rosetta