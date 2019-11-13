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

#include "com/centreon/engine/configuration/daterange.hh"

using namespace com::centreon::engine::configuration;

/**
 *  Constructor.
 *
 *  @param[in] type The date range type.
 */
daterange::daterange(type_range type)
    : _month_end(0),
      _month_start(0),
      _month_day_end(0),
      _month_day_start(0),
      _skip_interval(0),
      _type(type),
      _week_day_end(0),
      _week_day_start(0),
      _week_day_end_offset(0),
      _week_day_start_offset(0),
      _year_end(0),
      _year_start(0) {}

/**
 *  Copy constructor.
 *
 *  @param[in] right The object to copy.
 */
daterange::daterange(daterange const& right) {
  operator=(right);
}

/**
 *  Destructor.
 */
daterange::~daterange() {}

/**
 *  Copy operator.
 *
 *  @param[in] right The object to copy.
 *
 *  @return This object.
 */
daterange& daterange::operator=(daterange const& right) {
  if (this != &right) {
    _month_end = right._month_end;
    _month_start = right._month_start;
    _month_day_end = right._month_day_end;
    _month_day_start = right._month_day_start;
    _skip_interval = right._skip_interval;
    _timeranges = right._timeranges;
    _type = right._type;
    _week_day_end = right._week_day_end;
    _week_day_start = right._week_day_start;
    _week_day_end_offset = right._week_day_end_offset;
    _week_day_start_offset = right._week_day_start_offset;
    _year_end = right._year_end;
    _year_start = right._year_start;
  }
  return (*this);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if object is the same, otherwise false.
 */
bool daterange::operator==(daterange const& right) const throw() {
  return (_month_end == right._month_end &&
          _month_start == right._month_start &&
          _month_day_end == right._month_day_end &&
          _month_day_start == right._month_day_start &&
          _skip_interval == right._skip_interval &&
          _timeranges == right._timeranges && _type == right._type &&
          _week_day_end == right._week_day_end &&
          _week_day_start == right._week_day_start &&
          _week_day_end_offset == right._week_day_end_offset &&
          _week_day_start_offset == right._week_day_start_offset &&
          _year_end == right._year_end && _year_start == right._year_start);
}

/**
 *  Not equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if object is not the same, otherwise false.
 */
bool daterange::operator!=(daterange const& right) const throw() {
  return (!operator==(right));
}

/**
 *  Less-than operator.
 *
 *  @param[in] right Object to compare to.
 *
 *  @return True if this object is less than right.
 */
bool daterange::operator<(daterange const& right) const throw() {
  if (_month_end != right._month_end)
    return (_month_end < right._month_end);
  else if (_month_start != right._month_start)
    return (_month_start < right._month_start);
  else if (_month_day_end != right._month_day_end)
    return (_month_day_end < right._month_day_end);
  else if (_month_day_start != right._month_day_start)
    return (_month_day_start < right._month_day_start);
  else if (_skip_interval != right._skip_interval)
    return (_skip_interval < right._skip_interval);
  else if (_type != right._type)
    return (_type < right._type);
  else if (_week_day_end != right._week_day_end)
    return (_week_day_end < right._week_day_end);
  else if (_week_day_start != right._week_day_start)
    return (_week_day_start < right._week_day_start);
  else if (_week_day_end_offset != right._week_day_end_offset)
    return (_week_day_end_offset < right._week_day_end_offset);
  else if (_week_day_start_offset != right._week_day_start_offset)
    return (_week_day_start_offset < right._week_day_start_offset);
  else if (_year_end != right._year_end)
    return (_year_end < right._year_end);
  else if (_year_start != right._year_start)
    return (_year_start < right._year_start);
  return (_timeranges < right._timeranges);
}

/**
 *  Set month_end value.
 *
 *  @param[in] value The new month_end value.
 */
void daterange::month_end(unsigned int value) {
  _month_end = value;
}

/**
 *  Get month_end value.
 *
 *  @return The month_end value.
 */
unsigned int daterange::month_end() const throw() {
  return (_month_end);
}

/**
 *  Set month_start value.
 *
 *  @param[in] value The new month_start value.
 */
void daterange::month_start(unsigned int value) {
  _month_start = value;
}

/**
 *  Get month_start value.
 *
 *  @return The month_start value.
 */
unsigned int daterange::month_start() const throw() {
  return (_month_start);
}

/**
 *  Set month_day_end value.
 *
 *  @param[in] value The new month_day_end value.
 */
void daterange::month_day_end(int value) {
  _month_day_end = value;
}

/**
 *  Get month_day_end value.
 *
 *  @return The month_day_end value.
 */
int daterange::month_day_end() const throw() {
  return (_month_day_end);
}

/**
 *  Set month_day_start value.
 *
 *  @param[in] value The new month_day_start value.
 */
void daterange::month_day_start(int value) {
  _month_day_start = value;
}

/**
 *  Get month_day_start value.
 *
 *  @return The month_day_start value.
 */
int daterange::month_day_start() const throw() {
  return (_month_day_start);
}

/**
 *  Set skip_interval value.
 *
 *  @param[in] value The new skip_interval value.
 */
void daterange::skip_interval(unsigned int value) {
  _skip_interval = value;
}

/**
 *  Get skip_interval value.
 *
 *  @return The skip_interval value.
 */
unsigned int daterange::skip_interval() const throw() {
  return (_skip_interval);
}

/**
 *  Get timeranges value.
 *
 *  @return The timeranges value.
 */
void daterange::timeranges(std::list<timerange> const& value) {
  _timeranges = value;
}

/**
 *  Set timeranges value.
 *
 *  @param[in] value The new timeranges value.
 */
std::list<timerange> const& daterange::timeranges() const throw() {
  return (_timeranges);
}

/**
 *  Set type value.
 *
 *  @param[in] value The new type value.
 */
void daterange::type(type_range value) {
  _type = value;
}

/**
 *  Get type value.
 *
 *  @return The type value.
 */
daterange::type_range daterange::type() const throw() {
  return (_type);
}

/**
 *  Set week_day_end value.
 *
 *  @param[in] value The new week_day_end value.
 */
void daterange::week_day_end(unsigned int value) {
  _week_day_end = value;
}

/**
 *  Get week_day_end value.
 *
 *  @return The week_day_end value.
 */
unsigned int daterange::week_day_end() const throw() {
  return (_week_day_end);
}

/**
 *  Set week_day_start value.
 *
 *  @param[in] value The new week_day_start value.
 */
void daterange::week_day_start(unsigned int value) {
  _week_day_start = value;
}

/**
 *  Get week_day_start value.
 *
 *  @return The week_day_start value.
 */
unsigned int daterange::week_day_start() const throw() {
  return (_week_day_start);
}

/**
 *  Set week_day_end_offset value.
 *
 *  @param[in] value The new week_day_end_offset value.
 */
void daterange::week_day_end_offset(int value) {
  _week_day_end_offset = value;
}

/**
 *  Get week_day_end_offset value.
 *
 *  @return The week_day_end_offset value.
 */
int daterange::week_day_end_offset() const throw() {
  return (_week_day_end_offset);
}

/**
 *  Set week_day_start_offset value.
 *
 *  @param[in] value The new week_day_start_offset value.
 */
void daterange::week_day_start_offset(int value) {
  _week_day_start_offset = value;
}

/**
 *  Get week_day_start_offset value.
 *
 *  @return The week_day_start_offset value.
 */
int daterange::week_day_start_offset() const throw() {
  return (_week_day_start_offset);
}

/**
 *  Set year_end value.
 *
 *  @param[in] value The new year_end value.
 */
void daterange::year_end(unsigned int value) {
  _year_end = value;
}

/**
 *  Get year_end value.
 *
 *  @return The year_end value.
 */
unsigned int daterange::year_end() const throw() {
  return (_year_end);
}

/**
 *  Set year_start value.
 *
 *  @param[in] value The new year_start value.
 */
void daterange::year_start(unsigned int value) {
  _year_start = value;
}

/**
 *  Get year_start value.
 *
 *  @return The year_start value.
 */
unsigned int daterange::year_start() const throw() {
  return (_year_start);
}
