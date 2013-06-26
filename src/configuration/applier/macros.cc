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

#include <cstring>
#include "com/centreon/engine/configuration/applier/macros.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/macros/misc.hh"
#include "com/centreon/engine/misc/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;

static applier::macros* _instance = NULL;

/**
 *  Apply new configuration.
 *
 *  @param[in] config The new configuration.
 */
void applier::macros::apply(state& config) {
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
}

/**
 *  Get the singleton instance of macros applier.
 *
 *  @return Singleton instance.
 */
applier::macros& applier::macros::instance() {
  return (*_instance);
}

/**
 *  Load macros applier singleton.
 */
void applier::macros::load() {
  if (!_instance)
    _instance = new applier::macros;
}

/**
 *  Unload macros applier singleton.
 */
void applier::macros::unload() {
  delete _instance;
  _instance = NULL;
}

/**
 *  Default constructor.
 */
applier::macros::macros()
  : _mac(get_global_macros()) {
  memset(_mac, 0, sizeof(*_mac));
  memset(macro_user, 0, sizeof(*macro_user) * MAX_USER_MACROS);

  _set_macro(MACRO_TEMPFILE, "/tmp/centengine.tmp");
  _set_macro(MACRO_TEMPPATH, "/tmp");
}

/**
 *  Destructor.
 */
applier::macros::~macros() throw() {
  delete[] _mac->x[MACRO_ADMINEMAIL];
  delete[] _mac->x[MACRO_ADMINPAGER];
  delete[] _mac->x[MACRO_COMMANDFILE];
  delete[] _mac->x[MACRO_LOGFILE];
  delete[] _mac->x[MACRO_MAINCONFIGFILE];
  delete[] _mac->x[MACRO_RESOURCEFILE];
  delete[] _mac->x[MACRO_TEMPFILE];
  delete[] _mac->x[MACRO_TEMPPATH];

  delete[] _mac->x[MACRO_OBJECTCACHEFILE];
  delete[] _mac->x[MACRO_PROCESSSTARTTIME];
  delete[] _mac->x[MACRO_EVENTSTARTTIME];
  delete[] _mac->x[MACRO_RETENTIONDATAFILE];
  delete[] _mac->x[MACRO_STATUSDATAFILE];

  for (unsigned int i(0); i < MAX_USER_MACROS; ++i) {
    delete[] macro_user[i];
    macro_user[i] = NULL;
  }
}

/**
 *  Set the global macros.
 *
 *  @param[in] type  The type of macros to set.
 *  @param[in] value The value of the macro.
 */
void applier::macros::_set_macro(
       unsigned int type,
       std::string const& value) {
  if (type >= MACRO_X_COUNT)
    throw (engine_error() << "applier: invalid type of global macro");
  if (!_mac->x[type] || strcmp(_mac->x[type], value.c_str()))
    misc::setstr(_mac->x[type], value);
}

/**
 *  Set the user macros.
 *
 *  @param[in] idx   The index of the user macro to set.
 *  @param[in] value The value of the macro.
 */
void applier::macros::_set_macros_user(
       unsigned int idx,
       std::string const& value) {
  if (idx >= MAX_USER_MACROS)
    throw (engine_error() << "applier: invalid index of user macro");
  if (!macro_user[idx] || strcmp(macro_user[idx], value.c_str()))
    misc::setstr(macro_user[idx], value);
}
