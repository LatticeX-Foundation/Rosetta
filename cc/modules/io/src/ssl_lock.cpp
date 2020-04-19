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
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/crypto.h>

/**
 * for openssl lower version
 */

pthread_mutex_t* ssl_locks;
int ssl_num_locks;
bool ssl_lock_inited = false;

/* Implements a thread-ID function as requied by openssl */
static unsigned long get_thread_id_cb(void) {
  return (unsigned long)pthread_self();
}

static void thread_lock_cb(int mode, int which, const char* f, int l) {
  if (which < ssl_num_locks) {
    if (mode & CRYPTO_LOCK) {
      pthread_mutex_lock(&(ssl_locks[which]));
    } else {
      pthread_mutex_unlock(&(ssl_locks[which]));
    }
  }
}

int init_ssl_locking() {
  if (ssl_lock_inited)
    return 0;

  ssl_lock_inited = true;

  ssl_num_locks = CRYPTO_num_locks();
  ssl_locks = (pthread_mutex_t*)malloc(ssl_num_locks * sizeof(pthread_mutex_t));
  if (ssl_locks == NULL)
    return -1;

  for (int i = 0; i < ssl_num_locks; i++) {
    pthread_mutex_init(&(ssl_locks[i]), NULL);
  }

  CRYPTO_set_id_callback(get_thread_id_cb);
  CRYPTO_set_locking_callback(thread_lock_cb);

  return 0;
}
