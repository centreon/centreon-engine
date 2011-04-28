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

engine::engine()
  : _options(0), _id(0), _verbosity_level(0) {

}

engine::~engine() throw() {

}

engine& engine::instance() {
  static engine instance;
  return (instance);
}

void engine::log(char const* message,
		 unsigned long long type,
		 unsigned int verbosity) throw() {
  _mutex.lock();
  if (verbosity <= _verbosity_level && (type & _options)) {
    for (QHash<unsigned long, obj_info>::iterator it = _objects.begin(), end = _objects.end();
	 it != end;
	 ++it) {
      it.value().obj->log(message, type, verbosity);
    }
  }
  _mutex.unlock();
}

unsigned long engine::add_object(obj_info const& info) {
  _mutex.lock();
  unsigned int id = _id++;
  _objects.insert(id, info);
  _mutex.unlock();
  return (id);
}

void engine::remove_object(unsigned long id) throw() {
  _mutex.lock();
  QHash<unsigned long, obj_info>::iterator it = _objects.find(id);
  if (it != _objects.end()) {
    _objects.erase(it);
  }
  _mutex.unlock();
}

void engine::set_verbosity_level(unsigned int level) throw() {
  _mutex.lock();
  _verbosity_level = level;
  _mutex.unlock();
}

unsigned int engine::get_verbosity_level() throw() {
  _mutex.lock();
  unsigned int level = _verbosity_level;
  _mutex.unlock();
  return (level);
}

void engine::set_options(unsigned long long options) throw() {
  _mutex.lock();
  _options = options;
  _mutex.unlock();
}

unsigned long long engine::get_options() throw() {
  _mutex.lock();
  unsigned long long options = _options;
  _mutex.unlock();
  return (options);
}


engine::obj_info::obj_info()
  : type(0), verbosity(0) {

}

engine::obj_info::obj_info(QSharedPointer<object> _obj,
			   unsigned long long _type,
			   unsigned int _verbosity)
  : obj(_obj), type(_type), verbosity(_verbosity) {

}

engine::obj_info::obj_info(obj_info const& right)
  : type(0), verbosity(0) {
  operator=(right);
}

engine::obj_info::~obj_info() throw() {

}

engine::obj_info& engine::obj_info::operator=(obj_info const& right) {
  if (this != &right) {
    obj = right.obj;
    type = right.type;
    verbosity = right.verbosity;
  }
  return (*this);
}
