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

#include <QStringList>
#include "error.hh"
#include "commands/connector/version_response.hh"

using namespace com::centreon::engine::commands::connector;

version_response::version_response(unsigned int major, unsigned int minor)
  : request(request::version_r), _major(major), _minor(minor) {

}

version_response::version_response(version_response const& right)
  : request(right) {
  operator=(right);
}

version_response::~version_response() throw() {

}

version_response& version_response::operator=(version_response const& right) {
  if (this != &right) {
    request::operator=(right);
    _major = right._major;
    _minor = right._minor;
  }
  return (*this);
}

bool version_response::operator==(version_response const& right) const throw() {
  return (request::operator==(right) == true
	  && _major == right._major
	  && _minor == right._minor);
}

bool version_response::operator!=(version_response const& right) const throw() {
  return (!operator==(right));
}

request* version_response::clone() const {
  return (new version_response(*this));
}

QByteArray version_response::build() {
  return (QByteArray().setNum(_id) + '\0' +
	  QByteArray().setNum(_major) + '\0' +
	  QByteArray().setNum(_minor) +
	  cmd_ending());
}

void version_response::restore(QByteArray const& data) {
  QList<QByteArray> list = data.split('\0');
  if (list.size() != 3) {
    throw (engine_error() << "bad request argument.");
  }

  bool ok;
  unsigned int id = list[0].toUInt(&ok);
  if (ok == false || id != _id) {
    throw (engine_error() << "bad request id.");
  }

  _major = list[1].toUInt(&ok);
  if (ok == false) {
    throw (engine_error() << "bad request argument, invalid major.");
  }

  _minor = list[2].toUInt(&ok);
  if (ok == false) {
    throw (engine_error() << "bad request argument, invalid minor.");
  }
}

unsigned int version_response::get_major() const throw() {
  return (_major);
}

unsigned int version_response::get_minor() const throw() {
  return (_minor);
}
