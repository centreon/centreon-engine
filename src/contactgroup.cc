/*
** Copyright 2011-2019 Centreon
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

#include <memory>
#include <sstream>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/contactgroup.hh"
#include "com/centreon/engine/contact.hh"
#include "com/centreon/engine/contactgroup.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

contactgroup_map contactgroup::contactgroups;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 * Constructor.
 */
contactgroup::contactgroup() {}

/**
 *  Constructor from a configuration contactgroup
 *
 * @param obj Configuration contactgroup
 */
contactgroup::contactgroup(configuration::contactgroup const& obj)
  : _alias(obj.alias().empty() ? obj.contactgroup_name() : obj.alias()),
    _name(obj.contactgroup_name()) {
  // Make sure we have the data we need.
  if (_name.empty())
    throw (engine_error() << "contactgroup: Contact group name is empty");

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_group(
    NEBTYPE_CONTACTGROUP_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    this,
    &tv);
}

/**
 * Assignment operator.
 *
 * @param[in] other Object to copy.
 *
 * @return This object
 */
contactgroup& contactgroup::operator=(contactgroup const& other) {
  _alias = other._alias;
  _members = other._members;
  _name = other._name;
  return *this;
}

/**
 * Destructor.
 */
contactgroup::~contactgroup() {
  _members.clear();
}

std::string const& contactgroup::get_name() const {
  return _name;
}

void contactgroup::add_member(contact* cntct) {
  _members.insert({cntct->get_name(), cntct});

  timeval tv(get_broker_timestamp(NULL));
  broker_group(
    NEBTYPE_CONTACTGROUP_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    this,
    &tv);
}

void contactgroup::clear_members() {
  _members.clear();
}

contact_map_unsafe const& contactgroup::get_members() const {
  return _members;
}

std::string const& contactgroup::get_alias() const {
  return _alias;
}

void contactgroup::set_alias(std::string const& alias) {
  _alias = alias;
}

std::ostream& operator<<(
                std::ostream& os,
                contactgroup_map const& obj)
{
  for (contactgroup_map::const_iterator
         it(obj.begin()), end(obj.end());
       it != end;
       ++it) {
    os << it->first;
    if (next(it) != end)
      os << ", ";
    else
      os << "";
  }
  return os;
}
