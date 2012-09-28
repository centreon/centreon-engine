/*
** Copyright 2011-2012 Merethis
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

#include <cassert>
#include <cstdlib>
#include <cstring>
#include "com/centreon/concurrency/read_locker.hh"
#include "com/centreon/concurrency/write_locker.hh"
#include "com/centreon/engine/logging/engine.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon::engine::logging;

// logging::engine class instance.
static engine* _instance = NULL;

/**************************************
*                                     *
*       obj_info Public Methods       *
*                                     *
**************************************/

/**
 *  Default constructor.
 */
engine::obj_info::obj_info()
  : _id(0),
    _type(0),
    _verbosity(0) {

}

/**
 *  Constructor.
 *
 *  @param[in] obj       Pointer on object logging.
 *  @param[in] type      Message type to log with this object.
 *  @param[in] verbosity Verbosity level.
 */
engine::obj_info::obj_info(
                    com::centreon::shared_ptr<object> obj,
                    unsigned long long type,
                    unsigned int verbosity)
  : _id(0), _obj(obj), _type(type), _verbosity(verbosity) {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
engine::obj_info::obj_info(obj_info const& right) {
  _internal_copy(right);
}

/**
 *  Destructor.
 */
engine::obj_info::~obj_info() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 */
engine::obj_info& engine::obj_info::operator=(obj_info const& right) {
  if (this != &right)
    _internal_copy(right);
  return (*this);
}

/**
 *  Get the ID.
 *
 *  @return The ID.
 */
unsigned long engine::obj_info::id() const throw () {
  return (_id);
}

/**
 *  Get the type.
 *
 *  @return The type.
 */
unsigned long long engine::obj_info::type() const throw () {
  return (_type);
}

/**
 *  Get the verbosity.
 *
 *  @return The verbosity.
 */
unsigned int engine::obj_info::verbosity() const throw () {
  return (_verbosity);
}

/**************************************
*                                     *
*      obj_info Private Methods       *
*                                     *
**************************************/

/**
 *  Copy internal data members.
 *
 *  @param[in] right Object to copy.
 */
void engine::obj_info::_internal_copy(engine::obj_info const& right) {
  _id = right._id;
  _obj = right._obj;
  _type = right._type;
  _verbosity = right._verbosity;
  return;
}

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Add a new object logging into engine.
 *
 *  @param[in] info The object logging with type and verbosity.
 *
 *  @return The object ID.
 */
unsigned long engine::add_object(obj_info& info) {
  concurrency::read_locker lock(&_rwlock);
  info._id = ++_id;
  _objects.push_back(info);
  for (unsigned int i(0), end(info.verbosity()); i <= end; ++i)
    _type[i] |= info.type();
  return (info._id);
}

/**
 *  Get instance of engine singleton.
 *
 *  @return An instance of engine.
 */
engine& engine::instance() {
  return (*_instance);
}

/**
 *  Check whether a log entry should be logged according to its
 *  properties.
 *
 *  @param[in] type Log type.
 *  @param[in] verbosity Log verbosity.
 *
 *  @return true if entry will be logged if processed.
 */
bool engine::is_logged(
               unsigned long long type,
               unsigned int verbosity) const throw () {
  return ((verbosity > most)
          ? false
          : static_cast<bool>(_type[verbosity] & type));
}

/**
 *  Load engine instance.
 */
void engine::load() {
  if (!_instance)
    _instance = new engine;
  return;
}

/**
 *  Write message into all logging objects.
 *
 *  @param[in] message   Message to log.
 *  @param[in] type      Logging types.
 *  @param[in] verbosity Verbosity level.
 */
void engine::log(
               char const* message,
               unsigned long long type,
               unsigned int verbosity) throw () {
  if (message != NULL) {
    concurrency::read_locker lock(&_rwlock);
    for (std::vector<obj_info>::iterator
           it(_objects.begin()), end(_objects.end());
	 it != end;
	 ++it) {
      obj_info& info(*it);
      if (verbosity <= info.verbosity() && (type & info.type()))
	info._obj->log(message, type, verbosity);
    }
  }
  return;
}

/**
 *  Remove a logging object.
 *
 *  @param[in] id The object's ID.
 */
void engine::remove_object(unsigned long id) throw () {
  concurrency::write_locker lock(&_rwlock);
  memset(_type, 0, sizeof(_type));
  std::vector<obj_info>::iterator it_erase(_objects.end());
  for (std::vector<obj_info>::iterator
         it(_objects.begin()), end(_objects.end());
       it != end;
       ++it) {
    obj_info& obj(*it);
    if (obj._id != id)
      for (unsigned int i = 0, end = obj.verbosity(); i <= end; ++i)
        _type[i] |= obj.type();
    else
      it_erase = it;
  }
  if (it_erase != _objects.end()) {
    if (it_erase->id() + 1 == id)
      --_id;
    _objects.erase(it_erase);
  }
  return;
}

/**
 *  Remove a logging object.
 *
 *  @param[in] obj The object info.
 */
void engine::remove_object(obj_info& obj) throw () {
  remove_object(obj.id());
  obj._id = 0;
  return;
}

/**
 *  Unload engine singleton.
 */
void engine::unload() {
  delete _instance;
  _instance = NULL;
  return;
}

/**
 *  Update type and verbosity for an object.
 *
 *  @param[in] id        The object's id.
 *  @param[in] type      Logging types.
 *  @param[in] verbosity Verbosity level.
 */
void engine::update_object(
               unsigned long id,
               unsigned long long type,
               unsigned int verbosity) throw () {
  concurrency::write_locker lock(&_rwlock);
  memset(_type, 0, sizeof(_type));
  std::vector<obj_info>::iterator it_erase(_objects.end());
  for (std::vector<obj_info>::iterator
         it(_objects.begin()), end(_objects.end());
       it != end;
       ++it) {
    obj_info& obj(*it);
    if (obj._id != id)
      for (unsigned int i = 0, end = obj.verbosity(); i <= end; ++i)
        _type[i] |= obj.type();
    else
      it_erase = it;
  }
  if (it_erase != _objects.end()) {
    obj_info& info(*it_erase);
    info._type = type;
    info._verbosity = verbosity;
    _type[verbosity] |= type;
  }
  return;
}

/**************************************
*                                     *
*           Private Methods           *
*                                     *
**************************************/

/**
 *  Default constructor.
 */
engine::engine() : _id(0) {
  memset(_type, 0, sizeof(_type));
}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 */
engine::engine(engine const& right) {
  _internal_copy(right);
}

/**
 *  Destructor.
 */
engine::~engine() throw () {

}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
engine& engine::operator=(engine const& right) {
  _internal_copy(right);
  return (*this);
}

/**
 *  Copy internal data members.
 *
 *  @param[in] right Object to copy.
 */
void engine::_internal_copy(engine const& right) {
  (void)right;
  assert(!"logging engine is not copyable");
  abort();
  return;
}
