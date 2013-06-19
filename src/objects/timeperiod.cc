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

#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/timeperiod.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/misc/object.hh"
#include "com/centreon/engine/misc/string.hh"
#include "com/centreon/engine/objects/daterange.hh"
#include "com/centreon/engine/objects/timeperiod.hh"
#include "com/centreon/engine/objects/timeperiodexclusion.hh"
#include "com/centreon/engine/objects/timerange.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::misc;

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
     << "  exclusions: " << chkobj(os, obj.exclusions) << "\n";

  for (unsigned int i(0); i < sizeof(obj.days) / sizeof(obj.days[0]); ++i)
    os << get_weekday_name(i) << ": " << *obj.days[i] << "\n";

  for (unsigned int i(0); i < DATERANGE_TYPES; ++i)
    for (daterange const* it(obj.exceptions[i]); it; it = it->next)
      os << *it << "\n";
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

  // Allocate memory for the new timeperiod.
  shared_ptr<timeperiod> obj(new timeperiod, deleter::timeperiod);
  memset(obj.get(), 0, sizeof(*obj));

  try {
    // Copy string vars.
    obj->name = my_strdup(name);
    obj->alias = my_strdup(alias);

    // Add new timeperiod to the monitoring engine.
    std::string id(name);
    umap<std::string, shared_ptr<timeperiod_struct> >::const_iterator
      it(state::instance().timeperiods().find(id));
    if (it != state::instance().timeperiods().end()) {
      logger(log_config_error, basic)
        << "Error: Timeperiod '" << name << "' has already been defined";
      return (NULL);
    }

    // Add new items to the configuration state.
    state::instance().timeperiods()[id] = obj;

    // Add new items to the list.
    obj->next = timeperiod_list;
    timeperiod_list = obj.get();

    // Notify event broker.
    // XXX
  }
  catch (...) {
    obj.clear();
  }

  return (obj.get());
}
