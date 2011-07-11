/*
** Copyright 2011      Merethis
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

#include "logging/engine.hh"

using namespace com::centreon::engine::logging;

/**************************************
 *                                     *
 *           Public Methods            *
 *                                     *
 **************************************/

/**
 *  Default constructor.
 */
engine::engine()
  : _id(0) {

}

/**
 *  Default destructor.
 */
engine::~engine() throw() {

}

/**
 *  Get instance of engine singleton.
 *
 *  @return 
 */
engine& engine::instance() {
  static engine instance;
  return (instance);
}

/**
 *  Cleanup the engine singleton.
 */
void engine::cleanup() {
  engine& instance = engine::instance();
  instance._rwlock.lockForWrite();
  instance._objects.clear();
  instance._id = 0;
  instance._rwlock.unlock();
}

/**
 *  Write message into all objects logging.
 *
 *  @param[in] message   Message to log.
 *  @param[in] type      Logging types.
 *  @param[in] verbosity Verbosity level.
 */
void engine::log(char const* message,
		 unsigned long long type,
		 unsigned int verbosity) throw() {
  if (message != NULL) {
    _rwlock.lockForRead();
    for (QHash<unsigned long, obj_info>::iterator it = _objects.begin(), end = _objects.end();
	 it != end;
	 ++it) {
      obj_info& info = it.value();
      if (verbosity <= info.verbosity && (type & info.type)) {
	info.obj->log(message, type, verbosity);
      }
    }
    _rwlock.unlock();
  }
}

/**
 *  Add a new object logging into engine.
 *
 *  @param[in] info The object logging with type and verbosity.
 */
unsigned long engine::add_object(obj_info const& info) {
  _rwlock.lockForWrite();
  unsigned int id = ++_id;
  _objects.insert(id, info);
  _rwlock.unlock();
  return (id);
}

/**
 *  Remove an object logging.
 *
 *  @param[in] id The object's id.
 */
void engine::remove_object(unsigned long id) throw() {
  _rwlock.lockForWrite();
  QHash<unsigned long, obj_info>::iterator it = _objects.find(id);
  if (it != _objects.end()) {
    if (it.key() + 1 == id) {
      --_id;
    }
    _objects.erase(it);
  }
  _rwlock.unlock();
}

/**
 *  Update type and verbosity for an object.
 *
 *  @param[in] id        The object's id.
 *  @param[in] type      Logging types.
 *  @param[in] verbosity Verbosity level.
 */
void engine::update_object(unsigned long id,
			   unsigned long long type,
			   unsigned int verbosity) throw() {
  _rwlock.lockForWrite();
  QHash<unsigned long, obj_info>::iterator it = _objects.find(id);
  if (it != _objects.end()) {
    obj_info& info = it.value();
    info.type = type;
    info.verbosity = verbosity;
  }
  _rwlock.unlock();
}

/**
 *  Default constructor.
 */
engine::obj_info::obj_info()
  : type(0), verbosity(0) {

}

/**
 *  Constructor.
 *
 *  @param[in] _obj       Pointer on object logging.
 *  @param[in] _type      Message type to log with this object.
 *  @param[in] _verbosity Verbosity level.
 */
engine::obj_info::obj_info(QSharedPointer<object> _obj,
			   unsigned long long _type,
			   unsigned int _verbosity)
  : obj(_obj), type(_type), verbosity(_verbosity) {

}

/**
 *  Default copy constructor.
 *
 *  @param[in] right The class to copy.
 */
engine::obj_info::obj_info(obj_info const& right)
  : type(0), verbosity(0) {
  operator=(right);
}

/**
 *  Default destructor.
 */
engine::obj_info::~obj_info() throw() {

}

/**
 *  Default copy operator.
 *
 *  @param[in] right The class to copy.
 */
engine::obj_info& engine::obj_info::operator=(obj_info const& right) {
  if (this != &right) {
    obj = right.obj;
    type = right.type;
    verbosity = right.verbosity;
  }
  return (*this);
}
