/*
** Copyright 1999-2009 Ethan Galstad
** Copyright 2009-2011 Nagios Core Development Team and Community Contributors
** Copyright 2011      Merethis
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

#ifndef CCE_COMPATIBILITY_COMMON_H
# define CCE_COMPATIBILITY_COMMON_H

# include "shared.h"
# include "common.hh"

/* my_free has been freed from bondage as a function */
# ifndef my_free
#  include <stdlib.h>
#  define my_free(ptr) do { if(ptr) { free(ptr); ptr = NULL; } } while(0)
# endif /* !my_free */

#endif /* !CCE_COMPATIBILITY_COMMON_H */
