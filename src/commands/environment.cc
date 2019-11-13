/*
** Copyright 2012-2013 Merethis
**
** This file is part of Centreon Clib.
**
** Centreon Clib is free software: you can redistribute it
** and/or modify it under the terms of the GNU Affero General Public
** License as published by the Free Software Foundation, either version
** 3 of the License, or (at your option) any later version.
**
** Centreon Clib is distributed in the hope that it will be
** useful, but WITHOUT ANY WARRANTY; without even the implied warranty
** of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** Affero General Public License for more details.
**
** You should have received a copy of the GNU Affero General Public
** License along with Centreon Clib. If not, see
** <http://www.gnu.org/licenses/>.
*/

#include "com/centreon/engine/commands/environment.hh"
#include <cstring>
#include "com/centreon/engine/error.hh"

using namespace com::centreon::engine::commands;

// Default size.
static uint32_t const EXTRA_SIZE_ENV = 128;
static uint32_t const EXTRA_SIZE_BUFFER = 4096;

/**
 *  Constructor.
 */
environment::environment(char** env)
    : _buffer(nullptr),
      _env(nullptr),
      _pos_buffer(0),
      _pos_env(0),
      _size_buffer(0),
      _size_env(0) {
  if (env)
    for (uint32_t i(0); env[i]; ++i)
      add(env[i]);
}

/**
 *  Copy constructor.
 *
 *  @param[in] right  The object to copy.
 */
environment::environment(environment const& right) {
  _internal_copy(right);
}

/**
 *  Destructor.
 */
environment::~environment() throw() {
  delete[] _buffer;
  delete[] _env;
}

/**
 *  Copy operator.
 *
 *  @param[in] right  The object to copy.
 *
 *  @return This object.
 */
environment& environment::operator=(environment const& right) {
  _internal_copy(right);
  return (*this);
}

/**
 *  Equal operator.
 *
 *  @param[in] right  The object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool environment::operator==(environment const& right) const throw() {
  return (_pos_buffer == right._pos_buffer && _pos_env == right._pos_env &&
          !strncmp(_buffer, right._buffer, _pos_buffer));
}

/**
 *  Not equal operator.
 *
 *  @param[in] right  The object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool environment::operator!=(environment const& right) const throw() {
  return (!operator==(right));
}

/**
 *  Add an environement variable.
 *
 *  @param[in] line  The nane and value on form (name=value).
 */
void environment::add(char const* line) {
  if (!line)
    return;
  uint32_t size(strlen(line));
  uint32_t new_pos(_pos_buffer + size + 1);
  if (new_pos > _size_buffer) {
    if (new_pos < _size_buffer + EXTRA_SIZE_BUFFER)
      _realoc_buffer(_size_buffer + EXTRA_SIZE_BUFFER);
    else
      _realoc_buffer(new_pos + EXTRA_SIZE_BUFFER);
  }
  memcpy(_buffer + _pos_buffer, line, size + 1);
  if (_pos_env + 1 >= _size_env)
    _realoc_env(_size_env + EXTRA_SIZE_ENV);
  _env[_pos_env++] = _buffer + _pos_buffer;
  _env[_pos_env] = nullptr;
  _pos_buffer += size + 1;
  return;
}

/**
 *  Add an environement variable.
 *
 *  @param[in] name   The name of the environement variable.
 *  @param[in] value  The environement varaible value.
 */
void environment::add(char const* name, char const* value) {
  if (!name || !value)
    return;
  uint32_t size_name(strlen(name));
  uint32_t size_value(strlen(value));
  uint32_t new_pos(_pos_buffer + size_name + size_value + 2);
  if (new_pos > _size_buffer) {
    if (new_pos < _size_buffer + EXTRA_SIZE_BUFFER)
      _realoc_buffer(_size_buffer + EXTRA_SIZE_BUFFER);
    else
      _realoc_buffer(new_pos + EXTRA_SIZE_BUFFER);
  }
  memcpy(_buffer + _pos_buffer, name, size_name + 1);
  _buffer[_pos_buffer + size_name] = '=';
  memcpy(_buffer + _pos_buffer + size_name + 1, value, size_value + 1);
  if (_pos_env + 1 >= _size_env)
    _realoc_env(_size_env + EXTRA_SIZE_ENV);
  _env[_pos_env++] = _buffer + _pos_buffer;
  _env[_pos_env] = nullptr;
  _pos_buffer += size_name + size_value + 2;
  return;
}

/**
 *  Add an environement variable.
 *
 *  @param[in] The nane and value on form (name=value).
 */
void environment::add(std::string const& line) {
  if (line.empty())
    return;
  uint32_t new_pos(_pos_buffer + line.size() + 1);
  if (new_pos > _size_buffer) {
    if (new_pos < _size_buffer + EXTRA_SIZE_BUFFER)
      _realoc_buffer(_size_buffer + EXTRA_SIZE_BUFFER);
    else
      _realoc_buffer(new_pos + EXTRA_SIZE_BUFFER);
  }
  memcpy(_buffer + _pos_buffer, line.c_str(), line.size() + 1);
  if (_pos_env + 1 >= _size_env)
    _realoc_env(_size_env + EXTRA_SIZE_ENV);
  _env[_pos_env++] = _buffer + _pos_buffer;
  _env[_pos_env] = nullptr;
  _pos_buffer += line.size() + 1;
  return;
}

/**
 *  Add an environement variable.
 *
 *  @param[in] name   The name of the environement variable.
 *  @param[in] value  The environement varaible value.
 */
void environment::add(std::string const& name, std::string const& value) {
  if (name.empty())
    return;
  uint32_t new_pos(_pos_buffer + name.size() + value.size() + 2);
  if (new_pos > _size_buffer) {
    if (new_pos < _size_buffer + EXTRA_SIZE_BUFFER)
      _realoc_buffer(_size_buffer + EXTRA_SIZE_BUFFER);
    else
      _realoc_buffer(new_pos + EXTRA_SIZE_BUFFER);
  }
  memcpy(_buffer + _pos_buffer, name.c_str(), name.size() + 1);
  _buffer[_pos_buffer + name.size()] = '=';
  memcpy(_buffer + _pos_buffer + name.size() + 1, value.c_str(),
         value.size() + 1);
  if (_pos_env + 1 >= _size_env)
    _realoc_env(_size_env + EXTRA_SIZE_ENV);
  _env[_pos_env++] = _buffer + _pos_buffer;
  _env[_pos_env] = nullptr;
  _pos_buffer += name.size() + value.size() + 2;
  return;
}

/**
 *  Get environement.
 */
char** environment::data() throw() {
  return (_env);
}

/**
 *  Internal copy
 *
 *  @param[in] right  The object to copy.
 */
void environment::_internal_copy(environment const& right) {
  if (this != &right) {
    delete[] _buffer;
    delete[] _env;
    _pos_buffer = right._pos_buffer;
    _pos_env = right._pos_env;
    _size_buffer = right._size_buffer;
    _size_env = right._size_env;
    _buffer = new char[_size_buffer];
    _env = new char*[_size_env];
    memcpy(_buffer, right._buffer, _pos_buffer);
    _rebuild_env();
  }
  return;
}

/**
 *  Realoc internal buffer.
 *
 *  @param[in] size  New size.
 */
void environment::_realoc_buffer(uint32_t size) {
  if (_size_buffer >= size)
    throw(
        engine_error() << "Invalid size for command environment reallocation: "
                       << "Buffer size is greater than the requested size");
  char* new_buffer(new char[size]);
  if (_buffer)
    memcpy(new_buffer, _buffer, _pos_buffer);
  _size_buffer = size;
  delete[] _buffer;
  _buffer = new_buffer;
  _rebuild_env();
  return;
}

/**
 *  Realoc internal env array.
 *
 *  @param[in] size  New size.
 */
void environment::_realoc_env(uint32_t size) {
  if (_size_env >= size)
    throw(engine_error()
          << "Invalid size for command environment reallocation: "
          << "Environment size is greater than the requested size");
  char** new_env(new char*[size]);
  if (_env)
    memcpy(new_env, _env, sizeof(*new_env) * (_pos_env + 1));
  _size_env = size;
  delete[] _env;
  _env = new_env;
  return;
}

/**
 *  Rebuild environement array.
 */
void environment::_rebuild_env() {
  if (!_env)
    return;
  for (uint32_t i(0), pos(0); i < _pos_env; ++i) {
    _env[i] = _buffer + pos;
    pos += strlen(_buffer + pos + 1) + 2;
  }
  _env[_pos_env] = nullptr;
  return;
}
