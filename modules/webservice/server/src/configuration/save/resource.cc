/*
** Copyright 2012 Merethis
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

#include <fstream>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/macros/defines.hh"
#include "com/centreon/engine/modules/webservice/configuration/save/resource.hh"

using namespace com::centreon::engine::modules::webservice::configuration::save;

/**
 *  Default constructor.
 */
resource::resource() {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right  The object to copy.
 */
resource::resource(resource const& right) {
  _internal_copy(right);
}

/**
 *  Destructor.
 */
resource::~resource() throw () {

}

/**
 *  Assignment operator.
 *
 *  @param[in] right  The object to copy.
 *
 *  @return This object.
 */
resource& resource::operator=(resource const& right) {
  return (_internal_copy(right));
}

/**
 *  Add resource array to store.
 *
 *  @param[in] resources  The array of resources.
 */
void resource::add_resource(char* const* resources) {
  if (!resources)
    return;
  for (unsigned int i(0); i < MAX_USER_MACROS; ++i)
    if (resources[i])
      _stream << "$USER" << i + 1 << "$=" << resources[i] << std::endl;
}

/**
 *  Save into the file the current configuration.
 *
 *  @param[in] filename  The file path to save buffer.
 */
void resource::backup(std::string const& filename) const {
  std::ofstream output(filename.c_str());
  if (!output.is_open())
    throw (engine_error() << "save resource: open file '"
           << filename << "' failed");
  output << _stream.str();
}

/**
 *  Clear buffer.
 */
void resource::clear() {
  _stream.str("");
}

/**
 *  Get the save string.
 *
 *  @return The save string.
 */
std::string resource::to_string() const {
  return (_stream.str());
}

/**
 *  Internal copy.
 *
 *  @param[in] right  The object to copy.
 *
 *  @return This object.
 */
resource& resource::_internal_copy(resource const& right) {
  if (&right != this) {
    _stream.str(right._stream.str());
  }
  return (*this);
}
