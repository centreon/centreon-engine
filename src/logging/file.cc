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
#include <cstring>
#include "com/centreon/concurrency/locker.hh"
#include "com/centreon/concurrency/write_locker.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/file.hh"
#include "com/centreon/engine/statusdata.hh"

using namespace com::centreon;
using namespace com::centreon::engine::logging;

std::list<file*>             file::_files;
concurrency::read_write_lock file::_rwlock;

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
  : _file(new QFile(file.c_str())),
    _mutex(new concurrency::mutex),
    _size_limit(size_limit) {
  // Open file.
  _file->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
  if (_file->QFile::error() != QFile::NoError)
    throw (engine_error() << file << ": "
           << _file->errorString().toStdString());

  // Add file to list.
  concurrency::write_locker lock(&_rwlock);
  _files.push_back(this);
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
 *  Copy constructor.
 *
 * @param[in] right The class to copy.
 */
file::file(file const& right)
  : object(right),
    _size_limit(0) {
  operator=(right);
}

/**
 *  Default copy operator.
 *
 *  @param[in] right The class to copy.
 */
file& file::operator=(file const& right) {
  if (this != &right) {
    concurrency::locker lock(right._mutex.get());
    _file = right._file;
    _mutex = right._mutex;
    _size_limit = right._size_limit;
  }
  return (*this);
}

/**
 *  Get the file name.
 *
 *  @return The file name.
 */
std::string file::get_file_name() {
  concurrency::locker lock(_mutex.get());
  return (_file->fileName().toStdString());
}

/**
 *  Get the size limit.
 *
 *  @return The size limit.
 */
unsigned long long file::get_size_limit() const {
  concurrency::locker lock(_mutex.get());
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
    return ;

  // Rotate file if required.
  concurrency::locker lock(_mutex.get());
  if ((_size_limit > 0)
      && (static_cast<unsigned long long>(_file->size()
                                          + strlen(message))
          >= _size_limit)) {
    // Generate new and old file names.
    std::string old_name(_file->fileName().toStdString());
    std::string new_name(old_name);
    new_name.append(".old");

    // Close file.
    _file->close();

    // Remove old (N-2) rotated file.
    if ((!QFile::exists(new_name.c_str())
         || QFile::remove(new_name.c_str()))
	&& _file->rename(new_name.c_str()))
      _file->setFileName(old_name.c_str());

    // Open new log file.
    _file->open(QIODevice::WriteOnly
                | QIODevice::Text
                | QIODevice::Append);
  }

  // Write message to log file.
  _file->write(message);
  _file->flush();

  return ;
}

/**
 *  Close and open all files.
 */
void file::reopen() {
  concurrency::write_locker lock(&_rwlock);
  for (std::list<file*>::iterator it(_files.begin()), end(_files.end());
       it != end;
       ++it) {
    concurrency::locker lock((*it)->_mutex.get());
    (*it)->_file->close();
    (*it)->_file->open(
                    QIODevice::WriteOnly
                    | QIODevice::Text
                    | QIODevice::Append);
  }
  return ;
}

/**
 *  Set the new file's size limit.
 *
 *  @param[in] The size limit.
 */
void file::set_size_limit(unsigned long long size) {
  concurrency::locker lock(_mutex.get());
  _size_limit = size;
  return ;
}
