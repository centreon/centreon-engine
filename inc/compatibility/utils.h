/*
** Copyright 1999-2009 Ethan Galstad
** Copyright 2009-2011 Nagios Core Development Team and Community Contributors
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

#ifndef CCE_COMPATIBILITY_UTILS_H
#define CCE_COMPATIBILITY_UTILS_H

#include "com/centreon/engine/utils.hh"

#ifdef __cplusplus
extern "C" {
#endif  // C++

time_t get_next_log_rotation_time(void);

#ifdef __cplusplus
}
#endif  // C++

#endif  // !CCE_COMPATIBILITY_UTILS_H
