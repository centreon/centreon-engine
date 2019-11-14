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

#include "com/centreon/engine/retention/contact.hh"
#include <ctime>
#include "com/centreon/engine/error.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  Check contact.
 *
 *  @param[in] argc Size of argv array.
 *  @param[in] argv Argumments array.
 *
 *  @return 0 on success.
 */
int main_test(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  retention::contact ref;
  ref.set("contact_name", "contact_name");
  ref.set("host_notification_period", "host_notification_period");
  ref.set("host_notifications_enabled", "1");
  ref.set("last_host_notification", "1300000");
  ref.set("last_service_notification", "1300000");
  ref.set("modified_attributes", "1");
  ref.set("modified_host_attributes", "2");
  ref.set("modified_service_attributes", "3");
  ref.set("service_notification_period", "service_notification_period");
  ref.set("service_notifications_enabled", "1");

  // Check copy constructor.
  {
    retention::contact copy(ref);
    if (!(copy == ref) || copy != ref)
      throw(engine_error() << "copy constructor or operator "
                              "equal failed");
  }

  // Check copy operator.
  {
    retention::contact copy;
    copy = ref;
    if (!(copy == ref) || copy != ref)
      throw(engine_error() << "copy operator or operator "
                              "equal failed");
  }

  // Check operator equal and not equal.
  {
    retention::contact empty;
    if (empty == ref || !(empty != ref))
      throw(engine_error() << "operator equal failed");
  }

  // Check operator equal and not equal.
  {
    retention::contact diff(ref);
    diff.set("contact_name", "new_contact_name");
    if (diff == ref || !(diff != ref))
      throw(engine_error() << "copy operator or operator "
                              "equal failed");
  }
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}
