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

#include "com/centreon/engine/configuration/applier/timeperiod.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
applier::timeperiod::timeperiod() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::timeperiod::timeperiod(applier::timeperiod const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::timeperiod::~timeperiod() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 */
applier::timeperiod& applier::timeperiod::operator=(
                       applier::timeperiod const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new timeperiod.
 *
 *  @param[in] obj The new timeperiod to add into the monitoring engine.
 *  @param[in] s   Configuration being applied.
 */
void applier::timeperiod::add_object(
                            configuration::timeperiod const& obj,
                            configuration::state const& s) {
  (void)s;

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new timeperiod '" << obj.timeperiod_name() << "'.";

  // Create timeperiod.
  timeperiod_struct* tp(add_timeperiod(
                          obj.timeperiod_name().c_str(),
                          NULL_IF_EMPTY(obj.alias())));
  if (!tp)
    throw (engine_error() << "Error: Could not register timeperiod '"
           << obj.timeperiod_name() << "'.");

  // Add timeranges to timeperiod.
  {
    unsigned short day(0);
    for (std::vector<std::list<timerange> >::const_iterator
           it(obj.timeranges().begin()),
           end(obj.timeranges().end());
         it != end;
         ++it, ++day)
      for (std::list<timerange>::const_iterator
             it2(it->begin()),
             end2(it->end());
           it2 != end2;
           ++it2)
        if (!add_timerange_to_timeperiod(
               tp,
               day,
               it2->start(),
               it2->end()))
          throw (engine_error()
                 << "Error: Could not add timerange to timeperiod '"
                 << obj.timeperiod_name() << "'.");
  }

  // Add exceptions to timeperiod.
  for (std::vector<std::list<daterange> >::const_iterator
         it(obj.exceptions().begin()),
         end(obj.exceptions().end());
       it != end;
       ++it)
    for (std::list<daterange>::const_iterator
           it2(it->begin()),
           end2(it->end());
         it2 != end2;
         ++it2) {
      if (!add_exception_to_timeperiod(
             tp,
             it2->type(),
             it2->year_start(),
             it2->month_start(),
             it2->month_day_start(),
             it2->week_day_start(),
             it2->week_day_start_offset(),
             it2->year_end(),
             it2->month_end(),
             it2->month_day_end(),
             it2->week_day_end(),
             it2->week_day_end_offset(),
             it2->skip_interval()))
        throw (engine_error()
               << "Error: Could not add exception to timeperiod '"
               << obj.timeperiod_name() << "'.");
      for (std::list<timerange>::const_iterator
             it3(it2->timeranges().begin()),
             end3(it2->timeranges().end());
           it3 != end3;
           ++it3)
        if (!add_timerange_to_daterange(
               tp->exceptions[it2->type()],
               it3->start(),
               it3->end()))
          throw (engine_error()
                 << "Error: Could not add timerange to daterange of type "
                 << it2->type() << " of timeperiod '"
                 << obj.timeperiod_name() << "'.");
    }

  // Add exclusions to timeperiod.
  for (list_string::const_iterator
         it(obj.exclude().begin()),
         end(obj.exclude().end());
       it != end;
       ++it)
    if (!add_exclusion_to_timeperiod(
           tp,
           it->c_str()))
      throw (engine_error() << "Error: Could not add exclusion '"
             << *it << "' to timeperiod '" << obj.timeperiod_name()
             << "'.");

  return ;
}

/**
 *  Modified timeperiod.
 *
 *  @param[in] obj The new timeperiod to modify into the monitoring engine.
 *  @param[in] s   Configuration being applied.
 */
void applier::timeperiod::modify_object(
                            configuration::timeperiod const& obj,
                            configuration::state const& s) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying timeperiod '" << obj.timeperiod_name() << "'.";

  // XXX

  return ;
}

/**
 *  Remove old timeperiod.
 *
 *  @param[in] obj The new timeperiod to remove from the monitoring engine.
 *  @param[in] s   Configuration being applied.
 */
void applier::timeperiod::remove_object(
                            configuration::timeperiod const& obj,
                            configuration::state const& s) {
  (void)s;

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing timeperiod '" << obj.timeperiod_name() << "'.";

  // Unregister timeperiod.
  unregister_object<timeperiod_struct, &timeperiod_struct::name>(
    &timeperiod_list,
    obj.timeperiod_name().c_str());
  applier::state::instance().timeperiods().erase(obj.timeperiod_name());

  return ;
}

/**
 *  Resolve a timeperiod.
 *
 *  @param[in,out] obj Timeperiod object.
 *  @param[in] s       Configuration being applied.
 */
void applier::timeperiod::resolve_object(
                            configuration::timeperiod const& obj,
                            configuration::state const& s) {
  (void)s;

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Resolving timeperiod '" << obj.timeperiod_name() << "'.";

  return ;
}
