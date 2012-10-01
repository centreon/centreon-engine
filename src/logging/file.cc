/*
** Copyright 2011-2012 Merethis
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

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <list>
#include <sstream>
#include "com/centreon/concurrency/locker.hh"
#include "com/centreon/concurrency/read_locker.hh"
#include "com/centreon/concurrency/read_write_lock.hh"
#include "com/centreon/concurrency/write_locker.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/file.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/io/file_stream.hh"

using namespace com::centreon;
using namespace com::centreon::engine::logging;

// Local objects.
static std::list<file*>             _files;
static concurrency::read_write_lock _rwlock;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Constructor.
 *
 *  @param[in] file       The file name.
 *  @param[in] size_limit The file's size limit.
 */
file::file(std::string const& file, unsigned long long size_limit)
  : _filename(file),
    _size_limit(size_limit) {
  // Open file.
  _open();

  // Add file to list.
  concurrency::write_locker lock(&_rwlock);
  _files.push_back(this);
}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
file::file(file const& right)
  : object(right), _size_limit(0) {
  _internal_copy(right);
}

/**
 *  Destructor.
 */
file::~file() throw () {
  try {
    concurrency::write_locker lock(&_rwlock);
    std::list<file*>::iterator
      it(std::find(_files.begin(), _files.end(), this));
    if (it != _files.end())
      _files.erase(it);
  }
  catch (...) {}
}

/**
 *  Assignment operator.
 *
 *  @param[in] right The class to copy.
 */
file& file::operator=(file const& right) {
  if (this != &right) {
    _close();
    _internal_copy(right);
  }
  return (*this);
}

/**
 *  Get the file name.
 *
 *  @return The file name.
 */
std::string const& file::get_file_name() const throw () {
  return (_filename);
}

/**
 *  Get the size limit.
 *
 *  @return The size limit.
 */
unsigned long long file::get_size_limit() const {
  concurrency::locker lock(&_mutex);
  return (_size_limit);
}

/**
 *  Write log into file.
 *
 *  @param[in] message   Message to log.
 *  @param[in] type      Logging types.
 *  @param[in] verbosity Verbosity level.
 */
void file::log(
             char const* message,
             unsigned long long type,
             unsigned int verbosity) throw () {
  (void)type;
  (void)verbosity;

  if (!message)
    return;

  std::ostringstream oss;
  oss << "[" << time(NULL) << "] " << message;
  std::string const& data(oss.str());

  // Rotate file if required.
  concurrency::locker lock(&_mutex);
  if ((_size_limit > 0)
      && (static_cast<unsigned long long>(_written + data.size())
          >= _size_limit)) {
    // Generate new and old file names.
    std::string old_filename(_filename);
    old_filename.append(".old");

    // Close file.
    _close();

    // Remove old (N-2) rotated file.
    ::remove(old_filename.c_str());
    ::rename(_filename.c_str(), old_filename.c_str());

    // Open new log file.
    _open();
  }

  // Write message to log file.
  char const* msg(data.c_str());
  unsigned int msg_len(data.size());
  while (msg_len) {
    unsigned int wb(_file.write(msg, msg_len));
    _written += wb;
    msg_len -= wb;
    msg += wb;
  }

  // Flush file.
  _file.flush();
  return;
}

/**
 *  Close and open all files.
 */
void file::reopen() {
  concurrency::read_locker lock(&_rwlock);
  for (std::list<file*>::iterator it(_files.begin()), end(_files.end());
       it != end;
       ++it) {
    concurrency::locker lock(&(*it)->_mutex);
    (*it)->_close();
    (*it)->_open();
  }
  return;
}

/**
 *  Set the new file's size limit.
 *
 *  @param[in] The size limit.
 */
void file::set_size_limit(unsigned long long size) {
  concurrency::locker lock(&_mutex);
  _size_limit = size;
  return;
}

/**************************************
*                                     *
*           Private Methods           *
*                                     *
**************************************/

/**
 *  Close log file.
 */
void file::_close() {
  concurrency::locker lock(&_mutex);
  _file.close();
  _written = 0;
  return;
}

/**
 *  Copy internal data members.
 *
 *  @param[in] right Object to copy.
 */
void file::_internal_copy(file const& right) {
  // Copy properties.
  concurrency::locker lock1(&_mutex);
  {
    concurrency::locker lock2(&right._mutex);
    _filename = right._filename;
    _size_limit = right._size_limit;
  }

  // Reopen file.
  _open();
  return;
}

/**
 *  Open log file.
 */
void file::_open() {
  concurrency::locker lock(&_mutex);
  _file.open(_filename.c_str(), "ab");
  _written = _file.size();
  return;
}
