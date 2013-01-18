/*
** Copyright 1999-2009 Ethan Galstad
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

#include "common.h"

/**
 *  @brief Get the Centreon Engine last modification date.
 *
 *  This value is no longer maintained. The last modification date is
 *  set to 01-01-1970.
 *
 *  @return 01-01-1970.
 */
char const* get_program_modification_date() {
  return (PROGRAM_MODIFICATION_DATE);
}

/**
 *  Get the Centreon Engine version.
 *
 *  @return Centreon Engine version.
 */
char const* get_program_version() {
  return (PROGRAM_VERSION);
}
