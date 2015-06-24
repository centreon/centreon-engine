/*
** Copyright 2011-2014 Merethis
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
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/timeperiod.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/daterange.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/deleter/timeperiodexclusion.hh"
#include "com/centreon/engine/deleter/timerange.hh"
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
    throw (engine_error() << "Could not register time period '"
           << obj->timeperiod_name() << "'");

  // Fill time period structure.
  _add_time_ranges(obj->timeranges(), tp);
  _add_exceptions(obj->exceptions(), tp);
  _add_exclusions(obj->exclude(), tp);

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

  // Find old configuration.
  set_timeperiod::iterator
    it_cfg(config->timeperiods_find(obj->key()));
  if (it_cfg == config->timeperiods().end())
    throw (engine_error() << "Could not modify non-existing "
           << "time period '" << obj->timeperiod_name() << "'");

  // Find time period object.
  umap<std::string, shared_ptr<timeperiod_struct> >::iterator
    it_obj(applier::state::instance().timeperiods_find(obj->key()));
  if (it_obj == applier::state::instance().timeperiods().end())
    throw (engine_error() << "Could not modify non-existing "
           << "time period object '" << obj->timeperiod_name() << "'");
  timeperiod_struct* tp(it_obj->second.get());

  // Update the global configuration set.
  shared_ptr<configuration::timeperiod> old_cfg(*it_cfg);
  config->timeperiods().erase(it_cfg);
  config->timeperiods().insert(obj);

  // Modify properties.
  modify_if_different(
    tp->alias,
    (obj->alias().empty() ? obj->timeperiod_name() : obj->alias()).c_str());

  // Time ranges modified ?
  if (obj->timeranges() != old_cfg->timeranges()) {
    // Delete old time ranges.
    for (unsigned int i(0);
         i < sizeof(tp->days) / sizeof(*tp->days);
         ++i)
      deleter::listmember(tp->days[i], &deleter::timerange);
    // Create new time ranges.
    _add_time_ranges(obj->timeranges(), tp);
  }

  // Exceptions modified ?
  if (obj->exceptions() != old_cfg->exceptions()) {
    // Delete old exceptions.
    for (unsigned int i(0);
         i < sizeof(tp->exceptions) / sizeof(*tp->exceptions);
         ++i)
      deleter::listmember(tp->exceptions[i], &deleter::daterange);
    // Create new exceptions.
    _add_exceptions(obj->exceptions(), tp);
  }

  // Exclusions modified ?
  if (obj->exclude() != old_cfg->exclude()) {
    // Delete old exclusions.
    deleter::listmember(tp->exclusions, &deleter::timeperiodexclusion);
    // Create new exclusions.
    _add_exclusions(obj->exclude(), tp);
  }

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_adaptive_timeperiod_data(
    NEBTYPE_TIMEPERIOD_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    tp,
    CMD_NONE,
    &tv);

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

  // Find time period.
  umap<std::string, shared_ptr<timeperiod_struct> >::iterator
    it(applier::state::instance().timeperiods_find(obj->key()));
  if (it != applier::state::instance().timeperiods().end()) {
    timeperiod_struct* tp(it->second.get());

    // Remove timeperiod from its list.
    unregister_object<timeperiod_struct>(&timeperiod_list, tp);

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_adaptive_timeperiod_data(
      NEBTYPE_TIMEPERIOD_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      tp,
      CMD_NONE,
      &tv);

    // Erase time period (will effectively delete the object).
    applier::state::instance().timeperiods().erase(it);
  }

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
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Resolving time period '" << obj->timeperiod_name() << "'.";

  // Find time period.
  umap<std::string, shared_ptr<timeperiod_struct> >::iterator
    it(applier::state::instance().timeperiods_find(obj->key()));
  if (applier::state::instance().timeperiods().end() == it)
    throw (engine_error() << "Cannot resolve non-existing "
           << "time period '" << obj->timeperiod_name() << "'");

  // Resolve time period.
  if (!check_timeperiod(it->second.get(), &config_warnings, &config_errors))
    throw (engine_error() << "Cannot resolve time period '"
           << obj->timeperiod_name() << "'");

  return ;
}

/**
 *  Add exclusions to a time period.
 *
 *  @param[in]  exclusions Exclusions.
 *  @param[out] tp         Time period object.
 */
void applier::timeperiod::_add_exclusions(
                            std::list<std::string> const& exclusions,
                            timeperiod_struct* tp) {
  for (list_string::const_iterator
         it(exclusions.begin()),
         end(exclusions.end());
       it != end;
       ++it)
    if (!add_exclusion_to_timeperiod(
           tp,
           it->c_str()))
      throw (engine_error() << "Could not add exclusion '"
             << *it << "' to time period '" << tp->name << "'");
  return ;
}

/**
 *  Add exceptions to a time period.
 *
 *  @param[in]  exceptions Exceptions.
 *  @param[out] tp         Time period object.
 */
void applier::timeperiod::_add_exceptions(
                            std::vector<std::list<configuration::daterange> > const& exceptions,
                            timeperiod_struct* tp) {
  for (std::vector<std::list<daterange> >::const_iterator
         it(exceptions.begin()),
         end(exceptions.end());
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
               << "Could not add exception to time period '"
               << tp->name << "'");
      for (std::list<timerange>::const_iterator
             it3(it2->timeranges().begin()),
             end3(it2->timeranges().end());
           it3 != end3;
           ++it3)
        if (!add_timerange_to_daterange(
               tp->exceptions[it2->type()],
               it3->start() / (60 * 60),
               (it3->start() / 60) % 60,
               it3->end() / (60 * 60),
               (it3->end() / 60) % 60))
          throw (engine_error()
                 << "Could not add time range to date range of "
                 << "type " << it2->type() << " of time period '"
                 << tp->name << "'");
    }
  return ;
}

/**
 *  Add time ranges to a time period.
 *
 *  @param[in]  ranges Time ranges.
 *  @param[out] tp     Time period object.
 */
void applier::timeperiod::_add_time_ranges(
                            std::vector<std::list<configuration::timerange> > const& ranges,
                             timeperiod_struct* tp) {
  unsigned short day(0);
  for (std::vector<std::list<timerange> >::const_iterator
         it(ranges.begin()),
         end(ranges.end());
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
             it2->start() / (60 * 60),
             (it2->start() / 60) % 60,
             it2->end() / (60 * 60),
             (it2->end() / 60) % 60))
        throw (engine_error()
               << "Could not add time range to time period '"
               << tp->name << "'");
  return ;
}
