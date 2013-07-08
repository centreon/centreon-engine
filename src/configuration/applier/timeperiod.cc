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
 *  Add new time period.
 *
 *  @param[in] obj The new time period to add in the monitoring engine.
 */
void applier::timeperiod::add_object(
                            shared_ptr<configuration::timeperiod> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new time period '" << obj->timeperiod_name() << "'.";

  // Add time period to the global configuration set.
  config->timeperiods().insert(obj);

  // Create time period.
  timeperiod_struct* tp(add_timeperiod(
                          obj->timeperiod_name().c_str(),
                          NULL_IF_EMPTY(obj->alias())));
  if (!tp)
    throw (engine_error() << "Error: Could not register time period '"
           << obj->timeperiod_name() << "'.");

  // Fill time period structure.
  _fill_timeperiod_struct(*obj, tp);

  return ;
}

/**
 *  @brief Expand time period.
 *
 *  Time period objects do not need expansion. Therefore this method
 *  does nothing.
 *
 *  @param[in] obj Unused.
 *  @param[in] s   Unused.
 */
void applier::timeperiod::expand_object(
                            shared_ptr<configuration::timeperiod> obj,
                            configuration::state& s) {
  (void)obj;
  (void)s;
  return ;
}

/**
 *  Modify time period.
 *
 *  @param[in] obj The time period to modify in the monitoring engine.
 */
void applier::timeperiod::modify_object(
                            shared_ptr<configuration::timeperiod> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying time period '" << obj->timeperiod_name() << "'.";

  // Modify time period in global configuration set.
  // XXX



  return ;
}

/**
 *  Remove old time period.
 *
 *  @param[in] obj The time period to remove from the monitoring engine.
 */
void applier::timeperiod::remove_object(
                            shared_ptr<configuration::timeperiod> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing time period '" << obj->timeperiod_name() << "'.";

  // Unregister time period from the compatibility list.
  unregister_object<timeperiod_struct, &timeperiod_struct::name>(
    &timeperiod_list,
    obj->timeperiod_name().c_str());

  // Remove time period from the object list (will effectively deleted
  // the time periiod object).
  applier::state::instance().timeperiods().erase(obj->timeperiod_name());

  // Remove time period from the global configuration set.
  config->timeperiods().erase(obj);

  return ;
}

/**
 *  @brief Resolve a time period object.
 *
 *  This method does nothing because a time period object does not rely
 *  on any external object.
 *
 *  @param[in] obj Unused.
 */
void applier::timeperiod::resolve_object(
                            shared_ptr<configuration::timeperiod> obj) {
  (void)obj;
  return ;
}

/**
 *  Fill an already created time period struct from its configuration.
 *
 *  @param[in]  cfg Time period configuration.
 *  @param[out] obj Real time period object.
 */
void applier::timeperiod::_fill_timeperiod_struct(
                            configuration::timeperiod const& cfg,
                            timeperiod_struct* obj) {
  // Add time ranges to time period.
  {
    unsigned short day(0);
    for (std::vector<std::list<timerange> >::const_iterator
           it(cfg.timeranges().begin()),
           end(cfg.timeranges().end());
         it != end;
         ++it, ++day)
      for (std::list<timerange>::const_iterator
             it2(it->begin()),
             end2(it->end());
           it2 != end2;
           ++it2)
        if (!add_timerange_to_timeperiod(
               obj,
               day,
               it2->start(),
               it2->end()))
          throw (engine_error()
                 << "Error: Could not add time range to time period '"
                 << cfg.timeperiod_name() << "'.");
  }

  // Add exceptions to time period.
  for (std::vector<std::list<daterange> >::const_iterator
         it(cfg.exceptions().begin()),
         end(cfg.exceptions().end());
       it != end;
       ++it)
    for (std::list<daterange>::const_iterator
           it2(it->begin()),
           end2(it->end());
         it2 != end2;
         ++it2) {
      if (!add_exception_to_timeperiod(
             obj,
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
               << "Error: Could not add exception to time period '"
               << cfg.timeperiod_name() << "'.");
      for (std::list<timerange>::const_iterator
             it3(it2->timeranges().begin()),
             end3(it2->timeranges().end());
           it3 != end3;
           ++it3)
        if (!add_timerange_to_daterange(
               obj->exceptions[it2->type()],
               it3->start(),
               it3->end()))
          throw (engine_error()
                 << "Error: Could not add time range to date range of "
                 << "type " << it2->type() << " of time period '"
                 << cfg.timeperiod_name() << "'.");
    }

  // Add exclusions to time period.
  for (list_string::const_iterator
         it(cfg.exclude().begin()),
         end(cfg.exclude().end());
       it != end;
       ++it)
    if (!add_exclusion_to_timeperiod(
           obj,
           it->c_str()))
      throw (engine_error() << "Error: Could not add exclusion '"
             << *it << "' to time period '" << cfg.timeperiod_name()
             << "'.");

  return ;
}
