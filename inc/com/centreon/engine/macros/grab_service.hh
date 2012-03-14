/*
** Copyright 1999-2010 Ethan Galstad
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

#ifndef CCE_MACROS_GRAB_SERVICE_HH
#  define CCE_MACROS_GRAB_SERVICE_HH

#  include "com/centreon/engine/macros/defines.hh"
#  include "com/centreon/engine/objects.hh"

#  ifdef __cplusplus
extern "C" {
#  endif // C++

int grab_standard_service_macro_r(
      nagios_macros* mac,
      int macro_type,
      service* svc,
      char** output,
      int* free_macro);
int grab_standard_service_macro(
      int macro_type,
      service* svc,
      char** output,
      int* free_macro);
int grab_service_macros_r(nagios_macros* mac, service* svc);
int grab_service_macros(service* svc);

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_MACROS_GRAB_SERVICE_HH
