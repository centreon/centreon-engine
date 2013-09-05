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

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/timeperiod.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/daterange.hh"
#include "com/centreon/engine/objects/timeperiod.hh"
#include "com/centreon/engine/objects/timeperiodexclusion.hh"
#include "com/centreon/engine/objects/timerange.hh"
#include "com/centreon/engine/objects/tool.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::string;

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool operator==(
       timeperiod const& obj1,
       timeperiod const& obj2) throw () {
  if (is_equal(obj1.name, obj2.name)
      && is_equal(obj1.alias, obj2.alias)
      && is_equal(obj1.exclusions, obj2.exclusions)) {
    for (unsigned int i(0); i < DATERANGE_TYPES; ++i)
      if (!is_equal(obj1.exceptions[i], obj2.exceptions[i]))
        return (false);
    for (unsigned int i(0);
         i < sizeof(obj1.days) / sizeof(obj1.days[0]);
         ++i)
      if (!is_equal(obj1.days[i], obj2.days[i]))
        return (false);
    return (true);
  }
  return (false);
}

/**
 *  Not equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool operator!=(
       timeperiod const& obj1,
       timeperiod const& obj2) throw () {
  return (!operator==(obj1, obj2));
}

/**
 *  Dump timeperiod content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The timeperiod to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, timeperiod const& obj) {
  os << "timeperiod {\n"
     << "  name:  " << chkstr(obj.name) << "\n"
     << "  alias: " << chkstr(obj.alias) << "\n"
     << "  exclusions: " << chkobj(obj.exclusions) << "\n";

  for (unsigned int i(0); i < sizeof(obj.days) / sizeof(obj.days[0]); ++i)
    if (obj.days[i])
      os << "  " << get_weekday_name(i) << ": " << *obj.days[i] << "\n";

  for (unsigned int i(0); i < DATERANGE_TYPES; ++i)
    for (daterange const* it(obj.exceptions[i]); it; it = it->next)
      if (it)
        os << "  " << *it << "\n";
  os << "}\n";
  return (os);
}

/**
 *  Add a new timeperiod to the list in memory.
 *
 *  @param[in] name  Time period name.
 *  @param[in] alias Time period alias.
 *
 *  @return New timeperiod object.
 */
timeperiod* add_timeperiod(char const* name, char const* alias) {
  // Make sure we have the data we need.
  if (!name || !name[0] || !alias || !alias[0]) {
    logger(log_config_error, basic)
      << "Error: Name or alias for timeperiod is NULL";
    return (NULL);
  }

  // Check if the timeperiod already exist.
  std::string id(name);
  if (is_timeperiod_exist(id)) {
    logger(log_config_error, basic)
      << "Error: Timeperiod '" << name << "' has already been defined";
    return (NULL);
  }


  // Allocate memory for the new timeperiod.
  shared_ptr<timeperiod> obj(new timeperiod, deleter::timeperiod);
  memset(obj.get(), 0, sizeof(*obj));

  try {
    // Copy string vars.
    obj->name = string::dup(name);
    obj->alias = string::dup(alias);

    // Add new items to the configuration state.
    state::instance().timeperiods()[id] = obj;

    // Add new items to the list.
    obj->next = timeperiod_list;
    timeperiod_list = obj.get();

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_adaptive_timeperiod_data(
      NEBTYPE_TIMEPERIOD_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      obj.get(),
      CMD_NONE,
      &tv);
  }
  catch (...) {
    obj.clear();
  }

  return (obj.get());
}

/**
 *  Get timeperiod by name.
 *
 *  @param[in] name The timeperiod name.
 *
 *  @return The struct timeperiod or throw exception if the
 *          timeperiod is not found.
 */
timeperiod& engine::find_timperiod(std::string const& name) {
  umap<std::string, shared_ptr<timeperiod_struct> >::const_iterator
    it(state::instance().timeperiods().find(name));
  if (it == state::instance().timeperiods().end())
    throw (engine_error() << "Time period '"
           << name << " was not found");
  return (*it->second);
}

/**
 *  Get if timeperiod exist.
 *
 *  @param[in] name The timeperiod name.
 *
 *  @return True if the timeperiod is found, otherwise false.
 */
bool engine::is_timeperiod_exist(std::string const& name) throw () {
  umap<std::string, shared_ptr<timeperiod_struct> >::const_iterator
    it(state::instance().timeperiods().find(name));
  return (it != state::instance().timeperiods().end());
}
