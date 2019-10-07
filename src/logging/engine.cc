/*
** Copyright 2011-2013 Centreon
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
**
** For more information : contact@centreon.com
*/

#include "com/centreon/engine/logging/engine.hh"
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <memory>
#include "com/centreon/concurrency/locker.hh"
#include "com/centreon/engine/logging/backend.hh"
#include "com/centreon/exceptions/basic.hh"

using namespace com::centreon::concurrency;
using namespace com::centreon::engine::logging;

/**
 *  Add backend into the logging engine.
 *
 *  @param[in] obj      The backend to add into the logging engine.
 *  @param[in] types    The types to log with this backend.
 *  @param[in] verbose  The verbosity level to log with this backend.
 *
 *  @return The id of backend into the logging engine.
 */
unsigned long engine::add(backend* obj,
                          unsigned long long types,
                          unsigned int verbose) {
  if (!obj)
    throw(basic_error() << "add backend on the logging engine "
                           "failed: bad argument (null pointer)");
  if (verbose >= sizeof(unsigned int) * CHAR_BIT)
    throw(basic_error() << "add backend on the logging engine "
                           "failed: invalid verbose");

  std::unique_ptr<backend_info> info(new backend_info);
  info->obj = obj;
  info->types = types;
  info->verbose = verbose;

  // Lock engine.
  locker lock(&_mtx);
  info->id = ++_id;
  for (unsigned int i(0); i <= verbose; ++i)
    _list_types[i] |= types;

  _backends.push_back(info.get());
  return (info.release()->id);
}

/**
 *  Log messages.
 *
 *  @param[in] types    The logging type to log.
 *  @param[in] verbose  The verbosity level.
 *  @param[in] msg      The string to log.
 *  @param[in] size     The string size to log.
 */
void engine::log(unsigned long long types,
                 unsigned int verbose,
                 char const* msg,
                 unsigned int size) {
  if (!msg)
    return;

  // Lock engine.
  locker lock(&_mtx);
  for (std::vector<backend_info*>::const_iterator it(_backends.begin()),
       end(_backends.end());
       it != end; ++it)
    if (((*it)->types & types) && (*it)->verbose >= verbose)
      (*it)->obj->log(types, verbose, msg, size);
}

/**
 *  Remove backend by id.
 *
 *  @param[in] id  The backend id.
 *
 *  @return True if the backend was remove, otherwise false.
 */
bool engine::remove(unsigned long id) {
  // Lock engine.
  locker lock(&_mtx);
  for (std::vector<backend_info*>::iterator it(_backends.begin()),
       end(_backends.end());
       it != end; ++it)
    if ((*it)->id == id) {
      delete *it;
      _backends.erase(it);
      _rebuild_types();
      return (true);
    }
  return (false);
}

/**
 *  Remove backend.
 *
 *  @param[in] obj  The specific backend.
 *
 *  @return The number of backend was remove.
 */
unsigned int engine::remove(backend* obj) {
  if (!obj)
    throw(basic_error() << "remove backend on the logging engine "
                           "failed:bad argument (null pointer)");

  // Lock engine.
  locker lock(&_mtx);
  std::vector<backend_info*>::iterator it(_backends.begin());
  unsigned int count_remove(0);
  while (it != _backends.end()) {
    if ((*it)->obj != obj)
      ++it;
    else {
      delete *it;
      it = _backends.erase(it);
      ++count_remove;
    }
  }
  if (count_remove)
    _rebuild_types();
  return (count_remove);
}

/**
 *  Close and open all backend.
 */
void engine::reopen() {
  locker lock(&_mtx);
  for (std::vector<backend_info*>::const_iterator it(_backends.begin()),
       end(_backends.end());
       it != end; ++it)
    (*it)->obj->reopen();
}

/**
 *  Default constructor.
 */
engine::engine() : _id(0) {
  memset(_list_types, 0, sizeof(_list_types));
}

/**
 *  Destructor.
 */
engine::~engine() throw() {
  for (std::vector<backend_info*>::const_iterator it(_backends.begin()),
       end(_backends.end());
       it != end; ++it)
    delete *it;
}

/**
 *  Rebuild the types information.
 */
void engine::_rebuild_types() {
  memset(_list_types, 0, sizeof(_list_types));
  for (std::vector<backend_info*>::const_iterator it(_backends.begin()),
       end(_backends.end());
       it != end; ++it) {
    for (unsigned int i(0); i <= (*it)->verbose; ++i)
      _list_types[i] |= (*it)->types;
  }
}
