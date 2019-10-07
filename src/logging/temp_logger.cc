/*
** Copyright 2011-2013 Centreon
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
**
** For more information : contact@centreon.com
*/

#include "com/centreon/engine/logging/temp_logger.hh"

using namespace com::centreon::engine::logging;

/**
 *  Default constrcutor.
 *
 *  @param[in] type     Logging types.
 *  @param[in] verbose  Verbosity level.
 */
temp_logger::temp_logger(unsigned long long type, unsigned int verbose) throw()
    : _engine(engine::instance()), _type(type), _verbose(verbose) {}

/**
 *  Default copy constrcutor.
 *
 *  @param[in] right  The object to copy.
 */
temp_logger::temp_logger(temp_logger const& right) : _engine(right._engine) {
  _internal_copy(right);
}

/**
 *  Default destructor.
 */
temp_logger::~temp_logger() throw() {
  _engine.log(_type, _verbose, _buffer.data(), _buffer.size());
}

/**
 *  Default copy operator.
 *
 *  @param[in] right  The object to copy.
 *
 *  @return This object.
 */
temp_logger& temp_logger::operator=(temp_logger const& right) {
  return (_internal_copy(right));
}

/**
 *  Set float precision.
 *
 *  @param[in] obj The new precision.
 *
 *  @return This object.
 */
temp_logger& temp_logger::operator<<(setprecision const& obj) throw() {
  _buffer.precision(obj.precision);
  return (*this);
}

/**
 *  Internal copy.
 *
 *  @param[in] right  The object to copy.
 *
 *  @return This object.
 */
temp_logger& temp_logger::_internal_copy(temp_logger const& right) {
  if (this != &right) {
    _buffer = right._buffer;
    _type = right._type;
    _verbose = right._verbose;
  }
  return (*this);
}
