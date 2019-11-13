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

#include "com/centreon/engine/comment.hh"
#include "com/centreon/engine/retention/comment.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;

#define SETTER(type, method)                           \
  &retention::object::setter<retention::comment, type, \
                             &retention::comment::method>::generic

retention::comment::setters const retention::comment::_setters[] = {
    {"author", SETTER(std::string const&, _set_author)},
    {"comment_data", SETTER(std::string const&, _set_comment_data)},
    {"comment_id", SETTER(unsigned long, _set_comment_id)},
    {"entry_time", SETTER(time_t, _set_entry_time)},
    {"entry_type", SETTER(unsigned int, _set_entry_type)},
    {"expire_time", SETTER(time_t, _set_expire_time)},
    {"expires", SETTER(bool, _set_expires)},
    {"host_name", SETTER(std::string const&, _set_host_name)},
    {"persistent", SETTER(bool, _set_persistent)},
    {"service_description",
     SETTER(std::string const&, _set_service_description)},
    {"source", SETTER(int, _set_source)}};

/**
 *  Constructor.
 *
 *  @param[in] type This is a host or service comment.
 */
retention::comment::comment(type_id comment_type)
    : object(object::comment),
      _comment_id(0),
      _comment_type(comment_type),
      _entry_type(com::centreon::engine::comment::user),
      _expire_time(0),
      _expires(false),
      _persistent(false),
      _source(com::centreon::engine::comment::internal) {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
retention::comment::comment(comment const& right) : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
retention::comment::~comment() throw() {}

/**
 *  Copy operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
retention::comment& retention::comment::operator=(comment const& right) {
  if (this != &right) {
    object::operator=(right);
    _author = right._author;
    _comment_data = right._comment_data;
    _comment_id = right._comment_id;
    _comment_type = right._comment_type;
    _entry_time = right._entry_time;
    _entry_type = right._entry_type;
    _expire_time = right._expire_time;
    _expires = right._expires;
    _host_name = right._host_name;
    _persistent = right._persistent;
    _service_description = right._service_description;
    _source = right._source;
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
bool retention::comment::operator==(comment const& right) const throw() {
  return (object::operator==(right) && _author == right._author &&
          _comment_data == right._comment_data &&
          _comment_id == right._comment_id &&
          _comment_type == right._comment_type &&
          _entry_time == right._entry_time &&
          _entry_type == right._entry_type &&
          _expire_time == right._expire_time && _expires == right._expires &&
          _host_name == right._host_name && _persistent == right._persistent &&
          _service_description == right._service_description &&
          _source == right._source);
}

/**
 *  Not equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool retention::comment::operator!=(comment const& right) const throw() {
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
bool retention::comment::set(char const* key, char const* value) {
  for (unsigned int i(0); i < sizeof(_setters) / sizeof(_setters[0]); ++i)
    if (!strcmp(_setters[i].name, key))
      return ((_setters[i].func)(*this, value));
  return (false);
}

/**
 *  Get author.
 *
 *  @return The author.
 */
std::string const& retention::comment::author() const throw() {
  return (_author);
}

/**
 *  Get comment_data.
 *
 *  @return The comment_data.
 */
std::string const& retention::comment::comment_data() const throw() {
  return (_comment_data);
}

/**
 *  Get comment_id.
 *
 *  @return The comment_id.
 */
unsigned long retention::comment::comment_id() const throw() {
  return (_comment_id);
}

/**
 *  Get comment_type.
 *
 *  @return The comment_type.
 */
retention::comment::type_id retention::comment::comment_type() const throw() {
  return (_comment_type);
}

/**
 *  Get entry_time.
 *
 *  @return The entry_time.
 */
time_t retention::comment::entry_time() const throw() {
  return (_entry_time);
}

/**
 *  Get entry_type.
 *
 *  @return The entry_type.
 */
unsigned int retention::comment::entry_type() const throw() {
  return (_entry_type);
}

/**
 *  Get expire_time.
 *
 *  @return The expire_time.
 */
time_t retention::comment::expire_time() const throw() {
  return (_expire_time);
}

/**
 *  Get expires.
 *
 *  @return The expires.
 */
bool retention::comment::expires() const throw() {
  return (_expires);
}

/**
 *  Get host_name.
 *
 *  @return The host_name.
 */
std::string const& retention::comment::host_name() const throw() {
  return (_host_name);
}

/**
 *  Get persistent.
 *
 *  @return The persistent.
 */
bool retention::comment::persistent() const throw() {
  return (_persistent);
}

/**
 *  Get service_description.
 *
 *  @return The service_description.
 */
std::string const& retention::comment::service_description() const throw() {
  return (_service_description);
}

/**
 *  Get source.
 *
 *  @return The source.
 */
int retention::comment::source() const throw() {
  return (_source);
}

/**
 *  Set author.
 *
 *  @param[in] value The author.
 */
bool retention::comment::_set_author(std::string const& value) {
  _author = value;
  return (true);
}

/**
 *  Set comment_data.
 *
 *  @param[in] value The comment_data.
 */
bool retention::comment::_set_comment_data(std::string const& value) {
  _comment_data = value;
  return (true);
}

/**
 *  Set comment_id.
 *
 *  @param[in] value The comment_id.
 */
bool retention::comment::_set_comment_id(unsigned long value) {
  _comment_id = value;
  return (true);
}

/**
 *  Set entry_time.
 *
 *  @param[in] value The entry_time.
 */
bool retention::comment::_set_entry_time(time_t value) {
  _entry_time = value;
  return (true);
}

/**
 *  Set entry_type.
 *
 *  @param[in] value The entry_type.
 */
bool retention::comment::_set_entry_type(unsigned int value) {
  _entry_type = value;
  return (true);
}

/**
 *  Set expire_time.
 *
 *  @param[in] value The expire_time.
 */
bool retention::comment::_set_expire_time(time_t value) {
  _expire_time = value;
  return (true);
}

/**
 *  Set expires.
 *
 *  @param[in] value The expires.
 */
bool retention::comment::_set_expires(bool value) {
  _expires = value;
  return (true);
}

/**
 *  Set host_name.
 *
 *  @param[in] value The host_name.
 */
bool retention::comment::_set_host_name(std::string const& value) {
  _host_name = value;
  return (true);
}

/**
 *  Set persistent.
 *
 *  @param[in] value The persistent.
 */
bool retention::comment::_set_persistent(bool value) {
  _persistent = value;
  return (true);
}

/**
 *  Set service_description.
 *
 *  @param[in] value The service_description.
 */
bool retention::comment::_set_service_description(std::string const& value) {
  _service_description = value;
  return (true);
}

/**
 *  Set source.
 *
 *  @param[in] value The source.
 */
bool retention::comment::_set_source(int value) {
  _source = value;
  return (true);
}
