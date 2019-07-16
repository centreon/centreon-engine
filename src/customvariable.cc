/*
 * Copyright 2019 Centreon (https://www.centreon.com/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For more information : contact@centreon.com
 *
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
