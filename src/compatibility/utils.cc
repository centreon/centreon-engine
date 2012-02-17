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

check_result  check_result_info;
check_result* check_result_list = NULL;

/******************************************************************/
/************************* IPC FUNCTIONS **************************/
/******************************************************************/

/* deletes as check result file, as well as its ok-to-go file */
int delete_check_result_file(char const* fname) {
  /* delete the result file */
  unlink(fname);

  /* delete the ok-to-go file */
  std::string temp_buffer(fname);
  temp_buffer.append(".ok");

  unlink(temp_buffer.c_str());
  return (OK);
}

/* initializes a host/service check result */
int init_check_result(check_result* info) {
  if (info == NULL)
    return (ERROR);

  /* reset vars */
  info->object_check_type = HOST_CHECK;
  info->host_name = NULL;
  info->service_description = NULL;
  info->check_type = HOST_CHECK_ACTIVE;
  info->check_options = CHECK_OPTION_NONE;
  info->scheduled_check = FALSE;
  info->reschedule_check = FALSE;
  info->output_file_fp = NULL;
  info->output_file_fd = -1;
  info->latency = 0.0;
  info->start_time.tv_sec = 0;
  info->start_time.tv_usec = 0;
  info->finish_time.tv_sec = 0;
  info->finish_time.tv_usec = 0;
  info->early_timeout = FALSE;
  info->exited_ok = TRUE;
  info->return_code = 0;
  info->output = NULL;
  info->next = NULL;

  return (OK);
}

/* adds a new host/service check result to the list in memory */
int add_check_result_to_list(check_result* new_cr) {
  check_result* temp_cr = NULL;
  check_result* last_cr = NULL;

  if (new_cr == NULL)
    return (ERROR);

  /* add to list, sorted by finish time (asc) */

  /* find insertion point*/
  last_cr = check_result_list;
  for (temp_cr = check_result_list; temp_cr != NULL; temp_cr = temp_cr->next) {
    if (temp_cr->finish_time.tv_sec >= new_cr->finish_time.tv_sec) {
      if (temp_cr->finish_time.tv_sec > new_cr->finish_time.tv_sec)
        break;
      else if (temp_cr->finish_time.tv_usec > new_cr->finish_time.tv_usec)
        break;
    }
    last_cr = temp_cr;
  }

  /* item goes at head of list */
  if (check_result_list == NULL || temp_cr == check_result_list) {
    new_cr->next = check_result_list;
    check_result_list = new_cr;
  }

  /* item goes in middle or at end of list */
  else {
    new_cr->next = temp_cr;
    last_cr->next = new_cr;
  }

  return (OK);
}

/* reads check result(s) from a file */
int process_check_result_file(char* fname) {
  mmapfile* thefile = NULL;
  char* input = NULL;
  char* var = NULL;
  char* val = NULL;
  char* v1 = NULL;
  char* v2 = NULL;
  time_t current_time;
  check_result* new_cr = NULL;

  if (fname == NULL)
    return (ERROR);

  time(&current_time);

  logger(dbg_checks, more) << "Processing check result file: '" << fname << "'";

  /* open the file for reading */
  if ((thefile = mmap_fopen(fname)) == NULL) {
    /* try removing the file - zero length files can't be mmap()'ed, so it might exist */
    unlink(fname);

    return (ERROR);
  }

  /* read in all lines from the file */
  while (1) {

    /* free memory */
    delete[] input;

    /* read the next line */
    if ((input = mmap_fgets_multiline(thefile)) == NULL)
      break;

    /* skip comments */
    if (input[0] == '#')
      continue;

    /* empty line indicates end of record */
    else if (input[0] == '\n') {
      /* we have something... */
      if (new_cr) {
        /* do we have the minimum amount of data? */
        if (new_cr->host_name != NULL && new_cr->output != NULL) {

          /* add check result to list in memory */
          add_check_result_to_list(new_cr);

          /* reset pointer */
          new_cr = NULL;
        }

        /* discard partial input */
        else {
          free_check_result(new_cr);
          init_check_result(new_cr);
          new_cr->output_file = my_strdup(fname);
        }
      }
    }

    if ((var = my_strtok(input, "=")) == NULL)
      continue;
    if ((val = my_strtok(NULL, "\n")) == NULL)
      continue;

    /* allocate new check result if necessary */
    if (new_cr == NULL) {
      new_cr = new check_result;

      /* init values */
      init_check_result(new_cr);
      new_cr->output_file = my_strdup(fname);
    }

    if (!strcmp(var, "host_name"))
      new_cr->host_name = my_strdup(val);
    else if (!strcmp(var, "service_description")) {
      new_cr->service_description = my_strdup(val);
      new_cr->object_check_type = SERVICE_CHECK;
    }
    else if (!strcmp(var, "check_type"))
      new_cr->check_type = atoi(val);
    else if (!strcmp(var, "check_options"))
      new_cr->check_options = atoi(val);
    else if (!strcmp(var, "scheduled_check"))
      new_cr->scheduled_check = atoi(val);
    else if (!strcmp(var, "reschedule_check"))
      new_cr->reschedule_check = atoi(val);
    else if (!strcmp(var, "latency"))
      new_cr->latency = strtod(val, NULL);
    else if (!strcmp(var, "start_time")) {
      if ((v1 = strtok(val, ".")) == NULL)
	continue;
      if ((v2 = strtok(NULL, "\n")) == NULL)
	continue;
      new_cr->start_time.tv_sec = strtoul(v1, NULL, 0);
      new_cr->start_time.tv_usec = strtoul(v2, NULL, 0);
    }
    else if (!strcmp(var, "finish_time")) {
      if ((v1 = strtok(val, ".")) == NULL)
	continue;
      if ((v2 = strtok(NULL, "\n")) == NULL)
	continue;
      new_cr->finish_time.tv_sec = strtoul(v1, NULL, 0);
      new_cr->finish_time.tv_usec = strtoul(v2, NULL, 0);
    }
    else if (!strcmp(var, "early_timeout"))
      new_cr->early_timeout = atoi(val);
    else if (!strcmp(var, "exited_ok"))
      new_cr->exited_ok = atoi(val);
    else if (!strcmp(var, "return_code"))
      new_cr->return_code = atoi(val);
    else if (!strcmp(var, "output"))
      new_cr->output = my_strdup(val);
  }

  /* we have something */
  if (new_cr) {
    /* do we have the minimum amount of data? */
    if (new_cr->host_name != NULL && new_cr->output != NULL) {

      /* add check result to list in memory */
      add_check_result_to_list(new_cr);

      /* reset pointer */
      new_cr = NULL;
    }

    /* discard partial input */
    /* free memory for current check result record */
    else {
      free_check_result(new_cr);
      delete new_cr;
    }
  }

  /* free memory and close file */
  delete[] input;
  mmap_fclose(thefile);

  /* delete the file (as well its ok-to-go file) if it's too old */
  /* other (current) files are deleted later (when results are processed) */
  delete_check_result_file(fname);

  return (OK);
}

/* processes files in the check result queue directory */
int process_check_result_queue(char const* dirname) {
  char file[MAX_FILENAME_LENGTH];
  DIR* dirp = NULL;
  struct dirent* dirfile = NULL;
  int x = 0;
  struct stat stat_buf;
  struct stat ok_stat_buf;
  int result = OK;

  /* make sure we have what we need */
  if (dirname == NULL) {
    logger(log_config_error, basic)
      << "Error: No check result queue directory specified.";
    return (ERROR);
  }

  /* open the directory for reading */
  if ((dirp = opendir(dirname)) == NULL) {
    logger(log_config_error, basic)
      << "Error: Could not open check result queue directory '"
      << dirname << "' for reading.";
    return (ERROR);
  }

  logger(dbg_checks, more)
    << "Starting to read check result queue '" << dirname << "'...";

  /* process all files in the directory... */
  while ((dirfile = readdir(dirp)) != NULL) {

    /* create /path/to/file */
    snprintf(file, sizeof(file), "%s/%s", dirname, dirfile->d_name);
    file[sizeof(file) - 1] = '\x0';

    /* process this if it's a check result file... */
    x = strlen(dirfile->d_name);
    if (x == 7 && dirfile->d_name[0] == 'c') {

      if (stat(file, &stat_buf) == -1) {
        logger(log_runtime_warning, basic)
          << "Warning: Could not stat() check result file '" << file << "'.";
        continue;
      }

      switch (stat_buf.st_mode & S_IFMT) {
      case S_IFREG:
        /* don't process symlinked files */
        if (!S_ISREG(stat_buf.st_mode))
          continue;
        break;

      default:
        /* everything else we ignore */
        continue;
      }

      /* at this point we have a regular file... */

      /* can we find the associated ok-to-go file ? */
      std::string temp_buffer(file);
      temp_buffer.append(".ok");
      result = stat(temp_buffer.c_str(), &ok_stat_buf);
      if (result == -1)
        continue;

      /* process the file */
      result = process_check_result_file(file);

      /* break out if we encountered an error */
      if (result == ERROR)
        break;
    }
  }

  closedir(dirp);

  return (result);
}

/* reads the first host/service check result from the list in memory */
check_result* read_check_result(void) {
  check_result* first_cr = NULL;

  if (check_result_list == NULL)
    return (NULL);

  first_cr = check_result_list;
  check_result_list = check_result_list->next;

  return (first_cr);
}

/* frees all memory associated with the check result list */
int free_check_result_list(void) {
  check_result* this_cr = NULL;
  check_result* next_cr = NULL;

  for (this_cr = check_result_list; this_cr != NULL; this_cr = next_cr) {
    next_cr = this_cr->next;
    free_check_result(this_cr);
    delete this_cr;
  }

  check_result_list = NULL;

  return (OK);
}

/* move check result to queue directory */
int move_check_result_to_queue(char* checkresult_file) {
  char* output_file = NULL;
  int output_file_fd = -1;
  mode_t new_umask = 077;
  mode_t old_umask;
  int result = 0;

  /* save the file creation mask */
  old_umask = umask(new_umask);

  /* create a safe temp file */
  std::ostringstream oss;
  extern char const* check_result_path;
  oss << check_result_path << "/cXXXXXX";
  output_file = my_strdup(oss.str().c_str());
  output_file_fd = mkstemp(output_file);

  /* file created okay */
  if (output_file_fd >= 0) {

    logger(dbg_checks, most)
      << "Moving temp check result file '" << checkresult_file
      << "' to queue file '" << output_file << "'...";

#ifdef __CYGWIN__
    /* Cygwin cannot rename open files - gives Permission Denied */
    /* close the file */
    close(output_file_fd);
#endif

    /* move the original file */
    result = my_rename(checkresult_file, output_file);

#ifndef __CYGWIN__
    /* close the file */
    close(output_file_fd);
#endif

    /* create an ok-to-go indicator file */
    std::string temp_buffer(output_file);
    temp_buffer.append(".ok");
    if ((output_file_fd = open(temp_buffer.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR)) > 0)
      close(output_file_fd);

    /* delete the original file if it couldn't be moved */
    if (result != 0)
      unlink(checkresult_file);
  }
  else
    result = -1;

  /* reset the file creation mask */
  umask(old_umask);

  /* log a warning on errors */
  if (result != 0)
    logger(log_runtime_warning, basic)
      << "Warning: Unable to move file '" << checkresult_file
      << "' to check results queue.";

  /* free memory */
  delete[] output_file;

  return (OK);
}

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
