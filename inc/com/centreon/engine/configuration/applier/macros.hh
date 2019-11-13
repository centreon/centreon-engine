/*
** Copyright 2011-2013 Merethis
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

#ifndef CCE_CONFIGURATION_APPLIER_MACROS_HH
#define CCE_CONFIGURATION_APPLIER_MACROS_HH

#include <string>
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/namespace.hh"

// Forward declaration.
class nagios_macros;

CCE_BEGIN()

namespace configuration {
namespace applier {
/**
 *  @class macros macros.hh
 *  @brief Simple configuration applier for macros class.
 *
 *  Simple configuration applier for macros class.
 */
class macros {
 public:
  void apply(configuration::state& config);
  static macros& instance();
  static void load();
  static void unload();

 private:
  macros();
  macros(macros const&);
  ~macros() throw();
  macros& operator=(macros const&);
  void _set_macro(unsigned int type, std::string const& value);
  void _set_macros_user(unsigned int idx, std::string const& value);

  nagios_macros* _mac;
};
}  // namespace applier
}  // namespace configuration

CCE_END()

#endif  // !CCE_CONFIGURATION_APPLIER_MACROS_HH
