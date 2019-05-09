/*
** Copyright 2019 Centreon
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

#include "com/centreon/engine/customvariable.hh"

using namespace com::centreon::engine;

/**
 *  Constructor
 *
 *  @param value The value of the customvariable stored as a string
 *  @param is_sent A boolean telling if this custom variable must be sent to
 *  broker
 */
customvariable::customvariable(std::string const& value, bool is_sent)
  : _value{value}, _is_sent{is_sent}, _modified{true} {}

/**
 *  Constructor
 *
 *  @param value The value of the customvariable stored as a string
 *  @param is_sent A boolean telling if this custom variable must be sent to
 *  broker
 */
customvariable::customvariable(std::string&& value, bool is_sent)
  : _value{value}, _is_sent{is_sent}, _modified{true} {}

/**
 *  Copy constructor
 *
 * @param other Another customvariable
 */
customvariable::customvariable(customvariable const& other)
  : _value{other._value}, _is_sent{other._is_sent}, _modified{other._modified} {}

/**
 *  Affectation operator of customvariable
 *
 * @param other Another customvariable
 *
 * @return A reference to the newly affected customvariable
 */
customvariable& customvariable::operator=(customvariable const& other) {
  if (this != &other) {
    _value = other._value;
    _is_sent = other._is_sent;
  }
  return *this;
}

/**
 *  Customvariable destructor
 */
customvariable::~customvariable() {}

/**
 *  Setter of the sent boolean. If true, this customvariable will be sent to
 *  centreon-broker.
 *
 * @param sent A boolean telling if yes or no this customvariable must be sent.
 */
void customvariable::set_sent(bool sent) {
  _is_sent = sent;
}

/**
 *  Getter to the sent boolean. If true, this customvariable is sent.
 *
 * @return a boolean
 */
bool customvariable::is_sent() const {
  return _is_sent;
}

/**
 *  Value accessor
 *
 * @return The value of the customvariable
 */
std::string const& customvariable::get_value() const {
  return _value;
}

/**
 *  Setter of the customvariable value
 *
 * @param value The new value to affect.
 */
void customvariable::set_value(std::string const& value) {
  _value = value;
}

/**
 *  Equality comparison operator of the customvariable
 *
 * @param other Another customvariable
 *
 * @return a boolean telling if they are equal or not.
 */
bool customvariable::operator==(customvariable const& other) const {
  return _value == other._value && _is_sent == other._is_sent;
}

/**
 *  Order comparison operator of the customvariable
 *
 *  The comparison is made in this order:
 *  * the name of the customvariable
 *  * the value of the customvariable
 *  * the flag telling if the customvariable is sent
 *
 * @param other Another customvariable
 *
 * @return A boolean telling if this customvariable is before the other one.
 */
bool customvariable::operator<(customvariable const& other) const {
  return _value < other._value || _is_sent < other._is_sent;
}

/**
 *  Difference comparison operator of the customvariable
 *
 * @param other The customvariable to compare with.
 *
 * @return a boolean telling if the two customvariable are different.
 */
bool customvariable::operator!=(customvariable const& other) {
  return _value != other._value || _is_sent != other._is_sent;
}

bool customvariable::has_been_modified() const {
  return _modified;
}

/**
 *  This is the official way to update a custom variable. It checks if the new
 *  value has changed, and in that case it also sets the has_been_modified flag
 *  to <i>true</i>.
 *
 * @param value The new value of the custom variable.
 */
void customvariable::update(std::string const& value) {
  if (_value != value) {
    _value = value;
    _modified = true;
  }
}
