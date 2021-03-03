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

#include "cc/modules/io/include/internal/socket.h"

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <openssl/x509v3.h>

namespace rosetta {
namespace io {
int gmtassl_init_ssl_ctx(
  bool is_server,
  SSL_CTX** ctx,
  std::string root_,
  std::string prik_,
  std::string cert_,
  std::string enc_prik_,
  std::string enc_cert_,
  std::string pass_);
int openssl_init_ssl_ctx(
  bool is_server,
  SSL_CTX** ctx,
  std::string root_,
  std::string prik_,
  std::string cert_,
  std::string pass_);
} // namespace io
} // namespace rosetta

namespace netutil {
bool show_certs(SSL* ssl, const std::string& client_ip);
bool ssl_valid_check(void* x509_cert, void* x509_enc_cert, const std::string& ip);
bool ssl_params_valid_check_mpc(int partyid);
} // namespace netutil