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

#include <cstdio>
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/configuration/timeperiod.hh"
#include "com/centreon/engine/configuration/timerange.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/misc/string.hh"
#include "com/centreon/hash.hh"

using namespace com::centreon;
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
  : object(object::timeperiod) {
  _exceptions.resize(DATERANGE_TYPES);
  _timeranges.resize(7);
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
 *  Less-than operator.
 *
 *  @param[in] right Object to compare to.
 *
 *  @return True if this object is less than right.
 */
bool timeperiod::operator<(timeperiod const& right) const throw () {
  if (_timeperiod_name != right._timeperiod_name)
    return (_timeperiod_name < right._timeperiod_name);
  else if (_alias != right._alias)
    return (_alias < right._alias);
  else if (_exclude != right._exclude)
    return (_exclude < right._exclude);
  else if (_timeranges != right._timeranges)
    return (_timeranges < right._timeranges);
  return (_exceptions < right._exceptions);
}

/**
 *  Get the unique object id.
 *
 *  @return The object id.
 */
std::size_t timeperiod::id() const throw () {
  return (_id);
}

/**
 *  Check if the object is valid.
 *
 *  @return True if is a valid object, otherwise false.
 */
void timeperiod::check_validity() const {
  if (_timeperiod_name.empty())
    throw (engine_error() << "configuration: invalid timeperiod "
           "property timeperiod_name is missing");
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

  MRG_DEFAULT(_alias);
  MRG_DEFAULT(_exceptions);
  MRG_DEFAULT(_exclude);
  MRG_DEFAULT(_timeperiod_name);
  MRG_DEFAULT(_timeranges);
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
  return (_add_week_day(key, value));
}

/**
 *  Parse and set the timeperiod property.
 *
 *  @param[in] line  The configuration line.
 *
 *  @return True on success, otherwise false.
 */
bool timeperiod::parse(std::string const& line) {
  std::size_t pos(line.find_first_of(" \t\r", 0));
  if (pos == std::string::npos)
    return (false);
  std::string key(line.substr(0, pos));
  std::string value(line.substr(pos + 1));
  misc::trim(value);

  if (object::parse(key, value)
      || parse(key, misc::trim(value))
      || _add_calendar_date(line)
      || _add_other_date(line))
    return (true);
  return (false);
}

/**
 *  Get alias value.
 *
 *  @return The alias value.
 */
std::string const& timeperiod::alias() const throw () {
  return (_alias);
}

/**
 *  Get exceptions value.
 *
 *  @return The exceptions value.
 */
std::vector<std::list<daterange> > const& timeperiod::exceptions() const throw () {
  return (_exceptions);
}

/**
 *  Get exclude value.
 *
 *  @return The exclude value.
 */
list_string const& timeperiod::exclude() const throw () {
  return (_exclude);
}

/**
 *  Get timeperiod_name value.
 *
 *  @return The timeperiod_name value.
 */
std::string const& timeperiod::timeperiod_name() const throw () {
  return (_timeperiod_name);
}

/**
 *  Get timeranges.
 *
 *  @return The timeranges list.
 */
std::vector<std::list<timerange> > const& timeperiod::timeranges() const throw () {
  return (_timeranges);
}

/**
 *  Build time_t from timerange configuration.
 *
 *  @param[in]  time_str The time to parse (format 00:00-12:00).
 *  @param[out] ret      The value to fill.
 *
 *  @return True on success, otherwise false.
 */
bool timeperiod::_build_time_t(
       std::string const& time_str,
       unsigned long& ret) {
  std::size_t pos(time_str.find(':'));
  if (pos == std::string::npos)
    return (false);
  unsigned long hours;
  if (!misc::to(time_str.substr(0, pos), hours))
    return (false);
  unsigned long minutes;
  if (!misc::to(time_str.substr(pos + 1), minutes))
    return (false);
  ret = hours * 3600 + minutes * 60;
  return (true);
}

/**
 *  Build timerange from new line.
 *
 *  @param[in]  line       The line to parse.
 *  @param[out] timeranges The list to fill.
 *
 *  @return True on success, otherwise false.
 */
bool timeperiod::_build_timeranges(
       std::string const& line,
       std::list<timerange>& timeranges) {
  list_string timeranges_str;
  misc::split(line, timeranges_str, ',');
  for (list_string::const_iterator
         it(timeranges_str.begin()),
         end(timeranges_str.end());
       it != end;
       ++it) {
    std::size_t pos(it->find('-'));
    if (pos == std::string::npos)
      return (false);
    unsigned long start_time;
    if (!_build_time_t(it->substr(0, pos), start_time))
      return (false);
    unsigned long end_time;
    if (!_build_time_t(it->substr(pos + 1), end_time))
      return (false);
    timeranges.push_back(timerange(start_time, end_time));
  }
  return (true);
}

/**
 *  Add a calendar date.
 *
 *  @param[in] line The line to parse.
 *
 *  @return True on success, otherwise false.
 */
bool timeperiod::_add_calendar_date(std::string const& line) {
  int ret(0);
  int pos(0);
  bool fill_missing(false);
  unsigned int month_start(0);
  unsigned int month_end(0);
  unsigned int month_day_start(0);
  unsigned int month_day_end(0);
  unsigned int year_start(0);
  unsigned int year_end(0);
  unsigned int skip_interval(0);

  if ((ret = sscanf(
               line.c_str(),
               "%4u-%2u-%2u - %4u-%2u-%2u / %u %n",
               &year_start,
               &month_start,
               &month_day_start,
               &year_end,
               &month_end,
               &month_day_end,
               &skip_interval,
               &pos)) == 7)
    fill_missing = false;
  else if ((ret = sscanf(
                    line.c_str(),
                    "%4u-%2u-%2u - %4u-%2u-%2u %n",
                    &year_start,
                    &month_start,
                    &month_day_start,
                    &year_end,
                    &month_end,
                    &month_day_end,
                    &pos)) == 6)
    fill_missing = false;
  else if ((ret = sscanf(
                    line.c_str(),
                    "%4u-%2u-%2u / %u %n",
                    &year_start,
                    &month_start,
                    &month_day_start,
                    &skip_interval,
                    &pos)) == 4)
    fill_missing = true;
  else if ((ret = sscanf(
                    line.c_str(),
                    "%4u-%2u-%2u %n",
                    &year_start,
                    &month_start,
                    &month_day_start,
                    &pos)) == 3)
    fill_missing = true;

  if (ret) {
    if (fill_missing) {
      year_end = year_start;
      month_end = month_start;
      month_day_end = month_day_start;
    }

    std::list<timerange> timeranges;
    if (!_build_timeranges(line.substr(pos), timeranges))
      return (false);

    daterange range(daterange::calendar_date);
    range.year_start(year_start);
    range.month_start(month_start - 1);
    range.month_day_start(month_day_start);
    range.year_end(year_end);
    range.month_end(month_end - 1);
    range.month_day_end(month_day_end);
    range.skip_interval(skip_interval);
    range.timeranges(timeranges);

    _exceptions[daterange::calendar_date].push_back(range);
    return (true);
  }
  return (false);
}

/**
 *  Add other date.
 *
 *  @param[in] line The line to parse.
 *
 *  @return True on success, otherwise false.
 */
bool timeperiod::_add_other_date(std::string const& line) {
  int pos(0);
  daterange::type_range type(daterange::none);
  unsigned int month_start(0);
  unsigned int month_end(0);
  unsigned int month_day_start(0);
  unsigned int month_day_end(0);
  unsigned int skip_interval(0);
  unsigned int week_day_start(0);
  unsigned int week_day_end(0);
  unsigned int week_day_start_offset(0);
  unsigned int week_day_end_offset(0);
  char buffer[4][4096];

  if (line.size() > 1024)
    return (false);

  if (sscanf(
        line.c_str(),
        "%[a-z] %u %[a-z] - %[a-z] %u %[a-z] / %u %n",
        buffer[0],
        &week_day_start_offset,
        buffer[1],
        buffer[2],
        &week_day_end_offset,
        buffer[3],
        &skip_interval,
        &pos) == 7) {
    // wednesday 1 january - thursday 2 july / 3
    if (_get_day_id(buffer[0], week_day_start)
        && _get_month_id(buffer[1], month_start)
        && _get_day_id(buffer[2], week_day_end)
        && _get_month_id(buffer[3], month_end))
      type = daterange::month_week_day;
  }
  else if (sscanf(
             line.c_str(),
             "%[a-z] %u - %[a-z] %u / %u %n",
             buffer[0],
             &month_day_start,
             buffer[1],
             &month_day_end,
             &skip_interval,
             &pos) == 5) {
    // monday 2 - thursday 3 / 2
    if (_get_day_id(buffer[0], week_day_start)
        && _get_day_id(buffer[1], week_day_end)) {
      week_day_start_offset = month_day_start;
      week_day_end_offset = month_day_end;
      type = daterange::week_day;
    }
    // february 1 - march 15 / 3
    else if (_get_month_id(buffer[0], month_start)
	     && _get_month_id(buffer[1], month_end))
      type = daterange::month_date;
    // day 4 - 6 / 2
    else if (!strcmp(buffer[0], "day")
             && !strcmp(buffer[1], "day"))
      type = daterange::month_day;
  }
  else if (sscanf(
             line.c_str(),
             "%[a-z] %u - %u / %u %n",
             buffer[0],
             &month_day_start,
             &month_day_end,
             &skip_interval,
             &pos) == 4) {
    // thursday 2 - 4
    if (_get_day_id(buffer[0], week_day_start)) {
      week_day_start_offset = month_day_start;
      week_day_end = week_day_start;
      week_day_end_offset = month_day_end;
      type = daterange::week_day;
    }
    // february 3 - 5
    else if (_get_month_id(buffer[0], month_start)) {
      month_end = month_start;
      type = daterange::month_date;
    }
    // day 1 - 4
    else if (!strcmp(buffer[0], "day"))
      type = daterange::month_day;
  }
  else if (sscanf(
             line.c_str(),
             "%[a-z] %u %[a-z] - %[a-z] %u %[a-z] %n",
             buffer[0],
             &week_day_start_offset,
             buffer[1],
             buffer[2],
             &week_day_end_offset,
             buffer[3],
             &pos) == 6) {
    // wednesday 1 january - thursday 2 july
    if (_get_day_id(buffer[0], week_day_start)
        && _get_month_id(buffer[1], month_start)
        && _get_day_id(buffer[2], week_day_end)
        && _get_month_id(buffer[3], month_end))
      type = daterange::month_week_day;
  }
  else if (sscanf(
             line.c_str(),
             "%[a-z] %u - %u %n",
             buffer[0],
             &month_day_start,
             &month_day_end,
             &pos) == 3) {
    // thursday 2 - 4
    if (_get_day_id(buffer[0], week_day_start)) {
      week_day_start_offset = month_day_start;
      week_day_end = week_day_start;
      week_day_end_offset = month_day_end;
      type = daterange::week_day;
    }
    // february 3 - 5
    else if (_get_month_id(buffer[0], month_start)) {
      month_end = month_start;
      type = daterange::month_date;
    }
    // day 1 - 4
    else if (!strcmp(buffer[0], "day"))
      type = daterange::month_day;
  }
  else if (sscanf(
             line.c_str(),
             "%[a-z] %u - %[a-z] %u %n",
             buffer[0],
             &month_day_start,
             buffer[1],
             &month_day_end,
             &pos) == 4) {
    // monday 2 - thursday 3
    if (_get_day_id(buffer[0], week_day_start)
        && _get_day_id(buffer[1], week_day_end)) {
      week_day_start_offset = month_day_start;
      week_day_end_offset = month_day_end;
      type = daterange::week_day;
    }
    // february 1 - march 15
    else if (_get_month_id(buffer[0], month_start)
	     && _get_month_id(buffer[1], month_end))
      type = daterange::month_date;
    // day 1 - day 5
    else if (!strcmp(buffer[0], "day")
             && !strcmp(buffer[1], "day"))
      type = daterange::month_day;
  }
  else if (sscanf(
             line.c_str(),
             "%[a-z] %u %[a-z] %n",
             buffer[0],
             &week_day_start_offset,
             buffer[1],
             &pos) == 3) {
    // thursday 3 february
    if (_get_day_id(buffer[0], week_day_start)
        && _get_month_id(buffer[1], month_start)) {
      month_end = month_start;
      week_day_end = week_day_start;
      week_day_end_offset = week_day_start_offset;
      type = daterange::month_week_day;
    }
  }
  else if (sscanf(
             line.c_str(),
             "%[a-z] %u %n",
             buffer[0],
             &month_day_start,
             &pos) == 2) {
    // thursday 2
    if (_get_day_id(buffer[0], week_day_start)) {
      week_day_start_offset = month_day_start;
      week_day_end = week_day_start;
      week_day_end_offset = week_day_start_offset;
      type = daterange::week_day;
    }
    // february 3
    else if (_get_month_id(buffer[0], month_start)) {
      month_end = month_start;
      month_day_end = month_day_start;
      type = daterange::month_date;
    }
    // day 1
    else if (!strcmp(buffer[0], "day")) {
      month_day_end = month_day_start;
      type = daterange::month_day;
    }
  }

  if (type != daterange::none) {
    daterange range(type);
    if (type == daterange::month_day) {
      range.month_day_start(month_day_start);
      range.month_day_end(month_day_end);
    }
    else if (type == daterange::month_week_day) {
      range.month_start(month_start);
      range.week_day_start(week_day_start);
      range.week_day_start_offset(week_day_start_offset);
      range.month_end(month_end);
      range.week_day_end(week_day_end);
      range.week_day_end_offset(week_day_end_offset);
    }
    else if (type == daterange::week_day) {
      range.week_day_start(week_day_start);
      range.week_day_start_offset(week_day_start_offset);
      range.week_day_end(week_day_end);
      range.week_day_end_offset(week_day_end_offset);
    }
    else if (type == daterange::month_date) {
      range.month_start(month_start);
      range.month_day_start(month_day_start);
      range.month_end(month_end);
      range.month_day_end(month_day_end);
    }
    range.skip_interval(skip_interval);

    std::list<timerange> timeranges;
    if (!_build_timeranges(line.substr(pos), timeranges))
      return (false);

    range.timeranges(timeranges);
    _exceptions[daterange::calendar_date].push_back(range);
    return (true);
  }

  return (false);
}

/**
 *  Add a week day.
 *
 *  @param[in] key   The week day.
 *  @param[in] value The range.
 *
 *  @return True on success, otherwise false.
 */
bool timeperiod::_add_week_day(
       std::string const& key,
       std::string const& value) {
  unsigned int day_id;
  if (!_get_day_id(key, day_id))
    return (false);

  if (!_build_timeranges(value, _timeranges[day_id]))
    return (false);

  return (true);
}

/**
 *  Get the month id.
 *
 *  @param[in]  name The month name.
 *  @param[out] id   The id to fill.
 *
 *  @return True on success, otherwise false.
 */
bool timeperiod::_get_month_id(
       std::string const& name,
       unsigned int& id) {
  static std::string const months[] = {
    "january",
    "february",
    "march",
    "april",
    "may",
    "june",
    "july",
    "august",
    "september",
    "october",
    "november",
    "december"
  };
  for (id = 0; id < sizeof(months) / sizeof(months[0]); ++id)
    if (name == months[id])
      return (true);
  return (false);
}

/**
 *  Get the week day id.
 *
 *  @param[in]  name The week day name.
 *  @param[out] id   The id to fill.
 *
 *  @return True on success, otherwise false.
 */
bool timeperiod::_get_day_id(
       std::string const& name,
       unsigned int& id) {
  static std::string const days[] = {
    "sunday",
    "monday",
    "tuesday",
    "wednesday",
    "thursday",
    "friday",
    "saturday"
  };
  for (id = 0; id < sizeof(days) / sizeof(days[0]); ++id)
    if (name == days[id])
      return (true);
  return (false);
}

/**
 *  Set alias value.
 *
 *  @param[in] value The new alias value.
 *
 *  @return True on success, otherwise false.
 */
bool timeperiod::_set_alias(std::string const& value) {
  _alias = value;
  return (true);
}

/**
 *  Set exclude value.
 *
 *  @param[in] value The new exclude value.
 *
 *  @return True on success, otherwise false.
 */
bool timeperiod::_set_exclude(std::string const& value) {
  _exclude.clear();
  misc::split(value, _exclude, ',');
  return (true);
}

/**
 *  Set timeperiod_name value.
 *
 *  @param[in] value The new timeperiod_name value.
 *
 *  @return True on success, otherwise false.
 */
bool timeperiod::_set_timeperiod_name(std::string const& value) {
  _timeperiod_name = value;
  _id = hash(value);
  return (true);
}
