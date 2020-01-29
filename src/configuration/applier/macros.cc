/*
** Copyright 2011-2013,2016 Centreon
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

#include "com/centreon/engine/configuration/applier/macros.hh"
#include <cassert>
#include <cstring>
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/macros/misc.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;

/**
 *  @brief Is the key of this user macro old-style?
 *
 *  i.e USERn where n is a number.
 *
 *  @param[in] key   The key.
 *  @param[out] val  The parsed value n, if applicable.
 *
 *  @return  True if the key is old-style and has been parsed succesfully.
 */
static bool is_old_style_user_macro(std::string const& key, unsigned int& val) {
  if (::strncmp(key.c_str(), "USER", ::strlen("USER")) != 0)
    return (false);

  std::string rest = key.substr(4);
  // Super strict validation.
  for (size_t i = 0; i < rest.size(); ++i)
    if (rest[i] < '0' || rest[i] > '9')
      return (false);
  string::to(rest.c_str(), val);
  return (true);
}

/**
 *  Apply new configuration.
 *
 *  @param[in] config The new configuration.
 */
void applier::macros::apply(configuration::state& config) {
  _set_macro(MACRO_ADMINEMAIL, config.admin_email());
  _set_macro(MACRO_ADMINPAGER, config.admin_pager());
  _set_macro(MACRO_COMMANDFILE, config.command_file());
  _set_macro(MACRO_LOGFILE, config.log_file());
  _set_macro(MACRO_MAINCONFIGFILE, config.cfg_main());
  if (config.resource_file().size() > 0)
    _set_macro(MACRO_RESOURCEFILE, config.resource_file().front());
  _set_macro(MACRO_STATUSDATAFILE, config.status_file());
  _set_macro(MACRO_HOSTPERFDATAFILE, config.host_perfdata_file());
  _set_macro(MACRO_SERVICEPERFDATAFILE, config.service_perfdata_file());
  _set_macro(MACRO_POLLERNAME, config.poller_name());
  _set_macro(MACRO_POLLERID, std::to_string(config.poller_id()));

  std::unordered_map<std::string, std::string> const& users(config.user());
  applier::state::instance().user_macros() = users;
  // Save old style user macros into old style structures.
  for (std::unordered_map<std::string, std::string>::const_iterator
           it = users.begin(),
           end = users.end();
       it != end; ++it) {
    unsigned int val(1);
    if (is_old_style_user_macro(it->first, val))
      _set_macros_user(val - 1, it->second);
  }
}

/**
 *  Get the singleton instance of macros applier.
 *
 *  @return Singleton instance.
 */
applier::macros& applier::macros::instance() {
  static applier::macros instance;
  return instance;
}

void applier::macros::clear() {
  clear_volatile_macros_r(_mac);
  free_macrox_names();

  for (unsigned int i(0); i < MAX_USER_MACROS; ++i) {
    macro_user[i] = "";
  }

  _mac = get_global_macros();
  init_macros();

  _set_macro(MACRO_TEMPFILE, "/tmp/centengine.tmp");
  _set_macro(MACRO_TEMPPATH, "/tmp");
}

/**
 *  Default constructor.
 */
applier::macros::macros() : _mac(get_global_macros()) {
  init_macros();

  _set_macro(MACRO_TEMPFILE, "/tmp/centengine.tmp");
  _set_macro(MACRO_TEMPPATH, "/tmp");
}

/**
 *  Destructor.
 */
applier::macros::~macros() throw() {
  clear_volatile_macros_r(_mac);
  free_macrox_names();

  for (unsigned int i(0); i < MAX_USER_MACROS; ++i) {
    macro_user[i] = "";
  }
}

/**
 *  Set the global macros.
 *
 *  @param[in] type  The type of macros to set.
 *  @param[in] value The value of the macro.
 */
void applier::macros::_set_macro(unsigned int type, std::string const& value) {
  if (type >= MACRO_X_COUNT)
    throw(engine_error() << "Invalid type of global macro: " << type);
  if (_mac->x[type] != value)
    _mac->x[type] = value;
}

/**
 *  Set the user macros.
 *
 *  @param[in] idx   The index of the user macro to set.
 *  @param[in] value The value of the macro.
 */
void applier::macros::_set_macros_user(unsigned int idx,
                                       std::string const& value) {
  if (idx >= MAX_USER_MACROS)
    throw(engine_error() << "Invalid index of user macro: " << idx);
  if (macro_user[idx] != value)
    macro_user[idx] = value;
}
