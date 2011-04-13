/*
** Copyright 2000-2008 Ethan Galstad
** Copyright 2011      Merethis
**
** This file is part of Centreon Scheduler.
**
** Centreon Scheduler is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Scheduler is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Scheduler. If not, see
** <http://www.gnu.org/licenses/>.
*/

#ifndef CCS_COMPATIBILITY_DOWNTIME_H
# define CCS_COMPATIBILITY_DOWNTIME_H

# include "config.h"
# include "common.h"
# include "objects.h"
# include "downtime.hh"

/*
** If you are going to be adding a lot of downtime in sequence,
** set defer_downtime_sorting to 1 before you start and then
** call sort_downtime afterwards. Things will go MUCH faster.
*/
extern int defer_downtime_sorting;

#endif /* !CCS_COMPATIBILITY_DOWNTIME_H */
