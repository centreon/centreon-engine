/*
** Copyright 2000-2004 Ethan Galstad
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

#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/perfdata.hh"
#include "com/centreon/engine/xpddefault.hh"

/******************************************************************/
/****************** PERFORMANCE DATA FUNCTIONS ********************/
/******************************************************************/

/* updates host performance data */
int update_host_performance_data(com::centreon::engine::host* hst) {
  /* should we be processing performance data for anything? */
  if (!config->process_performance_data())
    return OK;

  /* should we process performance data for this host? */
  if (!hst->get_process_performance_data())
    return OK;

  /* process the performance data! */
  xpddefault_update_host_performance_data(hst);
  return OK;
}
