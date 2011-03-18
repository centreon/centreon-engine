/*
** Copyright 2001-2005 Ethan Galstad
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

#ifndef PERFDATA_H
# define PERFDATA_H

# include "objects.h"

# ifdef __cplusplus
  extern "C" {
# endif

int initialize_performance_data(char *);	                /* initializes performance data */
int cleanup_performance_data(char *);                           /* cleans up performance data */

int update_host_performance_data(host *);       	        /* updates host performance data */
int update_service_performance_data(service *);         	/* updates service performance data */

# ifdef __cplusplus
  }
# endif

#endif /* !PERFDATA_H */
