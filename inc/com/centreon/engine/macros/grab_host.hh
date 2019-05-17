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

#ifndef CCE_MACROS_GRAB_HOST_HH
#  define CCE_MACROS_GRAB_HOST_HH

#  include "com/centreon/engine/host.hh"
#  include "com/centreon/engine/macros/defines.hh"

#  ifdef __cplusplus
extern "C" {
#  endif // C++

int grab_standard_host_macro_r(
      nagios_macros* mac,
      int macro_type,
      com::centreon::engine::host* hst,
      char** output,
      int* free_macro);
int grab_standard_host_macro(
      int macro_type,
      com::centreon::engine::host* hst,
      char** output,
      int* free_macro);
int grab_host_macros_r(nagios_macros* mac, com::centreon::engine::host* hst);
int grab_host_macros(com::centreon::engine::host* hst);

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_MACROS_GRAB_HOST_HH
