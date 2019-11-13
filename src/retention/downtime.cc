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

#include "com/centreon/engine/downtimes/downtime.hh"
#include "com/centreon/engine/retention/downtime.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;

#define SETTER(type, method)                            \
  &retention::object::setter<retention::downtime, type, \
                             &retention::downtime::method>::generic

retention::downtime::setters const retention::downtime::_setters[] = {
    {"author", SETTER(std::string const&, _set_author)},
    {"comment", SETTER(std::string const&, _set_comment_data)},
    {"downtime_id", SETTER(unsigned long, _set_downtime_id)},
    {"duration", SETTER(unsigned long, _set_duration)},
    {"end_time", SETTER(time_t, _set_end_time)},
    {"entry_time", SETTER(time_t, _set_entry_time)},
    {"fixed", SETTER(bool, _set_fixed)},
    {"host_name", SETTER(std::string const&, _set_host_name)},
    {"service_description",
     SETTER(std::string const&, _set_service_description)},
    {"start_time", SETTER(time_t, _set_start_time)},
    {"triggered_by", SETTER(unsigned long, _set_triggered_by)}};

/**
 *  Constructor.
 *
 *  @param[in] type This is a host or service downtime.
 */
retention::downtime::downtime(type_id downtime_type)
    : object(object::downtime),
      _downtime_id(0),
      _downtime_type(downtime_type),
      _duration(0),
      _end_time(0),
      _entry_time(0),
      _fixed(false),
      _start_time(0),
      _triggered_by(0) {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
retention::downtime::downtime(downtime const& right) : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
retention::downtime::~downtime() throw() {}

/**
 *  Copy operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
retention::downtime& retention::downtime::operator=(downtime const& right) {
  if (this != &right) {
    object::operator=(right);
    _author = right._author;
    _comment_data = right._comment_data;
    _downtime_id = right._downtime_id;
    _downtime_type = right._downtime_type;
    _duration = right._duration;
    _end_time = right._end_time;
    _entry_time = right._entry_time;
    _fixed = right._fixed;
    _host_name = right._host_name;
    _service_description = right._service_description;
    _start_time = right._start_time;
    _triggered_by = right._triggered_by;
  }
  return (*this);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool retention::downtime::operator==(downtime const& right) const throw() {
  return (
      object::operator==(right) && _author == right._author &&
      _comment_data == right._comment_data &&
      _downtime_id == right._downtime_id &&
      _downtime_type == right._downtime_type && _duration == right._duration &&
      _end_time == right._end_time && _entry_time == right._entry_time &&
      _fixed == right._fixed && _host_name == right._host_name &&
      _service_description == right._service_description &&
      _start_time == right._start_time && _triggered_by == right._triggered_by);
}

/**
 *  Not equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool retention::downtime::operator!=(downtime const& right) const throw() {
  return (!operator==(right));
}

/**
 *  Set new value on specific property.
 *
 *  @param[in] key   The property to set.
 *  @param[in] value The new value.
 *
 *  @return True on success, otherwise false.
 */
bool retention::downtime::set(char const* key, char const* value) {
  for (unsigned int i(0); i < sizeof(_setters) / sizeof(_setters[0]); ++i)
    if (!strcmp(_setters[i].name, key))
      return ((_setters[i].func)(*this, value));
  return (false);
}

/**
 * Get author.
 *
 * @return The author.
 */
std::string retention::downtime::author() const throw() {
  return (_author);
}

/**
 * Get comment_data.
 *
 * @return The comment_data.
 */
std::string retention::downtime::comment_data() const throw() {
  return (_comment_data);
}

/**
 * Get downtime_id.
 *
 * @return The downtime_id.
 */
unsigned long retention::downtime::downtime_id() const throw() {
  return (_downtime_id);
}

/**
 * Get downtime_type.
 *
 * @return The downtime_type.
 */
retention::downtime::type_id retention::downtime::downtime_type() const
    throw() {
  return (_downtime_type);
}

/**
 * Get duration.
 *
 * @return The duration.
 */
unsigned long retention::downtime::duration() const throw() {
  return (_duration);
}

/**
 * Get end_time.
 *
 * @return The end_time.
 */
time_t retention::downtime::end_time() const throw() {
  return (_end_time);
}

/**
 * Get entry_time.
 *
 * @return The entry_time.
 */
time_t retention::downtime::entry_time() const throw() {
  return (_entry_time);
}

/**
 * Get fixed.
 *
 * @return The fixed.
 */
bool retention::downtime::fixed() const throw() {
  return (_fixed);
}

/**
 * Get host_name.
 *
 * @return The host_name.
 */
std::string retention::downtime::host_name() const throw() {
  return (_host_name);
}

/**
 * Get service_description.
 *
 * @return The service_description.
 */
std::string retention::downtime::service_description() const throw() {
  return (_service_description);
}

/**
 * Get start_time.
 *
 * @return The start_time.
 */
time_t retention::downtime::start_time() const throw() {
  return (_start_time);
}

/**
 * Get triggered_by.
 *
 * @return The triggered_by.
 */
unsigned long retention::downtime::triggered_by() const throw() {
  return (_triggered_by);
}

/**
 *  Set author.
 *
 *  @param[in] value The new author.
 */
bool retention::downtime::_set_author(std::string const& value) {
  _author = value;
  return (true);
}

/**
 *  Set comment_data.
 *
 *  @param[in] value The new comment_data.
 */
bool retention::downtime::_set_comment_data(std::string const& value) {
  _comment_data = value;
  return (true);
}

/**
 *  Set downtime_id.
 *
 *  @param[in] value The new downtime_id.
 */
bool retention::downtime::_set_downtime_id(unsigned long value) {
  _downtime_id = value;
  return (true);
}

/**
 *  Set duration.
 *
 *  @param[in] value The new duration.
 */
bool retention::downtime::_set_duration(unsigned long value) {
  _duration = value;
  return (true);
}

/**
 *  Set end_time.
 *
 *  @param[in] value The new end_time.
 */
bool retention::downtime::_set_end_time(time_t value) {
  _end_time = value;
  return (true);
}

/**
 *  Set entry_time.
 *
 *  @param[in] value The new entry_time.
 */
bool retention::downtime::_set_entry_time(time_t value) {
  _entry_time = value;
  return (true);
}

/**
 *  Set fixed.
 *
 *  @param[in] value The new fixed.
 */
bool retention::downtime::_set_fixed(bool value) {
  _fixed = value;
  return (true);
}

/**
 *  Set host_name.
 *
 *  @param[in] value The new host_name.
 */
bool retention::downtime::_set_host_name(std::string const& value) {
  _host_name = value;
  return (true);
}

/**
 *  Set service_description.
 *
 *  @param[in] value The new service_description.
 */
bool retention::downtime::_set_service_description(std::string const& value) {
  _service_description = value;
  return (true);
}

/**
 *  Set start_time.
 *
 *  @param[in] value The new start_time.
 */
bool retention::downtime::_set_start_time(time_t value) {
  _start_time = value;
  return (true);
}

/**
 *  Set triggered_by.
 *
 *  @param[in] value The new triggered_by.
 */
bool retention::downtime::_set_triggered_by(unsigned long value) {
  _triggered_by = value;
  return (true);
}
