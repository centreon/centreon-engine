/*
** Copyright 1999-2010 Ethan Galstad
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

#include "com/centreon/engine/macros/clear_servicegroup.hh"
#include "com/centreon/engine/macros/defines.hh"
#include "com/centreon/engine/macros/misc.hh"

extern "C" {

/**
 *  Clear servicegroup macros.
 *
 *  @param[in,out] mac Macro object.
 *
 *  @return OK on success.
 */
int clear_servicegroup_macros_r(nagios_macros* mac) {
  static unsigned int const to_free[] = {
    MACRO_SERVICEGROUPNAME,
    MACRO_SERVICEGROUPALIAS,
    MACRO_SERVICEGROUPMEMBERS,
    MACRO_SERVICEGROUPACTIONURL,
    MACRO_SERVICEGROUPNOTESURL,
    MACRO_SERVICEGROUPNOTES
  };
  for (unsigned int i = 0;
       i < sizeof(to_free) / sizeof(*to_free);
       ++i) {
    mac->x[i] = "";
  }

  // Clear pointers.
  mac->servicegroup_ptr = nullptr;

  return (OK);
}

}
