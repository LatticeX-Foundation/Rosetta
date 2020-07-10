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
#include "cc/modules/io/include/internal/client.h"
#include "cc/modules/common/include/utils/helper.h"

int init_ssl_locking();
namespace rosetta {
namespace io {

SSLClient::SSLClient(const std::string& ip, int port) : TCPClient(ip, port) {
  is_ssl_socket_ = true;

  init_ssl_locking();

  SSL_library_init();
  OpenSSL_add_all_algorithms();
  ERR_load_crypto_strings();
  SSL_load_error_strings();

  ctx_ = SSL_CTX_new(SSLv23_client_method());
  if (ctx_ == nullptr) {
    ERR_print_errors_fp(stdout);
    exit(1);
  }

  ssl_ = SSL_new(ctx_);
  log_debug << "ssl client init ssl library done!" << endl;
}

SSLClient::~SSLClient() {
  close();

  if (ctx_ != nullptr) {
    SSL_CTX_free(ctx_);
    ctx_ = nullptr;
  }
}

} // namespace io
} // namespace rosetta