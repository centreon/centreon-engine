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

#include "com/centreon/engine/objects/downtime.hh"
#include "com/centreon/engine/configuration/downtime.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;

#define SETTER(type, method) \
  &configuration::object::setter< \
     configuration::downtime, \
     type, \
     &configuration::downtime::method>::generic

configuration::downtime::setters const configuration::downtime::_setters[] = {
  { "author",              SETTER(std::string const&, _set_author) },
  { "comment",             SETTER(std::string const&, _set_comment_data) },
  { "downtime_id",         SETTER(unsigned long, _set_downtime_id) },
  { "duration",            SETTER(unsigned long, _set_duration) },
  { "end_time",            SETTER(time_t, _set_end_time) },
  { "entry_time",          SETTER(time_t, _set_entry_time) },
  { "fixed",               SETTER(bool, _set_fixed) },
  { "host_name",           SETTER(std::string const&, _set_host_name) },
  { "service_description", SETTER(std::string const&, _set_service_description) },
  { "start_time",          SETTER(time_t, _set_start_time) },
  { "triggered_by",        SETTER(unsigned long, _set_triggered_by) },
  { "recurring_interval",  SETTER(unsigned long, _set_recurring_interval) },
  { "recurring_period",    SETTER(std::string const&, _set_recurring_period_name) }
};

/**
 *  Parse and set the downtime property.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The property value.
 *
 *  @return True on success, otherwise false.
 */
bool configuration::downtime::parse(char const* key, char const* value) {
  for (unsigned int i(0);
       i < sizeof(_setters) / sizeof(_setters[0]);
       ++i)
    if (!strcmp(_setters[i].name, key))
      return ((_setters[i].func)(*this, value));
  return (false);
}

/**
 *  Constructor.
 *
 *  @param[in] type This is a host or service downtime.
 */
configuration::downtime::downtime(type_id downtime_type)
  : object(object::downtime),
    _downtime_id(0),
    _downtime_type(downtime_type),
    _duration(0),
    _end_time(0),
    _entry_time(0),
    _fixed(false),
    _start_time(0),
    _triggered_by(0),
    _recurring_interval(0),
    _recurring_period(NULL) {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
configuration::downtime::downtime(downtime const& right)
  : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
configuration::downtime::~downtime() throw () {}

/**
 *  Copy operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
configuration::downtime& configuration::downtime::operator=(downtime const& right) {
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
    _recurring_interval = right._recurring_interval;
    _recurring_period = right._recurring_period;
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
bool configuration::downtime::operator==(downtime const& right) const throw () {
  return (object::operator==(right)
          && _author == right._author
          && _comment_data == right._comment_data
          && _downtime_id == right._downtime_id
          && _downtime_type == right._downtime_type
          && _duration == right._duration
          && _end_time == right._end_time
          && _entry_time == right._entry_time
          && _fixed == right._fixed
          && _host_name == right._host_name
          && _service_description == right._service_description
          && _start_time == right._start_time
          && _triggered_by == right._triggered_by
          && _recurring_interval == right._recurring_interval
          && _recurring_period == right._recurring_period);
}

/**
 *  Not equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool configuration::downtime::operator!=(downtime const& right) const throw () {
  return (!operator==(right));
}

bool configuration::downtime::operator<(downtime const& right) const throw () {
  if (_author != right._author)
    return _author < right._author;
  else if (_comment_data != right._comment_data)
    return _comment_data < right._comment_data;
  else if (_downtime_id != right._downtime_id)
    return _downtime_id < right._downtime_id;
  else if (_duration != right._duration)
    return _duration < right._duration;
  else if (_end_time != right._end_time)
    return _end_time < right._end_time;
  else if (_entry_time != right._entry_time)
    return _entry_time < right._entry_time;
  else if (_fixed != right._fixed)
    return _fixed < right._fixed;
  else if (_host_name != right._host_name)
    return _host_name < right._host_name;
  else if (_service_description != right._service_description)
    return _service_description < right._service_description;
  else if (_start_time != right._start_time)
    return _start_time < right._start_time;
  else if (_triggered_by != right._triggered_by)
    return _triggered_by < right._triggered_by;
  else if (_recurring_interval != right._recurring_interval)
    return _recurring_interval < right._recurring_interval;
  else if (_recurring_period != right._recurring_period)
    return _recurring_period < right._recurring_period;
  else
    return false;
}

void configuration::downtime::check_validity() const {
  return ;
}

void configuration::downtime::merge(object const& obj) {
  return ;
}

configuration::downtime::key_type const& configuration::downtime::key() const throw () {
  return *this;
}

/**
 * Get author.
 *
 * @return The author.
 */
std::string const& configuration::downtime::author() const throw () {
 return (_author);
}

/**
 * Get comment_data.
 *
 * @return The comment_data.
 */
std::string const& configuration::downtime::comment_data() const throw () {
 return (_comment_data);
}

/**
 * Get downtime_id.
 *
 * @return The downtime_id.
 */
unsigned long configuration::downtime::downtime_id() const throw () {
 return (_downtime_id);
}

/**
 * Get downtime_type.
 *
 * @return The downtime_type.
 */
configuration::downtime::type_id configuration::downtime::downtime_type() const throw () {
 return (_downtime_type);
}

/**
 * Get duration.
 *
 * @return The duration.
 */
unsigned long configuration::downtime::duration() const throw () {
 return (_duration);
}

/**
 * Get end_time.
 *
 * @return The end_time.
 */
time_t configuration::downtime::end_time() const throw () {
 return (_end_time);
}

/**
 * Get entry_time.
 *
 * @return The entry_time.
 */
time_t configuration::downtime::entry_time() const throw () {
 return (_entry_time);
}

/**
 * Get fixed.
 *
 * @return The fixed.
 */
bool configuration::downtime::fixed() const throw () {
 return (_fixed);
}

/**
 * Get host_name.
 *
 * @return The host_name.
 */
std::string const& configuration::downtime::host_name() const throw () {
 return (_host_name);
}

/**
 * Get service_description.
 *
 * @return The service_description.
 */
std::string const& configuration::downtime::service_description() const throw () {
 return (_service_description);
}

/**
 * Get start_time.
 *
 * @return The start_time.
 */
time_t configuration::downtime::start_time() const throw () {
 return (_start_time);
}

/**
 * Get triggered_by.
 *
 * @return The triggered_by.
 */
unsigned long configuration::downtime::triggered_by() const throw () {
 return (_triggered_by);
}

unsigned long configuration::downtime::recurring_interval() const throw() {
  return (_recurring_interval);
}

timeperiod* configuration::downtime::recurring_period() const throw() {
  return (_recurring_period);
}

/**
 *  Set author.
 *
 *  @param[in] value The new author.
 */
bool configuration::downtime::_set_author(std::string const& value) {
  _author = value;
  return (true);
}

/**
 *  Set comment_data.
 *
 *  @param[in] value The new comment_data.
 */
bool configuration::downtime::_set_comment_data(std::string const& value) {
  _comment_data = value;
  return (true);
}

/**
 *  Set downtime_id.
 *
 *  @param[in] value The new downtime_id.
 */
bool configuration::downtime::_set_downtime_id(unsigned long value) {
  _downtime_id = value;
  return (true);
}

/**
 *  Set duration.
 *
 *  @param[in] value The new duration.
 */
bool configuration::downtime::_set_duration(unsigned long value) {
  _duration = value;
  return (true);
}

/**
 *  Set end_time.
 *
 *  @param[in] value The new end_time.
 */
bool configuration::downtime::_set_end_time(time_t value) {
  _end_time = value;
  return (true);
}

/**
 *  Set entry_time.
 *
 *  @param[in] value The new entry_time.
 */
bool configuration::downtime::_set_entry_time(time_t value) {
  _entry_time = value;
  return (true);
}

/**
 *  Set fixed.
 *
 *  @param[in] value The new fixed.
 */
bool configuration::downtime::_set_fixed(bool value) {
  _fixed = value;
  return (true);
}

/**
 *  Set host_name.
 *
 *  @param[in] value The new host_name.
 */
bool configuration::downtime::_set_host_name(std::string const& value) {
  _host_name = value;
  return (true);
}

/**
 *  Set service_description.
 *
 *  @param[in] value The new service_description.
 */
bool configuration::downtime::_set_service_description(std::string const& value) {
  _service_description = value;
  return (true);
}

/**
 *  Set start_time.
 *
 *  @param[in] value The new start_time.
 */
bool configuration::downtime::_set_start_time(time_t value) {
  _start_time = value;
  return (true);
}

/**
 *  Set triggered_by.
 *
 *  @param[in] value The new triggered_by.
 */
bool configuration::downtime::_set_triggered_by(unsigned long value) {
  _triggered_by = value;
  return (true);
}

bool configuration::downtime::_set_recurring_interval(unsigned long value) {
  _recurring_interval = value;
  return (true);
}

bool configuration::downtime::_set_recurring_period(::timeperiod* value) {
  _recurring_period = value;
  return (true);
}

bool configuration::downtime::_set_recurring_period_name(std::string const& value) {
  _recurring_period_name = value;
  return (true);
}

std::string const& configuration::downtime::recurring_period_name() const throw() {
  return _recurring_period_name;
}

bool configuration::downtime::resolve_recurring_period() {
  if (_recurring_period_name.empty())
    return (true);
  umap<std::string, shared_ptr<timeperiod_struct> >::iterator it =
      configuration::applier::state::instance().timeperiods().find(_recurring_period_name);
  if (it == configuration::applier::state::instance().timeperiods().end())
    return (false);
  _recurring_period = it->second.get();
  return (true);
}
