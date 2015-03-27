/*
** Copyright 1999-2009      Ethan Galstad
** Copyright 2011-2013,2015 Merethis
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

#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include "check_result.h"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/engine/utils.hh"
#include "globals.h"
#include "mmap.h"

using namespace com::centreon::engine;

extern "C" {
  /**
   *  @brief Adds a new host/service check result to the list in memory.
   *
   *  Unlike in Nagios, the submitted check_result object might be
   *  processed immediately. Also, the check_result won't be recoverable
   *  using read_check_result() or direct memory browsing after
   *  submission.
   *
   *  @param[in] new_cr New check result.
   *
   *  @return ERROR.
   */
  int add_check_result_to_list(check_result* new_cr) {
    if (new_cr)
      checks::checker::instance().push_check_result(*new_cr);
    return (OK);
  }

  /**
   *  Frees all memory associated with the check result list.
   *
   *  @return OK.
   */
  int free_check_result_list() {
    check_result* next_cr(NULL);
    for (check_result* this_cr = check_result_list;
         this_cr;
         this_cr = next_cr) {
      next_cr = this_cr->next;
      free_check_result(this_cr);
      delete this_cr;
    }
    check_result_list = NULL;
    return (OK);
  }

  /**
   *  Initializes a host/service check result.
   *
   *  @param[in] info Structure to initialize.
   *
   *  @return OK if info is not NULL, ERROR if info is NULL.
   */
  int init_check_result(check_result* info) {
    // Check pointer.
    if (!info)
      return (ERROR);

    // Reset vars.
    info->object_check_type = HOST_CHECK;
    info->host_name = NULL;
    info->service_description = NULL;
    info->check_type = HOST_CHECK_ACTIVE;
    info->check_options = CHECK_OPTION_NONE;
    info->scheduled_check = false;
    info->reschedule_check = false;
    info->output_file_fp = NULL;
    info->output_file_fd = -1;
    info->latency = 0.0;
    info->start_time.tv_sec = 0;
    info->start_time.tv_usec = 0;
    info->finish_time.tv_sec = 0;
    info->finish_time.tv_usec = 0;
    info->early_timeout = false;
    info->exited_ok = true;
    info->return_code = 0;
    info->output = NULL;
    info->next = NULL;

    return (OK);
  }

  /**
   *  Reads the first host/service check result from the list in memory.
   *
   *  @return First host/service check result.
   */
  check_result* read_check_result() {
    check_result* first_cr(check_result_list);
    if (first_cr)
      check_result_list = check_result_list->next;
    return (first_cr);
  }
}
