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
#include "internal/server.h"

int init_ssl_locking();
namespace rosetta {
namespace io {

static SSL_CTX* evssl_init(
  string server_cert, string server_prikey, string server_prikey_password) {
  SSL_CTX* ctx = nullptr;

  //! Initialize the OpenSSL library
  SSL_load_error_strings();
  SSL_library_init();

  if (!RAND_poll())
    return nullptr;

  ctx = SSL_CTX_new(SSLv23_server_method());

  //! set a password (if have).
  SSL_CTX_set_default_passwd_cb_userdata(ctx, (void*)server_prikey_password.c_str());

  if (!SSL_CTX_use_certificate_chain_file(ctx, server_cert.c_str())) {
    cerr << "error: SSL_CTX_use_certificate_file " << server_cert << endl;
    return nullptr;
  }

  if (!SSL_CTX_use_PrivateKey_file(ctx, server_prikey.c_str(), SSL_FILETYPE_PEM)) {
    cerr << "error: SSL_CTX_use_PrivateKey_file " << server_prikey << endl;
    return nullptr;
  }

  SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);

  cout << "ssl server init ssl library & load cert done!" << endl;
  return ctx;
}

bool SSLServer::init_ssl() {
  init_ssl_locking();
  ctx_ = evssl_init(server_cert_, server_prikey_, server_prikey_password_);
  if (ctx_ == NULL) {
    ERR_print_errors_fp(stderr);
    exit(0);
  }
  return true;
}
SSLServer::SSLServer() {
  is_ssl_socket_ = true;
}

SSLServer::~SSLServer() {
  stop();

  if (ctx_ != nullptr) {
    SSL_CTX_free(ctx_);
    ctx_ = nullptr;
  }
}

} // namespace io
} // namespace rosetta