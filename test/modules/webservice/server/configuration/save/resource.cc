/*
** Copyright 2011-2012 Merethis
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

#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/modules/webservice/configuration/save/resource.hh"
#include "com/centreon/engine/globals.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine::modules::webservice;

/**
 *  Check with null pointer.
 *
 *  @return True on success, otherwise false.
 */
static bool check_null_resources() {
  configuration::save::resource save;
  save.add_resource(NULL);
  return (save.to_string().empty());
}

/**
 *  Check with empty resources.
 *
 *  @return True on success, otherwise false.
 */
static bool check_empty_resources() {
  configuration::save::resource save;
  save.add_resource(macro_user);
  return (save.to_string().empty());
}

/**
 *  Check with some data in resources.
 *
 *  @return True on success, otherwise false.
 */
static bool check_with_data_resources() {
  std::ostringstream oss;
  for (unsigned int i(0); i < MAX_USER_MACROS; i += 2) {
    oss << "$USER" << (i + 1) << "$=" << __func__ << std::endl;
    delete[] macro_user[i];
    macro_user[i] = my_strdup(__func__);
  }
  configuration::save::resource save;
  save.add_resource(macro_user);
  return (save.to_string() == oss.str());
}

/**
 *  Check full data in resources.
 *
 *  @return True on success, otherwise false.
 */
static bool check_full_resources() {
  std::ostringstream oss;
  for (unsigned int i(0); i < MAX_USER_MACROS; ++i) {
    oss << "$USER" << (i + 1) << "$=" << __func__ << std::endl;
    delete[] macro_user[i];
    macro_user[i] = my_strdup(__func__);
  }
  configuration::save::resource save;
  save.add_resource(macro_user);
  return (save.to_string() == oss.str());
}

/**
 *  Check the save resources configuration.
 */
static int check_save_resources(int argc, char** argv) {
  (void)argc;
  (void)argv;

  if (!check_null_resources())
    throw (engine_error() << "check_null_resources failed.");
  if (!check_empty_resources())
    throw (engine_error() << "check_empty_resources failed.");
  if (!check_with_data_resources())
    throw (engine_error() << "check_with_data_resources failed.");
  if (!check_full_resources())
    throw (engine_error() << "check_full_resources failed.");
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  com::centreon::engine::unittest utest(argc, argv, &check_save_resources);
  return (utest.run());
}
