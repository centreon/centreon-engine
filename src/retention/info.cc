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

#include "com/centreon/engine/retention/info.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine::retention;

#define SETTER(type, method) &object::setter<info, type, &info::method>::generic

info::setters const info::_setters[] = {
    {"created", SETTER(time_t, _set_created)},
    {"version", SETTER(std::string const&, _set_unused)},
    {"update_available", SETTER(std::string const&, _set_unused)},
    {"update_uid", SETTER(std::string const&, _set_unused)},
    {"last_version", SETTER(std::string const&, _set_unused)},
    {"new_version", SETTER(std::string const&, _set_unused)}};

/**
 *  Constructor.
 */
info::info() : object(object::info), _created(0) {}

/**
 *  Copy constructor.
 *
 *  @param[in] right The object to copy.
 */
info::info(info const& right) : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
info::~info() throw() {}

/**
 *  Copy operator.
 *
 *  @param[in] right The object to copy.
 *
 *  @return This object.
 */
info& info::operator=(info const& right) {
  if (this != &right) {
    object::operator=(right);
    _created = right._created;
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
bool info::operator==(info const& right) const throw() {
  return (object::operator==(right) && _created == right._created);
}

/**
 *  Not equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool info::operator!=(info const& right) const throw() {
  return (!operator==(right));
}

/**
 *  Set new value on specific property.
 *
 *  @param[in] key   The property to set.
 *  @param[in] value The new value.
 *
 *  @return True on success, otherwise false.
 */
bool info::set(char const* key, char const* value) {
  for (unsigned int i(0); i < sizeof(_setters) / sizeof(_setters[0]); ++i)
    if (!strcmp(_setters[i].name, key))
      return ((_setters[i].func)(*this, value));
  return (false);
}

/**
 *  Get created time.
 *
 *  @return The created time.
 */
time_t info::created() const throw() {
  return (_created);
}

/**
 *  Set created time.
 *
 *  @param[in] value The new created time.
 */
bool info::_set_created(time_t value) {
  _created = value;
  return (true);
}

/**
 *  Do nothing.
 *
 *  @param[in] value unused.
 */
bool info::_set_unused(std::string const& value) {
  (void)value;
  return (true);
}
