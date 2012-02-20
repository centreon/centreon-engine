/*
** Copyright 1999-2009 Ethan Galstad
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

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include "globals.hh"
#include "compatibility/nagios.h"
#include "logging/logger.hh"

using namespace com::centreon::engine::logging;

/******************************************************************/
/******************** SIGNAL HANDLER FUNCTIONS ********************/
/******************************************************************/

/* handle timeouts when executing service checks */
/* 07/16/08 EG also called when parent process gets a TERM signal */
void service_check_sighandler(int sig) {
  struct timeval end_time;

  (void)sig;

  /* get the current time */
  gettimeofday(&end_time, NULL);

#ifdef SERVICE_CHECK_TIMEOUTS_RETURN_UNKNOWN
  check_result_info.return_code = STATE_UNKNOWN;
#else
  check_result_info.return_code = STATE_CRITICAL;
#endif
  check_result_info.finish_time = end_time;
  check_result_info.early_timeout = TRUE;

  /* write check result to file */
  if (check_result_info.output_file_fp) {
    FILE* fp;

    /* avoid races with signal handling */
    fp = check_result_info.output_file_fp;
    check_result_info.output_file_fp = NULL;

    fprintf(fp, "finish_time=%lu.%lu\n",
            static_cast<unsigned long>(check_result_info.finish_time.tv_sec),
            static_cast<unsigned long>(check_result_info.finish_time.tv_usec));
    fprintf(fp, "early_timeout=%d\n",
            check_result_info.early_timeout);
    fprintf(fp, "exited_ok=%d\n",
            check_result_info.exited_ok);
    fprintf(fp, "return_code=%d\n",
            check_result_info.return_code);
    fprintf(fp, "output=%s\n",
            "(Service Check Timed Out)");

    /* close the temp file */
    fclose(fp);

    /* move check result to queue directory */
    move_check_result_to_queue(check_result_info.output_file);
  }

  /* free check result memory */
  free_check_result(&check_result_info);

  /* try to kill the command that timed out by sending termination signal to our process group */
  /* we also kill ourselves while doing this... */
  kill((pid_t) 0, SIGKILL);

  /* force the child process (service check) to exit... */
  _exit(STATE_CRITICAL);
}

/* handle timeouts when executing host checks */
/* 07/16/08 EG also called when parent process gets a TERM signal */
void host_check_sighandler(int sig) {
  struct timeval end_time;

  (void)sig;

  /* get the current time */
  gettimeofday(&end_time, NULL);

  check_result_info.return_code = STATE_CRITICAL;
  check_result_info.finish_time = end_time;
  check_result_info.early_timeout = TRUE;

  /* write check result to file */
  if (check_result_info.output_file_fp) {
    FILE* fp;

    /* avoid races with signal handling */
    fp = check_result_info.output_file_fp;
    check_result_info.output_file_fp = NULL;

    fprintf(fp, "finish_time=%lu.%lu\n",
            static_cast<unsigned long>(check_result_info.finish_time.tv_sec),
            static_cast<unsigned long>(check_result_info.finish_time.tv_usec));
    fprintf(fp, "early_timeout=%d\n",
            check_result_info.early_timeout);
    fprintf(fp, "exited_ok=%d\n",
            check_result_info.exited_ok);
    fprintf(fp, "return_code=%d\n",
            check_result_info.return_code);
    fprintf(fp, "output=%s\n",
            "(Host Check Timed Out)");

    /* close the temp file */
    fclose(fp);

    /* move check result to queue directory */
    move_check_result_to_queue(check_result_info.output_file);
  }

  /* free check result memory */
  free_check_result(&check_result_info);

  /* try to kill the command that timed out by sending termination signal to our process group */
  /* we also kill ourselves while doing this... */
  kill((pid_t) 0, SIGKILL);

  /* force the child process (service check) to exit... */
  _exit(STATE_CRITICAL);
}

/* handle timeouts when executing commands via my_system_r() */
void my_system_sighandler(int sig) {
  (void)sig;

  /* force the child process to exit... */
  _exit(STATE_CRITICAL);
}
