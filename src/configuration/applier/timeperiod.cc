/*
** Copyright 2011-2013,2017 Centreon
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
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/exceptions/error.hh"
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
applier::timeperiod::~timeperiod() throw() {}

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
 *  @param[in] obj  The new time period to add in the monitoring engine.
 */
void applier::timeperiod::add_object(configuration::timeperiod const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
      << "Creating new time period '" << obj.timeperiod_name() << "'.";

  // Add time period to the global configuration set.
  config->timeperiods().insert(obj);

  // Create time period.
  std::shared_ptr<engine::timeperiod> tp{
      new engine::timeperiod(obj.timeperiod_name(), obj.alias())};

  engine::timeperiod::timeperiods.insert({obj.timeperiod_name(), tp});

  // Notify event broker.
  timeval tv(get_broker_timestamp(nullptr));
  broker_adaptive_timeperiod_data(NEBTYPE_TIMEPERIOD_ADD, NEBFLAG_NONE,
                                  NEBATTR_NONE, tp.get(), CMD_NONE, &tv);

  // Fill time period structure.
  _add_time_ranges(obj.timeranges(), tp.get());
  _add_exceptions(obj.exceptions(), tp.get());
  _add_exclusions(obj.exclude(), tp.get());
}

/**
 *  @brief Expand time period.
 *
 *  Time period objects do not need expansion. Therefore this method
 *  does nothing.
 *
 *  @param[in] s  Unused.
 */
void applier::timeperiod::expand_objects(configuration::state& s) {
  (void)s;
}

/**
 *  Modify time period.
 *
 *  @param[in] obj  The time period to modify in the monitoring engine.
 */
void applier::timeperiod::modify_object(configuration::timeperiod const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
      << "Modifying time period '" << obj.timeperiod_name() << "'.";

  // Find old configuration.
  set_timeperiod::iterator it_cfg(config->timeperiods_find(obj.key()));
  if (it_cfg == config->timeperiods().end())
    throw(engine_error() << "Could not modify non-existing "
                         << "time period '" << obj.timeperiod_name() << "'");

  // Find time period object.
  timeperiod_map::iterator it_obj(
      engine::timeperiod::timeperiods.find(obj.key()));
  if (it_obj == engine::timeperiod::timeperiods.end() || !it_obj->second)
    throw(engine_error() << "Could not modify non-existing "
                         << "time period object '" << obj.timeperiod_name()
                         << "'");
  engine::timeperiod* tp(it_obj->second.get());

  // Update the global configuration set.
  configuration::timeperiod old_cfg(*it_cfg);
  config->timeperiods().erase(it_cfg);
  config->timeperiods().insert(obj);

  // Modify properties.
  if (obj.alias() != tp->get_alias())
    tp->set_alias(obj.alias().empty() ? obj.timeperiod_name() : obj.alias());

  // Time ranges modified ?
  if (obj.timeranges() != old_cfg.timeranges()) {
    // Delete old time ranges.
    for (unsigned int i(0); i < tp->days.size(); ++i)
      tp->days[i].clear();
    // Create new time ranges.
    _add_time_ranges(obj.timeranges(), tp);
  }

  // Exceptions modified ?
  if (obj.exceptions() != old_cfg.exceptions()) {
    // Delete old exceptions.
    for (unsigned int i(0); i < tp->exceptions.size(); ++i)
      tp->exceptions[i].clear();

    // Create new exceptions.
    _add_exceptions(obj.exceptions(), tp);
  }

  // Exclusions modified ?
  if (obj.exclude() != old_cfg.exclude()) {
    // Delete old exclusions.
    tp->get_exclusions().clear();
    // Create new exclusions.
    _add_exclusions(obj.exclude(), tp);
  }

  // Notify event broker.
  timeval tv(get_broker_timestamp(nullptr));
  broker_adaptive_timeperiod_data(NEBTYPE_TIMEPERIOD_UPDATE, NEBFLAG_NONE,
                                  NEBATTR_NONE, tp, CMD_NONE, &tv);
}

/**
 *  Remove old time period.
 *
 *  @param[in] obj  The time period to remove from the monitoring engine.
 */
void applier::timeperiod::remove_object(configuration::timeperiod const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
      << "Removing time period '" << obj.timeperiod_name() << "'.";

  // Find time period.
  timeperiod_map::iterator it(engine::timeperiod::timeperiods.find(obj.key()));
  if (it != engine::timeperiod::timeperiods.end() && it->second) {
    // Notify event broker.
    timeval tv(get_broker_timestamp(nullptr));
    broker_adaptive_timeperiod_data(NEBTYPE_TIMEPERIOD_DELETE, NEBFLAG_NONE,
                                    NEBATTR_NONE, it->second.get(), CMD_NONE,
                                    &tv);

    // Erase time period (will effectively delete the object).
    engine::timeperiod::timeperiods.erase(it);
  }

  // Remove time period from the global configuration set.
  config->timeperiods().erase(obj);
}

/**
 *  @brief Resolve a time period object.
 *
 *  This method does nothing because a time period object does not rely
 *  on any external object.
 *
 *  @param[in] obj Unused.
 */
void applier::timeperiod::resolve_object(configuration::timeperiod const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
      << "Resolving time period '" << obj.timeperiod_name() << "'.";

  // Find time period.
  timeperiod_map::iterator it{engine::timeperiod::timeperiods.find(obj.key())};
  if (engine::timeperiod::timeperiods.end() == it || !it->second)
    throw engine_error() << "Cannot resolve non-existing "
                         << "time period '" << obj.timeperiod_name() << "'";

  // Resolve time period.
  it->second->resolve(config_warnings, config_errors);
}

/**
 *  Add exclusions to a time period.
 *
 *  @param[in]  exclusions Exclusions.
 *  @param[out] tp         Time period object.
 */
void applier::timeperiod::_add_exclusions(
    std::set<std::string> const& exclusions,
    engine::timeperiod* tp) {
  for (set_string::const_iterator it(exclusions.begin()), end(exclusions.end());
       it != end; ++it)
    tp->get_exclusions().insert({*it, nullptr});
}

/**
 *  Add exceptions to a time period.
 *
 *  @param[in]  exceptions Exceptions.
 *  @param[out] tp         Time period object.
 */
void applier::timeperiod::_add_exceptions(
    std::vector<std::list<configuration::daterange> > const& exceptions,
    engine::timeperiod* tp) {
  for (std::vector<std::list<daterange> >::const_iterator
           it(exceptions.begin()),
       end(exceptions.end());
       it != end; ++it)
    for (std::list<daterange>::const_iterator it2(it->begin()), end2(it->end());
         it2 != end2; ++it2) {
      std::shared_ptr<engine::daterange> dr{new engine::daterange(
          it2->type(), it2->year_start(), it2->month_start(),
          it2->month_day_start(), it2->week_day_start(),
          it2->week_day_start_offset(), it2->year_end(), it2->month_end(),
          it2->month_day_end(), it2->week_day_end(), it2->week_day_end_offset(),
          it2->skip_interval())};
      tp->exceptions[it2->type()].push_back(dr);
      for (std::list<timerange>::const_iterator it3(it2->timeranges().begin()),
           end3(it2->timeranges().end());
           it3 != end3; ++it3) {
        std::shared_ptr<engine::timerange> tr{
            new engine::timerange(it3->start(), it3->end())};
        for (daterange_list::iterator it4(tp->exceptions[it2->type()].begin()),
             end4(tp->exceptions[it2->type()].end());
             it4 != end4; ++it4)
          (*it4)->times.push_back(tr);
      }
    }
}

/**
 *  Add time ranges to a time period.
 *
 *  @param[in]  ranges Time ranges.
 *  @param[out] tp     Time period object.
 */
void applier::timeperiod::_add_time_ranges(
    std::vector<std::list<configuration::timerange> > const& ranges,
    engine::timeperiod* tp) {
  unsigned short day(0);
  for (std::vector<std::list<timerange> >::const_iterator it(ranges.begin()),
       end(ranges.end());
       it != end; ++it, ++day)
    for (std::list<timerange>::const_iterator it2(it->begin()), end2(it->end());
         it2 != end2; ++it2) {
      std::shared_ptr<engine::timerange> tr{
          new engine::timerange(it2->start(), it2->end())};
      tp->days[day].push_back(tr);
    }
}
