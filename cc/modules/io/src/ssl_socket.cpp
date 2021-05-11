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
#include "cc/modules/io/include/internal/ssl_socket.h"

#include <iostream>
using namespace std;

namespace rosetta {
namespace io {

int gmtassl_init_ssl_ctx(
  bool is_server,
  SSL_CTX** ctx,
  std::string root_,
  std::string cert_,
  std::string prik_,
  std::string enc_cert_,
  std::string enc_prik_,
  std::string pass_) {
  // init ssl
  ////////////////////////////////////////////

  SSL_library_init();
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();

#if USE_GMTASSL
  if (is_server) {
    *ctx = SSL_CTX_new(SSLv23_server_method()); // SSL V2/V3
    //*ctx = SSL_CTX_new(GMTLS_server_method());
  } else {
    *ctx = SSL_CTX_new(CNTLS_client_method());
    //*ctx = SSL_CTX_new(GMTLS_client_method());
  }
#else // USE_OPENSSL
  if (is_server) {
    *ctx = SSL_CTX_new(SSLv23_server_method()); // SSL V2/V3
  } else {
    *ctx = SSL_CTX_new(SSLv23_client_method());
  }
#endif

  if (*ctx == NULL) {
    ERR_print_errors_fp(stdout);
    log_error << "Init, SSL_CTX_new failed." << endl;
    //throw socket_exp("Init, SSL_CTX_new failed.");
    return -1;
  }

  // auto retry
  signal(SIGPIPE, SIG_IGN);
  signal(SIGCHLD, SIG_IGN);
  if (!(SSL_CTX_set_mode(*ctx, SSL_MODE_AUTO_RETRY) & SSL_MODE_AUTO_RETRY)) {
    ERR_print_errors_fp(stdout);
    log_error << "Init, SSL_CTX_set_mode failed." << endl;
    //throw socket_exp("Init, SSL_CTX_set_mode failed.");
    return -1;
  }

  // set verify, call SSL_get_verify_result() do verify
  if (is_server) {
    SSL_CTX_set_verify(*ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
  } else {
    SSL_CTX_set_verify(*ctx, SSL_VERIFY_PEER, NULL);
  }

  // set a password (if have).
  if (!pass_.empty()) {
    //SSL_CTX_set_default_passwd_cb_userdata(*ctx, (void*)pass_.c_str());
    SSL_CTX_set_default_passwd_cb_userdata(*ctx, (void*)pass_.c_str());
  }

  // root CA
  if (SSL_CTX_load_verify_locations(*ctx, root_.c_str(), NULL) <= 0) {
    ERR_print_errors_fp(stdout);
    log_error << "Init, SSL_CTX_load_verify_locations failed." << endl;
    //throw socket_exp("Init, SSL_CTX_load_verify_locations failed.");
    return -1;
  }

  //SSL_CTX_set_client_CA_list(*ctx, SSL_load_client_CA_file(root_.c_str()));

  {
    // sign cert
    if (SSL_CTX_use_certificate_file(*ctx, cert_.c_str(), SSL_FILETYPE_PEM) <= 0) {
      ERR_print_errors_fp(stdout);
      log_error << "Init, SSL_CTX_use_certificate_file failed." << endl;
      //throw socket_exp("Init, SSL_CTX_use_certificate_file failed.");
      return -1;
    }

    // sign private key
    if (SSL_CTX_use_PrivateKey_file(*ctx, prik_.c_str(), SSL_FILETYPE_PEM) <= 0) {
      ERR_print_errors_fp(stdout);
      log_error << "Init, SSL_CTX_use_PrivateKey_file failed." << endl;
      //throw socket_exp("Init, SSL_CTX_use_PrivateKey_file failed.");
      return -1;
    }

    // check sign private key is ok
    if (!SSL_CTX_check_private_key(*ctx)) {
      ERR_print_errors_fp(stdout);
      log_error << "Init, SSL_CTX_check_private_key failed." << endl;
      //throw socket_exp("Init, SSL_CTX_check_private_key failed.");
      return -1;
    }
  }

#if USE_GMTASSL
  {
    // enc cert
    if (SSL_CTX_use_enc_certificate_file(*ctx, enc_cert_.c_str(), SSL_FILETYPE_PEM) <= 0) {
      ERR_print_errors_fp(stdout);
      log_error << "Init, SSL_CTX_use_enc_certificate_file failed." << endl;
      //throw socket_exp("Init, SSL_CTX_use_enc_certificate_file failed.");
      return -1;
    }

    // enc private key
    if (SSL_CTX_use_enc_PrivateKey_file(*ctx, enc_prik_.c_str(), SSL_FILETYPE_PEM) <= 0) {
      ERR_print_errors_fp(stdout);
      log_error << "Init, SSL_CTX_use_enc_PrivateKey_file failed." << endl;
      //throw socket_exp("Init, SSL_CTX_use_enc_PrivateKey_file failed.");
      return -1;
    }

    // check enc private key is ok
    if (!SSL_CTX_check_enc_private_key(*ctx)) {
      ERR_print_errors_fp(stdout);
      log_error << "Init, SSL_CTX_check_enc_private_key failed." << endl;
      //throw socket_exp("Init, SSL_CTX_check_enc_private_key failed.");
      return -1;
    }
  }

  if (!is_server) {
    if (!SSL_CTX_set_cipher_list(*ctx, "ECC-SM4-SM3")) {
      log_error << "Init, SSL_CTX_set_cipher_list failed." << endl;
      //throw ssl_socket_exp("Init, SSL_CTX_set_cipher_list failed.");
      return -1;
    }
  }
#endif

  //SSL_CTX_set_verify_depth(*ctx, 1);
  SSL_CTX_set_verify_depth(*ctx, 10);

  return 0;
}

int openssl_init_ssl_ctx(
  bool is_server,
  SSL_CTX** ctx,
  std::string root_,
  std::string cert_,
  std::string prik_,
  std::string pass_) {
  return gmtassl_init_ssl_ctx(is_server, ctx, root_, cert_, prik_, "", "", pass_);
}

} // namespace io
} // namespace rosetta

namespace netutil {
bool show_certs(SSL* ssl, const std::string& client_ip) {
  if (SSL_get_verify_result(ssl) == X509_V_OK) {
    log_info << "verify success" << endl;
  } else {
    log_error << "verify failed" << endl;
    return false;
  }

  //if (!netutil::is_check_whitelist()) {
  // do not check
  return true;
  //}

  STACK_OF(X509)* sk = SSL_get_peer_cert_chain(ssl);
  if (sk != nullptr) {
    int ii = 0;
    while (1) {
      X509* cert = sk_X509_pop(sk);
      if (cert == nullptr)
        break;

      char* CC = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
      log_debug << ((client_ip != "") ? "in server" : "in client") << ", the peer cert [" << ii++
                << "] subject name:" << CC << endl;

      //X509_free(cert);
    }
    //sk_X509_free(sk);
  }

  // whitelist check [only for server]
  if (client_ip != "") {
    log_info << "whitelist check [only for server], client ip:" << client_ip << endl;

    X509* cert = SSL_get_peer_certificate(ssl);
    if (cert != nullptr) {
      if (!netutil::ssl_valid_check((void*)cert, nullptr, client_ip)) {
        log_error << "whitelist check failed." << endl;
        X509_free(cert);
        return false;
      } else {
        log_info << "whitelist check success." << endl;
      }
      X509_free(cert);
    } else {
      log_error << "no peer cert info." << endl;
      return false;
    }
  }
  return true;
}
} // namespace netutil