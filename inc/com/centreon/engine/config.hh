/*
** Copyright 2002-2006 Ethan Galstad
** Copyright 2011-2019 Centreon
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

#ifndef CCE_CONFIG_HH
#  define CCE_CONFIG_HH

#  include "com/centreon/engine/objects.hh"
#  include "com/centreon/engine/servicegroup.hh"
#  include "com/centreon/engine/timeperiod.hh"

#  ifdef __cplusplus
extern "C" {
#  endif // C++

// Detects circular dependencies and paths.
int pre_flight_circular_check(int* w, int* e);

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_CONFIG_HH
