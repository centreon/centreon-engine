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

#include "commands/connector/request.hh"

using namespace com::centreon::engine::commands::connector;

request::request(e_type id)
  : _id(id) {

}

request::request(request const& right) {
  operator=(right);
}

request::~request() throw() {

}

request& request::operator=(request const& right) {
  if (this != &right) {
    _id = right._id;
  }
  return (*this);
}

bool request::operator==(request const& right) const throw() {
  return (_id == right._id);
}

bool request::operator!=(request const& right) const throw() {
  return (!operator==(right));
}

QByteArray const& request::cmd_ending() throw() {
  static QByteArray ending(4, '\0');
  return (ending);
}

request::e_type request::get_id() const throw() {
  return (_id);
}
