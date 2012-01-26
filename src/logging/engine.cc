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

engine* engine::_instance = NULL;

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
  memset(_type, 0, sizeof(_type));
}

/**
 *  Default destructor.
 */
engine::~engine() throw() {

}

/**
 *  Get instance of engine singleton.
 *
 *  @return An instance on the engine.
 */
engine& engine::instance() {
  if (_instance == NULL)
    _instance = new engine();
  return (*_instance);
}

/**
 *  Cleanup the engine singleton.
 */
void engine::cleanup() {
  delete _instance;
  _instance = NULL;
}

bool engine::is_logged(unsigned long long type,
                       unsigned int verbosity) const throw() {
  if (verbosity > most)
    return (false);
  return (static_cast<bool>(_type[verbosity] & type));
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
    for (std::vector<obj_info>::iterator it = _objects.begin(),
           end = _objects.end();
	 it != end;
	 ++it) {
      obj_info& info(*it);
      if (verbosity <= info.verbosity() && (type & info.type())) {
	info._obj->log(message, type, verbosity);
      }
    }
    _rwlock.unlock();
  }
}

/**
 *  Add a new object logging into engine.
 *
 *  @param[in] info The object logging with type and verbosity.
 *
 *  @return The object id.
 */
unsigned long engine::add_object(obj_info& info) {
  _rwlock.lockForWrite();
  info._id = ++_id;
  _objects.push_back(info);
  for (unsigned int i = 0, end = info.verbosity(); i <= end; ++i)
    _type[i] |= info.type();
  _rwlock.unlock();
  return (info._id);
}

/**
 *  Remove an object logging.
 *
 *  @param[in] id The object's id.
 */
void engine::remove_object(unsigned long id) throw() {
  _rwlock.lockForWrite();
  memset(_type, 0, sizeof(_type));
  std::vector<obj_info>::iterator it_erase = _objects.end();
  for (std::vector<obj_info>::iterator it = _objects.begin(),
         end = _objects.end();
       it != end;
       ++it) {
    obj_info& obj(*it);
    if (obj._id != id) {
      for (unsigned int i = 0, end = obj.verbosity(); i <= end; ++i)
        _type[i] |= obj.type();
    }
    else
      it_erase = it;
  }
  if (it_erase != _objects.end()) {
    if (it_erase->id() + 1 == id)
      --_id;
    _objects.erase(it_erase);
  }
  _rwlock.unlock();
}

/**
 *  Remove an object logging.
 *
 *  @param[in] id The object info.
 */
void engine::remove_object(obj_info& obj) throw() {
  remove_object(obj.id());
  obj._id = 0;
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
  memset(_type, 0, sizeof(_type));
  std::vector<obj_info>::iterator it_erase = _objects.end();
  for (std::vector<obj_info>::iterator it = _objects.begin(),
         end = _objects.end();
       it != end;
       ++it) {
    obj_info& obj(*it);
    if (obj._id != id) {
      for (unsigned int i = 0, end = obj.verbosity(); i <= end; ++i)
        _type[i] |= obj.type();
    }
    else
      it_erase = it;
  }
  if (it_erase != _objects.end()) {
    obj_info& info(*it_erase);
    info._type = type;
    info._verbosity = verbosity;
    _type[verbosity] |= type;
  }
  _rwlock.unlock();
}

/**
 *  Default constructor.
 */
engine::obj_info::obj_info()
  : _type(0), _id(0), _verbosity(0) {

}

/**
 *  Constructor.
 *
 *  @param[in] _obj       Pointer on object logging.
 *  @param[in] _type      Message type to log with this object.
 *  @param[in] _verbosity Verbosity level.
 */
engine::obj_info::obj_info(QSharedPointer<object> obj,
			   unsigned long long type,
			   unsigned int verbosity)
  : _obj(obj), _type(type), _verbosity(verbosity) {

}

/**
 *  Default copy constructor.
 *
 *  @param[in] right The class to copy.
 */
engine::obj_info::obj_info(obj_info const& right)
  : _type(0), _id(0), _verbosity(0) {
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
    _obj = right._obj;
    _type = right._type;
    _id = right._id;
    _verbosity = right._verbosity;
  }
  return (*this);
}

/**
 *  Get the type.
 *
 *  @return The Type.
 */
unsigned long long engine::obj_info::type() const throw() {
  return (_type);
}

/**
 *  Get the id.
 *
 *  @return The id.
 */
unsigned long engine::obj_info::id() const throw() {
  return (_id);
}

/**
 *  Get the verbosity.
 *
 *  @return The verbosity.
 */
unsigned int engine::obj_info::verbosity() const throw() {
  return (_verbosity);
}
