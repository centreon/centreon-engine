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
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/file.hh"
#include "com/centreon/engine/statusdata.hh"

using namespace com::centreon;
using namespace com::centreon::engine::logging;

std::list<file*> file::_files;
QReadWriteLock   file::_rwlock;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Default constructor.
 *
 *  @param[in] file       The file name.
 *  @param[in] size_limit The file's size limit.
 */
file::file(std::string const& file, unsigned long long size_limit)
  : _mutex(new concurrency::mutex),
    _file(new QFile(file.c_str())),
    _size_limit(size_limit) {
  _file->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
  if (_file->QFile::error() != QFile::NoError)
    throw (engine_error() << file << ": "
           << _file->errorString().toStdString());

  _rwlock.lockForWrite();
  _files.push_back(this);
  _rwlock.unlock();
}

/**
 *  Default destructor.
 */
file::~file() throw() {
  _rwlock.lockForWrite();
  std::list<file*>::iterator
    it(std::find(_files.begin(), _files.end(), this));
  if (it != _files.end())
    _files.erase(it);
  _rwlock.unlock();
}

/**
 *  Default copy constructor.
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
    right._mutex->lock();
    _mutex = right._mutex;
    _file = right._file;
    _size_limit = right._size_limit;
    right._mutex->unlock();
  }
  return (*this);
}

/**
 *  Get the file name.
 *
 *  @return The file name.
 */
std::string file::get_file_name() throw() {
  _mutex->lock();
  std::string filename = qPrintable(_file->fileName());
  _mutex->unlock();
  return (filename);
}

/**
 *  Get the size limit.
 *
 *  @return The size limit.
 */
unsigned long long file::get_size_limit() const throw() {
  _mutex->lock();
  unsigned long long size = _size_limit;
  _mutex->unlock();
  return (size);
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
             unsigned int verbosity) throw() {
  (void)type;
  (void)verbosity;

  if (message == NULL) {
    return;
  }

  _mutex->lock();
  if ((_size_limit > 0)
      && (static_cast<unsigned long long>(_file->size() + strlen(message))
          >= _size_limit)) {
    std::string old_name = qPrintable(_file->fileName());
    std::string new_name = old_name + ".old";

    _file->close();
    if ((QFile::exists(new_name.c_str()) == false || QFile::remove(new_name.c_str()) == true)
	&& _file->rename(new_name.c_str()) == true) {
      _file->setFileName(old_name.c_str());
    }
    _file->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
  }

  _file->write(message);
  _file->flush();

  _mutex->unlock();
}

/**
 *  Close and open all files.
 */
void file::reopen() {
  _rwlock.lockForWrite();
  for (std::list<file*>::iterator it(_files.begin()), end(_files.end());
       it != end;
       ++it) {
    (*it)->_mutex->lock();
    (*it)->_file->close();
    (*it)->_file->open(
                    QIODevice::WriteOnly
                    | QIODevice::Text
                    | QIODevice::Append);
    (*it)->_mutex->unlock();
  }
  _rwlock.unlock();
}

/**
 *  Set the new file's size limit.
 *
 *  @param[in] The size limit.
 */
void file::set_size_limit(unsigned long long size) throw() {
  _mutex->lock();
  _size_limit = (size > 0 ? size : 0);
  _mutex->unlock();
}
