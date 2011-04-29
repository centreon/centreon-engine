/*
** Copyright 2011      Merethis
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
#include "error.hh"
#include "logging/file.hh"

using namespace com::centreon::engine::logging;

QList<file*> file::_files;

/**************************************
 *                                     *
 *           Public Methods            *
 *                                     *
 **************************************/

/**
 *  Default constructor.
 */
file::file(char const* file, char const* archive_path, long long size_limit)
  : _file(file), _archive_path(archive_path), _size_limit(size_limit) {
  _file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
  if (_file.QFile::error() != QFile::NoError) {
    throw (engine_error() << _file.errorString());
  }
  _files.push_back(this);
}

/**
 *  Default destructor.
 */
file::~file() throw() {
  _file.close();
  QList<file*>::iterator it = std::find(_files.begin(), _files.end(), this);
  if (it != _files.end()) {
    _files.erase(it);
  }
}

void file::rotate_all() {
  for (QList<file*>::iterator it = _files.begin(), end = _files.end();
       it != end;
       ++it) {
    (*it)->rotate();
  }
}

void file::rotate() {
  time_t now = time(NULL);
  tm t;

  localtime_r(&now, &t);

  QString old_name = _file.fileName();
  QString new_name = QString("%1/%2-%3-%4-%5-%6.log")
    .arg(_archive_path)
    .arg(old_name)
    .arg(t.tm_mon + 1, 2, 10, QLatin1Char('0'))
    .arg(t.tm_mday, 2, 10, QLatin1Char('0'))
    .arg(t.tm_year + 1900)
    .arg(t.tm_hour, 2, 10, QLatin1Char('0'));

    _file.close();
    QFile::remove(new_name);
    _file.rename(new_name);
    _file.setFileName(old_name);
    _file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
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

  _file.write(message);

  if (_file.size() >= _size_limit) {
    QString old_name = _file.fileName();
    QString new_name = old_name + ".old";

    _file.close();
    QFile::remove(new_name);
    _file.rename(new_name);

    _file.setFileName(old_name);
    _file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
  }
}

/**
 *  Set the archive path.
 *
 *  @param[in] path The new archive path.
 */
void file::set_archive_path(char const* path) throw() {
  _archive_path = path;
}

/**
 *  Get the archive path.
 *
 *  @return The archive path.
 */
QString const& file::get_archive_path() const throw() {
  return (_archive_path);
}

/**
 *  Set the new file's size limit.
 *
 *  @param[in] The size limit.
 */
void file::set_size_limit(long long size) throw() {
  _size_limit = (size > 0 ? size : 0);
}

/**
 *  Get the size limit.
 *
 *  @return The size limit.
 */
long long file::get_size_limit() const throw() {
  return (_size_limit);
}

