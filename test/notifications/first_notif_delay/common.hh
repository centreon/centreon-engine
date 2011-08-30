/*
** Copyright 2011 Merethis
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

#ifndef CCE_FIRST_NOTIF_DELAY_COMMON_HH_
# define CCE_FIRST_NOTIF_DELAY_COMMON_HH_

# define FIRST_NOTIF_DELAY 3
# define FLAG_FILE "/tmp/centreon_engine_unit_test.tmp"

namespace                  com {
  namespace                centreon {
    namespace              engine {
      int                  first_notif_delay_default_setup();
    }
  }
}

#endif /* !CCE_FIRST_NOTIF_DELAY_COMMON_HH_ */
