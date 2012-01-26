/*
** Copyright 2011 Merethis
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
#include <string.h>
#include "statusdata.hh"
#include "globals.hh"
#include "error.hh"
#include "logging/file.hh"

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
  : _mutex(new QMutex),
    _file(new QFile(file.c_str())),
    _size_limit(size_limit) {
  _file->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
  if (_file->QFile::error() != QFile::NoError) {
    throw (engine_error() << file << ": " << _file->errorString());
  }
  _rwlock.lockForWrite();
  _files.push_back(this);
  _rwlock.unlock();
}

/**
 *  Constructor.
 *
 *  @param[in] file         The file name.
 *  @param[in] archive_path The archive path for the rotation.
 */
file::file(std::string const& file, std::string const& archive_path)
  : _mutex(new QMutex),
    _file(new QFile(file.c_str())),
    _archive_path(archive_path),
    _size_limit(0) {
  _file->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
  if (_file->QFile::error() != QFile::NoError) {
    throw (engine_error() << file << ": " << _file->errorString());
  }
  _rwlock.lockForWrite();
  _files.push_back(this);
  _rwlock.unlock();
}

/**
 *  Default destructor.
 */
file::~file() throw() {
  _rwlock.lockForWrite();
  std::list<file*>::iterator it = std::find(_files.begin(), _files.end(), this);
  if (it != _files.end()) {
    _files.erase(it);
  }
  _rwlock.unlock();
}

/**
 *  Default copy constructor.
 *
 * @param[in] right The class to copy.
 */
file::file(file const& right)
  : object(right), _size_limit(0) {
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
    _archive_path = right._archive_path;
    _size_limit = right._size_limit;
    right._mutex->unlock();
  }
  return (*this);
}

/**
 *  Archive all files.
 */
void file::rotate_all() {
  _rwlock.lockForRead();

  // update the last log rotation time and status log.
  last_log_rotation = time(NULL);
  update_program_status(FALSE);

  for (std::list<file*>::iterator it = _files.begin(), end = _files.end();
       it != end;
       ++it) {
    (*it)->rotate();
  }
  _rwlock.unlock();
}

 /**
 *  Archive file.
 */
void file::rotate() {
  if (_size_limit > 0) {
    return;
  }

  _mutex->lock();
  time_t now = time(NULL);
  tm t;

  localtime_r(&now, &t);

  std::string old_name = qPrintable(_file->fileName());
  // XXX: todo.
  std::string new_name;/* = QString("%1/%2-%3-%4-%5-%6.log")
    .arg(_archive_path)
    .arg(old_name)
    .arg(t.tm_mon + 1, 2, 10, QLatin1Char('0'))
    .arg(t.tm_mday, 2, 10, QLatin1Char('0'))
    .arg(t.tm_year + 1900)
    .arg(t.tm_hour, 2, 10, QLatin1Char('0'));
                       */
    _file->close();
    if ((QFile::exists(new_name.c_str()) == false || QFile::remove(new_name.c_str()) == true)
	&& _file->rename(new_name.c_str()) == true) {
      _file->setFileName(old_name.c_str());
    }
    _file->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
  _mutex->unlock();
}

/**
 *  Write log into file.
 *
 *  @param[in] message   Message to log.
 *  @param[in] type      Logging types.
 *  @param[in] verbosity Verbosity level.
 */
void file::log(char const* message,
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
 *  Set the archive path.
 *
 *  @param[in] path The new archive path.
 */
void file::set_archive_path(std::string const& path) throw() {
  _mutex->lock();
  _archive_path = path;
  _mutex->unlock();
}

/**
 *  Get the archive path.
 *
 *  @return The archive path.
 */
std::string file::get_archive_path() const throw() {
  _mutex->lock();
  std::string path = _archive_path;
  _mutex->unlock();
  return (path);
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
