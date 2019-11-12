/*
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

#include <array>
#include "com/centreon/engine/retention/state.hh"

using namespace com::centreon::engine::retention;

/**
 *  Default constructor.
 */
state::state() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right The object to copy.
 */
state::state(state const& right) {
  operator=(right);
}

/**
 *  Destructor.
 */
state::~state() throw () {}

/**
 *  Copy operator.
 *
 *  @param[in] right The object to copy.
 *
 *  @return This object.
 */
state& state::operator=(state const& right) {
  if (this != &right) {
    _comments = right._comments;
    _contacts = right._contacts;
    _downtimes = right._downtimes;
    _globals = right._globals;
    _hosts = right._hosts;
    _info = right._info;
    _services = right._services;
  }
  return (*this);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool state::operator==(state const& right) const throw () {
  return (_comments == right._comments
          && _contacts == right._contacts
          && _downtimes == right._downtimes
          && _globals == right._globals
          && _hosts == right._hosts
          && _info == right._info
          && _services == right._services);
}

/**
 *  Not equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool state::operator!=(state const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Get comments.
 *
 *  @return The comment list.
 */
list_comment& state::comments() throw () {
  return (_comments);
}

/**
 *  Get comments.
 *
 *  @return The comment list.
 */
list_comment const& state::comments() const throw () {
  return (_comments);
}

/**
 *  Get contacts.
 *
 *  @return The contact list.
 */
list_contact& state::contacts() throw () {
  return (_contacts);
}

/**
 *  Get contacts.
 *
 *  @return The contact list.
 */
list_contact const& state::contacts() const throw () {
  return (_contacts);
}

/**
 *  Get downtimes.
 *
 *  @return The downtime list.
 */
list_downtime& state::downtimes() throw () {
  return (_downtimes);
}

/**
 *  Get downtimes.
 *
 *  @return The downtime list.
 */
list_downtime const& state::downtimes() const throw () {
  return (_downtimes);
}

/**
 *  Get globals.
 *
 *  @return The globals.
 */
program& state::globals() throw () {
  return (_globals);
}

/**
 *  Get globals.
 *
 *  @return The globals.
 */
program const& state::globals() const throw () {
  return (_globals);
}

/**
 *  Get hosts.
 *
 *  @return The host list.
 */
list_host& state::hosts() throw () {
  return (_hosts);
}

/**
 *  Get hosts.
 *
 *  @return The host list.
 */
list_host const& state::hosts() const throw () {
  return (_hosts);
}

/**
 *  Get informations.
 *
 *  @return The informations.
 */
info& state::informations() throw () {
  return (_info);
}

/**
 *  Get informations.
 *
 *  @return The informations.
 */
info const& state::informations() const throw () {
  return (_info);
}

/**
 *  Get services.
 *
 *  @return The service list.
 */
list_service& state::services() throw () {
  return (_services);
}

/**
 *  Get services.
 *
 *  @return The service list.
 */
list_service const& state::services() const throw () {
  return (_services);
}
