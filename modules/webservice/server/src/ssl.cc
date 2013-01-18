/*
** Copyright (C) 2000-2007, Robert van Engelen, Genivia Inc. All Rights Reserved.
** Copyright 2011-2013 Merethis
**
** This file is part of Centreon Engine.
**
** Centreon Engine is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Engine is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Engine. If not, see
** <http://www.gnu.org/licenses/>.
*/

#ifdef WITH_OPENSSL

# include <stdlib.h>
# include <unistd.h>
# include <openssl/crypto.h>
# include "soapH.h"

# if defined(WIN32)
#  define MUTEX_TYPE             HANDLE
#  define MUTEX_SETUP(x)         (x) = CreateMutex(NULL, false, NULL)
#  define MUTEX_CLEANUP(x)       CloseHandle(x)
#  define MUTEX_LOCK(x)          WaitForSingleObject((x), INFINITE)
#  define MUTEX_UNLOCK(x)        ReleaseMutex(x)
#  define THREAD_ID              GetCurrentThreadId()
# elif defined(_POSIX_THREADS) || defined(_SC_THREADS)
#  include <pthread.h>
#  define MUTEX_TYPE             pthread_mutex_t
#  define MUTEX_SETUP(x)         pthread_mutex_init(&(x), NULL)
#  define MUTEX_CLEANUP(x)       pthread_mutex_destroy(&(x))
#  define MUTEX_LOCK(x)          pthread_mutex_lock(&(x))
#  define MUTEX_UNLOCK(x)        pthread_mutex_unlock(&(x))
#  define THREAD_ID              pthread_self()
# else
#  error "You must define mutex operations appropriate for your platform"
#  error "See OpenSSL /threads/th-lock.c on how to implement mutex on your platform"
# endif

struct CRYPTO_dynlock_value {
  MUTEX_TYPE mutex;
};

static MUTEX_TYPE* mutex_buf;

static CRYPTO_dynlock_value* dyn_create_function(char const* file, int line) {
  (void)file;
  (void)line;

  CRYPTO_dynlock_value* value;
  if ((value = (CRYPTO_dynlock_value*)malloc(sizeof(*value))) != NULL) {
    MUTEX_SETUP(value->mutex);
  }
  return (value);
}

static void dyn_lock_function(int mode, CRYPTO_dynlock_value* l, char const* file, int line) {
  (void)file;
  (void)line;

  if (mode & CRYPTO_LOCK) {
    MUTEX_LOCK(l->mutex);
  }
  else {
    MUTEX_UNLOCK(l->mutex);
  }
}

static void dyn_destroy_function(CRYPTO_dynlock_value* l, char const* file, int line) {
  (void)file;
  (void)line;

  MUTEX_CLEANUP(l->mutex);
  free(l);
}

static void locking_function(int mode, int n, char const* file, int line) {
  (void)file;
  (void)line;

  if (mode & CRYPTO_LOCK) {
    MUTEX_LOCK(mutex_buf[n]);
  }
  else {
    MUTEX_UNLOCK(mutex_buf[n]);
  }
}

static unsigned long id_function() {
  return ((unsigned long)THREAD_ID);
}

int CRYPTO_thread_setup() {
  mutex_buf = (MUTEX_TYPE*)malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));
  if (!mutex_buf) {
    return (SOAP_EOM);
  }
  for (int i = 0; i < CRYPTO_num_locks(); ++i) {
    MUTEX_SETUP(mutex_buf[i]);
  }
  CRYPTO_set_id_callback(id_function);
  CRYPTO_set_locking_callback(locking_function);
  CRYPTO_set_dynlock_create_callback(dyn_create_function);
  CRYPTO_set_dynlock_lock_callback(dyn_lock_function);
  CRYPTO_set_dynlock_destroy_callback(dyn_destroy_function);
  return (SOAP_OK);
}

void CRYPTO_thread_cleanup() {
  if (mutex_buf) {
    CRYPTO_set_id_callback(NULL);
    CRYPTO_set_locking_callback(NULL);
    CRYPTO_set_dynlock_create_callback(NULL);
    CRYPTO_set_dynlock_lock_callback(NULL);
    CRYPTO_set_dynlock_destroy_callback(NULL);
    for (int i = 0; i < CRYPTO_num_locks(); ++i) {
      MUTEX_CLEANUP(mutex_buf[i]);
    }
    free(mutex_buf);
    mutex_buf = NULL;
  }
}

#endif // !WITH_OPENSSL
