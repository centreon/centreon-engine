/*
** Copyright 1999-2009 Ethan Galstad
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
#include "com/centreon/engine/utils.hh"
#include "globals.h"

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
   *  Deletes as check result file, as well as its ok-to-go file.
   *
   *  @param[in] fname Check result file name.
   *
   *  @return OK.
   */
  int delete_check_result_file(char const* fname) {
    // Delete the result file.
    remove(fname);

    // Delete the ok-to-go file.
    std::string ok_to_go(fname);
    ok_to_go.append(".ok");
    remove(ok_to_go.c_str());

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

  /**
   *  Move check result to queue directory.
   *
   *  @param[in] checkresult_file Check result file.
   *
   *  @return OK on success.
   */
  int move_check_result_to_queue(char const* checkresult_file) {
    // Save the file creation mask.
    mode_t new_umask(077);
    mode_t old_umask(umask(new_umask));

    // Create a safe temp file.
    std::ostringstream oss;
    oss << check_result_path << "/cXXXXXX";
    char* output_file(my_strdup(oss.str()));
    int output_file_fd(mkstemp(output_file));

    // File created okay.
    int result(0);
    if (output_file_fd >= 0) {
      logger(logging::dbg_checks, logging::most)
        << "Moving temp check result file '" << checkresult_file
        << "' to queue file '" << output_file << "'...";

#ifdef __CYGWIN__
      // Cygwin cannot rename open files - gives Permission Denied.
      // Close the file.
      close(output_file_fd);
#endif // Cygwin

      // Move the original file.
      result = my_rename(checkresult_file, output_file);

#ifndef __CYGWIN__
      // Close the file.
      close(output_file_fd);
#endif // !Cygwin

      // Create an ok-to-go indicator file.
      std::string temp_buffer(output_file);
      temp_buffer.append(".ok");
      output_file_fd = open(
                         temp_buffer.c_str(),
                         O_CREAT
                         | O_WRONLY
                         | O_TRUNC
                         | S_IRUSR
                         | S_IWUSR,
                         0660);
      if (output_file_fd > 0)
        close(output_file_fd);

      // Delete the original file if it couldn't be moved.
      if (result != 0)
        remove(checkresult_file);
    }
    else
      result = -1;

    // Reset the file creation mask.
    umask(old_umask);

    // Log a warning on errors.
    if (result != 0)
      logger(logging::log_runtime_warning, logging::basic)
        << "Warning: Unable to move file '" << checkresult_file
        << "' to check results queue.";

    // Free memory.
    delete[] output_file;

    return (OK);
  }

  /**
   *  Reads check result(s) from a file.
   *
   *  @param[in] fname Check result file.
   *
   *  @return OK on success.
   */
  int process_check_result_file(char const* fname) {
    // Check pointer.
    if (fname == NULL)
      return (ERROR);

    // Log message.
    logger(logging::dbg_checks, logging::more)
      << "Processing check result file: '" << fname << "'";

    // Open the file for reading.
    mmapfile* thefile(NULL);
    if ((thefile = mmap_fopen(fname)) == NULL) {
      // Try removing the file - zero length files can't be mmap()'ed,
      // so it might exist.
      remove(fname);
      return (ERROR);
    }

    time_t current_time(time(NULL));

    // Read in all lines from the file.
    char* input(NULL);
    check_result* new_cr(NULL);
    char* v1(NULL);
    char* v2(NULL);
    char* var(NULL);
    char* val(NULL);
    while (1) {
      // Free memory.
      delete[] input;

      // Read the next line.
      if ((input = mmap_fgets_multiline(thefile)) == NULL)
        break;

      // Skip comments.
      if (input[0] == '#')
        continue;

      // Empty line indicates end of record.
      else if (input[0] == '\n') {
        // We have something...
        if (new_cr) {
          // Do we have the minimum amount of data?
          if (new_cr->host_name && new_cr->output) {
            // Add check result to list in memory.
            add_check_result_to_list(new_cr);

            // Reset pointer.
            new_cr = NULL;
          }
          // Discard partial input.
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

      // if file is too old, remove it.
      if (!strcmp(var, "file_time")
          && config->max_check_result_file_age() > 0) {
        unsigned long diff(current_time - strtoul(val, NULL, 0));
        if (diff > config->max_check_result_file_age()) {
          free_check_result(new_cr);
          delete new_cr;
          new_cr = NULL;
          break;
        }
      }

      // Allocate new check result if necessary.
      if (!new_cr) {
        new_cr = new check_result;
        init_check_result(new_cr);
        new_cr->output_file = my_strdup(fname);
      }

      // Process variable.
      if (!strcmp(var, "host_name"))
        new_cr->host_name = my_strdup(val);
      else if (!strcmp(var, "service_description")) {
        new_cr->service_description = my_strdup(val);
        new_cr->object_check_type = SERVICE_CHECK;
      }
      else if (!strcmp(var, "check_type"))
        new_cr->check_type = strtol(val, NULL, 0);
      else if (!strcmp(var, "check_options"))
        new_cr->check_options = strtol(val, NULL, 0);
      else if (!strcmp(var, "scheduled_check"))
        new_cr->scheduled_check = strtol(val, NULL, 0);
      else if (!strcmp(var, "reschedule_check"))
        new_cr->reschedule_check = strtol(val, NULL, 0);
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
        new_cr->early_timeout = strtol(val, NULL, 0);
      else if (!strcmp(var, "exited_ok"))
        new_cr->exited_ok = strtol(val, NULL, 0);
      else if (!strcmp(var, "return_code"))
        new_cr->return_code = strtol(val, NULL, 0);
      else if (!strcmp(var, "output"))
        new_cr->output = my_strdup(val);
    }

    // We have something.
    if (new_cr) {
      // Do we have the minimum amount of data?
      if (new_cr->host_name && new_cr->output) {
        // Add check result to list in memory.
        add_check_result_to_list(new_cr);

        // Reset pointer.
        new_cr = NULL;
      }

      // Discard partial input and free memory for current check result
      // record.
      else {
        free_check_result(new_cr);
        delete new_cr;
      }
    }

    // Free memory and close file.
    delete[] input;
    mmap_fclose(thefile);

    // Delete the file (as well its ok-to-go file) if it's too old.
    // Other (current) files are deleted later (when results are
    // processed).
    delete_check_result_file(fname);

    return (OK);
  }

  /**
   *  Processes files in the check result queue directory.
   *
   *  @param[in] dirname Check result queue directory.
   *
   *  @return OK on success.
   */
  int process_check_result_queue(char const* dirname) {
    // Function result.
    int result(OK);

    // Make sure we have what we need.
    if (!dirname) {
      logger(logging::log_config_error, logging::basic)
        << "Error: No check result queue directory specified.";
      return (ERROR);
    }

    // Open the directory for reading.
    DIR* dirp(NULL);
    if ((dirp = opendir(dirname)) == NULL) {
      logger(logging::log_config_error, logging::basic)
        << "Error: Could not open check result queue directory '"
        << dirname << "' for reading.";
      return (ERROR);
    }

    // Process all files in the directory...
    logger(logging::dbg_checks, logging::more)
      << "Starting to read check result queue '" << dirname << "'...";
    struct dirent* dirfile(NULL);
    while ((dirfile = readdir(dirp)) != NULL) {
      // Create /path/to/file.
      char file[MAX_FILENAME_LENGTH];
      snprintf(file, sizeof(file), "%s/%s", dirname, dirfile->d_name);
      file[sizeof(file) - 1] = '\x0';

      // Process this if it's a check result file...
      struct stat stat_buf;
      struct stat ok_stat_buf;
      int x(strlen(dirfile->d_name));
      if (x == 7 && dirfile->d_name[0] == 'c') {
        if (stat(file, &stat_buf) == -1) {
          logger(logging::log_runtime_warning, logging::basic)
            << "Warning: Could not stat() check result file '"
            << file << "'.";
          continue;
        }

        switch (stat_buf.st_mode & S_IFMT) {
        case S_IFREG:
          // Don't process symlinked files.
          if (!S_ISREG(stat_buf.st_mode))
            continue;
          break;
        default:
          // Everything else we ignore.
          continue;
        }

        // At this point we have a regular file...

        // Can we find the associated ok-to-go file ?
        std::string temp_buffer(file);
        temp_buffer.append(".ok");
        result = stat(temp_buffer.c_str(), &ok_stat_buf);
        if (result == -1)
          continue;

        // Process the file.
        result = process_check_result_file(file);

        // Break out if we encountered an error.
        if (result == ERROR)
          break;
      }
    }
    closedir(dirp);

    return (result);
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
