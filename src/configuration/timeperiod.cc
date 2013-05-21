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

#include "com/centreon/engine/configuration/timeperiod.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/misc/string.hh"

using namespace com::centreon::engine::configuration;

#define SETTER(type, method) \
  &object::setter<timeperiod, type, &timeperiod::method>::generic

static struct {
  std::string const name;
  bool (*func)(timeperiod&, std::string const&);
} gl_setters[] = {
  { "alias",           SETTER(std::string const&, _set_alias) },
  { "exclude",         SETTER(std::string const&, _set_exclude) },
  { "timeperiod_name", SETTER(std::string const&, _set_timeperiod_name) }
};

/**
 *  Default constructor.
 */
timeperiod::timeperiod()
  : object("timeperiod") {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The timeperiod to copy.
 */
timeperiod::timeperiod(timeperiod const& right)
  : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
timeperiod::~timeperiod() throw () {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The timeperiod to copy.
 *
 *  @return This timeperiod.
 */
timeperiod& timeperiod::operator=(timeperiod const& right) {
  if (this != &right) {
    object::operator=(right);
    _alias = right._alias;
    _exclude = right._exclude;
    _timeperiod_name = right._timeperiod_name;
  }
  return (*this);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The timeperiod to compare.
 *
 *  @return True if is the same timeperiod, otherwise false.
 */
bool timeperiod::operator==(timeperiod const& right) const throw () {
  return (object::operator==(right)
          && _alias == right._alias
          && _exclude == right._exclude
          && _timeperiod_name == right._timeperiod_name);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The timeperiod to compare.
 *
 *  @return True if is not the same timeperiod, otherwise false.
 */
bool timeperiod::operator!=(timeperiod const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void timeperiod::merge(object const& obj) {
  if (obj.type() != _type)
    throw (engine_error() << "merge failed: invalid object type");
  timeperiod const& tmpl(static_cast<timeperiod const&>(obj));

  MRG_STRING(_alias);
  // XXX: MERGE_TAB(_exclude);
  MRG_STRING(_timeperiod_name);
}

/**
 *  Parse and set the timeperiod property.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The property value.
 *
 *  @return True on success, otherwise false.
 */
bool timeperiod::parse(
       std::string const& key,
       std::string const& value) {
  for (unsigned int i(0);
       i < sizeof(gl_setters) / sizeof(gl_setters[0]);
       ++i)
    if (gl_setters[i].name == key)
      return ((gl_setters[i].func)(*this, value));

  return (false);
}

/**
 *  Parse and set the timeperiod property.
 *
 *  @param[in] line  The configuration line.
 *
 *  @return True on success, otherwise false.
 */
bool timeperiod::parse(std::string const& line) {
  // XXX: timeperiod parser
  // parse(key, misc::trim(value));
  return (false);
}

bool timeperiod::_set_alias(std::string const& value) {
  _alias = value;
  return (true);
}

bool timeperiod::_set_exclude(std::string const& value) {
  _exclude.clear();
  misc::split(value, _exclude, ',');
  // XXX:
  return (false);
}

bool timeperiod::_set_timeperiod_name(std::string const& value) {
  _timeperiod_name = value;
  return (true);
}
