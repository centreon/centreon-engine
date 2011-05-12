/*
** Copyright 2011 Merethis
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

#include "argument.hh"

using namespace com::centreon::engine::script;

/**
 *  Default constructor.
 *
 *  @param[in] type The variable type.
 *  @param[in] name The variable name.
 *  @param[in] help The explicit name.
 */
argument::argument(QString const& type,
		   QString const& name,
		   QString const& help)
  : _type(type), _name(name), _help(help) {
  if (_help == "") {
    _help = _name;
  }
}

/**
 *  Default copy constructor.
 *
 *  @param[in] right The object to copy.
 */
argument::argument(argument const& right) {
  operator=(right);
}

/**
 *  Default destructor.
 */
argument::~argument() throw() {

}

/**
 *  Default copy operator.
 *
 *  @param[in] right The object to copy.
 *
 *  @return Return This object.
 */
argument& argument::operator=(argument const& right) {
  if (this != &right) {
    _name = right._name;
    _type = right._type;
    _help = right._help;
    _list = right._list;
  }
  return (*this);
}

/**
 *  Default equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return Return true is the same object, false otherwise.
 */
bool argument::operator==(argument const& right) const throw() {
  return (_name == right._name
	  && _type == right._type
	  && _help == right._help
	  && _list == right._list);
}

/**
 *  Default not equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return Return true is different object, false otherwise.
 */
bool argument::operator!=(argument const& right) const throw() {
  return (!operator==(right));
}

/**
 *  Get the variable type.
 *
 *  @return Return The variable type.
 */
QString const& argument::get_type() const throw() {
  return (_type);
}

/**
 *  Get the variable name.
 *
 *  @return The variable name.
 */
QString const& argument::get_name() const throw() {
  return (_name);
}

/**
 *  Get the explicite name (use in the help).
 *
 *  @return Return the explicite name.
 */
QString const& argument::get_help() const throw() {
  return (_help);
}

/**
 *  Set the variable name.
 *
 *  @param[in] name The variable name.
 *
 *  @return This object.
 */
argument& argument::set_name(QString const& name) {
  if (_name == _help) {
    _help = name;
  }
  _name = name;
  return (*this);
}

/**
 *  Set the explicite name.
 *
 *  @param[in] help The explicite name.
 *
 *  @return Return this object.
 */
argument& argument::set_help(QString const& help) {
  _help = help;
  return (*this);
}

/**
 *  Add a new argument.
 *
 *  @param[in] arg The new argument.
 *
 *  @return Return this object.
 */
argument& argument::add(argument const& arg) {
  _list.push_back(arg);
  return (_list.last());
}

/**
 *  Get all arguments in this argument.
 *
 *  @return Return the list of arguments.
 */
QList<argument> const& argument::get_args() const throw() {
  return (_list);
}

/**
 *  Check if this argument is primitif.
 *
 *  @return Return true is this argument is primitif.
 */
bool argument::is_primitive() const throw() {
  return (_list.size() == 0);
}

