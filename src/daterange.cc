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

#include <array>
#include <iomanip>
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/daterange.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/engine/timeperiod.hh"
#include "com/centreon/engine/timerange.hh"

using namespace com::centreon::engine;

/**
 *  Create a new exception to a timeperiod.
 *
 *  @param[in] period        Base period.
 *  @param[in] type
 *  @param[in] syear
 *  @param[in] smon
 *  @param[in] smday
 *  @param[in] swday
 *  @param[in] swday_offset
 *  @param[in] eyear
 *  @param[in] emon
 *  @param[in] emday
 *  @param[in] ewday
 *  @param[in] ewday_offset
 *  @param[in] skip_interval
 */
daterange::daterange(int type,
                     int syear,
                     int smon,
                     int smday,
                     int swday,
                     int swday_offset,
                     int eyear,
                     int emon,
                     int emday,
                     int ewday,
                     int ewday_offset,
                     int skip_interval)
  : _type{type},
    _syear{syear},
    _smon{smon},
    _smday{smday},
    _swday{swday},
    _swday_offset{swday_offset},
    _eyear{eyear},
    _emon{emon},
    _emday{emday},
    _ewday{ewday},
    _ewday_offset{ewday_offset},
    _skip_interval{skip_interval} {}

int daterange::get_type() const {
  return _type;
}

void daterange::set_type(int type) {
  _type = type;
}

int daterange::get_syear() const {
  return _syear;
}

void daterange::set_syear(int syear) {
  _syear = syear;
}

int daterange::get_smon() const {
  return _smon;
}

void daterange::set_smon(int smon) {
  _smon = smon;
}

int daterange::get_smday() const {
  return _smday;
}

void daterange::set_smday(int smday) {
  _smday = smday;
}

int daterange::get_swday() const {
  return _swday;
}

void daterange::set_swday(int swday) {
  _swday = swday;
}

int daterange::get_swday_offset() const {
  return _swday_offset;
}

void daterange::set_swday_offset(int swday_offset) {
  _swday_offset = swday_offset;
}

int daterange::get_eyear() const {
  return _eyear;
}

void daterange::set_eyear(int eyear) {
  _eyear = eyear;
}

int daterange::get_emon() const {
  return _emon;
}

void daterange::set_emon(int emon) {
  _emon = emon;
}

int daterange::get_emday() const {
  return _emday;
}

void daterange::set_emday(int emday) {
  _emday = emday;
}

int daterange::get_ewday() const {
  return _ewday;
}

void daterange::set_ewday(int ewday) {
  _ewday = ewday;
}

int daterange::get_ewday_offset() const {
  return _ewday_offset;
}

void daterange::set_ewday_offset(int ewday_offset) {
  _ewday_offset = ewday_offset;
}

int daterange::get_skip_interval() const {
  return _skip_interval;
}

void daterange::set_skip_interval(int skip_interval) {
  _skip_interval = skip_interval;
}

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool daterange::operator==(daterange const& obj) throw () {
  return _type == obj.get_type()
      && _syear == obj.get_syear()
      && _smon == obj.get_smon()
      && _smday == obj.get_smday()
      && _swday == obj.get_swday()
      && _swday_offset == obj.get_swday_offset()
      && _eyear == obj.get_eyear()
      && _emon == obj.get_emon()
      && _emday == obj.get_emday()
      && _ewday == obj.get_ewday()
      && _ewday_offset == obj.get_ewday_offset()
      && _skip_interval == obj.get_skip_interval()
      && times == obj.times;
}

/**
 *  Not equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool daterange::operator!=(daterange const& obj) throw () {
  return !(*this == obj);
}

/**
 *  Dump the daterange value into the calendar date format.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The daterange to dump.
 *
 *  @return The output stream.
 */
static std::ostream& _dump_calendar_date(std::ostream& os, daterange const& obj) {
  os << std::setfill('0') << std::setw(2) << obj.get_syear() << "-"
     << std::setfill('0') << std::setw(2) << obj.get_smon() + 1 << "-"
     << std::setfill('0') << std::setw(2) << obj.get_smday();
  if (obj.get_syear() != obj.get_eyear()
      || obj.get_smon() != obj.get_emon()
      || obj.get_smday() != obj.get_emday())
    os << " - "
       << std::setfill('0') << std::setw(2) << obj.get_eyear() << "-"
       << std::setfill('0') << std::setw(2) << obj.get_emon() + 1 << "-"
       << std::setfill('0') << std::setw(2) << obj.get_emday();
  if (obj.get_skip_interval())
    os << " / " << obj.get_skip_interval();
  return (os);
}

/**
 *  Dump the daterange value into the month date format.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The daterange to stringify.
 *
 *  @return The output stream.
 */
static std::ostream& _dump_month_date(std::ostream& os, daterange const& obj) {
  std::string const& smon(daterange::get_month_name(obj.get_smon()));
  std::string const& emon(daterange::get_month_name(obj.get_emon()));
  os << smon << " " << obj.get_smday();
  if (smon != emon)
    os << " - " << emon << " " << obj.get_emday();
  else if (obj.get_smday() != obj.get_emday())
    os << " - " << obj.get_emday();
  if (obj.get_skip_interval())
    os << " / " << obj.get_skip_interval();
  return (os);
}

/**
 *  Dump the daterange value into the month day format.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The daterange to stringify.
 *
 *  @return The output stream.
 */
static std::ostream& _dump_month_day(std::ostream& os, daterange const& obj) {
  os << "day " << obj.get_smday();
  if (obj.get_smday() != obj.get_emday())
    os << " - " << obj.get_emday();
  if (obj.get_skip_interval())
    os << " / " << obj.get_skip_interval();
  return (os);
}

/**
 *  Dump the daterange value into the month week day
 *  format.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The daterange to stringify.
 *
 *  @return The output stream.
 */
static std::ostream& _dump_month_week_day(std::ostream& os, daterange const& obj) {
  os << daterange::get_weekday_name(obj.get_swday()) << " " << obj.get_swday_offset() << " "
     << daterange::get_month_name(obj.get_smon());
  if (obj.get_swday() != obj.get_ewday()
      || obj.get_swday_offset() != obj.get_ewday_offset()
      || obj.get_smon() != obj.get_emon())
    os << " - " << daterange::get_weekday_name(obj.get_ewday())
       << " " << obj.get_ewday_offset() << " " << daterange::get_month_name(obj.get_emon());
  if (obj.get_skip_interval())
    os << " / " << obj.get_skip_interval();
  return (os);
}

/**
 *  Dump the daterange value into the week day format.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The daterange to stringify.
 *
 *  @return The output stream.
 */
static std::ostream& _dump_week_day(std::ostream& os, daterange const& obj) {
  os << daterange::get_weekday_name(obj.get_swday()) << " " << obj.get_swday_offset();
  if (obj.get_swday() != obj.get_ewday() || obj.get_swday_offset() != obj.get_ewday_offset())
    os << " - " << daterange::get_weekday_name(obj.get_ewday()) << " " << obj.get_ewday_offset();
  if (obj.get_skip_interval())
    os << " / " << obj.get_skip_interval();
  return (os);
}

/**
 *  Dump daterange content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The daterange to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, daterange const& obj) {
  typedef std::ostream& (*func)(std::ostream&, daterange const&);
  static func tab[] = {
    &_dump_calendar_date,
    &_dump_month_date,
    &_dump_month_day,
    &_dump_month_week_day,
    &_dump_week_day,
  };

  if (obj.get_type() < 0 || obj.get_type() >= DATERANGE_TYPES)
    os << "unknown type " << obj.get_type();
  else {
    (*(tab[obj.get_type()]))(os, obj);
    os << " " << obj.times;
  }
  return (os);
}

/**
 *  Get the month name.
 *
 *  @param[in] index  The month position.
 *
 *  @return The month name.
 */
std::string const& daterange::get_month_name(unsigned int index) {
  static std::string const unknown("unknown");
  static std::string const month[] = {
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
  if (index >= sizeof(month) / sizeof(*month))
    return (unknown);
  return (month[index]);
}

/**
 *  Get the weekday name.
 *
 *  @param[in] index  The weekday position.
 *
 *  @return The weekday name.
 */
std::string const& daterange::get_weekday_name(unsigned int index) {
  static std::string const unknown("unknown");
  static std::string const days[] = {
    "sunday",
    "monday",
    "tuesday",
    "wednesday",
    "thursday",
    "friday",
    "saturday"
  };
  if (index >= sizeof(days) / sizeof(*days))
    return (unknown);
  return (days[index]);
}
