/*
** Copyright 2001-2009 Ethan Galstad
** Copyright 2011-2015 Merethis
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

#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <libgen.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/commands/connector.hh"
#include "com/centreon/engine/commands/forward.hh"
#include "com/centreon/engine/commands/raw.hh"
#include "com/centreon/engine/commands/set.hh"
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/shared_ptr.hh"
#include "nagios.h"
#include "skiplist.h"
#include "xodtemplate.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

static xodtemplate_timeperiod*        xodtemplate_timeperiod_list = NULL;
static xodtemplate_command*           xodtemplate_command_list = NULL;
static xodtemplate_connector*         xodtemplate_connector_list = NULL;
static xodtemplate_hostgroup*         xodtemplate_hostgroup_list = NULL;
static xodtemplate_servicegroup*      xodtemplate_servicegroup_list = NULL;
static xodtemplate_servicedependency* xodtemplate_servicedependency_list = NULL;
static xodtemplate_host*              xodtemplate_host_list = NULL;
static xodtemplate_service*           xodtemplate_service_list = NULL;
static xodtemplate_hostdependency*    xodtemplate_hostdependency_list = NULL;

static xodtemplate_timeperiod*        xodtemplate_timeperiod_list_tail = NULL;
static xodtemplate_command*           xodtemplate_command_list_tail = NULL;
static xodtemplate_connector*         xodtemplate_connector_list_tail = NULL;
static xodtemplate_hostgroup*         xodtemplate_hostgroup_list_tail = NULL;
static xodtemplate_servicegroup*      xodtemplate_servicegroup_list_tail = NULL;
static xodtemplate_servicedependency* xodtemplate_servicedependency_list_tail = NULL;
static xodtemplate_host*              xodtemplate_host_list_tail = NULL;
static xodtemplate_service*           xodtemplate_service_list_tail = NULL;
static xodtemplate_hostdependency*    xodtemplate_hostdependency_list_tail = NULL;

static skiplist*                      xobject_template_skiplists[NUM_XOBJECT_SKIPLISTS];
static skiplist*                      xobject_skiplists[NUM_XOBJECT_SKIPLISTS];

static void*                          xodtemplate_current_object = NULL;
static int                            xodtemplate_current_object_type = XODTEMPLATE_NONE;

static int                            xodtemplate_current_config_file = 0;
static char**                         xodtemplate_config_files = NULL;

/*
 * Macro magic used to determine if a service is assigned
 * via hostgroup_name or host_name. Those assigned via host_name
 * take precedence.
 */
#define X_SERVICE_IS_FROM_HOSTGROUP      (1 << 1)       /* flag to know if service come from a hostgroup def, apply on srv->have_initial_state */
#define xodtemplate_set_service_is_from_hostgroup(srv) \
  srv->have_initial_state |= X_SERVICE_IS_FROM_HOSTGROUP
#define xodtemplate_unset_service_is_from_hostgroup(srv) \
  srv->have_initial_state &= ~X_SERVICE_IS_FROM_HOSTGROUP
#define xodtemplate_is_service_is_from_hostgroup(srv) \
  ((srv->have_initial_state & X_SERVICE_IS_FROM_HOSTGROUP) != 0)

/* returns the name of a numbered config file */
static char const* xodtemplate_config_file_name(int config_file) {
  if (config_file <= xodtemplate_current_config_file)
    return (xodtemplate_config_files[config_file - 1]);
  return ("?");
}

/******************************************************************/
/************* TOP-LEVEL CONFIG DATA INPUT FUNCTION ***************/
/******************************************************************/

/* process all config files - both core and CGIs pass in name of main config file */
int xodtemplate_read_config_data(
      char const* main_config_file,
      int options) {
  struct timeval tv[14];
  int result = OK;

  if (main_config_file == NULL) {
    printf("Error: No main config file passed to object routines!\n");
    return (ERROR);
  }

  /* get variables from main config file */
  xodtemplate_grab_config_info(main_config_file);

  /* initialize variables */
  xodtemplate_timeperiod_list = NULL;
  xodtemplate_command_list = NULL;
  xodtemplate_hostgroup_list = NULL;
  xodtemplate_servicegroup_list = NULL;
  xodtemplate_servicedependency_list = NULL;
  xodtemplate_host_list = NULL;
  xodtemplate_service_list = NULL;
  xodtemplate_hostdependency_list = NULL;

  /* initialize skiplists */
  xodtemplate_init_xobject_skiplists();

  xodtemplate_current_object = NULL;
  xodtemplate_current_object_type = XODTEMPLATE_NONE;

  /* allocate memory for 256 config files (increased dynamically) */
  xodtemplate_current_config_file = 0;
  xodtemplate_config_files = new char*[256];

  /* are the objects we're reading already pre-sorted? */
  if (test_scheduling == true)
    gettimeofday(&tv[0], NULL);

  /* determine the directory of the main config file */
  char* config_file(string::dup(main_config_file));
  char* config_base_dir(string::dup(dirname(config_file)));
  delete[] config_file;

  /* open the main config file for reading (we need to find all the config files to read) */
  mmapfile* thefile(mmap_fopen(main_config_file));
  if (!thefile) {
    delete[] config_base_dir;
    delete[] xodtemplate_config_files;
    xodtemplate_config_files = NULL;
    printf(
           "Unable to open main config file '%s'\n",
           main_config_file);
    return (ERROR);
  }

  /* daemon reads all config files/dirs specified in the main config file */
  /* read in all lines from the main config file */
  char* input(NULL);
  while (1) {
    /* free memory */
    delete[] input;

    /* get the next line */
    if ((input = mmap_fgets_multiline(thefile)) == NULL)
      break;

    /* strip input */
    strip(input);

    /* skip blank lines and comments */
    if (input[0] == '#' || input[0] == ';' || input[0] == '\x0')
      continue;

    char* var(strtok(input, "="));
    if (!var)
      continue;

    char* val(strtok(NULL, "\n"));
    if (!val)
      continue;

    /* process a single config file */
    if (!strcmp(var, "xodtemplate_config_file")
        || !strcmp(var, "cfg_file")) {
      char* temp_buffer(string::dup(val));
      if (config_base_dir != NULL && val[0] != '/') {
        std::ostringstream oss;
        oss << config_base_dir << '/' << temp_buffer;
        config_file = string::dup(oss.str());
        delete[] temp_buffer;
      }
      else
        config_file = temp_buffer;

      /* process the config file... */
      result = xodtemplate_process_config_file(config_file, options);

      delete[] config_file;

      /* if there was an error processing the config file, break out of loop */
      if (result == ERROR)
        break;
    }
    /* process all files in a config directory */
    else if (!strcmp(var, "xodtemplate_config_dir")
             || !strcmp(var, "cfg_dir")) {
      char* temp_buffer(string::dup(val));
      if (config_base_dir != NULL && val[0] != '/') {
        std::ostringstream oss;
        oss << config_base_dir << '/' << temp_buffer;
        config_file = string::dup(oss.str());
        delete[] temp_buffer;
      }
      else
        config_file = temp_buffer;

      /* strip trailing / if necessary */
      if (config_file != NULL
          && config_file[strlen(config_file) - 1] == '/')
        config_file[strlen(config_file) - 1] = '\x0';

      /* process the config directory... */
      result = xodtemplate_process_config_dir(config_file, options);

      delete[] config_file;

      /* if there was an error processing the config file, break out of loop */
      if (result == ERROR)
        break;
    }
  }

  /* free memory and close the file */
  delete[] config_base_dir;
  delete[] input;
  mmap_fclose(thefile);

  if (test_scheduling == true)
    gettimeofday(&tv[1], NULL);

  /* resolve objects definitions */
  if (result == OK)
    result = xodtemplate_resolve_objects();
  if (test_scheduling == true)
    gettimeofday(&tv[2], NULL);

  /* cleanup some additive inheritance stuff... */
  xodtemplate_clean_additive_strings();

  /* do the meat and potatoes stuff... */
  if (result == OK)
    result = xodtemplate_recombobulate_hostgroups();
  if (test_scheduling == true)
    gettimeofday(&tv[4], NULL);

  if (result == OK)
    result = xodtemplate_duplicate_services();
  if (test_scheduling == true)
    gettimeofday(&tv[5], NULL);

  if (result == OK)
    result = xodtemplate_recombobulate_servicegroups();
  if (test_scheduling == true)
    gettimeofday(&tv[6], NULL);

  if (result == OK)
    result = xodtemplate_duplicate_objects();
  if (test_scheduling == true)
    gettimeofday(&tv[7], NULL);

  if (test_scheduling == true)
    gettimeofday(&tv[8], NULL);

  /* sort objects */
  if (result == OK)
    result = xodtemplate_sort_objects();
  if (test_scheduling == true)
    gettimeofday(&tv[10], NULL);

  if (test_scheduling == true)
    gettimeofday(&tv[11], NULL);

  /* register objects */
  if (result == OK)
    result = xodtemplate_register_objects();
  if (test_scheduling == true)
    gettimeofday(&tv[12], NULL);

  /* cleanup */
  xodtemplate_free_memory();
  if (test_scheduling == true)
    gettimeofday(&tv[13], NULL);

  if (test_scheduling == true) {
    double runtime[14];
    runtime[0] = (double)((double)(tv[1].tv_sec - tv[0].tv_sec)
                          + (double)((tv[1].tv_usec - tv[0].tv_usec)
                                     / 1000.0) / 1000.0);
    runtime[1] = (double)((double)(tv[2].tv_sec - tv[1].tv_sec)
                          + (double)((tv[2].tv_usec - tv[1].tv_usec)
                                     / 1000.0) / 1000.0);
    runtime[2] = (double)((double)(tv[3].tv_sec - tv[2].tv_sec) +
                          (double)((tv[3].tv_usec - tv[2].tv_usec)
                                   / 1000.0) / 1000.0);
    runtime[3] = (double)((double)(tv[4].tv_sec - tv[3].tv_sec)
                          + (double)((tv[4].tv_usec - tv[3].tv_usec)
                                     / 1000.0) / 1000.0);
    runtime[4] = (double)((double)(tv[5].tv_sec - tv[4].tv_sec)
                          + (double)((tv[5].tv_usec - tv[4].tv_usec)
                                     / 1000.0) / 1000.0);
    runtime[5] = (double)((double)(tv[6].tv_sec - tv[5].tv_sec)
                          + (double)((tv[6].tv_usec - tv[5].tv_usec)
                                     / 1000.0) / 1000.0);
    runtime[6] = (double)((double)(tv[7].tv_sec - tv[6].tv_sec)
                          + (double)((tv[7].tv_usec - tv[6].tv_usec)
                                     / 1000.0) / 1000.0);
    runtime[7] = (double)((double)(tv[8].tv_sec - tv[7].tv_sec)
                          + (double)((tv[8].tv_usec - tv[7].tv_usec)
                                     / 1000.0) / 1000.0);
    runtime[8] = (double)((double)(tv[9].tv_sec - tv[8].tv_sec)
                          + (double)((tv[9].tv_usec - tv[8].tv_usec)
                                     / 1000.0) / 1000.0);
    runtime[9] = (double)((double)(tv[10].tv_sec - tv[9].tv_sec)
                          + (double)((tv[10].tv_usec - tv[9].tv_usec)
                                     / 1000.0) / 1000.0);
    runtime[10] = (double)((double)(tv[11].tv_sec - tv[10].tv_sec)
                           + (double)((tv[11].tv_usec - tv[10].tv_usec)
                                      / 1000.0) / 1000.0);
    runtime[11] = (double)((double)(tv[12].tv_sec - tv[11].tv_sec)
                           + (double)((tv[12].tv_usec - tv[11].tv_usec)
                                      / 1000.0) / 1000.0);
    runtime[12] = (double)((double)(tv[13].tv_sec - tv[12].tv_sec)
			   + (double)((tv[13].tv_usec - tv[12].tv_usec)
                                      / 1000.0) / 1000.0);
    runtime[13] = (double)((double)(tv[13].tv_sec - tv[0].tv_sec)
			   + (double)((tv[13].tv_usec - tv[0].tv_usec)
                                      / 1000.0) / 1000.0);

    printf(
      "Timing information on object configuration\n"
      "processing is listed below.\n\n");

    printf("OBJECT CONFIG PROCESSING TIMES\n");
    printf("------------------------------\n");
    printf("Read:                 %.6f sec\n", runtime[0]);
    printf("Resolve:              %.6f sec\n", runtime[1]);
    printf("Recomb Hostgroups:    %.6f sec\n", runtime[3]);
    printf("Dup Services:         %.6f sec\n", runtime[4]);
    printf("Recomb Servicegroups: %.6f sec\n", runtime[5]);
    printf("Duplicate:            %.6f sec\n", runtime[6]);
    printf("Inherit:              %.6f sec\n", runtime[7]);
    printf("Sort:                 %.6f sec\n", runtime[9]);
    printf("Register:             %.6f sec\n", runtime[11]);
    printf("Free:                 %.6f sec\n", runtime[12]);
    printf("                      ============\n");
    printf("TOTAL:                %.6f sec  ", runtime[13]);
    printf("\n");
    printf("\n\n");
  }

  return (result);
}

/* grab config variable from main config file */
int xodtemplate_grab_config_info(char const* main_config_file) {
  char* input = NULL;
  char* var = NULL;
  char* val = NULL;
  mmapfile* thefile = NULL;
  nagios_macros* mac;

  /* open the main config file for reading */
  if ((thefile = mmap_fopen(main_config_file)) == NULL)
    return (ERROR);

  /* read in all lines from the main config file */
  while (1) {
    /* free memory */
    delete[] input;

    /* read the next line */
    if ((input = mmap_fgets_multiline(thefile)) == NULL)
      break;

    /* strip input */
    strip(input);

    /* skip blank lines and comments */
    if (input[0] == '#' || input[0] == ';' || input[0] == '\x0')
      continue;

    if ((var = strtok(input, "=")) == NULL)
      continue;

    if ((val = strtok(NULL, "\n")) == NULL)
      continue;
  }

  /* close the file */
  mmap_fclose(thefile);

  mac = get_global_macros();

  return (OK);
}

/* process all files in a specific config directory */
int xodtemplate_process_config_dir(char* dirname, int options) {
  char file[MAX_FILENAME_LENGTH];
  DIR* dirp = NULL;
  struct dirent* dirfile = NULL;
  int result = OK;
  int x = 0;
  struct stat stat_buf;

  if (verify_config == true)
    printf("Processing object config directory '%s'...\n", dirname);

  /* open the directory for reading */
  dirp = opendir(dirname);
  if (dirp == NULL) {
    logger(log_config_error, basic)
      << "Error: Could not open config directory '"
      << dirname << "' for reading.";
    return (ERROR);
  }

  /* process all files in the directory... */
  while ((dirfile = readdir(dirp)) != NULL) {
    /* skip hidden files and directories, and current and parent dir */
    if (dirfile->d_name[0] == '.')
      continue;

    /* create /path/to/file */
    snprintf(file, sizeof(file), "%s/%s", dirname, dirfile->d_name);
    file[sizeof(file) - 1] = '\x0';

    /* process this if it's a non-hidden config file... */
    if (stat(file, &stat_buf) == -1) {
      logger(log_runtime_error, basic)
        << "Error: Could not open config directory member '" << file
        << "' for reading.";
      closedir(dirp);
      return (ERROR);
    }

    switch (stat_buf.st_mode & S_IFMT) {
    case S_IFREG:
      x = strlen(dirfile->d_name);
      if (x <= 4 || strcmp(dirfile->d_name + (x - 4), ".cfg"))
        break;

      /* process the config file */
      result = xodtemplate_process_config_file(file, options);

      if (result == ERROR) {
        closedir(dirp);
        return (ERROR);
      }
      break;

    case S_IFDIR:
      /* recurse into subdirectories... */
      result = xodtemplate_process_config_dir(file, options);

      if (result == ERROR) {
        closedir(dirp);
        return (ERROR);
      }
      break;

    default:
      /* everything else we ignore */
      break;
    }
  }

  closedir(dirp);
  return (result);
}

/* process data in a specific config file */
int xodtemplate_process_config_file(char* filename, int options) {
  mmapfile* thefile = NULL;
  char* input = NULL;
  int in_definition = false;
  int current_line = 0;
  int result = OK;
  int x = 0;
  int y = 0;
  char* ptr = NULL;

  if (verify_config == true)
    printf("Processing object config file '%s'...\n", filename);

  /* save config file name */
  xodtemplate_config_files[xodtemplate_current_config_file++]
    = string::dup(filename);

  /* reallocate memory for config files */
  if (!(xodtemplate_current_config_file % 256)) {
    char** new_tab = new char*[xodtemplate_current_config_file + 256];
    memcpy(
      new_tab,
      xodtemplate_config_files,
      sizeof(*new_tab) * xodtemplate_current_config_file);
    delete[] xodtemplate_config_files;
    xodtemplate_config_files = new_tab;
  }

  /* open the config file for reading */
  if ((thefile = mmap_fopen(filename)) == NULL) {
    logger(log_config_error, basic)
      << "Error: Cannot open config file '" << filename
      << "' for reading: " << strerror(errno);
    return (ERROR);
  }

  /* read in all lines from the config file */
  while (1) {
    /* free memory */
    delete[] input;
    input = NULL;

    /* read the next line */
    if ((input = mmap_fgets_multiline(thefile)) == NULL)
      break;

    current_line = thefile->current_line;

    /* grab data before comment delimiter - faster than a strtok() and strncpy()... */
    for (x = 0; input[x] != '\x0'; x++) {
      if (input[x] == ';') {
        if (x == 0)
          break;
        else if (input[x - 1] != '\\')
          break;
      }
    }
    input[x] = '\x0';

    /* strip input */
    strip(input);

    /* skip empty lines */
    if (input[0] == '\x0' || input[0] == '#')
      continue;

    /* this is the start of an object definition */
    if (strstr(input, "define") == input) {
      /* get the type of object we're defining... */
      for (x = 6; input[x] != '\x0'; x++)
        if (input[x] != ' ' && input[x] != '\t')
          break;
      for (y = 0; input[x] != '\x0'; x++) {
        if (input[x] == ' ' || input[x] == '\t' || input[x] == '{')
          break;
        else
          input[y++] = input[x];
      }
      input[y] = '\x0';

      /* make sure an object type is specified... */
      if (input[0] == '\x0') {
        logger(log_config_error, basic)
          << "Error: No object type specified in file '"
          << filename << "' on line " << current_line << ".";
        result = ERROR;
        break;
      }

      /* check validity of object type */
      if (strcmp(input, "timeperiod")
          && strcmp(input, "command")
          && strcmp(input, "host")
          && strcmp(input, "hostgroup")
          && strcmp(input, "servicegroup")
          && strcmp(input, "service")
          && strcmp(input, "servicedependency")
          && strcmp(input, "hostdependency")
	  && strcmp(input, "connector")) {
        logger(log_config_error, basic)
          << "Error: Invalid object definition type '" << input
          << "' in file '" << filename << "' on line "
          << current_line << ".";
        result = ERROR;
        break;
      }

      /* we're already in an object definition... */
      if (in_definition == true) {
        logger(log_config_error, basic)
          << "Error: Unexpected start of object definition in file '"
          << filename << "' on line " << current_line
          << ".  Make sure you close preceding objects before "
          "starting a new one.";
        result = ERROR;
        break;
      }

      /* start a new definition */
      if (xodtemplate_begin_object_definition(
            input,
            options,
            xodtemplate_current_config_file,
            current_line) == ERROR) {
        logger(log_config_error, basic)
          << "Error: Could not add object definition in file '"
          << filename << "' on line " << current_line << ".";
        result = ERROR;
        break;
      }

      in_definition = true;
    }
    /* we're currently inside an object definition */
    else if (in_definition == true) {
      /* this is the close of an object definition */
      if (!strcmp(input, "}")) {
        in_definition = false;

        /* close out current definition */
        if (xodtemplate_end_object_definition(options) == ERROR) {
          logger(log_config_error, basic)
            << "Error: Could not complete object definition in file '"
            << filename << "' on line " << current_line << ".";
          result = ERROR;
          break;
        }
      }
      /* this is a directive inside an object definition */
      else {
        /* add directive to object definition */
        if (xodtemplate_add_object_property(input, options) == ERROR) {
          logger(log_config_error, basic)
            << "Error: Could not add object property in file '"
            << filename << "' on line " << current_line << ".";
          result = ERROR;
          break;
        }
      }
    }
    /* include another file */
    else if (strstr(input, "include_file=") == input) {
      ptr = strtok(input, "=");
      ptr = strtok(NULL, "\n");

      if (ptr != NULL) {
        result = xodtemplate_process_config_file(ptr, options);
        if (result == ERROR)
          break;
      }
    }
    /* include a directory */
    else if (strstr(input, "include_dir") == input) {
      ptr = strtok(input, "=");
      ptr = strtok(NULL, "\n");

      if (ptr != NULL) {
        result = xodtemplate_process_config_dir(ptr, options);
        if (result == ERROR)
          break;
      }
    }
    /* unexpected token or statement */
    else {
      logger(log_config_error, basic)
        << "Error: Unexpected token or statement in file '"
        << filename << "' on line " << current_line << ".";
      result = ERROR;
      break;
    }
  }

  /* free memory and close file */
  delete[] input;
  input = NULL;
  mmap_fclose(thefile);

  /* whoops - EOF while we were in the middle of an object definition... */
  if (in_definition == true && result == OK) {
    logger(log_config_error, basic)
      << "Error: Unexpected EOF in file '" << filename
      << "' on line " << current_line
      << " - check for a missing closing bracket.",
      result = ERROR;
  }

  return (result);
}

/******************************************************************/
/***************** OBJECT DEFINITION FUNCTIONS ********************/
/******************************************************************/

/*
 * all objects start the same way, so we can get rid of quite
 * a lot of code with this struct-offset-insensitive macro
 */
#define xod_begin_def(type)						\
  do {									\
    new_##type = new xodtemplate_##type;				\
    memset(new_##type, 0, sizeof(*new_##type));				\
    									\
    new_##type->register_object = true;					\
    new_##type->_config_file = config_file;				\
    new_##type->_start_line = start_line;                               \
									\
    /* add new object to head of list in memory */			\
    new_##type->next = xodtemplate_##type##_list;			\
    xodtemplate_##type##_list = new_##type;				\
      									\
    /* update current object pointer */				\
    xodtemplate_current_object = xodtemplate_##type##_list;		\
  } while (0)

/* starts a new object definition */
int xodtemplate_begin_object_definition(
      char* input,
      int options,
      int config_file,
      int start_line) {
  (void)options;

  int result = OK;
  xodtemplate_timeperiod* new_timeperiod = NULL;
  xodtemplate_command* new_command = NULL;
  xodtemplate_connector* new_connector = NULL;
  xodtemplate_hostgroup* new_hostgroup = NULL;
  xodtemplate_servicegroup* new_servicegroup = NULL;
  xodtemplate_servicedependency* new_servicedependency = NULL;
  xodtemplate_host* new_host = NULL;
  xodtemplate_service* new_service = NULL;
  xodtemplate_hostdependency* new_hostdependency = NULL;

  if (!strcmp(input, "service")) {
    xodtemplate_current_object_type = XODTEMPLATE_SERVICE;
    xod_begin_def(service);

    new_service->initial_state = STATE_OK;
    new_service->max_check_attempts = -2;
    new_service->check_timeout = 0;
    new_service->check_interval = 5.0;
    new_service->retry_interval = 1.0;
    new_service->active_checks_enabled = true;
    new_service->obsess_over_service = true;
    new_service->event_handler_enabled = true;
    new_service->flap_detection_enabled = true;
    new_service->flap_detection_on_ok = true;
    new_service->flap_detection_on_warning = true;
    new_service->flap_detection_on_unknown = true;
    new_service->flap_detection_on_critical = true;

    /* true service, so is not from host group, must be set AFTER have_initial_state */
    xodtemplate_unset_service_is_from_hostgroup(new_service);
  }
  else if (!strcmp(input, "host")) {
    xodtemplate_current_object_type = XODTEMPLATE_HOST;
    xod_begin_def(host);

    new_host->check_interval = 5.0;
    new_host->retry_interval = 1.0;
    new_host->active_checks_enabled = true;
    new_host->obsess_over_host = true;
    new_host->max_check_attempts = -2;
    new_host->check_timeout = 0;
    new_host->event_handler_enabled = true;
    new_host->flap_detection_enabled = true;
    new_host->flap_detection_on_up = true;
    new_host->flap_detection_on_down = true;
    new_host->flap_detection_on_unreachable = true;
  }
  else if (!strcmp(input, "command")) {
    xodtemplate_current_object_type = XODTEMPLATE_COMMAND;
    xod_begin_def(command);
  }
  else if (!strcmp(input, "timeperiod")) {
    xodtemplate_current_object_type = XODTEMPLATE_TIMEPERIOD;
    xod_begin_def(timeperiod);
  }
  else if (!strcmp(input, "hostgroup")) {
    xodtemplate_current_object_type = XODTEMPLATE_HOSTGROUP;
    xod_begin_def(hostgroup);
  }
  else if (!strcmp(input, "servicegroup")) {
    xodtemplate_current_object_type = XODTEMPLATE_SERVICEGROUP;
    xod_begin_def(servicegroup);
  }
  else if (!strcmp(input, "servicedependency")) {
    xodtemplate_current_object_type = XODTEMPLATE_SERVICEDEPENDENCY;
    xod_begin_def(servicedependency);
  }
  else if (!strcmp(input, "hostdependency")) {
    xodtemplate_current_object_type = XODTEMPLATE_HOSTDEPENDENCY;
    xod_begin_def(hostdependency);
  }
  else if (!strcmp(input, "connector")) {
    xodtemplate_current_object_type = XODTEMPLATE_CONNECTOR;
    xod_begin_def(connector);
  }
  else
    return (ERROR);

  return (result);
}

#undef xod_begin_def            /* we don't need this anymore */

/* adds a property to an object definition */
int xodtemplate_add_object_property(char* input, int options) {
  (void)options;

  int result = OK;
  char* variable = NULL;
  char* value = NULL;
  char* temp_ptr = NULL;
  char* customvarname = NULL;
  char* customvarvalue = NULL;
  xodtemplate_timeperiod* temp_timeperiod = NULL;
  xodtemplate_command* temp_command = NULL;
  xodtemplate_connector* temp_connector = NULL;
  xodtemplate_hostgroup* temp_hostgroup = NULL;
  xodtemplate_servicegroup* temp_servicegroup = NULL;
  xodtemplate_servicedependency* temp_servicedependency = NULL;
  xodtemplate_host* temp_host = NULL;
  xodtemplate_service* temp_service = NULL;
  xodtemplate_hostdependency* temp_hostdependency = NULL;
  int x = 0;
  int y = 0;

  /* get variable name */
  variable = string::dup(input);

  /* trim at first whitespace occurance */
  for (x = 0, y = 0; variable[x] != '\x0'; x++) {
    if (variable[x] == ' ' || variable[x] == '\t')
      break;
    y++;
  }
  variable[y] = '\x0';

  /* get variable value */
  if ((value = string::dup(input + x)) == NULL) {
    delete[] variable;
    return (ERROR);
  }
  strip(value);

  switch (xodtemplate_current_object_type) {
  case XODTEMPLATE_SERVICE:
    temp_service = (xodtemplate_service*)xodtemplate_current_object;

    if (!strcmp(variable, "use"))
      temp_service->tmpl = string::dup(value);
    else if (!strcmp(variable, "name")) {
      temp_service->name = string::dup(value);

      /* add service to template skiplist for fast searches */
      result = skiplist_insert(
                 xobject_template_skiplists[X_SERVICE_SKIPLIST],
                 (void*)temp_service);
      switch (result) {
      case SKIPLIST_OK:
        result = OK;
        break;

      case SKIPLIST_ERROR_DUPLICATE:
        logger(log_config_warning, basic)
          << "Warning: Duplicate definition found for service '"
          << value << "' (config file '"
          << xodtemplate_config_file_name(temp_service->_config_file)
          << "', starting on line " << temp_service->_start_line << ")";
        result = ERROR;
        break;

      default:
        result = ERROR;
        break;
      }
    }
    else if (!strcmp(variable, "host") || !strcmp(variable, "hosts")
             || !strcmp(variable, "host_name")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_service->host_name = string::dup(value);
      temp_service->have_host_name = true;
    }
    else if (!strcmp(variable, "service_description")
             || !strcmp(variable, "description")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_service->service_description = string::dup(value);
      temp_service->have_service_description = true;
    }
    else if (!strcmp(variable, "hostgroup")
             || !strcmp(variable, "hostgroups")
             || !strcmp(variable, "hostgroup_name")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_service->hostgroup_name = string::dup(value);
      temp_service->have_hostgroup_name = true;
    }
    else if (!strcmp(variable, "service_groups")
             || !strcmp(variable, "servicegroups")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_service->service_groups = string::dup(value);
      temp_service->have_service_groups = true;
    }
    else if (!strcmp(variable, "check_command")) {
      if (strcmp(value, XODTEMPLATE_NULL)) {
        if (value[0] == '!') {
          temp_service->have_important_check_command = true;
          temp_ptr = value + 1;
        }
        else
          temp_ptr = value;
        temp_service->check_command = string::dup(temp_ptr);
      }
      temp_service->have_check_command = true;
    }
    else if (!strcmp(variable, "check_period")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_service->check_period = string::dup(value);
      temp_service->have_check_period = true;
    }
    else if (!strcmp(variable, "event_handler")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_service->event_handler = string::dup(value);
      temp_service->have_event_handler = true;
    }
    else if (!strcmp(variable, "timezone")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_service->timezone = string::dup(value);
      temp_service->have_timezone = true;
    }
    else if (!strcmp(variable, "initial_state")) {
      if (!strcmp(value, "o") || !strcmp(value, "ok"))
        temp_service->initial_state = STATE_OK;
      else if (!strcmp(value, "w") || !strcmp(value, "warning"))
        temp_service->initial_state = STATE_WARNING;
      else if (!strcmp(value, "u") || !strcmp(value, "unknown"))
        temp_service->initial_state = STATE_UNKNOWN;
      else if (!strcmp(value, "c") || !strcmp(value, "critical"))
        temp_service->initial_state = STATE_CRITICAL;
      else {
        logger(log_config_error, basic)
          << "Error: Invalid initial state '" << value
          << "' in service definition.";
        result = ERROR;
      }
      temp_service->have_initial_state = true;
    }
    else if (!strcmp(variable, "max_check_attempts")) {
      temp_service->max_check_attempts = atoi(value);
      temp_service->have_max_check_attempts = true;
    }
    else if (!strcmp(variable, "check_timeout")) {
      temp_service->check_timeout = atoi(value);
      temp_service->have_check_timeout = true;
    }
    else if (!strcmp(variable, "check_interval")
             || !strcmp(variable, "normal_check_interval")) {
      temp_service->check_interval = strtod(value, NULL);
      temp_service->have_check_interval = true;
    }
    else if (!strcmp(variable, "retry_interval")
             || !strcmp(variable, "retry_check_interval")) {
      temp_service->retry_interval = strtod(value, NULL);
      temp_service->have_retry_interval = true;
    }
    else if (!strcmp(variable, "active_checks_enabled")) {
      temp_service->active_checks_enabled = (atoi(value) > 0) ? true : false;
      temp_service->have_active_checks_enabled = true;
    }
    else if (!strcmp(variable, "is_volatile")) {
      temp_service->is_volatile = (atoi(value) > 0) ? true : false;
      temp_service->have_is_volatile = true;
    }
    else if (!strcmp(variable, "obsess_over_service")) {
      temp_service->obsess_over_service = (atoi(value) > 0) ? true : false;
      temp_service->have_obsess_over_service = true;
    }
    else if (!strcmp(variable, "event_handler_enabled")) {
      temp_service->event_handler_enabled = (atoi(value) > 0) ? true : false;
      temp_service->have_event_handler_enabled = true;
    }
    else if (!strcmp(variable, "check_freshness")) {
      temp_service->check_freshness = (atoi(value) > 0) ? true : false;
      temp_service->have_check_freshness = true;
    }
    else if (!strcmp(variable, "freshness_threshold")) {
      temp_service->freshness_threshold = atoi(value);
      temp_service->have_freshness_threshold = true;
    }
    else if (!strcmp(variable, "low_flap_threshold")) {
      temp_service->low_flap_threshold = strtod(value, NULL);
      temp_service->have_low_flap_threshold = true;
    }
    else if (!strcmp(variable, "high_flap_threshold")) {
      temp_service->high_flap_threshold = strtod(value, NULL);
      temp_service->have_high_flap_threshold = true;
    }
    else if (!strcmp(variable, "flap_detection_enabled")) {
      temp_service->flap_detection_enabled = (atoi(value) > 0) ? true : false;
      temp_service->have_flap_detection_enabled = true;
    }
    else if (!strcmp(variable, "flap_detection_options")) {
      /* user is specifying something, so discard defaults... */
      temp_service->flap_detection_on_ok = false;
      temp_service->flap_detection_on_warning = false;
      temp_service->flap_detection_on_unknown = false;
      temp_service->flap_detection_on_critical = false;

      for (temp_ptr = strtok(value, ", ");
	   temp_ptr != NULL;
           temp_ptr = strtok(NULL, ", ")) {
        if (!strcmp(temp_ptr, "o") || !strcmp(temp_ptr, "ok"))
          temp_service->flap_detection_on_ok = true;
        else if (!strcmp(temp_ptr, "w") || !strcmp(temp_ptr, "warning"))
          temp_service->flap_detection_on_warning = true;
        else if (!strcmp(temp_ptr, "u") || !strcmp(temp_ptr, "unknown"))
          temp_service->flap_detection_on_unknown = true;
        else if (!strcmp(temp_ptr, "c") || !strcmp(temp_ptr, "critical"))
          temp_service->flap_detection_on_critical = true;
        else if (!strcmp(temp_ptr, "n") || !strcmp(temp_ptr, "none")) {
          temp_service->flap_detection_on_ok = false;
          temp_service->flap_detection_on_warning = false;
          temp_service->flap_detection_on_unknown = false;
          temp_service->flap_detection_on_critical = false;
        }
        else if (!strcmp(temp_ptr, "a") || !strcmp(temp_ptr, "all")) {
          temp_service->flap_detection_on_ok = true;
          temp_service->flap_detection_on_warning = true;
          temp_service->flap_detection_on_unknown = true;
          temp_service->flap_detection_on_critical = true;
        }
        else {
          logger(log_config_error, basic)
            << "Error: Invalid flap detection option '" << temp_ptr
            << "' in service definition.";
          return (ERROR);
        }
      }
      temp_service->have_flap_detection_options = true;
    }
    else if (!strcmp(variable, "register"))
      temp_service->register_object = (atoi(value) > 0) ? true : false;
    else if (variable[0] == '_') {
      /* get the variable name */
      customvarname = string::dup(variable + 1);

      /* make sure we have a variable name */
      if (!strcmp(customvarname, "")) {
        logger(log_config_error, basic)
          << "Error: Null custom variable name.";
        delete[] customvarname;
        return (ERROR);
      }

      /* get the variable value */
      if (strcmp(value, XODTEMPLATE_NULL))
        customvarvalue = string::dup(value);
      else
        customvarvalue = NULL;

      /* add the custom variable */
      if (xodtemplate_add_custom_variable_to_service(
            temp_service,
            customvarname,
            customvarvalue) == NULL) {
        delete[] customvarname;
        delete[] customvarvalue;
        return (ERROR);
      }

      /* free memory */
      delete[] customvarname;
      delete[] customvarvalue;
    }
    else {
      logger(log_config_error, basic)
        << "Error: Invalid service object directive '"
        << variable << "'.";
      return (ERROR);
    }
    break;

  case XODTEMPLATE_HOST:
    temp_host = (xodtemplate_host*)xodtemplate_current_object;

    if (!strcmp(variable, "use"))
      temp_host->tmpl = string::dup(value);
    else if (!strcmp(variable, "name")) {
      temp_host->name = string::dup(value);

      /* add host to template skiplist for fast searches */
      result = skiplist_insert(
                 xobject_template_skiplists[X_HOST_SKIPLIST],
                 (void*)temp_host);
      switch (result) {
      case SKIPLIST_OK:
        result = OK;
        break;

      case SKIPLIST_ERROR_DUPLICATE:
        logger(log_config_warning, basic)
          << "Warning: Duplicate definition found for host '"
          << value << "' (config file '"
          << xodtemplate_config_file_name(temp_host->_config_file)
          << "', starting on line " << temp_host->_start_line << ")";
        result = ERROR;
        break;

      default:
        result = ERROR;
        break;
      }
    }
    else if (!strcmp(variable, "host_name")) {
      temp_host->host_name = string::dup(value);

      /* add host to template skiplist for fast searches */
      result = skiplist_insert(
                 xobject_skiplists[X_HOST_SKIPLIST],
                 (void*)temp_host);
      switch (result) {
      case SKIPLIST_OK:
        result = OK;
        break;

      case SKIPLIST_ERROR_DUPLICATE:
        logger(log_config_warning, basic)
          << "Warning: Duplicate definition found for host '"
          << value << "' (config file '"
          << xodtemplate_config_file_name(temp_host->_config_file)
          << "', starting on line " << temp_host->_start_line << ")";
        result = ERROR;
        break;

      default:
        result = ERROR;
        break;
      }
    }
    else if (!strcmp(variable, "alias"))
      temp_host->alias = string::dup(value);
    else if (!strcmp(variable, "address"))
      temp_host->address = string::dup(value);
    else if (!strcmp(variable, "parents")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_host->parents = string::dup(value);
      temp_host->have_parents = true;
    }
    else if (!strcmp(variable, "host_groups")
	     || !strcmp(variable, "hostgroups")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_host->host_groups = string::dup(value);
      temp_host->have_host_groups = true;
    }
    else if (!strcmp(variable, "check_command")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_host->check_command = string::dup(value);
      temp_host->have_check_command = true;
    }
    else if (!strcmp(variable, "check_period")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_host->check_period = string::dup(value);
      temp_host->have_check_period = true;
    }
    else if (!strcmp(variable, "event_handler")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_host->event_handler = string::dup(value);
      temp_host->have_event_handler = true;
    }
    else if (!strcmp(variable, "timezone")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_host->timezone = string::dup(value);
      temp_host->have_timezone = true;
    }
    else if (!strcmp(variable, "initial_state")) {
      if (!strcmp(value, "o") || !strcmp(value, "up"))
        temp_host->initial_state = 0;   /* HOST_UP */
      else if (!strcmp(value, "d") || !strcmp(value, "down"))
        temp_host->initial_state = 1;   /* HOST_DOWN */
      else if (!strcmp(value, "u") || !strcmp(value, "unreachable"))
        temp_host->initial_state = 2;   /* HOST_UNREACHABLE */
      else {
        logger(log_config_error, basic)
          << "Error: Invalid initial state '" << value
          << "' in host definition.";
        result = ERROR;
      }
      temp_host->have_initial_state = true;
    }
    else if (!strcmp(variable, "check_interval")
	     || !strcmp(variable, "normal_check_interval")) {
      temp_host->check_interval = strtod(value, NULL);
      temp_host->have_check_interval = true;
    }
    else if (!strcmp(variable, "retry_interval")
             || !strcmp(variable, "retry_check_interval")) {
      temp_host->retry_interval = strtod(value, NULL);
      temp_host->have_retry_interval = true;
    }
    else if (!strcmp(variable, "max_check_attempts")) {
      temp_host->max_check_attempts = atoi(value);
      temp_host->have_max_check_attempts = true;
    }
    else if (!strcmp(variable, "check_timeout")) {
        temp_host->check_timeout = atoi(value);
        temp_host->have_check_timeout = true;
    }
    else if (!strcmp(variable, "checks_enabled")
             || !strcmp(variable, "active_checks_enabled")) {
      temp_host->active_checks_enabled = (atoi(value) > 0) ? true : false;
      temp_host->have_active_checks_enabled = true;
    }
    else if (!strcmp(variable, "event_handler_enabled")) {
      temp_host->event_handler_enabled = (atoi(value) > 0) ? true : false;
      temp_host->have_event_handler_enabled = true;
    }
    else if (!strcmp(variable, "check_freshness")) {
      temp_host->check_freshness = (atoi(value) > 0) ? true : false;
      temp_host->have_check_freshness = true;
    }
    else if (!strcmp(variable, "freshness_threshold")) {
      temp_host->freshness_threshold = atoi(value);
      temp_host->have_freshness_threshold = true;
    }
    else if (!strcmp(variable, "low_flap_threshold")) {
      temp_host->low_flap_threshold = strtod(value, NULL);
      temp_host->have_low_flap_threshold = true;
    }
    else if (!strcmp(variable, "high_flap_threshold")) {
      temp_host->high_flap_threshold = strtod(value, NULL);
      temp_host->have_high_flap_threshold = true;
    }
    else if (!strcmp(variable, "flap_detection_enabled")) {
      temp_host->flap_detection_enabled = (atoi(value) > 0) ? true : false;
      temp_host->have_flap_detection_enabled = true;
    }
    else if (!strcmp(variable, "flap_detection_options")) {
      /* user is specifying something, so discard defaults... */
      temp_host->flap_detection_on_up = false;
      temp_host->flap_detection_on_down = false;
      temp_host->flap_detection_on_unreachable = false;

      for (temp_ptr = strtok(value, ", ");
	   temp_ptr != NULL;
           temp_ptr = strtok(NULL, ", ")) {
        if (!strcmp(temp_ptr, "o") || !strcmp(temp_ptr, "up"))
          temp_host->flap_detection_on_up = true;
        else if (!strcmp(temp_ptr, "d") || !strcmp(temp_ptr, "down"))
          temp_host->flap_detection_on_down = true;
        else if (!strcmp(temp_ptr, "u") || !strcmp(temp_ptr, "unreachable"))
          temp_host->flap_detection_on_unreachable = true;
        else if (!strcmp(temp_ptr, "n") || !strcmp(temp_ptr, "none")) {
          temp_host->flap_detection_on_up = false;
          temp_host->flap_detection_on_down = false;
          temp_host->flap_detection_on_unreachable = false;
        }
        else if (!strcmp(temp_ptr, "a") || !strcmp(temp_ptr, "all")) {
          temp_host->flap_detection_on_up = true;
          temp_host->flap_detection_on_down = true;
          temp_host->flap_detection_on_unreachable = true;
        }
        else {
          logger(log_config_error, basic)
            << "Error: Invalid flap detection option '" << temp_ptr
            << "' in host definition.";
          result = ERROR;
        }
      }
      temp_host->have_flap_detection_options = true;
    }
    else if (!strcmp(variable, "obsess_over_host")) {
      temp_host->obsess_over_host = (atoi(value) > 0) ? true : false;
      temp_host->have_obsess_over_host = true;
    }
    else if (!strcmp(variable, "register"))
      temp_host->register_object = (atoi(value) > 0) ? true : false;
    else if (variable[0] == '_') {
      /* get the variable name */
      customvarname = string::dup(variable + 1);

      /* make sure we have a variable name */
      if (!strcmp(customvarname, "")) {
        logger(log_config_error, basic)
          << "Error: Null custom variable name.";
        delete[] customvarname;
        return (ERROR);
      }

      /* get the variable value */
      customvarvalue = NULL;
      if (strcmp(value, XODTEMPLATE_NULL))
        customvarvalue = string::dup(value);

      /* add the custom variable */
      if (xodtemplate_add_custom_variable_to_host(
            temp_host,
            customvarname,
            customvarvalue) == NULL) {
        delete[] customvarname;
        delete[] customvarvalue;
        return (ERROR);
      }

      /* free memory */
      delete[] customvarname;
      delete[] customvarvalue;
    }
    else {
      logger(log_config_error, basic)
        << "Error: Invalid host object directive '" << variable << "'.";
      return (ERROR);
    }
    break;

  case XODTEMPLATE_TIMEPERIOD:
    temp_timeperiod
      = (xodtemplate_timeperiod*)xodtemplate_current_object;

    if (!strcmp(variable, "use"))
      temp_timeperiod->tmpl = string::dup(value);
    else if (!strcmp(variable, "name")) {
      temp_timeperiod->name = string::dup(value);

      /* add timeperiod to template skiplist for fast searches */
      result = skiplist_insert(
                 xobject_template_skiplists[X_TIMEPERIOD_SKIPLIST],
                 (void*)temp_timeperiod);
      switch (result) {
      case SKIPLIST_OK:
        result = OK;
        break;

      case SKIPLIST_ERROR_DUPLICATE:
        logger(log_config_warning, basic)
          << "Warning: Duplicate definition found for timeperiod '"
          << value << "' (config file '"
          << xodtemplate_config_file_name(temp_timeperiod->_config_file)
          << "', starting on line "
          << temp_timeperiod->_start_line << ")";
        result = ERROR;
        break;

      default:
        result = ERROR;
        break;
      }
    }
    else if (!strcmp(variable, "timeperiod_name")) {
      temp_timeperiod->timeperiod_name = string::dup(value);

      result = skiplist_insert(
                 xobject_skiplists[X_TIMEPERIOD_SKIPLIST],
                 (void*)temp_timeperiod);
      switch (result) {
      case SKIPLIST_OK:
        result = OK;
        break;

      case SKIPLIST_ERROR_DUPLICATE:
        logger(log_config_warning, basic)
          << "Warning: Duplicate definition found for timeperiod '"
          << value << "' (config file '"
          << xodtemplate_config_file_name(temp_timeperiod->_config_file)
          << "', starting on line "
          << temp_timeperiod->_start_line << ")";
        result = ERROR;
        break;

      default:
        result = ERROR;
        break;
      }
    }
    else if (!strcmp(variable, "alias"))
      temp_timeperiod->alias = string::dup(value);
    else if (!strcmp(variable, "exclude"))
      temp_timeperiod->exclusions = string::dup(value);
    else if (!strcmp(variable, "register"))
      temp_timeperiod->register_object = (atoi(value) > 0) ? true : false;
    else if (xodtemplate_parse_timeperiod_directive(
               temp_timeperiod,
               variable,
               value) == OK)
      result = OK;
    else {
      logger(log_config_error, basic)
        << "Error: Invalid timeperiod object directive '"
        << variable << "'.";
      return (ERROR);
    }
    break;

  case XODTEMPLATE_COMMAND:
    temp_command = (xodtemplate_command*)xodtemplate_current_object;

    if (!strcmp(variable, "use"))
      temp_command->tmpl = string::dup(value);
    else if (!strcmp(variable, "name")) {
      temp_command->name = string::dup(value);

      /* add command to template skiplist for fast searches */
      result = skiplist_insert(
                 xobject_template_skiplists[X_COMMAND_SKIPLIST],
                 (void*)temp_command);
      switch (result) {
      case SKIPLIST_OK:
        result = OK;
        break;

      case SKIPLIST_ERROR_DUPLICATE:
        logger(log_config_warning, basic)
          << "Warning: Duplicate definition found for command '"
          << value << "' (config file '"
          << xodtemplate_config_file_name(temp_command->_config_file)
          << "', starting on line " << temp_command->_start_line << ")";
        result = ERROR;
        break;

      default:
        result = ERROR;
        break;
      }
    }
    else if (!strcmp(variable, "command_name")) {
      temp_command->command_name = string::dup(value);

      /* add command to template skiplist for fast searches */
      result = skiplist_insert(
                 xobject_skiplists[X_COMMAND_SKIPLIST],
                 (void*)temp_command);
      switch (result) {
      case SKIPLIST_OK:
        result = OK;
        break;

      case SKIPLIST_ERROR_DUPLICATE:
        logger(log_config_warning, basic)
          << "Warning: Duplicate definition found for command '"
          << value << "' (config file '"
          << xodtemplate_config_file_name(temp_command->_config_file)
          << "', starting on line " << temp_command->_start_line << ")";
        result = ERROR;
        break;

      default:
        result = ERROR;
        break;
      }
    }
    else if (!strcmp(variable, "command_line"))
      temp_command->command_line = string::dup(value);
    else if (!strcmp(variable, "register"))
      temp_command->register_object = (atoi(value) > 0) ? true : false;
    else if (!strcmp(variable, "connector"))
      temp_command->connector_name = string::dup(value);
    else {
      logger(log_config_error, basic)
        << "Error: Invalid command object directive '"
        << variable << "'.";
      return (ERROR);
    }
    break;

  case XODTEMPLATE_CONNECTOR:
    temp_connector = (xodtemplate_connector*)xodtemplate_current_object;

    if (!strcmp(variable, "connector_line"))
      temp_connector->connector_line = string::dup(value);
    else if (!strcmp(variable, "connector_name")) {
      temp_connector->connector_name = string::dup(value);

      /* add command to template skiplist for fast searches */
      result = skiplist_insert(
                 xobject_skiplists[X_CONNECTOR_SKIPLIST],
                 (void*)temp_connector);
      switch (result) {
      case SKIPLIST_OK:
        result = OK;
        break;

      case SKIPLIST_ERROR_DUPLICATE:
        logger(log_config_warning, basic)
          << "Warning: Duplicate definition found for connector '"
          << value << "' (config file '"
          << xodtemplate_config_file_name(temp_connector->_config_file)
          << "', starting on line " << temp_connector->_start_line << ")";
        result = ERROR;
        break;

      default:
        result = ERROR;
        break;
      }
    }
    else {
      logger(log_config_error, basic)
        << "Error: Invalid command object directive '"
        << variable << "'.";
      return (ERROR);
    }
    break;

  case XODTEMPLATE_HOSTGROUP:
    temp_hostgroup = (xodtemplate_hostgroup*)xodtemplate_current_object;

    if (!strcmp(variable, "use"))
      temp_hostgroup->tmpl = string::dup(value);
    else if (!strcmp(variable, "name")) {
      temp_hostgroup->name = string::dup(value);

      /* add hostgroup to template skiplist for fast searches */
      result = skiplist_insert(
                 xobject_template_skiplists[X_HOSTGROUP_SKIPLIST],
                 (void*)temp_hostgroup);
      switch (result) {
      case SKIPLIST_OK:
        result = OK;
        break;

      case SKIPLIST_ERROR_DUPLICATE:
        logger(log_config_warning, basic)
          << "Warning: Duplicate definition found for hostgroup '"
          << value << "' (config file '"
          << xodtemplate_config_file_name(temp_hostgroup->_config_file)
          << "', starting on line " << temp_hostgroup->_start_line << ")";
        result = ERROR;
        break;

      default:
        result = ERROR;
        break;
      }
    }
    else if (!strcmp(variable, "hostgroup_name")) {
      temp_hostgroup->hostgroup_name = string::dup(value);

      /* add hostgroup to template skiplist for fast searches */
      result = skiplist_insert(
                 xobject_skiplists[X_HOSTGROUP_SKIPLIST],
                 (void*)temp_hostgroup);
      switch (result) {
      case SKIPLIST_OK:
        result = OK;
        break;

      case SKIPLIST_ERROR_DUPLICATE:
        logger(log_config_warning, basic)
          << "Warning: Duplicate definition found for hostgroup '"
          << value << "' (config file '"
          << xodtemplate_config_file_name(temp_hostgroup->_config_file)
          << "', starting on line " << temp_hostgroup->_start_line << ")";
        result = ERROR;
        break;

      default:
        result = ERROR;
        break;
      }
    }
    else if (!strcmp(variable, "alias"))
      temp_hostgroup->alias = string::dup(value);
    else if (!strcmp(variable, "members")) {
      if (strcmp(value, XODTEMPLATE_NULL)) {
        if (temp_hostgroup->members == NULL)
          temp_hostgroup->members = string::dup(value);
        else {
          temp_hostgroup->members
            = resize_string(
                temp_hostgroup->members,
                strlen(temp_hostgroup->members) + strlen(value) + 2);
          strcat(temp_hostgroup->members, ",");
          strcat(temp_hostgroup->members, value);
        }
        if (temp_hostgroup->members == NULL)
          result = ERROR;
      }
      temp_hostgroup->have_members = true;
    }
    else if (!strcmp(variable, "hostgroup_members")) {
      if (strcmp(value, XODTEMPLATE_NULL)) {
        if (temp_hostgroup->hostgroup_members == NULL)
          temp_hostgroup->hostgroup_members = string::dup(value);
        else {
          temp_hostgroup->hostgroup_members
            = resize_string(
                temp_hostgroup->hostgroup_members,
                strlen(temp_hostgroup->hostgroup_members) + strlen(value) + 2);
          strcat(temp_hostgroup->hostgroup_members, ",");
          strcat(temp_hostgroup->hostgroup_members, value);
        }
        if (temp_hostgroup->hostgroup_members == NULL)
          result = ERROR;
      }
      temp_hostgroup->have_hostgroup_members = true;
    }
    else if (!strcmp(variable, "register"))
      temp_hostgroup->register_object = (atoi(value) > 0) ? true : false;
    else {
      logger(log_config_error, basic)
        << "Error: Invalid hostgroup object directive '" << variable << "'.";
      return (ERROR);
    }

    break;

  case XODTEMPLATE_SERVICEGROUP:
    temp_servicegroup = (xodtemplate_servicegroup*)xodtemplate_current_object;

    if (!strcmp(variable, "use"))
      temp_servicegroup->tmpl = string::dup(value);
    else if (!strcmp(variable, "name")) {
      temp_servicegroup->name = string::dup(value);

      /* add servicegroup to template skiplist for fast searches */
      result = skiplist_insert(
                 xobject_template_skiplists[X_SERVICEGROUP_SKIPLIST],
                 (void*)temp_servicegroup);
      switch (result) {
      case SKIPLIST_OK:
        result = OK;
        break;

      case SKIPLIST_ERROR_DUPLICATE:
        logger(log_config_warning, basic)
          << "Warning: Duplicate definition found for servicegroup '"
          << value << "' (config file '"
          << xodtemplate_config_file_name(temp_servicegroup->_config_file)
          << "', starting on line " << temp_servicegroup->_start_line << ")";
        result = ERROR;
        break;

      default:
        result = ERROR;
        break;
      }
    }
    else if (!strcmp(variable, "servicegroup_name")) {
      temp_servicegroup->servicegroup_name = string::dup(value);

      /* add servicegroup to template skiplist for fast searches */
      result = skiplist_insert(
                 xobject_skiplists[X_SERVICEGROUP_SKIPLIST],
                 (void*)temp_servicegroup);
      switch (result) {
      case SKIPLIST_OK:
        result = OK;
        break;

      case SKIPLIST_ERROR_DUPLICATE:
        logger(log_config_warning, basic)
          << "Warning: Duplicate definition found for servicegroup '"
          << value << "' (config file '"
          << xodtemplate_config_file_name(temp_servicegroup->_config_file)
          << "', starting on line " << temp_servicegroup->_start_line << ")";
        result = ERROR;
        break;

      default:
        result = ERROR;
        break;
      }
    }
    else if (!strcmp(variable, "alias"))
      temp_servicegroup->alias = string::dup(value);
    else if (!strcmp(variable, "members")) {
      if (strcmp(value, XODTEMPLATE_NULL)) {
        if (temp_servicegroup->members == NULL)
          temp_servicegroup->members = string::dup(value);
        else {
          temp_servicegroup->members
            = resize_string(
                temp_servicegroup->members,
                strlen(temp_servicegroup->members) + strlen(value) + 2);
          strcat(temp_servicegroup->members, ",");
          strcat(temp_servicegroup->members, value);
        }
        if (temp_servicegroup->members == NULL)
          result = ERROR;
      }
      temp_servicegroup->have_members = true;
    }
    else if (!strcmp(variable, "servicegroup_members")) {
      if (strcmp(value, XODTEMPLATE_NULL)) {
        if (temp_servicegroup->servicegroup_members == NULL)
          temp_servicegroup->servicegroup_members = string::dup(value);
        else {
          temp_servicegroup->servicegroup_members
	    = resize_string(
                temp_servicegroup->servicegroup_members,
                strlen(temp_servicegroup-> servicegroup_members) + strlen(value) + 2);
          strcat(temp_servicegroup->servicegroup_members, ",");
          strcat(temp_servicegroup->servicegroup_members, value);
        }
        if (temp_servicegroup->servicegroup_members == NULL)
          result = ERROR;
      }
      temp_servicegroup->have_servicegroup_members = true;
    }
    else if (!strcmp(variable, "register"))
      temp_servicegroup->register_object = (atoi(value) > 0) ? true : false;
    else {
      logger(log_config_error, basic)
        << "Error: Invalid servicegroup object directive '"
        << variable << "'.";
      return (ERROR);
    }

    break;

  case XODTEMPLATE_SERVICEDEPENDENCY:
    temp_servicedependency
      = (xodtemplate_servicedependency*)xodtemplate_current_object;

    if (!strcmp(variable, "use"))
      temp_servicedependency->tmpl = string::dup(value);
    else if (!strcmp(variable, "name")) {
      temp_servicedependency->name = string::dup(value);

      /* add dependency to template skiplist for fast searches */
      result = skiplist_insert(
                 xobject_template_skiplists[X_SERVICEDEPENDENCY_SKIPLIST],
                 (void*)temp_servicedependency);
      switch (result) {
      case SKIPLIST_OK:
        result = OK;
        break;

      case SKIPLIST_ERROR_DUPLICATE:
        logger(log_config_warning, basic)
          << "Warning: Duplicate definition found for service dependency '"
          << variable << "' (config file '"
          << xodtemplate_config_file_name(temp_servicedependency->_config_file)
          << "', starting on line " << temp_servicedependency->_start_line << ")";
        result = ERROR;
        break;

      default:
        result = ERROR;
        break;
      }
    }
    else if (!strcmp(variable, "servicegroup")
             || !strcmp(variable, "servicegroups")
             || !strcmp(variable, "servicegroup_name")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_servicedependency->servicegroup_name = string::dup(value);
      temp_servicedependency->have_servicegroup_name = true;
    }
    else if (!strcmp(variable, "hostgroup")
             || !strcmp(variable, "hostgroups")
             || !strcmp(variable, "hostgroup_name")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_servicedependency->hostgroup_name = string::dup(value);
      temp_servicedependency->have_hostgroup_name = true;
    }
    else if (!strcmp(variable, "host") || !strcmp(variable, "host_name")
             || !strcmp(variable, "master_host")
             || !strcmp(variable, "master_host_name")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_servicedependency->host_name = string::dup(value);
      temp_servicedependency->have_host_name = true;
    }
    else if (!strcmp(variable, "description")
             || !strcmp(variable, "service_description")
             || !strcmp(variable, "master_description")
             || !strcmp(variable, "master_service_description")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_servicedependency->service_description = string::dup(value);
      temp_servicedependency->have_service_description = true;
    }
    else if (!strcmp(variable, "dependent_servicegroup")
             || !strcmp(variable, "dependent_servicegroups")
             || !strcmp(variable, "dependent_servicegroup_name")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_servicedependency->dependent_servicegroup_name = string::dup(value);
      temp_servicedependency->have_dependent_servicegroup_name = true;
    }
    else if (!strcmp(variable, "dependent_hostgroup")
             || !strcmp(variable, "dependent_hostgroups")
             || !strcmp(variable, "dependent_hostgroup_name")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_servicedependency->dependent_hostgroup_name = string::dup(value);
      temp_servicedependency->have_dependent_hostgroup_name = true;
    }
    else if (!strcmp(variable, "dependent_host")
             || !strcmp(variable, "dependent_host_name")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_servicedependency->dependent_host_name = string::dup(value);
      temp_servicedependency->have_dependent_host_name = true;
    }
    else if (!strcmp(variable, "dependent_description")
             || !strcmp(variable, "dependent_service_description")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_servicedependency->dependent_service_description = string::dup(value);
      temp_servicedependency->have_dependent_service_description = true;
    }
    else if (!strcmp(variable, "dependency_period")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_servicedependency->dependency_period = string::dup(value);
      temp_servicedependency->have_dependency_period = true;
    }
    else if (!strcmp(variable, "inherits_parent")) {
      temp_servicedependency->inherits_parent = (atoi(value) > 0) ? true : false;
      temp_servicedependency->have_inherits_parent = true;
    }
    else if (!strcmp(variable, "failure_options")
             || !strcmp(variable, "execution_failure_options")
             || !strcmp(variable, "execution_failure_criteria")) {
      for (temp_ptr = strtok(value, ", ");
	   temp_ptr != NULL;
           temp_ptr = strtok(NULL, ", ")) {
        if (!strcmp(temp_ptr, "o") || !strcmp(temp_ptr, "ok"))
          temp_servicedependency->fail_on_ok = true;
        else if (!strcmp(temp_ptr, "u") || !strcmp(temp_ptr, "unknown"))
          temp_servicedependency->fail_on_unknown = true;
        else if (!strcmp(temp_ptr, "w") || !strcmp(temp_ptr, "warning"))
          temp_servicedependency->fail_on_warning = true;
        else if (!strcmp(temp_ptr, "c") || !strcmp(temp_ptr, "critical"))
          temp_servicedependency->fail_on_critical = true;
        else if (!strcmp(temp_ptr, "p") || !strcmp(temp_ptr, "pending"))
          temp_servicedependency->fail_on_pending = true;
        else if (!strcmp(temp_ptr, "n") || !strcmp(temp_ptr, "none")) {
          temp_servicedependency->fail_on_ok = false;
          temp_servicedependency->fail_on_unknown = false;
          temp_servicedependency->fail_on_warning = false;
          temp_servicedependency->fail_on_critical = false;
          temp_servicedependency->fail_on_pending = false;
        }
        else if (!strcmp(temp_ptr, "a") || !strcmp(temp_ptr, "all")) {
          temp_servicedependency->fail_on_ok = true;
          temp_servicedependency->fail_on_unknown = true;
          temp_servicedependency->fail_on_warning = true;
          temp_servicedependency->fail_on_critical = true;
          temp_servicedependency->fail_on_pending = true;
        }
        else {
          logger(log_config_error, basic)
            << "Error: Invalid execution dependency option '"
            << temp_ptr << "' in servicedependency definition.";
          return (ERROR);
        }
      }
      temp_servicedependency->have_dependency_options = true;
    }
    else if (!strcmp(variable, "register"))
      temp_servicedependency->register_object = (atoi(value) > 0) ? true : false;
    else {
      logger(log_config_error, basic)
        << "Error: Invalid servicedependency object directive '"
        << variable << "'.";
      return (ERROR);
    }
    break;

  case XODTEMPLATE_HOSTDEPENDENCY:
    temp_hostdependency
      =(xodtemplate_hostdependency*)xodtemplate_current_object;

    if (!strcmp(variable, "use"))
      temp_hostdependency->tmpl = string::dup(value);
    else if (!strcmp(variable, "name")) {
      temp_hostdependency->name = string::dup(value);

      /* add dependency to template skiplist for fast searches */
      result = skiplist_insert(
                 xobject_template_skiplists[X_HOSTDEPENDENCY_SKIPLIST],
                 (void*)temp_hostdependency);
      switch (result) {
      case SKIPLIST_OK:
        result = OK;
        break;

      case SKIPLIST_ERROR_DUPLICATE:
        logger(log_config_warning, basic)
          << "Warning: Duplicate definition found for host dependency '"
          << value << "' (config file '"
          << xodtemplate_config_file_name(temp_hostdependency-> _config_file)
          << "', starting on line " << temp_hostdependency->_start_line << ")";
        result = ERROR;
        break;

      default:
        result = ERROR;
        break;
      }
    }
    else if (!strcmp(variable, "hostgroup")
             || !strcmp(variable, "hostgroups")
             || !strcmp(variable, "hostgroup_name")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_hostdependency->hostgroup_name = string::dup(value);
      temp_hostdependency->have_hostgroup_name = true;
    }
    else if (!strcmp(variable, "host")
             || !strcmp(variable, "host_name")
             || !strcmp(variable, "master_host")
             || !strcmp(variable, "master_host_name")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_hostdependency->host_name = string::dup(value);
      temp_hostdependency->have_host_name = true;
    }
    else if (!strcmp(variable, "dependent_hostgroup")
             || !strcmp(variable, "dependent_hostgroups")
             || !strcmp(variable, "dependent_hostgroup_name")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_hostdependency->dependent_hostgroup_name = string::dup(value);
      temp_hostdependency->have_dependent_hostgroup_name = true;
    }
    else if (!strcmp(variable, "dependent_host")
             || !strcmp(variable, "dependent_host_name")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_hostdependency->dependent_host_name = string::dup(value);
      temp_hostdependency->have_dependent_host_name = true;
    }
    else if (!strcmp(variable, "dependency_period")) {
      if (strcmp(value, XODTEMPLATE_NULL))
        temp_hostdependency->dependency_period = string::dup(value);
      temp_hostdependency->have_dependency_period = true;
    }
    else if (!strcmp(variable, "inherits_parent")) {
      temp_hostdependency->inherits_parent = (atoi(value) > 0) ? true : false;
      temp_hostdependency->have_inherits_parent = true;
    }
    else if (!strcmp(variable, "failure_options")
             || !strcmp(variable, "execution_failure_options")
             || !strcmp(variable, "execution_failure_criteria")) {
      for (temp_ptr = strtok(value, ", ");
	   temp_ptr != NULL;
           temp_ptr = strtok(NULL, ", ")) {
        if (!strcmp(temp_ptr, "o") || !strcmp(temp_ptr, "up"))
          temp_hostdependency->fail_on_up = true;
        else if (!strcmp(temp_ptr, "d") || !strcmp(temp_ptr, "down"))
          temp_hostdependency->fail_on_down = true;
        else if (!strcmp(temp_ptr, "u") || !strcmp(temp_ptr, "unreachable"))
          temp_hostdependency->fail_on_unreachable = true;
        else if (!strcmp(temp_ptr, "p") || !strcmp(temp_ptr, "pending"))
          temp_hostdependency->fail_on_pending = true;
        else if (!strcmp(temp_ptr, "n") || !strcmp(temp_ptr, "none")) {
          temp_hostdependency->fail_on_up = false;
          temp_hostdependency->fail_on_down = false;
          temp_hostdependency->fail_on_unreachable = false;
          temp_hostdependency->fail_on_pending = false;
        }
        else if (!strcmp(temp_ptr, "a") || !strcmp(temp_ptr, "all")) {
          temp_hostdependency->fail_on_up = true;
          temp_hostdependency->fail_on_down = true;
          temp_hostdependency->fail_on_unreachable = true;
          temp_hostdependency->fail_on_pending = true;
        }
        else {
          logger(log_config_error, basic)
            << "Error: Invalid execution dependency option '"
            << temp_ptr << "' in hostdependency definition.";
          return (ERROR);
        }
      }
      temp_hostdependency->have_dependency_options = true;
    }
    else if (!strcmp(variable, "register"))
      temp_hostdependency->register_object = (atoi(value) > 0) ? true : false;
    else {
      logger(log_config_error, basic)
        << "Error: Invalid hostdependency object directive '"
        << variable << "'.";
      return (ERROR);
    }
    break;

  default:
    return (ERROR);
    break;
  }

  /* free memory */
  delete[] variable;
  delete[] value;

  return (result);
}

/* completes an object definition */
int xodtemplate_end_object_definition(int options) {
  (void)options;

  xodtemplate_current_object = NULL;
  xodtemplate_current_object_type = XODTEMPLATE_NONE;

  return (OK);
}

/* adds a custom variable to a host */
xodtemplate_customvariablesmember* xodtemplate_add_custom_variable_to_host(
                                     xodtemplate_host* hst,
                                     char* varname,
                                     char* varvalue) {
  return (xodtemplate_add_custom_variable_to_object(
            &hst->custom_variables,
            varname,
            varvalue));
}

/* adds a custom variable to a service */
xodtemplate_customvariablesmember* xodtemplate_add_custom_variable_to_service(
                                     xodtemplate_service* svc,
                                     char* varname,
                                     char* varvalue) {
  return (xodtemplate_add_custom_variable_to_object(
            &svc->custom_variables,
            varname,
            varvalue));
}

/* adds a custom variable to an object */
xodtemplate_customvariablesmember* xodtemplate_add_custom_variable_to_object(
                                     xodtemplate_customvariablesmember** object_ptr,
                                     char* varname,
                                     char* varvalue) {
  xodtemplate_customvariablesmember* new_customvariablesmember = NULL;
  int x = 0;

  /* make sure we have the data we need */
  if (object_ptr == NULL)
    return (NULL);

  if (varname == NULL || !strcmp(varname, ""))
    return (NULL);

  /* allocate memory for a new member */
  new_customvariablesmember = new xodtemplate_customvariablesmember;
  new_customvariablesmember->variable_name = string::dup(varname);
  if (varvalue)
    new_customvariablesmember->variable_value = string::dup(varvalue);
  else
    new_customvariablesmember->variable_value = NULL;

  /* convert varname to all uppercase (saves CPU time during macro functions) */
  for (x = 0; new_customvariablesmember->variable_name[x] != '\x0'; x++)
    new_customvariablesmember->variable_name[x]
      = toupper(new_customvariablesmember->variable_name[x]);

  /* add the new member to the head of the member list */
  new_customvariablesmember->next = *object_ptr;
  *object_ptr = new_customvariablesmember;

  return (new_customvariablesmember);
}

/* parses a timeperod directive... :-) */
int xodtemplate_parse_timeperiod_directive(
      xodtemplate_timeperiod* tperiod,
      char const* var,
      char const* val) {
  char* input = NULL;
  char temp_buffer[5][MAX_INPUT_BUFFER] = { "", "", "", "", "" };
  int result = OK;
  int syear = 0;
  int smon = 0;
  int smday = 0;
  int swday = 0;
  int swday_offset = 0;
  int eyear = 0;
  int emon = 0;
  int emday = 0;
  int ewday = 0;
  int ewday_offset = 0;
  int skip_interval = 0;

  /* make sure we've got the reqs */
  if (tperiod == NULL || var == NULL || val == NULL)
    return (ERROR);

  /* we'll need the full (unsplit) input later */
  input = new char[strlen(var) + strlen(val) + 2];

  strcpy(input, var);
  strcat(input, " ");
  strcat(input, val);

  if (0)
    return (OK);
  /* calendar dates */
  else if (sscanf(
             input,
             "%4d-%2d-%2d - %4d-%2d-%2d / %d %[0-9:, -]",
             &syear,
             &smon,
             &smday,
             &eyear,
             &emon,
             &emday,
             &skip_interval,
             temp_buffer[0]) == 8) {
    /* add timerange exception */
    if (xodtemplate_add_exception_to_timeperiod(
          tperiod,
          DATERANGE_CALENDAR_DATE,
          syear,
          smon - 1,
          smday,
          0,
          0,
          eyear,
          emon - 1,
          emday,
          0,
          0,
          skip_interval,
          temp_buffer[0]) == NULL)
      result = ERROR;
  }
  else  if (sscanf(
              input,
              "%4d-%2d-%2d / %d %[0-9:, -]",
              &syear,
              &smon,
              &smday,
              &skip_interval,
              temp_buffer[0]) == 5) {
    eyear = syear;
    emon = smon;
    emday = smday;
    /* add timerange exception */
    if (xodtemplate_add_exception_to_timeperiod(
          tperiod,
          DATERANGE_CALENDAR_DATE,
          syear,
          smon - 1,
          smday,
          0,
          0,
          eyear,
          emon - 1,
          emday,
          0,
          0,
          skip_interval,
          temp_buffer[0]) == NULL)
      result = ERROR;
  }
  else if (sscanf(
             input,
             "%4d-%2d-%2d - %4d-%2d-%2d %[0-9:, -]",
             &syear,
             &smon,
             &smday,
             &eyear,
             &emon,
             &emday,
             temp_buffer[0]) == 7) {
    /* add timerange exception */
    if (xodtemplate_add_exception_to_timeperiod(
          tperiod,
          DATERANGE_CALENDAR_DATE,
          syear,
          smon - 1,
          smday,
          0,
          0,
          eyear,
          emon - 1,
          emday,
          0,
          0,
          0,
          temp_buffer[0]) == NULL)
      result = ERROR;
  }
  else if (sscanf(
             input,
             "%4d-%2d-%2d %[0-9:, -]",
             &syear,
             &smon,
             &smday,
             temp_buffer[0]) == 4) {
    eyear = syear;
    emon = smon;
    emday = smday;
    /* add timerange exception */
    if (xodtemplate_add_exception_to_timeperiod(
          tperiod,
          DATERANGE_CALENDAR_DATE,
          syear,
          smon - 1,
          smday,
          0,
          0,
          eyear,
          emon - 1,
          emday,
          0,
          0,
          0,
          temp_buffer[0]) == NULL)
      result = ERROR;
  }
  /* other types... */
  else if (sscanf(
             input,
             "%[a-z] %d %[a-z] - %[a-z] %d %[a-z] / %d %[0-9:, -]",
             temp_buffer[0],
             &swday_offset,
             temp_buffer[1],
             temp_buffer[2],
             &ewday_offset,
             temp_buffer[3],
             &skip_interval,
             temp_buffer[4]) == 8) {
    /* wednesday 1 january - thursday 2 july / 3 */
    if ((result = xodtemplate_get_weekday_from_string(temp_buffer[0], &swday)) == OK
        && (result = xodtemplate_get_month_from_string(temp_buffer[1], &smon)) == OK
        && (result = xodtemplate_get_weekday_from_string(temp_buffer[2], &ewday)) == OK
        && (result = xodtemplate_get_month_from_string(temp_buffer[3], &emon)) == OK) {
      /* add timeperiod exception */
      if (xodtemplate_add_exception_to_timeperiod(
            tperiod,
            DATERANGE_MONTH_WEEK_DAY,
            0,
            smon,
            0,
            swday,
            swday_offset,
            0,
            emon,
            0,
            ewday,
            ewday_offset,
            skip_interval,
            temp_buffer[4]) == NULL)
        result = ERROR;
    }
  }
  else if (sscanf(
             input,
             "%[a-z] %d - %[a-z] %d / %d %[0-9:, -]",
             temp_buffer[0],
             &smday,
             temp_buffer[1],
             &emday,
             &skip_interval,
             temp_buffer[2]) == 6) {
    /* february 1 - march 15 / 3 */
    /* monday 2 - thursday 3 / 2 */
    /* day 4 - day 6 / 2 */
    if ((result = xodtemplate_get_weekday_from_string(temp_buffer[0], &swday)) == OK
        && (result = xodtemplate_get_weekday_from_string(temp_buffer[1], &ewday)) == OK) {
      /* monday 2 - thursday 3 / 2 */
      swday_offset = smday;
      ewday_offset = emday;
      /* add timeperiod exception */
      if (xodtemplate_add_exception_to_timeperiod(
            tperiod,
            DATERANGE_WEEK_DAY,
            0,
            0,
            0,
            swday,
            swday_offset,
            0,
            0,
            0,
            ewday,
            ewday_offset,
            skip_interval,
            temp_buffer[2]) == NULL)
        result = ERROR;
    }
    else if ((result = xodtemplate_get_month_from_string(temp_buffer[0], &smon)) == OK
	     && (result = xodtemplate_get_month_from_string(temp_buffer[1], &emon)) == OK) {
      /* february 1 - march 15 / 3 */
      /* add timeperiod exception */
      if (xodtemplate_add_exception_to_timeperiod(
            tperiod,
            DATERANGE_MONTH_DATE,
            0,
            smon,
            smday,
            0,
            0,
            0,
            emon,
            emday,
            0,
            0,
            skip_interval,
            temp_buffer[2]) == NULL)
        result = ERROR;
    }
    else if (!strcmp(temp_buffer[0], "day")
             && !strcmp(temp_buffer[1], "day")) {
      /* day 4 - 6 / 2 */
      /* add timeperiod exception */
      result = OK;
      if (xodtemplate_add_exception_to_timeperiod(
            tperiod,
            DATERANGE_MONTH_DAY,
            0,
            0,
            smday,
            0,
            0,
            0,
            0,
            emday,
            0,
            0,
            skip_interval,
            temp_buffer[2]) == NULL)
        result = ERROR;
    }
  }
  else if (sscanf(
             input,
             "%[a-z] %d - %d / %d %[0-9:, -]",
             temp_buffer[0],
             &smday,
             &emday,
             &skip_interval,
             temp_buffer[1]) == 5) {
    /* february 1 - 15 / 3 */
    /* monday 2 - 3 / 2 */
    /* day 1 - 25 / 4 */
    if ((result = xodtemplate_get_weekday_from_string(
                    temp_buffer[0],
                    &swday)) == OK) {
      /* thursday 2 - 4 */
      swday_offset = smday;
      ewday = swday;
      ewday_offset = emday;
      /* add timeperiod exception */
      if (xodtemplate_add_exception_to_timeperiod(
            tperiod,
            DATERANGE_WEEK_DAY,
            0,
            0,
            0,
            swday,
            swday_offset,
            0,
            0,
            0,
            ewday,
            ewday_offset,
            skip_interval,
            temp_buffer[1]) == NULL)
        result = ERROR;
    }
    else if ((result = xodtemplate_get_month_from_string(
                         temp_buffer[0],
                         &smon)) == OK) {
      /* february 3 - 5 */
      emon = smon;
      /* add timeperiod exception */
      if (xodtemplate_add_exception_to_timeperiod(
            tperiod,
            DATERANGE_MONTH_DATE,
            0,
            smon,
            smday,
            0,
            0,
            0,
            emon,
            emday,
            0,
            0,
            skip_interval,
            temp_buffer[1]) == NULL)
        result = ERROR;
    }
    else if (!strcmp(temp_buffer[0], "day")) {
      /* day 1 - 4 */
      /* add timeperiod exception */
      result = OK;
      if (xodtemplate_add_exception_to_timeperiod(
            tperiod,
            DATERANGE_MONTH_DAY,
            0,
            0,
            smday,
            0,
            0,
            0,
            0,
            emday,
            0,
            0,
            skip_interval,
            temp_buffer[1]) == NULL)
        result = ERROR;
    }
  }
  else if (sscanf(
             input,
             "%[a-z] %d %[a-z] - %[a-z] %d %[a-z] %[0-9:, -]",
             temp_buffer[0],
             &swday_offset,
             temp_buffer[1],
             temp_buffer[2],
             &ewday_offset,
             temp_buffer[3],
             temp_buffer[4]) == 7) {
    /* wednesday 1 january - thursday 2 july */
    if ((result = xodtemplate_get_weekday_from_string(temp_buffer[0], &swday)) == OK
        && (result = xodtemplate_get_month_from_string(temp_buffer[1], &smon)) == OK
        && (result = xodtemplate_get_weekday_from_string(temp_buffer[2], &ewday)) == OK
        && (result = xodtemplate_get_month_from_string(temp_buffer[3], &emon)) == OK) {
      /* add timeperiod exception */
      if (xodtemplate_add_exception_to_timeperiod(
            tperiod,
            DATERANGE_MONTH_WEEK_DAY,
            0,
            smon,
            0,
            swday,
            swday_offset,
            0,
            emon,
            0,
            ewday,
            ewday_offset,
            0,
            temp_buffer[4]) == NULL)
        result = ERROR;
    }
  }
  else if (sscanf(
             input,
             "%[a-z] %d - %d %[0-9:, -]",
             temp_buffer[0],
             &smday,
             &emday,
             temp_buffer[1]) == 4) {
    /* february 3 - 5 */
    /* thursday 2 - 4 */
    /* day 1 - 4 */
    if ((result = xodtemplate_get_weekday_from_string(
                    temp_buffer[0],
                    &swday)) == OK) {
      /* thursday 2 - 4 */
      swday_offset = smday;
      ewday = swday;
      ewday_offset = emday;
      /* add timeperiod exception */
      if (xodtemplate_add_exception_to_timeperiod(
            tperiod,
            DATERANGE_WEEK_DAY,
            0,
            0,
            0,
            swday,
            swday_offset,
            0,
            0,
            0,
            ewday,
            ewday_offset,
            0,
            temp_buffer[1]) == NULL)
        result = ERROR;
    }
    else if ((result = xodtemplate_get_month_from_string(
                         temp_buffer[0],
                         &smon)) == OK) {
      /* february 3 - 5 */
      emon = smon;
      /* add timeperiod exception */
      if (xodtemplate_add_exception_to_timeperiod(
            tperiod,
            DATERANGE_MONTH_DATE,
            0,
            smon,
            smday,
            0,
            0,
            0,
            emon,
            emday,
            0,
            0,
            0,
            temp_buffer[1]) == NULL)
        result = ERROR;
    }
    else if (!strcmp(temp_buffer[0], "day")) {
      /* day 1 - 4 */
      /* add timeperiod exception */
      result = OK;
      if (xodtemplate_add_exception_to_timeperiod(
            tperiod,
            DATERANGE_MONTH_DAY,
            0,
            0,
            smday,
            0,
            0,
            0,
            0,
            emday,
            0,
            0,
            0,
            temp_buffer[1]) == NULL)
        result = ERROR;
    }
  }
  else if (sscanf(
             input,
             "%[a-z] %d - %[a-z] %d %[0-9:, -]",
             temp_buffer[0],
             &smday,
             temp_buffer[1],
             &emday,
             temp_buffer[2]) == 5) {
    /* february 1 - march 15 */
    /* monday 2 - thursday 3 */
    /* day 1 - day 5 */
    if ((result = xodtemplate_get_weekday_from_string(temp_buffer[0], &swday)) == OK
        && (result = xodtemplate_get_weekday_from_string(temp_buffer[1], &ewday)) == OK) {
      /* monday 2 - thursday 3 */
      swday_offset = smday;
      ewday_offset = emday;
      /* add timeperiod exception */
      if (xodtemplate_add_exception_to_timeperiod(
            tperiod,
            DATERANGE_WEEK_DAY,
            0,
            0,
            0,
            swday,
            swday_offset,
            0,
            0,
            0,
            ewday,
            ewday_offset,
            0,
            temp_buffer[2]) == NULL)
        result = ERROR;
    }
    else if ((result = xodtemplate_get_month_from_string(temp_buffer[0], &smon)) == OK
	     && (result = xodtemplate_get_month_from_string(temp_buffer[1], &emon)) == OK) {
      /* february 1 - march 15 */
      /* add timeperiod exception */
      if (xodtemplate_add_exception_to_timeperiod(
            tperiod,
            DATERANGE_MONTH_DATE,
            0,
            smon,
            smday,
            0,
            0,
            0,
            emon,
            emday,
            0,
            0,
            0,
            temp_buffer[2]) == NULL)
        result = ERROR;
    }
    else if (!strcmp(temp_buffer[0], "day")
             && !strcmp(temp_buffer[1], "day")) {
      /* day 1 - day 5 */
      /* add timeperiod exception */
      result = OK;
      if (xodtemplate_add_exception_to_timeperiod(
            tperiod,
            DATERANGE_MONTH_DAY,
            0,
            0,
            smday,
            0,
            0,
            0,
            0,
            emday,
            0,
            0,
            0,
            temp_buffer[2]) == NULL)
        result = ERROR;
    }
  }
  else if (sscanf(
             input,
             "%[a-z] %d%*[ \t]%[0-9:, -]",
             temp_buffer[0],
             &smday,
             temp_buffer[1]) == 3) {
    /* february 3 */
    /* thursday 2 */
    /* day 1 */
    if ((result = xodtemplate_get_weekday_from_string(
                    temp_buffer[0],
                    &swday)) == OK) {
      /* thursday 2 */
      swday_offset = smday;
      ewday = swday;
      ewday_offset = swday_offset;
      /* add timeperiod exception */
      if (xodtemplate_add_exception_to_timeperiod(
            tperiod,
            DATERANGE_WEEK_DAY,
            0,
            0,
            0,
            swday,
            swday_offset,
            0,
            0,
            0,
            ewday,
            ewday_offset,
            0,
            temp_buffer[1]) == NULL)
        result = ERROR;
    }
    else if ((result = xodtemplate_get_month_from_string(
                         temp_buffer[0],
                         &smon)) == OK) {
      /* february 3 */
      emon = smon;
      emday = smday;
      /* add timeperiod exception */
      if (xodtemplate_add_exception_to_timeperiod(
            tperiod,
            DATERANGE_MONTH_DATE,
            0,
            smon,
            smday,
            0,
            0,
            0,
            emon,
            emday,
            0,
            0,
            0,
            temp_buffer[1]) == NULL)
        result = ERROR;
    }
    else if (!strcmp(temp_buffer[0], "day")) {
      /* day 1 */
      emday = smday;
      /* add timeperiod exception */
      result = OK;
      if (xodtemplate_add_exception_to_timeperiod(
            tperiod,
            DATERANGE_MONTH_DAY,
            0,
            0,
            smday,
            0,
            0,
            0,
            0,
            emday,
            0,
            0,
            0,
            temp_buffer[1]) == NULL)
        result = ERROR;
    }
  }
  else if (sscanf(
             input,
             "%[a-z] %d %[a-z] %[0-9:, -]",
             temp_buffer[0],
             &swday_offset,
             temp_buffer[1],
             temp_buffer[2]) == 4) {
    /* thursday 3 february */
    if ((result = xodtemplate_get_weekday_from_string(temp_buffer[0], &swday)) == OK
        && (result = xodtemplate_get_month_from_string(temp_buffer[1], &smon)) == OK) {
      emon = smon;
      ewday = swday;
      ewday_offset = swday_offset;
      /* add timeperiod exception */
      if (xodtemplate_add_exception_to_timeperiod(
            tperiod,
            DATERANGE_MONTH_WEEK_DAY,
            0,
            smon,
            0,
            swday,
            swday_offset,
            0,
            emon,
            0,
            ewday,
            ewday_offset,
            0,
            temp_buffer[2]) == NULL)
        result = ERROR;
    }
  }
  else if (sscanf(
             input,
             "%[a-z] %[0-9:, -]",
             temp_buffer[0],
             temp_buffer[1]) == 2) {
    /* monday */
    if ((result = xodtemplate_get_weekday_from_string(
                    temp_buffer[0],
                    &swday)) == OK) {
      /* add normal weekday timerange */
      tperiod->timeranges[swday] = string::dup(temp_buffer[1]);
    }
  }
  else
    result = ERROR;

  /* free memory */
  delete[] input;

  if (result == ERROR) {
    printf("Error: Could not parse timeperiod directive '%s'!", input);
    return (ERROR);
  }

  return (OK);
}

/* add a new exception to a timeperiod */
xodtemplate_daterange* xodtemplate_add_exception_to_timeperiod(
                         xodtemplate_timeperiod* period,
                         int type,
                         int syear,
                         int smon,
                         int smday,
                         int swday,
                         int swday_offset,
                         int eyear,
                         int emon,
                         int emday,
                         int ewday,
                         int ewday_offset,
                         int skip_interval,
                         char* timeranges) {
  xodtemplate_daterange *new_daterange = NULL;

  /* make sure we have the data we need */
  if (period == NULL || timeranges == NULL)
    return (NULL);

  /* allocate memory for the date range range */
  new_daterange = new xodtemplate_daterange;

  new_daterange->next = NULL;
  new_daterange->type = type;
  new_daterange->syear = syear;
  new_daterange->smon = smon;
  new_daterange->smday = smday;
  new_daterange->swday = swday;
  new_daterange->swday_offset = swday_offset;
  new_daterange->eyear = eyear;
  new_daterange->emon = emon;
  new_daterange->emday = emday;
  new_daterange->ewday = ewday;
  new_daterange->ewday_offset = ewday_offset;
  new_daterange->skip_interval = skip_interval;
  new_daterange->timeranges = string::dup(timeranges);

  /* add the new date range to the head of the range list for this exception type */
  new_daterange->next = period->exceptions[type];
  period->exceptions[type] = new_daterange;
  return (new_daterange);
}

int xodtemplate_get_month_from_string(char* str, int* month) {
  static char const* months[12] = {
    "january",
    "february",
    "march",
    "april",
    "may",
    "june",
    "july",
    "august",
    "september",
    "october",
    "november",
    "december"
  };

  if (str == NULL || month == NULL)
    return (ERROR);

  for (int x = 0; x < 12; x++) {
    if (!strcmp(str, months[x])) {
      *month = x;
      return (OK);
    }
  }
  return (ERROR);
}

int xodtemplate_get_weekday_from_string(char* str, int* weekday) {
  static char const* days[7] = {
    "sunday",
    "monday",
    "tuesday",
    "wednesday",
    "thursday",
    "friday",
    "saturday"
  };

  if (str == NULL || weekday == NULL)
    return (ERROR);

  for (int x = 0; x < 7; x++) {
    if (!strcmp(str, days[x])) {
      *weekday = x;
      return (OK);
    }
  }
  return (ERROR);
}

/******************************************************************/
/***************** OBJECT DUPLICATION FUNCTIONS *******************/
/******************************************************************/

/* duplicates service definitions */
int xodtemplate_duplicate_services() {
  int result = OK;
  xodtemplate_service* temp_service = NULL;
  xodtemplate_memberlist* temp_memberlist = NULL;
  xodtemplate_memberlist* temp_rejectlist = NULL;
  xodtemplate_memberlist* this_memberlist = NULL;
  char* host_name = NULL;
  int first_item = false;

  /****** DUPLICATE SERVICE DEFINITIONS WITH ONE OR MORE HOSTGROUP AND/OR HOST NAMES ******/
  for (temp_service = xodtemplate_service_list;
       temp_service != NULL;
       temp_service = temp_service->next) {

    /* skip service definitions without enough data */
    if (temp_service->hostgroup_name == NULL
        && temp_service->host_name == NULL)
      continue;

    if (temp_service->hostgroup_name != NULL) {
      if (xodtemplate_expand_hostgroups(
            &temp_memberlist,
            &temp_rejectlist,
            temp_service->hostgroup_name,
            temp_service->_config_file,
            temp_service->_start_line) == ERROR) {
        return (ERROR);
      }
      else {
        xodtemplate_free_memberlist(&temp_rejectlist);
        if (temp_memberlist != NULL)
          xodtemplate_free_memberlist(&temp_memberlist);
        else {
          continue ;
        }
      }
    }

    /* skip services that shouldn't be registered */
    if (temp_service->register_object == false)
      continue;

    /* get list of hosts */
    temp_memberlist = xodtemplate_expand_hostgroups_and_hosts(
                        temp_service->hostgroup_name,
                        temp_service->host_name,
                        temp_service->_config_file,
                        temp_service->_start_line);
    if (temp_memberlist == NULL) {
      logger(log_config_error, basic)
        << "Error: Could not expand hostgroups and/or hosts specified in "
        "service (config file '"
        << xodtemplate_config_file_name(temp_service->_config_file)
        << "', starting on line " << temp_service->_start_line <<")";
      return (ERROR);
    }

    /* add a copy of the service for every host in the hostgroup/host name list */
    first_item = true;
    for (this_memberlist = temp_memberlist;
	 this_memberlist != NULL;
         this_memberlist = this_memberlist->next) {

      /* if this is the first duplication, use the existing entry */
      if (first_item == true) {

        delete[] temp_service->host_name;
        temp_service->host_name = string::dup(this_memberlist->name1);

        first_item = false;
        continue;
      }

      /* duplicate service definition */
      result = xodtemplate_duplicate_service(
                 temp_service,
                 this_memberlist->name1);

      /* exit on error */
      if (result == ERROR) {
        delete[] host_name;
        return (ERROR);
      }
    }

    /* free memory we used for host list */
    xodtemplate_free_memberlist(&temp_memberlist);
  }

  /***************************************/
  /* SKIPLIST STUFF FOR FAST SORT/SEARCH */
  /***************************************/

  /* First loop for single host service definition */
  for (temp_service = xodtemplate_service_list;
       temp_service != NULL;
       temp_service = temp_service->next) {

    /* skip services that shouldn't be registered */
    if (temp_service->register_object == false)
      continue;

    /* skip service definitions without enough data */
    if (temp_service->host_name == NULL
        || temp_service->service_description == NULL)
      continue;

    if (xodtemplate_is_service_is_from_hostgroup(temp_service)) {
      continue;
    }

    result = skiplist_insert(
               xobject_skiplists[X_SERVICE_SKIPLIST],
               (void*)temp_service);
    switch (result) {
    case SKIPLIST_OK:
      result = OK;
      break;

    case SKIPLIST_ERROR_DUPLICATE:
      logger(log_config_warning, basic)
        << "Warning: Duplicate definition found for service '"
        << temp_service->service_description << "' on host '"
        << temp_service->host_name << "' (config file '"
        << xodtemplate_config_file_name(temp_service->_config_file)
        << "', starting on line " << temp_service->_start_line << ")";
      result = ERROR;
      break;

    default:
      result = ERROR;
      break;
    }
  }

  /* second loop for host group service definition */
  /* add services to skiplist for fast searches */
  for (temp_service = xodtemplate_service_list;
       temp_service != NULL;
       temp_service = temp_service->next) {

    /* skip services that shouldn't be registered */
    if (temp_service->register_object == false)
      continue;

    /* skip service definitions without enough data */
    if (temp_service->host_name == NULL
        || temp_service->service_description == NULL)
      continue;

    if (!xodtemplate_is_service_is_from_hostgroup(temp_service)) {
      continue;
    }
    /*The flag X_SERVICE_IS_FROM_HOSTGROUP is set, unset it */
    xodtemplate_unset_service_is_from_hostgroup(temp_service);

    result = skiplist_insert(
               xobject_skiplists[X_SERVICE_SKIPLIST],
               (void*)temp_service);
    switch (result) {
    case SKIPLIST_OK:
      result = OK;
      break;

    case SKIPLIST_ERROR_DUPLICATE:
      logger(log_config_warning, basic)
        << "Warning: Duplicate definition found for service '"
        << temp_service->service_description << "' on host '"
        << temp_service->host_name << "' (config file '"
        << xodtemplate_config_file_name(temp_service->_config_file)
        << "', starting on line " << temp_service->_start_line << ")";
      result = ERROR;
      break;

    default:
      result = ERROR;
      break;
    }
  }

  return (OK);
}

/* duplicates object definitions */
int xodtemplate_duplicate_objects() {
  int result = OK;
  xodtemplate_hostdependency* temp_hostdependency = NULL;
  xodtemplate_servicedependency* temp_servicedependency = NULL;
  xodtemplate_memberlist* master_hostlist = NULL;
  xodtemplate_memberlist* dependent_hostlist = NULL;
  xodtemplate_memberlist* master_servicelist = NULL;
  xodtemplate_memberlist* dependent_servicelist = NULL;
  xodtemplate_memberlist* temp_masterhost = NULL;
  xodtemplate_memberlist* temp_dependenthost = NULL;
  xodtemplate_memberlist* temp_masterservice = NULL;
  xodtemplate_memberlist* temp_dependentservice = NULL;
  char* service_descriptions = NULL;
  int first_item = false;
  int same_host_servicedependency = false;

  /*************************************/
  /* SERVICES ARE DUPLICATED ELSEWHERE */
  /*************************************/

  /****** DUPLICATE HOST DEPENDENCY DEFINITIONS WITH MULTIPLE HOSTGROUP AND/OR HOST NAMES (MASTER AND DEPENDENT) ******/
  for (temp_hostdependency = xodtemplate_hostdependency_list;
       temp_hostdependency != NULL;
       temp_hostdependency = temp_hostdependency->next) {

    /* skip host dependencies without enough data */
    if (temp_hostdependency->hostgroup_name == NULL
        && temp_hostdependency->dependent_hostgroup_name == NULL
        && temp_hostdependency->host_name == NULL
        && temp_hostdependency->dependent_host_name == NULL)
      continue;

    /* get list of master host names */
    master_hostlist = xodtemplate_expand_hostgroups_and_hosts(
                        temp_hostdependency->hostgroup_name,
                        temp_hostdependency->host_name,
                        temp_hostdependency->_config_file,
                        temp_hostdependency->_start_line);
    if (master_hostlist == NULL) {
      logger(log_config_error, basic)
        << "Error: Could not expand master hostgroups and/or hosts "
        "specified in host dependency (config file '"
        << xodtemplate_config_file_name(temp_hostdependency->_config_file)
        << "', starting on line " << temp_hostdependency->_start_line
        << ")";
        return (ERROR);
    }

    /* get list of dependent host names */
    dependent_hostlist = xodtemplate_expand_hostgroups_and_hosts(
                           temp_hostdependency->dependent_hostgroup_name,
                           temp_hostdependency->dependent_host_name,
                           temp_hostdependency->_config_file,
                           temp_hostdependency->_start_line);
    if (dependent_hostlist == NULL) {
      logger(log_config_error, basic)
        << "Error: Could not expand dependent hostgroups and/or hosts "
        "specified in host dependency (config file '"
        << xodtemplate_config_file_name(temp_hostdependency->_config_file)
        << "', starting on line " << temp_hostdependency->_start_line
        << ")";
      xodtemplate_free_memberlist(&master_hostlist);
      return (ERROR);
    }

    /* duplicate the dependency definitions */
    first_item = true;
    for (temp_masterhost = master_hostlist;
	 temp_masterhost != NULL;
         temp_masterhost = temp_masterhost->next) {

      for (temp_dependenthost = dependent_hostlist;
           temp_dependenthost != NULL;
           temp_dependenthost = temp_dependenthost->next) {

        /* temp=master, this=dep */

        /* existing definition gets first names */
        if (first_item == true) {
          delete[] temp_hostdependency->host_name;
          delete[] temp_hostdependency->dependent_host_name;
          temp_hostdependency->host_name
            = string::dup(temp_masterhost->name1);
          temp_hostdependency->dependent_host_name
            = string::dup(temp_dependenthost->name1);
          first_item = false;
          continue;
        }
        else
          result = xodtemplate_duplicate_hostdependency(
                     temp_hostdependency,
                     temp_masterhost->name1,
                     temp_dependenthost->name1);
        /* exit on error */
        if (result == ERROR) {
          xodtemplate_free_memberlist(&master_hostlist);
          xodtemplate_free_memberlist(&dependent_hostlist);
          return (ERROR);
        }
      }
    }

    /* free memory used to store host lists */
    xodtemplate_free_memberlist(&master_hostlist);
    xodtemplate_free_memberlist(&dependent_hostlist);
  }

  /****** PROCESS SERVICE DEPENDENCIES WITH MASTER SERVICEGROUPS *****/
  for (temp_servicedependency = xodtemplate_servicedependency_list;
       temp_servicedependency != NULL;
       temp_servicedependency = temp_servicedependency->next) {

    /* skip templates */
    if (temp_servicedependency->register_object == 0)
      continue;

    /* expand master servicegroups into a list of services */
    if (temp_servicedependency->servicegroup_name) {
      master_servicelist = xodtemplate_expand_servicegroups_and_services(
                             temp_servicedependency->servicegroup_name,
                             NULL,
                             NULL,
                             temp_servicedependency->_config_file,
                             temp_servicedependency->_start_line);
      if (master_servicelist == NULL) {
        logger(log_config_error, basic)
          << "Error: Could not expand master servicegroups "
          "specified in service dependency (config file '"
          << xodtemplate_config_file_name(temp_servicedependency->_config_file)
          << "', starting on line "
          << temp_servicedependency->_start_line << ")";
        return (ERROR);
      }

      /* if dependency also has master host, hostgroup, and/or service, we must split that off to another definition */
      if (temp_servicedependency->host_name != NULL
          || temp_servicedependency->hostgroup_name != NULL
          || temp_servicedependency->service_description != NULL) {

        /* duplicate everything except master servicegroup */
        xodtemplate_duplicate_servicedependency(
          temp_servicedependency,
          temp_servicedependency->host_name,
          temp_servicedependency->service_description,
          temp_servicedependency->hostgroup_name,
          NULL,
          temp_servicedependency->dependent_host_name,
          temp_servicedependency->dependent_service_description,
          temp_servicedependency->dependent_hostgroup_name,
          temp_servicedependency->dependent_servicegroup_name);

        /* clear values in this definition */
        temp_servicedependency->have_host_name = false;
        temp_servicedependency->have_service_description = false;
        temp_servicedependency->have_hostgroup_name = false;
        delete[] temp_servicedependency->host_name;
        delete[] temp_servicedependency->service_description;
        delete[] temp_servicedependency->hostgroup_name;
        temp_servicedependency->host_name = NULL;
        temp_servicedependency->service_description = NULL;
        temp_servicedependency->hostgroup_name = NULL;
      }

      /* duplicate service dependency entries */
      first_item = true;
      for (temp_masterservice = master_servicelist;
           temp_masterservice != NULL;
           temp_masterservice = temp_masterservice->next) {

        /* just in case */
        if (temp_masterservice->name1 == NULL
            || temp_masterservice->name2 == NULL)
          continue;

        /* if this is the first duplication, use the existing entry */
        if (first_item == true) {

          delete[] temp_servicedependency->host_name;
          temp_servicedependency->host_name
            = string::dup(temp_masterservice->name1);

          delete[] temp_servicedependency->service_description;
          temp_servicedependency->service_description
            = string::dup(temp_masterservice->name2);

          /* clear the master servicegroup */
          temp_servicedependency->have_servicegroup_name = false;
          delete[] temp_servicedependency->servicegroup_name;
          temp_servicedependency->servicegroup_name = NULL;

          first_item = false;
          continue;
        }

        /* duplicate service dependency definition */
        result = xodtemplate_duplicate_servicedependency(
                   temp_servicedependency,
                   temp_masterservice->name1,
                   temp_masterservice->name2,
                   NULL,
                   NULL,
                   temp_servicedependency->dependent_host_name,
                   temp_servicedependency->dependent_service_description,
                   temp_servicedependency->dependent_hostgroup_name,
                   temp_servicedependency->dependent_servicegroup_name);
        /* exit on error */
        if (result == ERROR) {
          xodtemplate_free_memberlist(&master_servicelist);
          return (ERROR);
        }
      }

      /* free memory we used for service list */
      xodtemplate_free_memberlist(&master_servicelist);
    }
  }

  /****** PROCESS SERVICE DEPENDENCY MASTER HOSTS/HOSTGROUPS/SERVICES *****/
  for (temp_servicedependency = xodtemplate_servicedependency_list;
       temp_servicedependency != NULL;
       temp_servicedependency = temp_servicedependency->next) {

    /* skip templates */
    if (temp_servicedependency->register_object == 0)
      continue;

    /* expand master hosts/hostgroups into a list of host names */
    if (temp_servicedependency->host_name != NULL
        || temp_servicedependency->hostgroup_name != NULL) {

#ifdef DEBUG_SERVICE_DEPENDENCIES
      printf(
        "1a) H: %s  HG: %s  SD: %s\n",
        temp_servicedependency->host_name,
        temp_servicedependency->hostgroup_name,
        temp_servicedependency->service_description);
#endif

      master_hostlist = xodtemplate_expand_hostgroups_and_hosts(
                          temp_servicedependency->hostgroup_name,
                          temp_servicedependency->host_name,
                          temp_servicedependency->_config_file,
                          temp_servicedependency->_start_line);
      if (master_hostlist == NULL) {
        logger(log_config_error, basic)
          << "Error: Could not expand master hostgroups and/or hosts specified "
          "in service dependency (config file '"
          << xodtemplate_config_file_name(temp_servicedependency->_config_file)
          << "', starting on line "
          << temp_servicedependency->_start_line << ")";
        return (ERROR);
      }

      /* save service descriptions for later */
      if (temp_servicedependency->service_description)
        service_descriptions
          = string::dup(temp_servicedependency->service_description);

      /* for each host, expand master services */
      first_item = true;
      for (temp_masterhost = master_hostlist;
	   temp_masterhost != NULL;
           temp_masterhost = temp_masterhost->next) {

        master_servicelist = xodtemplate_expand_servicegroups_and_services(
                               NULL,
                               temp_masterhost->name1,
                               service_descriptions,
                               temp_servicedependency->_config_file,
                               temp_servicedependency->_start_line);
        if (master_servicelist == NULL) {
          logger(log_config_error, basic)
            << "Error: Could not expand master services specified in "
            "service dependency (config file '"
            << xodtemplate_config_file_name(temp_servicedependency->_config_file)
            << "', starting on line "
            << temp_servicedependency->_start_line << ")";
          return (ERROR);
        }

        /* duplicate service dependency entries */
        for (temp_masterservice = master_servicelist;
             temp_masterservice != NULL;
             temp_masterservice = temp_masterservice->next) {

          /* just in case */
          if (temp_masterservice->name1 == NULL
              || temp_masterservice->name2 == NULL)
            continue;

          /* if this is the first duplication, use the existing entry */
          if (first_item == true) {

            delete[] temp_servicedependency->host_name;
            temp_servicedependency->host_name
              = string::dup(temp_masterhost->name1);

            delete[] temp_servicedependency->service_description;
            temp_servicedependency->service_description
              = string::dup(temp_masterservice->name2);

            first_item = false;
            continue;
          }

          /* duplicate service dependency definition */
          result = xodtemplate_duplicate_servicedependency(
                     temp_servicedependency,
                     temp_masterhost->name1,
                     temp_masterservice->name2,
                     NULL,
                     NULL,
                     temp_servicedependency->dependent_host_name,
                     temp_servicedependency->dependent_service_description,
                     temp_servicedependency->dependent_hostgroup_name,
                     temp_servicedependency->dependent_servicegroup_name);
          /* exit on error */
          if (result == ERROR) {
            xodtemplate_free_memberlist(&master_hostlist);
            xodtemplate_free_memberlist(&master_servicelist);
            return (ERROR);
          }
        }

        /* free memory we used for service list */
        xodtemplate_free_memberlist(&master_servicelist);
      }

      /* free service descriptions */
      delete[] service_descriptions;
      service_descriptions = NULL;

      /* free memory we used for host list */
      xodtemplate_free_memberlist(&master_hostlist);
    }
  }


#ifdef DEBUG_SERVICE_DEPENDENCIES
  for (temp_servicedependency = xodtemplate_servicedependency_list;
       temp_servicedependency != NULL;
       temp_servicedependency = temp_servicedependency->next) {
    printf(
      "1**) H: %s  HG: %s  SG: %s  SD: %s  DH: %s  DHG: %s  DSG: %s  DSD: %s\n",
      temp_servicedependency->host_name,
      temp_servicedependency->hostgroup_name,
      temp_servicedependency->servicegroup_name,
      temp_servicedependency->service_description,
      temp_servicedependency->dependent_host_name,
      temp_servicedependency->dependent_hostgroup_name,
      temp_servicedependency->dependent_servicegroup_name,
      temp_servicedependency->dependent_service_description);
  }
  printf("\n");
#endif

  /****** PROCESS SERVICE DEPENDENCIES WITH DEPENDENT SERVICEGROUPS *****/
  for (temp_servicedependency = xodtemplate_servicedependency_list;
       temp_servicedependency != NULL;
       temp_servicedependency = temp_servicedependency->next) {

    /* skip templates */
    if (temp_servicedependency->register_object == 0)
      continue;

    /* expand dependent servicegroups into a list of services */
    if (temp_servicedependency->dependent_servicegroup_name) {
      dependent_servicelist
        = xodtemplate_expand_servicegroups_and_services(
            temp_servicedependency->dependent_servicegroup_name,
            NULL,
            NULL,
            temp_servicedependency->_config_file,
            temp_servicedependency->_start_line);
      if (dependent_servicelist == NULL) {
        logger(log_config_error, basic)
          << "Error: Could not expand dependent servicegroups "
          "specified in service dependency (config file '"
          << xodtemplate_config_file_name(temp_servicedependency->_config_file)
          << "', starting on line "
          << temp_servicedependency->_start_line << ")";
        return (ERROR);
      }

      /* if dependency also has dependent host, hostgroup, and/or service, we must split that off to another definition */
      if (temp_servicedependency->dependent_host_name != NULL
          || temp_servicedependency->dependent_hostgroup_name != NULL
          || temp_servicedependency->dependent_service_description != NULL) {

        /* duplicate everything except dependent servicegroup */
        xodtemplate_duplicate_servicedependency(
          temp_servicedependency,
          temp_servicedependency->host_name,
          temp_servicedependency->service_description,
          temp_servicedependency->hostgroup_name,
          temp_servicedependency->servicegroup_name,
          temp_servicedependency->dependent_host_name,
          temp_servicedependency->dependent_service_description,
          temp_servicedependency->dependent_hostgroup_name,
          NULL);

        /* clear values in this definition */
        temp_servicedependency->have_dependent_host_name = false;
        temp_servicedependency->have_dependent_service_description = false;
        temp_servicedependency->have_dependent_hostgroup_name = false;
        delete[] temp_servicedependency->dependent_host_name;
        delete[] temp_servicedependency->dependent_service_description;
        delete[] temp_servicedependency->dependent_hostgroup_name;
        temp_servicedependency->dependent_host_name = NULL;
        temp_servicedependency->dependent_service_description = NULL;
        temp_servicedependency->dependent_hostgroup_name = NULL;
      }

      /* Detected same host servicegroups dependencies */
      same_host_servicedependency = false;
      if (temp_servicedependency->host_name == NULL
          && temp_servicedependency->hostgroup_name == NULL)
        same_host_servicedependency = true;

      /* duplicate service dependency entries */
      first_item = true;
      for (temp_dependentservice = dependent_servicelist;
           temp_dependentservice != NULL;
           temp_dependentservice = temp_dependentservice->next) {

        /* just in case */
        if (temp_dependentservice->name1 == NULL
            || temp_dependentservice->name2 == NULL)
          continue;

        /* if this is the first duplication, use the existing entry */
        if (first_item == true) {

          delete[] temp_servicedependency->dependent_host_name;
          temp_servicedependency->dependent_host_name
            = string::dup(temp_dependentservice->name1);

          delete[] temp_servicedependency->dependent_service_description;
          temp_servicedependency->dependent_service_description
            = string::dup(temp_dependentservice->name2);

          /* Same host servicegroups dependencies: Use dependentservice host_name for master host_name */
          if (same_host_servicedependency == true)
            temp_servicedependency->host_name
              = string::dup(temp_dependentservice->name1);

          /* clear the dependent servicegroup */
          temp_servicedependency->have_dependent_servicegroup_name = false;
          delete[] temp_servicedependency->dependent_servicegroup_name;
          temp_servicedependency->dependent_servicegroup_name = NULL;

          first_item = false;
          continue;
        }

        /* duplicate service dependency definition */
        /* Same host servicegroups dependencies: Use dependentservice host_name for master host_name instead of undefined (not yet) master host_name */
        if (same_host_servicedependency == true)
          result = xodtemplate_duplicate_servicedependency(
                     temp_servicedependency,
                     temp_dependentservice->name1,
                     temp_servicedependency->service_description,
                     NULL,
                     NULL,
                     temp_dependentservice->name1,
                     temp_dependentservice->name2,
                     NULL,
                     NULL);
        else
          result = xodtemplate_duplicate_servicedependency(
                     temp_servicedependency,
                     temp_servicedependency->host_name,
                     temp_servicedependency->service_description,
                     NULL,
                     NULL,
                     temp_dependentservice->name1,
                     temp_dependentservice->name2,
                     NULL,
                     NULL);
        /* exit on error */
        if (result == ERROR) {
          xodtemplate_free_memberlist(&dependent_servicelist);
          return (ERROR);
        }
      }

      /* free memory we used for service list */
      xodtemplate_free_memberlist(&dependent_servicelist);
    }
  }

#ifdef DEBUG_SERVICE_DEPENDENCIES
  printf("\n");
  for (temp_servicedependency = xodtemplate_servicedependency_list;
       temp_servicedependency != NULL;
       temp_servicedependency = temp_servicedependency->next) {
    printf(
      "2**) H: %s  HG: %s  SG: %s  SD: %s  DH: %s  DHG: %s  DSG: %s  DSD: %s\n",
      temp_servicedependency->host_name,
      temp_servicedependency->hostgroup_name,
      temp_servicedependency->servicegroup_name,
      temp_servicedependency->service_description,
      temp_servicedependency->dependent_host_name,
      temp_servicedependency->dependent_hostgroup_name,
      temp_servicedependency->dependent_servicegroup_name,
      temp_servicedependency->dependent_service_description);
  }
#endif

  /****** PROCESS SERVICE DEPENDENCY DEPENDENT HOSTS/HOSTGROUPS/SERVICES *****/
  for (temp_servicedependency = xodtemplate_servicedependency_list;
       temp_servicedependency != NULL;
       temp_servicedependency = temp_servicedependency->next) {

    /* skip templates */
    if (temp_servicedependency->register_object == 0)
      continue;

    /* ADDED 02/04/2007 - special case for "same host" dependencies */
    if (temp_servicedependency->dependent_host_name == NULL
        && temp_servicedependency->dependent_hostgroup_name == NULL) {
      if (temp_servicedependency->host_name)
        temp_servicedependency->dependent_host_name
          = string::dup(temp_servicedependency->host_name);
    }

    /* expand dependent hosts/hostgroups into a list of host names */
    if (temp_servicedependency->dependent_host_name != NULL
        || temp_servicedependency->dependent_hostgroup_name != NULL) {

      dependent_hostlist
        = xodtemplate_expand_hostgroups_and_hosts(
            temp_servicedependency->dependent_hostgroup_name,
            temp_servicedependency->dependent_host_name,
            temp_servicedependency->_config_file,
            temp_servicedependency->_start_line);
      if (dependent_hostlist == NULL) {
        logger(log_config_error, basic)
          << "Error: Could not expand dependent hostgroups and/or "
          "hosts specified in service dependency (config file '"
          << xodtemplate_config_file_name(temp_servicedependency->_config_file)
          << "', starting on line "
          << temp_servicedependency->_start_line << ")";
          return (ERROR);
      }

      /* save service descriptions for later */
      if (temp_servicedependency->dependent_service_description)
        service_descriptions
          = string::dup(temp_servicedependency->dependent_service_description);

      /* for each host, expand dependent services */
      first_item = true;
      for (temp_dependenthost = dependent_hostlist;
           temp_dependenthost != NULL;
           temp_dependenthost = temp_dependenthost->next) {

        dependent_servicelist
          = xodtemplate_expand_servicegroups_and_services(
              NULL,
              temp_dependenthost->name1,
              service_descriptions,
              temp_servicedependency->_config_file,
              temp_servicedependency->_start_line);
        if (dependent_servicelist == NULL) {
          logger(log_config_error, basic)
            << "Error: Could not expand dependent services "
            "specified in service dependency (config file '"
            << xodtemplate_config_file_name(temp_servicedependency->_config_file)
            << "', starting on line "
            << temp_servicedependency->_start_line << ")";
          return (ERROR);
        }

        /* duplicate service dependency entries */
        for (temp_dependentservice = dependent_servicelist;
             temp_dependentservice != NULL;
             temp_dependentservice = temp_dependentservice->next) {

          /* just in case */
          if (temp_dependentservice->name1 == NULL
              || temp_dependentservice->name2 == NULL)
            continue;

          /* if this is the first duplication, use the existing entry */
          if (first_item == true) {

            delete[] temp_servicedependency->dependent_host_name;
            temp_servicedependency->dependent_host_name
              = string::dup(temp_dependentservice->name1);

            delete[] temp_servicedependency->dependent_service_description;
            temp_servicedependency->dependent_service_description
              = string::dup(temp_dependentservice->name2);

            first_item = false;
            continue;
          }

          /* duplicate service dependency definition */
          result = xodtemplate_duplicate_servicedependency(
                     temp_servicedependency,
                     temp_servicedependency->host_name,
                     temp_servicedependency->service_description,
                     NULL,
                     NULL,
                     temp_dependentservice->name1,
                     temp_dependentservice->name2,
                     NULL,
                     NULL);
          /* exit on error */
          if (result == ERROR) {
            xodtemplate_free_memberlist(&dependent_servicelist);
            xodtemplate_free_memberlist(&dependent_hostlist);
            return (ERROR);
          }
        }

        /* free memory we used for service list */
        xodtemplate_free_memberlist(&dependent_servicelist);
      }

      /* free service descriptions */
      delete[] service_descriptions;
      service_descriptions = NULL;

      /* free memory we used for host list */
      xodtemplate_free_memberlist(&dependent_hostlist);
    }
  }


#ifdef DEBUG_SERVICE_DEPENDENCIES
  printf("\n");
  for (temp_servicedependency = xodtemplate_servicedependency_list;
       temp_servicedependency != NULL;
       temp_servicedependency = temp_servicedependency->next) {
    printf(
      "3**)  MAS: %s/%s  DEP: %s/%s\n",
      temp_servicedependency->host_name,
      temp_servicedependency->service_description,
      temp_servicedependency->dependent_host_name,
      temp_servicedependency->dependent_service_description);
  }
#endif


  /***************************************/
  /* SKIPLIST STUFF FOR FAST SORT/SEARCH */
  /***************************************/

  /* host dependencies */
  for (temp_hostdependency = xodtemplate_hostdependency_list;
       temp_hostdependency != NULL;
       temp_hostdependency = temp_hostdependency->next) {

    /* skip dependencys that shouldn't be registered */
    if (temp_hostdependency->register_object == false)
      continue;

    /* skip dependency definitions without enough data */
    if (temp_hostdependency->host_name == NULL)
      continue;

    result = skiplist_insert(
               xobject_skiplists[X_HOSTDEPENDENCY_SKIPLIST],
               (void*)temp_hostdependency);
    switch (result) {
    case SKIPLIST_OK:
      result = OK;
      break;

    default:
      result = ERROR;
      break;
    }
  }

  /* service dependencies */
  for (temp_servicedependency = xodtemplate_servicedependency_list;
       temp_servicedependency != NULL;
       temp_servicedependency = temp_servicedependency->next) {

    /* skip dependencys that shouldn't be registered */
    if (temp_servicedependency->register_object == false)
      continue;

    /* skip dependency definitions without enough data */
    if (temp_servicedependency->dependent_host_name == NULL
        || temp_servicedependency->dependent_service_description == NULL)
      continue;

    result = skiplist_insert(
               xobject_skiplists[X_SERVICEDEPENDENCY_SKIPLIST],
               (void*)temp_servicedependency);
    switch (result) {
    case SKIPLIST_OK:
      result = OK;
      break;

    default:
      result = ERROR;
      break;
    }
  }

  return (OK);
}

/* duplicates a service definition (with a new host name) */
int xodtemplate_duplicate_service(
      xodtemplate_service* temp_service,
      char* host_name) {
  xodtemplate_service* new_service = NULL;
  xodtemplate_customvariablesmember* temp_customvariablesmember = NULL;

  /* allocate memory for a new service definition */
  new_service = new xodtemplate_service;

  /* standard items */
  new_service->tmpl = NULL;
  new_service->name = NULL;
  new_service->has_been_resolved = temp_service->has_been_resolved;
  new_service->register_object = temp_service->register_object;
  new_service->_config_file = temp_service->_config_file;
  new_service->_start_line = temp_service->_start_line;
  /*tag service apply on host group */
  xodtemplate_set_service_is_from_hostgroup(new_service);

  /* string defaults */
  new_service->hostgroup_name = NULL;
  new_service->have_hostgroup_name = temp_service->have_hostgroup_name;
  new_service->host_name = NULL;
  new_service->have_host_name = temp_service->have_host_name;
  new_service->service_description = NULL;
  new_service->have_service_description = temp_service->have_service_description;
  new_service->service_groups = NULL;
  new_service->have_service_groups = temp_service->have_service_groups;
  new_service->check_command = NULL;
  new_service->have_check_command = temp_service->have_check_command;
  new_service->check_period = NULL;
  new_service->have_check_period = temp_service->have_check_period;
  new_service->event_handler = NULL;
  new_service->have_event_handler = temp_service->have_event_handler;
  new_service->timezone = NULL;
  new_service->have_timezone = temp_service->have_timezone;
  new_service->custom_variables = NULL;

  /* make sure hostgroup member in new service definition is NULL */
  new_service->hostgroup_name = NULL;

  /* allocate memory for and copy string members of service definition (host name provided, DO NOT duplicate hostgroup member!) */
  if (temp_service->host_name != NULL)
    new_service->host_name = string::dup(host_name);
  if (temp_service->tmpl != NULL)
    new_service->tmpl = string::dup(temp_service->tmpl);
  if (temp_service->name != NULL)
    new_service->name = string::dup(temp_service->name);
  if (temp_service->service_description != NULL)
    new_service->service_description = string::dup(temp_service->service_description);
  if (temp_service->service_groups != NULL)
    new_service->service_groups = string::dup(temp_service->service_groups);
  if (temp_service->check_command != NULL)
    new_service->check_command = string::dup(temp_service->check_command);
  if (temp_service->check_period != NULL)
    new_service->check_period = string::dup(temp_service->check_period);
  if (temp_service->event_handler != NULL)
    new_service->event_handler = string::dup(temp_service->event_handler);
  if (temp_service->timezone != NULL)
    new_service->timezone = string::dup(temp_service->timezone);

  /* duplicate custom variables */
  for (temp_customvariablesmember = temp_service->custom_variables;
       temp_customvariablesmember != NULL;
       temp_customvariablesmember = temp_customvariablesmember->next)
    xodtemplate_add_custom_variable_to_service(
      new_service,
      temp_customvariablesmember->variable_name,
      temp_customvariablesmember->variable_value);

  /* duplicate non-string members */
  new_service->initial_state = temp_service->initial_state;
  new_service->max_check_attempts = temp_service->max_check_attempts;
  new_service->have_max_check_attempts = temp_service->have_max_check_attempts;
  new_service->check_timeout = temp_service->check_timeout;
  new_service->have_check_timeout = temp_service->have_check_timeout;
  new_service->check_interval = temp_service->check_interval;
  new_service->have_check_interval = temp_service->have_check_interval;
  new_service->retry_interval = temp_service->retry_interval;
  new_service->have_retry_interval = temp_service->have_retry_interval;
  new_service->active_checks_enabled = temp_service->active_checks_enabled;
  new_service->have_active_checks_enabled = temp_service->have_active_checks_enabled;
  new_service->is_volatile = temp_service->is_volatile;
  new_service->have_is_volatile = temp_service->have_is_volatile;
  new_service->obsess_over_service = temp_service->obsess_over_service;
  new_service->have_obsess_over_service = temp_service->have_obsess_over_service;
  new_service->event_handler_enabled = temp_service->event_handler_enabled;
  new_service->have_event_handler_enabled = temp_service->have_event_handler_enabled;
  new_service->check_freshness = temp_service->check_freshness;
  new_service->have_check_freshness = temp_service->have_check_freshness;
  new_service->freshness_threshold = temp_service->freshness_threshold;
  new_service->have_freshness_threshold = temp_service->have_freshness_threshold;
  new_service->flap_detection_enabled = temp_service->flap_detection_enabled;
  new_service->have_flap_detection_enabled = temp_service->have_flap_detection_enabled;
  new_service->low_flap_threshold = temp_service->low_flap_threshold;
  new_service->have_low_flap_threshold = temp_service->have_low_flap_threshold;
  new_service->high_flap_threshold = temp_service->high_flap_threshold;
  new_service->have_high_flap_threshold = temp_service->have_high_flap_threshold;
  new_service->flap_detection_on_ok = temp_service->flap_detection_on_ok;
  new_service->flap_detection_on_warning = temp_service->flap_detection_on_warning;
  new_service->flap_detection_on_unknown = temp_service->flap_detection_on_unknown;
  new_service->flap_detection_on_critical = temp_service->flap_detection_on_critical;
  new_service->have_flap_detection_options = temp_service->have_flap_detection_options;

  /* add new service to head of list in memory */
  new_service->next = xodtemplate_service_list;
  xodtemplate_service_list = new_service;

  return (OK);
}

/* duplicates a host dependency definition (with master and dependent host names) */
int xodtemplate_duplicate_hostdependency(
      xodtemplate_hostdependency* temp_hostdependency,
      char* master_host_name,
      char* dependent_host_name) {
  xodtemplate_hostdependency *new_hostdependency = NULL;

  /* allocate memory for a new host dependency definition */
  new_hostdependency = new xodtemplate_hostdependency;

  /* standard items */
  new_hostdependency->tmpl = NULL;
  new_hostdependency->name = NULL;
  new_hostdependency->has_been_resolved = temp_hostdependency->has_been_resolved;
  new_hostdependency->register_object = temp_hostdependency->register_object;
  new_hostdependency->_config_file = temp_hostdependency->_config_file;
  new_hostdependency->_start_line = temp_hostdependency->_start_line;

  /* string defaults */
  new_hostdependency->hostgroup_name = NULL;
  new_hostdependency->have_hostgroup_name = false;
  new_hostdependency->dependent_hostgroup_name = NULL;
  new_hostdependency->have_dependent_hostgroup_name = false;
  new_hostdependency->host_name = NULL;
  new_hostdependency->have_host_name = temp_hostdependency->have_host_name;
  new_hostdependency->dependent_host_name = NULL;
  new_hostdependency->have_dependent_host_name = temp_hostdependency->have_dependent_host_name;
  new_hostdependency->dependency_period = NULL;
  new_hostdependency->have_dependency_period = temp_hostdependency->have_dependency_period;

  /* allocate memory for and copy string members of hostdependency definition */
  if (master_host_name != NULL)
    new_hostdependency->host_name = string::dup(master_host_name);
  if (dependent_host_name != NULL)
    new_hostdependency->dependent_host_name = string::dup(dependent_host_name);

  if (temp_hostdependency->dependency_period != NULL)
    new_hostdependency->dependency_period = string::dup(temp_hostdependency->dependency_period);
  if (temp_hostdependency->tmpl != NULL)
    new_hostdependency->tmpl = string::dup(temp_hostdependency->tmpl);
  if (temp_hostdependency->name != NULL)
    new_hostdependency->name = string::dup(temp_hostdependency->name);

  /* duplicate non-string members */
  new_hostdependency->fail_on_up = temp_hostdependency->fail_on_up;
  new_hostdependency->fail_on_down =  temp_hostdependency->fail_on_down;
  new_hostdependency->fail_on_unreachable = temp_hostdependency->fail_on_unreachable;
  new_hostdependency->fail_on_pending = temp_hostdependency->fail_on_pending;
  new_hostdependency->have_dependency_options = temp_hostdependency->have_dependency_options;
  new_hostdependency->inherits_parent = temp_hostdependency->inherits_parent;
  new_hostdependency->have_inherits_parent = temp_hostdependency->have_inherits_parent;

  /* add new hostdependency to head of list in memory */
  new_hostdependency->next = xodtemplate_hostdependency_list;
  xodtemplate_hostdependency_list = new_hostdependency;

  return (OK);
}

/* duplicates a service dependency definition */
int xodtemplate_duplicate_servicedependency(
      xodtemplate_servicedependency* temp_servicedependency,
      char* master_host_name,
      char* master_service_description,
      char* master_hostgroup_name,
      char* master_servicegroup_name,
      char* dependent_host_name,
      char* dependent_service_description,
      char* dependent_hostgroup_name,
      char* dependent_servicegroup_name) {
  xodtemplate_servicedependency* new_servicedependency = NULL;

  /* allocate memory for a new service dependency definition */
  new_servicedependency = new xodtemplate_servicedependency;

  /* standard items */
  new_servicedependency->tmpl = NULL;
  new_servicedependency->name = NULL;
  new_servicedependency->has_been_resolved = temp_servicedependency->has_been_resolved;
  new_servicedependency->register_object = temp_servicedependency->register_object;
  new_servicedependency->_config_file = temp_servicedependency->_config_file;
  new_servicedependency->_start_line = temp_servicedependency->_start_line;

  /* string defaults */
  new_servicedependency->host_name = NULL;
  new_servicedependency->have_host_name = (master_host_name) ? true : false;
  new_servicedependency->service_description = NULL;
  new_servicedependency->have_service_description = (master_service_description) ? true : false;
  new_servicedependency->hostgroup_name = NULL;
  new_servicedependency->have_hostgroup_name = (master_hostgroup_name) ? true : false;
  new_servicedependency->servicegroup_name = NULL;
  new_servicedependency->have_servicegroup_name = (master_servicegroup_name) ? true : false;

  new_servicedependency->dependent_host_name = NULL;
  new_servicedependency->have_dependent_host_name = (dependent_host_name) ? true : false;
  new_servicedependency->dependent_service_description = NULL;
  new_servicedependency->have_dependent_service_description = (dependent_service_description) ? true : false;
  new_servicedependency->dependent_hostgroup_name = NULL;
  new_servicedependency->have_dependent_hostgroup_name = (dependent_hostgroup_name) ? true : false;
  new_servicedependency->dependent_servicegroup_name = NULL;
  new_servicedependency->have_dependent_servicegroup_name = (dependent_servicegroup_name) ? true : false;

  new_servicedependency->dependency_period = NULL;
  new_servicedependency->have_dependency_period = temp_servicedependency->have_dependency_period;
  new_servicedependency->service_description = NULL;
  new_servicedependency->dependent_service_description = NULL;

  /* duplicate strings */
  if (master_host_name != NULL)
    new_servicedependency->host_name = string::dup(master_host_name);
  if (master_service_description != NULL)
    new_servicedependency->service_description = string::dup(master_service_description);
  if (master_hostgroup_name != NULL)
    new_servicedependency->hostgroup_name = string::dup(master_hostgroup_name);
  if (master_servicegroup_name != NULL)
    new_servicedependency->servicegroup_name = string::dup(master_servicegroup_name);
  if (dependent_host_name != NULL)
    new_servicedependency->dependent_host_name = string::dup(dependent_host_name);
  if (dependent_service_description != NULL)
    new_servicedependency->dependent_service_description = string::dup(dependent_service_description);
  if (dependent_hostgroup_name != NULL)
    new_servicedependency->dependent_hostgroup_name = string::dup(dependent_hostgroup_name);
  if (dependent_servicegroup_name != NULL)
    new_servicedependency->dependent_servicegroup_name = string::dup(dependent_servicegroup_name);

  if (temp_servicedependency->dependency_period != NULL)
    new_servicedependency->dependency_period = string::dup(temp_servicedependency->dependency_period);
  if (temp_servicedependency->tmpl != NULL)
    new_servicedependency->tmpl = string::dup(temp_servicedependency->tmpl);
  if (temp_servicedependency->name != NULL)
    new_servicedependency->name = string::dup(temp_servicedependency->name);

  /* duplicate non-string members */
  new_servicedependency->fail_on_ok = temp_servicedependency->fail_on_ok;
  new_servicedependency->fail_on_unknown = temp_servicedependency->fail_on_unknown;
  new_servicedependency->fail_on_warning = temp_servicedependency->fail_on_warning;
  new_servicedependency->fail_on_critical = temp_servicedependency->fail_on_critical;
  new_servicedependency->fail_on_pending = temp_servicedependency->fail_on_pending;
  new_servicedependency->have_dependency_options = temp_servicedependency->have_dependency_options;
  new_servicedependency->inherits_parent = temp_servicedependency->inherits_parent;
  new_servicedependency->have_inherits_parent = temp_servicedependency->have_inherits_parent;

  /* add new servicedependency to head of list in memory */
  new_servicedependency->next = xodtemplate_servicedependency_list;
  xodtemplate_servicedependency_list = new_servicedependency;

  return (OK);
}

/******************************************************************/
/***************** OBJECT RESOLUTION FUNCTIONS ********************/
/******************************************************************/

/* resolves object definitions */
int xodtemplate_resolve_objects() {
  xodtemplate_timeperiod* temp_timeperiod = NULL;
  xodtemplate_command* temp_command = NULL;
  xodtemplate_hostgroup* temp_hostgroup = NULL;
  xodtemplate_servicegroup* temp_servicegroup = NULL;
  xodtemplate_servicedependency* temp_servicedependency = NULL;
  xodtemplate_host* temp_host = NULL;
  xodtemplate_service* temp_service = NULL;
  xodtemplate_hostdependency* temp_hostdependency = NULL;

  /* resolve all timeperiod objects */
  for (temp_timeperiod = xodtemplate_timeperiod_list;
       temp_timeperiod != NULL;
       temp_timeperiod = temp_timeperiod->next) {
    if (xodtemplate_resolve_timeperiod(temp_timeperiod) == ERROR)
      return (ERROR);
  }

  /* resolve all command objects */
  for (temp_command = xodtemplate_command_list;
       temp_command != NULL;
       temp_command = temp_command->next) {
    if (xodtemplate_resolve_command(temp_command) == ERROR)
      return (ERROR);
  }

  /* resolve all hostgroup objects */
  for (temp_hostgroup = xodtemplate_hostgroup_list;
       temp_hostgroup != NULL;
       temp_hostgroup = temp_hostgroup->next) {
    if (xodtemplate_resolve_hostgroup(temp_hostgroup) == ERROR)
      return (ERROR);
  }

  /* resolve all servicegroup objects */
  for (temp_servicegroup = xodtemplate_servicegroup_list;
       temp_servicegroup != NULL;
       temp_servicegroup = temp_servicegroup->next) {
    if (xodtemplate_resolve_servicegroup(temp_servicegroup) == ERROR)
      return (ERROR);
  }

  /* resolve all servicedependency objects */
  for (temp_servicedependency = xodtemplate_servicedependency_list;
       temp_servicedependency != NULL;
       temp_servicedependency = temp_servicedependency->next) {
    if (xodtemplate_resolve_servicedependency(temp_servicedependency) == ERROR)
      return (ERROR);
  }

  /* resolve all host objects */
  for (temp_host = xodtemplate_host_list;
       temp_host != NULL;
       temp_host = temp_host->next) {
    if (xodtemplate_resolve_host(temp_host) == ERROR)
      return (ERROR);
  }

  /* resolve all service objects */
  for (temp_service = xodtemplate_service_list;
       temp_service != NULL;
       temp_service = temp_service->next) {
    if (xodtemplate_resolve_service(temp_service) == ERROR)
      return (ERROR);
  }

  /* resolve all hostdependency objects */
  for (temp_hostdependency = xodtemplate_hostdependency_list;
       temp_hostdependency != NULL;
       temp_hostdependency = temp_hostdependency->next) {
    if (xodtemplate_resolve_hostdependency(temp_hostdependency) == ERROR)
      return (ERROR);
  }

  return (OK);
}

/* resolves a timeperiod object */
int xodtemplate_resolve_timeperiod(
      xodtemplate_timeperiod* this_timeperiod) {
  char* temp_ptr = NULL;
  char* template_names = NULL;
  char* template_name_ptr = NULL;
  xodtemplate_daterange* template_daterange = NULL;
  xodtemplate_daterange* this_daterange = NULL;
  xodtemplate_daterange* new_daterange = NULL;
  xodtemplate_timeperiod* template_timeperiod = NULL;
  int x;

  /* return if this timeperiod has already been resolved */
  if (this_timeperiod->has_been_resolved == true)
    return (OK);

  /* set the resolved flag */
  this_timeperiod->has_been_resolved = true;

  /* return if we have no template */
  if (this_timeperiod->tmpl == NULL)
    return (OK);

  template_names = string::dup(this_timeperiod->tmpl);

  /* apply all templates */
  template_name_ptr = template_names;
  for (temp_ptr = my_strsep(&template_name_ptr, ",");
       temp_ptr != NULL;
       temp_ptr = my_strsep(&template_name_ptr, ",")) {

    template_timeperiod = xodtemplate_find_timeperiod(temp_ptr);
    if (template_timeperiod == NULL) {
      logger(log_config_error, basic)
        << "Error: Template '" << temp_ptr << "' specified in "
        "timeperiod definition could not be not found (config file '"
        << xodtemplate_config_file_name(this_timeperiod->_config_file)
        << "', starting on line " << this_timeperiod->_start_line
        << ")";
      delete[] template_names;
      return (ERROR);
    }

    /* resolve the template timeperiod... */
    xodtemplate_resolve_timeperiod(template_timeperiod);

    /* apply missing properties from template timeperiod... */
    if (this_timeperiod->timeperiod_name == NULL
        && template_timeperiod->timeperiod_name != NULL)
      this_timeperiod->timeperiod_name
        = string::dup(template_timeperiod->timeperiod_name);
    if (this_timeperiod->alias == NULL
        && template_timeperiod->alias != NULL)
      this_timeperiod->alias = string::dup(template_timeperiod->alias);
    if (this_timeperiod->exclusions == NULL
        && template_timeperiod->exclusions != NULL)
      this_timeperiod->exclusions = string::dup(template_timeperiod->exclusions);
    for (x = 0; x < 7; x++) {
      if (this_timeperiod->timeranges[x] == NULL
          && template_timeperiod->timeranges[x] != NULL) {
        this_timeperiod->timeranges[x]
          = string::dup(template_timeperiod->timeranges[x]);
      }
    }
    /* daterange exceptions require more work to apply missing ranges... */
    for (x = 0; x < DATERANGE_TYPES; x++) {
      for (template_daterange = template_timeperiod->exceptions[x];
           template_daterange != NULL;
           template_daterange = template_daterange->next) {

        /* see if this same daterange already exists in the timeperiod */
        for (this_daterange = this_timeperiod->exceptions[x];
             this_daterange != NULL;
             this_daterange = this_daterange->next) {
          if ((this_daterange->type == template_daterange->type)
              && (this_daterange->syear == template_daterange->syear)
              && (this_daterange->smon == template_daterange->smon)
              && (this_daterange->smday == template_daterange->smday)
              && (this_daterange->swday == template_daterange->swday)
              && (this_daterange->swday_offset == template_daterange->swday_offset)
              && (this_daterange->eyear == template_daterange->eyear)
              && (this_daterange->emon == template_daterange->emon)
              && (this_daterange->emday == template_daterange->emday)
              && (this_daterange->ewday == template_daterange->ewday)
              && (this_daterange->ewday_offset == template_daterange->ewday_offset)
              && (this_daterange->skip_interval == template_daterange->skip_interval))
            break;
        }

        /* this daterange already exists in the timeperiod, so don't inherit it */
        if (this_daterange != NULL)
          continue;

        /* inherit the daterange from the template */
        new_daterange = new xodtemplate_daterange;

        new_daterange->type = template_daterange->type;
        new_daterange->syear = template_daterange->syear;
        new_daterange->smon = template_daterange->smon;
        new_daterange->smday = template_daterange->smday;
        new_daterange->swday = template_daterange->swday;
        new_daterange->swday_offset = template_daterange->swday_offset;
        new_daterange->eyear = template_daterange->eyear;
        new_daterange->emon = template_daterange->emon;
        new_daterange->emday = template_daterange->emday;
        new_daterange->ewday = template_daterange->ewday;
        new_daterange->ewday_offset = template_daterange->ewday_offset;
        new_daterange->skip_interval = template_daterange->skip_interval;
        new_daterange->timeranges = NULL;
        if (template_daterange->timeranges != NULL)
          new_daterange->timeranges = string::dup(template_daterange->timeranges);

        /* add new daterange to head of list (should it be added to the end instead?) */
        new_daterange->next = this_timeperiod->exceptions[x];
        this_timeperiod->exceptions[x] = new_daterange;
      }
    }
  }

  delete[] template_names;

  return (OK);
}

/* resolves a command object */
int xodtemplate_resolve_command(xodtemplate_command* this_command) {
  char* temp_ptr = NULL;
  char* template_names = NULL;
  char* template_name_ptr = NULL;
  xodtemplate_command* template_command = NULL;

  /* return if this command has already been resolved */
  if (this_command->has_been_resolved == true)
    return (OK);

  /* set the resolved flag */
  this_command->has_been_resolved = true;

  /* return if we have no template */
  if (this_command->tmpl == NULL)
    return (OK);

  template_names = string::dup(this_command->tmpl);

  /* apply all templates */
  template_name_ptr = template_names;
  for (temp_ptr = my_strsep(&template_name_ptr, ",");
       temp_ptr != NULL;
       temp_ptr = my_strsep(&template_name_ptr, ",")) {

    template_command = xodtemplate_find_command(temp_ptr);
    if (template_command == NULL) {
      logger(log_config_error, basic)
        << "Error: Template '" << temp_ptr << "' specified in command "
        "definition could not be not found (config file '"
        << xodtemplate_config_file_name(this_command->_config_file)
        << "', starting on line " << this_command->_start_line << ")";
      delete[] template_names;
      return (ERROR);
    }

    /* resolve the template command... */
    xodtemplate_resolve_command(template_command);

    /* apply missing properties from template command... */
    if (this_command->command_name == NULL
        && template_command->command_name != NULL)
      this_command->command_name
        = string::dup(template_command->command_name);
    if (this_command->command_line == NULL
        && template_command->command_line != NULL)
      this_command->command_line
        = string::dup(template_command->command_line);
  }

  delete[] template_names;

  return (OK);
}

/* resolves a hostgroup object */
int xodtemplate_resolve_hostgroup(
      xodtemplate_hostgroup* this_hostgroup) {
  char* temp_ptr = NULL;
  char* template_names = NULL;
  char* template_name_ptr = NULL;
  xodtemplate_hostgroup* template_hostgroup = NULL;

  /* return if this hostgroup has already been resolved */
  if (this_hostgroup->has_been_resolved == true)
    return (OK);

  /* set the resolved flag */
  this_hostgroup->has_been_resolved = true;

  /* return if we have no template */
  if (this_hostgroup->tmpl == NULL)
    return (OK);

  template_names = string::dup(this_hostgroup->tmpl);

  /* apply all templates */
  template_name_ptr = template_names;
  for (temp_ptr = my_strsep(&template_name_ptr, ",");
       temp_ptr != NULL;
       temp_ptr = my_strsep(&template_name_ptr, ",")) {

    template_hostgroup = xodtemplate_find_hostgroup(temp_ptr);
    if (template_hostgroup == NULL) {
      logger(log_config_error, basic)
        << "Error: Template '" << temp_ptr << "' specified in "
        "hostgroup definition could not be not found (config file '"
        << xodtemplate_config_file_name(this_hostgroup->_config_file)
        << "', starting on line " << this_hostgroup->_start_line
        << ")";
      delete[] template_names;
      return (ERROR);
    }

    /* resolve the template hostgroup... */
    xodtemplate_resolve_hostgroup(template_hostgroup);

    /* apply missing properties from template hostgroup... */
    if (this_hostgroup->hostgroup_name == NULL
        && template_hostgroup->hostgroup_name != NULL)
      this_hostgroup->hostgroup_name
        = string::dup(template_hostgroup->hostgroup_name);
    if (this_hostgroup->alias == NULL
        && template_hostgroup->alias != NULL)
      this_hostgroup->alias = string::dup(template_hostgroup->alias);

    xodtemplate_get_inherited_string(
      &template_hostgroup->have_members,
      &template_hostgroup->members,
      &this_hostgroup->have_members,
      &this_hostgroup->members);
    xodtemplate_get_inherited_string(
      &template_hostgroup->have_hostgroup_members,
      &template_hostgroup->hostgroup_members,
      &this_hostgroup->have_hostgroup_members,
      &this_hostgroup->hostgroup_members);
  }

  delete[] template_names;

  return (OK);
}

/* resolves a servicegroup object */
int xodtemplate_resolve_servicegroup(
      xodtemplate_servicegroup* this_servicegroup) {
  char* temp_ptr = NULL;
  char* template_names = NULL;
  char* template_name_ptr = NULL;
  xodtemplate_servicegroup* template_servicegroup = NULL;

  /* return if this servicegroup has already been resolved */
  if (this_servicegroup->has_been_resolved == true)
    return (OK);

  /* set the resolved flag */
  this_servicegroup->has_been_resolved = true;

  /* return if we have no template */
  if (this_servicegroup->tmpl == NULL)
    return (OK);

  template_names = string::dup(this_servicegroup->tmpl);

  /* apply all templates */
  template_name_ptr = template_names;
  for (temp_ptr = my_strsep(&template_name_ptr, ",");
       temp_ptr != NULL;
       temp_ptr = my_strsep(&template_name_ptr, ",")) {

    template_servicegroup = xodtemplate_find_servicegroup(temp_ptr);
    if (template_servicegroup == NULL) {
      logger(log_config_error, basic)
        << "Error: Template '" << temp_ptr << "' specified in "
        "servicegroup definition could not be not found (config file '"
        << xodtemplate_config_file_name(this_servicegroup->_config_file)
        << "', starting on line " << this_servicegroup->_start_line
        << ")";
      delete[] template_names;
      return (ERROR);
    }

    /* resolve the template servicegroup... */
    xodtemplate_resolve_servicegroup(template_servicegroup);

    /* apply missing properties from template servicegroup... */
    if (this_servicegroup->servicegroup_name == NULL
        && template_servicegroup->servicegroup_name != NULL)
      this_servicegroup->servicegroup_name
        = string::dup(template_servicegroup->servicegroup_name);
    if (this_servicegroup->alias == NULL
        && template_servicegroup->alias != NULL)
      this_servicegroup->alias = string::dup(template_servicegroup->alias);

    xodtemplate_get_inherited_string(
      &template_servicegroup->have_members,
      &template_servicegroup->members,
      &this_servicegroup->have_members,
      &this_servicegroup->members);
    xodtemplate_get_inherited_string(
      &template_servicegroup->have_servicegroup_members,
      &template_servicegroup->servicegroup_members,
      &this_servicegroup->have_servicegroup_members,
      &this_servicegroup->servicegroup_members);
  }

  delete[] template_names;

  return (OK);
}

/* resolves a servicedependency object */
int xodtemplate_resolve_servicedependency(
      xodtemplate_servicedependency* this_servicedependency) {
  char* temp_ptr = NULL;
  char* template_names = NULL;
  char* template_name_ptr = NULL;
  xodtemplate_servicedependency* template_servicedependency = NULL;

  /* return if this servicedependency has already been resolved */
  if (this_servicedependency->has_been_resolved == true)
    return (OK);

  /* set the resolved flag */
  this_servicedependency->has_been_resolved = true;

  /* return if we have no template */
  if (this_servicedependency->tmpl == NULL)
    return (OK);

  template_names = string::dup(this_servicedependency->tmpl);

  /* apply all templates */
  template_name_ptr = template_names;
  for (temp_ptr = my_strsep(&template_name_ptr, ",");
       temp_ptr != NULL;
       temp_ptr = my_strsep(&template_name_ptr, ",")) {

    template_servicedependency = xodtemplate_find_servicedependency(temp_ptr);
    if (template_servicedependency == NULL) {
      logger(log_config_error, basic)
        << "Error: Template '" << temp_ptr << "' specified in service "
        "dependency definition could not be not found (config file '"
        << xodtemplate_config_file_name(this_servicedependency->_config_file)
        << "', starting on line " << this_servicedependency->_start_line
        << ")";
      delete[] template_names;
      return (ERROR);
    }

    /* resolve the template servicedependency... */
    xodtemplate_resolve_servicedependency(template_servicedependency);

    /* apply missing properties from template servicedependency... */
    xodtemplate_get_inherited_string(
      &template_servicedependency->have_servicegroup_name,
      &template_servicedependency->servicegroup_name,
      &this_servicedependency->have_servicegroup_name,
      &this_servicedependency->servicegroup_name);
    xodtemplate_get_inherited_string(
      &template_servicedependency->have_hostgroup_name,
      &template_servicedependency->hostgroup_name,
      &this_servicedependency->have_hostgroup_name,
      &this_servicedependency->hostgroup_name);
    xodtemplate_get_inherited_string(
      &template_servicedependency->have_host_name,
      &template_servicedependency->host_name,
      &this_servicedependency->have_host_name,
      &this_servicedependency->host_name);
    xodtemplate_get_inherited_string(
      &template_servicedependency->have_service_description,
      &template_servicedependency->service_description,
      &this_servicedependency->have_service_description,
      &this_servicedependency->service_description);
    xodtemplate_get_inherited_string(
      &template_servicedependency->have_dependent_servicegroup_name,
      &template_servicedependency->dependent_servicegroup_name,
      &this_servicedependency->have_dependent_servicegroup_name,
      &this_servicedependency->dependent_servicegroup_name);
    xodtemplate_get_inherited_string(
      &template_servicedependency->have_dependent_hostgroup_name,
      &template_servicedependency->dependent_hostgroup_name,
      &this_servicedependency->have_dependent_hostgroup_name,
      &this_servicedependency->dependent_hostgroup_name);
    xodtemplate_get_inherited_string(
      &template_servicedependency->have_dependent_host_name,
      &template_servicedependency->dependent_host_name,
      &this_servicedependency->have_dependent_host_name,
      &this_servicedependency->dependent_host_name);
    xodtemplate_get_inherited_string(
      &template_servicedependency->have_dependent_service_description,
      &template_servicedependency->dependent_service_description,
      &this_servicedependency->have_dependent_service_description,
      &this_servicedependency->dependent_service_description);

    if (this_servicedependency->have_dependency_period == false
        && template_servicedependency->have_dependency_period == true) {
      if (this_servicedependency->dependency_period == NULL
          && template_servicedependency->dependency_period != NULL)
        this_servicedependency->dependency_period
          = string::dup(template_servicedependency->dependency_period);
      this_servicedependency->have_dependency_period = true;
    }
    if (this_servicedependency->have_inherits_parent == false
        && template_servicedependency->have_inherits_parent == true) {
      this_servicedependency->inherits_parent
        = template_servicedependency->inherits_parent;
      this_servicedependency->have_inherits_parent = true;
    }
    if (this_servicedependency->have_dependency_options == false
        && template_servicedependency->have_dependency_options == true) {
      this_servicedependency->fail_on_ok
        = template_servicedependency->fail_on_ok;
      this_servicedependency->fail_on_unknown
        = template_servicedependency->fail_on_unknown;
      this_servicedependency->fail_on_warning
        = template_servicedependency->fail_on_warning;
      this_servicedependency->fail_on_critical
        = template_servicedependency->fail_on_critical;
      this_servicedependency->fail_on_pending
        = template_servicedependency->fail_on_pending;
      this_servicedependency->have_dependency_options = true;
    }
  }

  delete[] template_names;

  return (OK);
}

/* resolves a host object */
int xodtemplate_resolve_host(xodtemplate_host* this_host) {
  char* temp_ptr = NULL;
  char* template_names = NULL;
  char* template_name_ptr = NULL;
  xodtemplate_host* template_host = NULL;
  xodtemplate_customvariablesmember* this_customvariablesmember = NULL;
  xodtemplate_customvariablesmember* temp_customvariablesmember = NULL;

  /* return if this host has already been resolved */
  if (this_host->has_been_resolved == true)
    return (OK);

  /* set the resolved flag */
  this_host->has_been_resolved = true;

  /* return if we have no template */
  if (this_host->tmpl == NULL)
    return (OK);

  template_names = string::dup(this_host->tmpl);

  /* apply all templates */
  template_name_ptr = template_names;
  for (temp_ptr = my_strsep(&template_name_ptr, ",");
       temp_ptr != NULL;
       temp_ptr = my_strsep(&template_name_ptr, ",")) {

    template_host = xodtemplate_find_host(temp_ptr);
    if (template_host == NULL) {
      logger(log_config_error, basic)
        << "Error: Template '" << temp_ptr << "' specified in host "
        "definition could not be not found (config file '"
        << xodtemplate_config_file_name(this_host->_config_file)
        << "', starting on line " << this_host->_start_line << ")";
      delete[] template_names;
      return (ERROR);
    }

    /* resolve the template host... */
    xodtemplate_resolve_host(template_host);

    /* apply missing properties from template host... */
    if (this_host->host_name == NULL
        && template_host->host_name != NULL)
      this_host->host_name = string::dup(template_host->host_name);
    if (this_host->alias == NULL && template_host->alias != NULL)
      this_host->alias = string::dup(template_host->alias);
    if (this_host->address == NULL && template_host->address != NULL)
      this_host->address = string::dup(template_host->address);

    xodtemplate_get_inherited_string(
      &template_host->have_parents,
      &template_host->parents,
      &this_host->have_parents,
      &this_host->parents);
    xodtemplate_get_inherited_string(
      &template_host->have_host_groups,
      &template_host->host_groups,
      &this_host->have_host_groups,
      &this_host->host_groups);

    if (this_host->have_check_command == false
        && template_host->have_check_command == true) {
      if (this_host->check_command == NULL
          && template_host->check_command != NULL)
        this_host->check_command
          = string::dup(template_host->check_command);
      this_host->have_check_command = true;
    }
    if (this_host->have_check_period == false
        && template_host->have_check_period == true) {
      if (this_host->check_period == NULL
          && template_host->check_period != NULL)
        this_host->check_period
          = string::dup(template_host->check_period);
      this_host->have_check_period = true;
    }
    if (this_host->have_event_handler == false
        && template_host->have_event_handler == true) {
      if (this_host->event_handler == NULL
          && template_host->event_handler != NULL)
        this_host->event_handler
          = string::dup(template_host->event_handler);
      this_host->have_event_handler = true;
    }
    if (this_host->have_timezone == false
        && template_host->have_timezone == true) {
      if (this_host->timezone == NULL
          && template_host->timezone != NULL)
        this_host->timezone = string::dup(template_host->timezone);
      this_host->have_timezone = true;
    }
    if (this_host->have_initial_state == false
        && template_host->have_initial_state == true) {
      this_host->initial_state = template_host->initial_state;
      this_host->have_initial_state = true;
    }
    if (this_host->have_check_interval == false
        && template_host->have_check_interval == true) {
      this_host->check_interval = template_host->check_interval;
      this_host->have_check_interval = true;
    }
    if (this_host->have_retry_interval == false
        && template_host->have_retry_interval == true) {
      this_host->retry_interval = template_host->retry_interval;
      this_host->have_retry_interval = true;
    }
    if (this_host->have_max_check_attempts == false
        && template_host->have_max_check_attempts == true) {
      this_host->max_check_attempts = template_host->max_check_attempts;
      this_host->have_max_check_attempts = true;
    }
    if (this_host->have_check_timeout == false
        && template_host->have_check_timeout == true) {
      this_host->check_timeout = template_host->check_timeout;
      this_host->have_check_timeout = true;
    }
    if (this_host->have_active_checks_enabled == false
        && template_host->have_active_checks_enabled == true) {
      this_host->active_checks_enabled
        = template_host->active_checks_enabled;
      this_host->have_active_checks_enabled = true;
    }
    if (this_host->have_obsess_over_host == false
        && template_host->have_obsess_over_host == true) {
      this_host->obsess_over_host
        = template_host->obsess_over_host;
      this_host->have_obsess_over_host = true;
    }
    if (this_host->have_event_handler_enabled == false
        && template_host->have_event_handler_enabled == true) {
      this_host->event_handler_enabled
        = template_host->event_handler_enabled;
      this_host->have_event_handler_enabled = true;
    }
    if (this_host->have_check_freshness == false
        && template_host->have_check_freshness == true) {
      this_host->check_freshness = template_host->check_freshness;
      this_host->have_check_freshness = true;
    }
    if (this_host->have_freshness_threshold == false
        && template_host->have_freshness_threshold == true) {
      this_host->freshness_threshold
        = template_host->freshness_threshold;
      this_host->have_freshness_threshold = true;
    }
    if (this_host->have_low_flap_threshold == false
        && template_host->have_low_flap_threshold == true) {
      this_host->low_flap_threshold = template_host->low_flap_threshold;
      this_host->have_low_flap_threshold = true;
    }
    if (this_host->have_high_flap_threshold == false
        && template_host->have_high_flap_threshold == true) {
      this_host->high_flap_threshold
        = template_host->high_flap_threshold;
      this_host->have_high_flap_threshold = true;
    }
    if (this_host->have_flap_detection_enabled == false
        && template_host->have_flap_detection_enabled == true) {
      this_host->flap_detection_enabled
        = template_host->flap_detection_enabled;
      this_host->have_flap_detection_enabled = true;
    }
    if (this_host->have_flap_detection_options == false
        && template_host->have_flap_detection_options == true) {
      this_host->flap_detection_on_up
        = template_host->flap_detection_on_up;
      this_host->flap_detection_on_down
        = template_host->flap_detection_on_down;
      this_host->flap_detection_on_unreachable
        = template_host->flap_detection_on_unreachable;
      this_host->have_flap_detection_options = true;
    }

    /* apply missing custom variables from template host... */
    for (temp_customvariablesmember = template_host->custom_variables;
         temp_customvariablesmember != NULL;
         temp_customvariablesmember = temp_customvariablesmember->next) {

      /* see if this host has a variable by the same name */
      for (this_customvariablesmember = this_host->custom_variables;
           this_customvariablesmember != NULL;
           this_customvariablesmember = this_customvariablesmember->next) {
        if (!strcmp(temp_customvariablesmember->variable_name,
		    this_customvariablesmember->variable_name))
          break;
      }

      /* we didn't find the same variable name, so add a new custom variable */
      if (this_customvariablesmember == NULL)
        xodtemplate_add_custom_variable_to_host(
          this_host,
          temp_customvariablesmember->variable_name,
          temp_customvariablesmember->variable_value);
    }
  }

  delete[] template_names;

  return (OK);
}

/* resolves a service object */
int xodtemplate_resolve_service(xodtemplate_service* this_service) {
  char* temp_ptr = NULL;
  char* template_names = NULL;
  char* template_name_ptr = NULL;
  xodtemplate_service* template_service = NULL;
  xodtemplate_customvariablesmember* this_customvariablesmember = NULL;
  xodtemplate_customvariablesmember* temp_customvariablesmember = NULL;

  /* return if this service has already been resolved */
  if (this_service->has_been_resolved == true)
    return (OK);

  /* set the resolved flag */
  this_service->has_been_resolved = true;

  /* return if we have no template */
  if (this_service->tmpl == NULL)
    return (OK);

  template_names = string::dup(this_service->tmpl);

  /* apply all templates */
  template_name_ptr = template_names;
  for (temp_ptr = my_strsep(&template_name_ptr, ",");
       temp_ptr != NULL;
       temp_ptr = my_strsep(&template_name_ptr, ",")) {

    template_service = xodtemplate_find_service(temp_ptr);
    if (template_service == NULL) {
      logger(log_config_error, basic)
        << "Error: Template '" << temp_ptr << "' specified in service "
        "definition could not be not found (config file '"
        << xodtemplate_config_file_name(this_service->_config_file)
        << "', starting on line " << this_service->_start_line << ")";
      delete[] template_names;
      return (ERROR);
    }

    /* resolve the template service... */
    xodtemplate_resolve_service(template_service);

    /* apply missing properties from template service... */
    if (this_service->have_service_description == false
        && template_service->have_service_description == true) {
      if (this_service->service_description == NULL
          && template_service->service_description != NULL)
        this_service->service_description
          = string::dup(template_service->service_description);
      this_service->have_service_description = true;
    }

    xodtemplate_get_inherited_string(
      &template_service->have_host_name,
      &template_service->host_name,
      &this_service->have_host_name,
      &this_service->host_name);
    xodtemplate_get_inherited_string(
      &template_service->have_hostgroup_name,
      &template_service->hostgroup_name,
      &this_service->have_hostgroup_name,
      &this_service->hostgroup_name);
    xodtemplate_get_inherited_string(
      &template_service->have_service_groups,
      &template_service->service_groups,
      &this_service->have_service_groups,
      &this_service->service_groups);

    if (template_service->have_check_command == true) {
      if (template_service->have_important_check_command == true) {
        delete[] this_service->check_command;
        this_service->check_command = NULL;
        this_service->have_check_command = false;
      }
      if (this_service->have_check_command == false) {
        if (this_service->check_command == NULL
            && template_service->check_command != NULL)
          this_service->check_command
            = string::dup(template_service->check_command);
        this_service->have_check_command = true;
      }
    }
    if (this_service->have_check_period == false
        && template_service->have_check_period == true) {
      if (this_service->check_period == NULL
          && template_service->check_period != NULL)
        this_service->check_period
          = string::dup(template_service->check_period);
      this_service->have_check_period = true;
    }
    if (this_service->have_event_handler == false
        && template_service->have_event_handler == true) {
      if (this_service->event_handler == NULL
          && template_service->event_handler != NULL)
        this_service->event_handler
          = string::dup(template_service->event_handler);
      this_service->have_event_handler = true;
    }
    if (this_service->have_timezone == false
        && template_service->have_timezone == true) {
      if (this_service->timezone == NULL
          && template_service->timezone != NULL)
        this_service->timezone = string::dup(template_service->timezone);
      this_service->have_timezone = true;
    }
    if (this_service->have_initial_state == false
        && template_service->have_initial_state == true) {
      this_service->initial_state = template_service->initial_state;
      this_service->have_initial_state = true;
    }
    if (this_service->have_max_check_attempts == false
        && template_service->have_max_check_attempts == true) {
      this_service->max_check_attempts
        = template_service->max_check_attempts;
      this_service->have_max_check_attempts = true;
    }
    if (this_service->have_check_timeout == false
        && template_service->have_check_timeout == true) {
      this_service->check_timeout
        = template_service->check_timeout;
      this_service->have_check_timeout = true;
    }
    if (this_service->have_check_interval == false
        && template_service->have_check_interval == true) {
      this_service->check_interval
        = template_service->check_interval;
      this_service->have_check_interval = true;
    }
    if (this_service->have_retry_interval == false
        && template_service->have_retry_interval == true) {
      this_service->retry_interval = template_service->retry_interval;
      this_service->have_retry_interval = true;
    }
    if (this_service->have_active_checks_enabled == false
        && template_service->have_active_checks_enabled == true) {
      this_service->active_checks_enabled
        = template_service->active_checks_enabled;
      this_service->have_active_checks_enabled = true;
    }
    if (this_service->have_is_volatile == false
        && template_service->have_is_volatile == true) {
      this_service->is_volatile = template_service->is_volatile;
      this_service->have_is_volatile = true;
    }
    if (this_service->have_obsess_over_service == false
        && template_service->have_obsess_over_service == true) {
      this_service->obsess_over_service
        = template_service->obsess_over_service;
      this_service->have_obsess_over_service = true;
    }
    if (this_service->have_event_handler_enabled == false
        && template_service->have_event_handler_enabled == true) {
      this_service->event_handler_enabled
        = template_service->event_handler_enabled;
      this_service->have_event_handler_enabled = true;
    }
    if (this_service->have_check_freshness == false
        && template_service->have_check_freshness == true) {
      this_service->check_freshness = template_service->check_freshness;
      this_service->have_check_freshness = true;
    }
    if (this_service->have_freshness_threshold == false
        && template_service->have_freshness_threshold == true) {
      this_service->freshness_threshold
        = template_service->freshness_threshold;
      this_service->have_freshness_threshold = true;
    }
    if (this_service->have_low_flap_threshold == false
        && template_service->have_low_flap_threshold == true) {
      this_service->low_flap_threshold
        = template_service->low_flap_threshold;
      this_service->have_low_flap_threshold = true;
    }
    if (this_service->have_high_flap_threshold == false
        && template_service->have_high_flap_threshold == true) {
      this_service->high_flap_threshold
        = template_service->high_flap_threshold;
      this_service->have_high_flap_threshold = true;
    }
    if (this_service->have_flap_detection_enabled == false
        && template_service->have_flap_detection_enabled == true) {
      this_service->flap_detection_enabled
        = template_service->flap_detection_enabled;
      this_service->have_flap_detection_enabled = true;
    }
    if (this_service->have_flap_detection_options == false
        && template_service->have_flap_detection_options == true) {
      this_service->flap_detection_on_ok
        = template_service->flap_detection_on_ok;
      this_service->flap_detection_on_unknown
        = template_service->flap_detection_on_unknown;
      this_service->flap_detection_on_warning
        = template_service->flap_detection_on_warning;
      this_service->flap_detection_on_critical
        = template_service->flap_detection_on_critical;
      this_service->have_flap_detection_options = true;
    }

    /* apply missing custom variables from template service... */
    for (temp_customvariablesmember = template_service->custom_variables;
         temp_customvariablesmember != NULL;
         temp_customvariablesmember = temp_customvariablesmember->next) {

      /* see if this host has a variable by the same name */
      for (this_customvariablesmember = this_service->custom_variables;
           this_customvariablesmember != NULL;
           this_customvariablesmember = this_customvariablesmember->next) {
        if (!strcmp(temp_customvariablesmember->variable_name,
		    this_customvariablesmember->variable_name))
          break;
      }

      /* we didn't find the same variable name, so add a new custom variable */
      if (this_customvariablesmember == NULL)
        xodtemplate_add_custom_variable_to_service(
          this_service,
          temp_customvariablesmember->variable_name,
          temp_customvariablesmember->variable_value);
    }
  }

  delete[] template_names;

  return (OK);
}

/* resolves a hostdependency object */
int xodtemplate_resolve_hostdependency(
      xodtemplate_hostdependency* this_hostdependency) {
  char* temp_ptr = NULL;
  char* template_names = NULL;
  char* template_name_ptr = NULL;
  xodtemplate_hostdependency* template_hostdependency = NULL;

  /* return if this hostdependency has already been resolved */
  if (this_hostdependency->has_been_resolved == true)
    return (OK);

  /* set the resolved flag */
  this_hostdependency->has_been_resolved = true;

  /* return if we have no template */
  if (this_hostdependency->tmpl == NULL)
    return (OK);

  template_names = string::dup(this_hostdependency->tmpl);

  /* apply all templates */
  template_name_ptr = template_names;
  for (temp_ptr = my_strsep(&template_name_ptr, ",");
       temp_ptr != NULL;
       temp_ptr = my_strsep(&template_name_ptr, ",")) {

    template_hostdependency = xodtemplate_find_hostdependency(temp_ptr);
    if (template_hostdependency == NULL) {
      logger(log_config_error, basic)
        << "Error: Template '" << temp_ptr << "' specified in host "
        "dependency definition could not be not found (config file '"
        << xodtemplate_config_file_name(this_hostdependency->_config_file)
        << "', starting on line " << this_hostdependency->_start_line
        << ")";
      delete[] template_names;
      return (ERROR);
    }

    /* resolve the template hostdependency... */
    xodtemplate_resolve_hostdependency(template_hostdependency);

    /* apply missing properties from template hostdependency... */

    xodtemplate_get_inherited_string(
      &template_hostdependency->have_host_name,
      &template_hostdependency->host_name,
      &this_hostdependency->have_host_name,
      &this_hostdependency->host_name);
    xodtemplate_get_inherited_string(
      &template_hostdependency->have_dependent_host_name,
      &template_hostdependency->dependent_host_name,
      &this_hostdependency->have_dependent_host_name,
      &this_hostdependency->dependent_host_name);
    xodtemplate_get_inherited_string(
      &template_hostdependency->have_hostgroup_name,
      &template_hostdependency->hostgroup_name,
      &this_hostdependency->have_hostgroup_name,
      &this_hostdependency->hostgroup_name);
    xodtemplate_get_inherited_string(
      &template_hostdependency->have_dependent_hostgroup_name,
      &template_hostdependency->dependent_hostgroup_name,
      &this_hostdependency->have_dependent_hostgroup_name,
      &this_hostdependency->dependent_hostgroup_name);

    if (this_hostdependency->have_dependency_period == false
        && template_hostdependency->have_dependency_period == true) {
      if (this_hostdependency->dependency_period == NULL
          && template_hostdependency->dependency_period != NULL)
        this_hostdependency->dependency_period
          = string::dup(template_hostdependency->dependency_period);
      this_hostdependency->have_dependency_period = true;
    }
    if (this_hostdependency->have_inherits_parent == false
        && template_hostdependency->have_inherits_parent == true) {
      this_hostdependency->inherits_parent
        = template_hostdependency->inherits_parent;
      this_hostdependency->have_inherits_parent = true;
    }
    if (this_hostdependency->have_dependency_options == false
        && template_hostdependency->have_dependency_options == true) {
      this_hostdependency->fail_on_up
        = template_hostdependency->fail_on_up;
      this_hostdependency->fail_on_down
        = template_hostdependency->fail_on_down;
      this_hostdependency->fail_on_unreachable
        = template_hostdependency->fail_on_unreachable;
      this_hostdependency->fail_on_pending
        = template_hostdependency->fail_on_pending;
      this_hostdependency->have_dependency_options = true;
    }
  }

  delete[] template_names;

  return (OK);
}

/******************************************************************/
/*************** OBJECT RECOMBOBULATION FUNCTIONS *****************/
/******************************************************************/

/* recombobulates hostgroup definitions */
int xodtemplate_recombobulate_hostgroups() {
  xodtemplate_host* temp_host = NULL;
  xodtemplate_hostgroup* temp_hostgroup = NULL;
  xodtemplate_memberlist* temp_memberlist = NULL;
  xodtemplate_memberlist* this_memberlist = NULL;
  char* hostgroup_names = NULL;
  char* temp_ptr = NULL;
  char* new_members = NULL;

#ifdef DEBUG
  printf("** PRE-EXPANSION 1\n");
  for (temp_hostgroup = xodtemplate_hostgroup_list;
       temp_hostgroup != NULL;
       temp_hostgroup = temp_hostgroup->next) {
    printf("HOSTGROUP [%s]\n", temp_hostgroup->hostgroup_name);
    printf("H MEMBERS: %s\n", temp_hostgroup->members);
    printf("G MEMBERS: %s\n", temp_hostgroup->hostgroup_members);
    printf("\n");
  }
#endif

  /* This should happen before we expand hostgroup members, to avoid duplicate host memberships 01/07/2006 EG */
  /* process all hosts that have hostgroup directives */
  for (temp_host = xodtemplate_host_list;
       temp_host != NULL;
       temp_host = temp_host->next) {

    /* skip hosts without hostgroup directives or host names */
    if (temp_host->host_groups == NULL || temp_host->host_name == NULL)
      continue;

    /* skip hosts that shouldn't be registered */
    if (temp_host->register_object == false)
      continue;

    /* preprocess the hostgroup list, to change "grp1,grp2,grp3,!grp2" into "grp1,grp3" */
    /* 10/18/07 EG an empty return value means an error occured */
    if ((hostgroup_names = xodtemplate_process_hostgroup_names(
                             temp_host->host_groups,
                             temp_host->_config_file,
                             temp_host->_start_line)) == NULL)
      return (ERROR);

    /* process the list of hostgroups */
    for (temp_ptr = strtok(hostgroup_names, ",");
	 temp_ptr != NULL;
         temp_ptr = strtok(NULL, ",")) {

      /* strip trailing spaces */
      strip(temp_ptr);

      /* find the hostgroup */
      temp_hostgroup = xodtemplate_find_real_hostgroup(temp_ptr);
      if (temp_hostgroup == NULL) {
        logger(log_config_error, basic)
          << "Error: Could not find hostgroup '" << temp_ptr
          << "' specified in host '" << temp_host->host_name
          << "' definition (config file '"
          << xodtemplate_config_file_name(temp_host->_config_file)
          << "', starting on line " << temp_host->_start_line << ")";
        delete[] hostgroup_names;
        return (ERROR);
      }

      /* add this list to the hostgroup members directive */
      if (temp_hostgroup->members == NULL)
        temp_hostgroup->members = string::dup(temp_host->host_name);
      else {
        new_members = resize_string(
                        temp_hostgroup->members,
                        strlen(temp_hostgroup->members) +
                        strlen(temp_host->host_name) + 2);
        temp_hostgroup->members = new_members;
        strcat(temp_hostgroup->members, ",");
        strcat(temp_hostgroup->members, temp_host->host_name);
      }
    }

    /* free memory */
    delete[] hostgroup_names;
    hostgroup_names = NULL;
  }

#ifdef DEBUG
  printf("** POST-EXPANSION 1\n");
  for (temp_hostgroup = xodtemplate_hostgroup_list;
       temp_hostgroup != NULL;
       temp_hostgroup = temp_hostgroup->next) {
    printf("HOSTGROUP [%s]\n", temp_hostgroup->hostgroup_name);
    printf("H MEMBERS: %s\n", temp_hostgroup->members);
    printf("G MEMBERS: %s\n", temp_hostgroup->hostgroup_members);
    printf("\n");
  }
#endif

  /* expand subgroup membership recursively */
  for (temp_hostgroup = xodtemplate_hostgroup_list;
       temp_hostgroup != NULL;
       temp_hostgroup = temp_hostgroup->next)
    xodtemplate_recombobulate_hostgroup_subgroups(temp_hostgroup, NULL);

  /* expand members of all hostgroups - this could be done in xodtemplate_register_hostgroup(), but we can save the CGIs some work if we do it here */
  for (temp_hostgroup = xodtemplate_hostgroup_list;
       temp_hostgroup != NULL;
       temp_hostgroup = temp_hostgroup->next) {

    if (temp_hostgroup->members == NULL
        && temp_hostgroup->hostgroup_members == NULL)
      continue;

    /* skip hostgroups that shouldn't be registered */
    if (temp_hostgroup->register_object == false)
      continue;

    /* get list of hosts in the hostgroup */
    temp_memberlist = xodtemplate_expand_hostgroups_and_hosts(
                        NULL,
                        temp_hostgroup->members,
                        temp_hostgroup->_config_file,
                        temp_hostgroup->_start_line);

    /* add all members to the host group */
    if (temp_memberlist == NULL) {
      logger(log_config_error, basic)
        << "Error: Could not expand members specified in hostgroup "
        "(config file '"
        << xodtemplate_config_file_name(temp_hostgroup->_config_file)
        << "', starting on line " << temp_hostgroup->_start_line << ")";
      return (ERROR);
    }
    delete[] temp_hostgroup->members;
    temp_hostgroup->members = NULL;
    for (this_memberlist = temp_memberlist;
	 this_memberlist != NULL;
         this_memberlist = this_memberlist->next) {

      /* add this host to the hostgroup members directive */
      if (temp_hostgroup->members == NULL)
        temp_hostgroup->members = string::dup(this_memberlist->name1);
      else {
        new_members = resize_string(
                        temp_hostgroup->members,
                        strlen(temp_hostgroup->members)
                        + strlen(this_memberlist->name1) + 2);
        temp_hostgroup->members = new_members;
        strcat(temp_hostgroup->members, ",");
        strcat(temp_hostgroup->members, this_memberlist->name1);
      }
    }
    xodtemplate_free_memberlist(&temp_memberlist);
  }

#ifdef DEBUG
  printf("** POST-EXPANSION 2\n");
  for (temp_hostgroup = xodtemplate_hostgroup_list;
       temp_hostgroup != NULL;
       temp_hostgroup = temp_hostgroup->next) {
    printf("HOSTGROUP [%s]\n", temp_hostgroup->hostgroup_name);
    printf("H MEMBERS: %s\n", temp_hostgroup->members);
    printf("G MEMBERS: %s\n", temp_hostgroup->hostgroup_members);
    printf("\n");
  }
#endif

  return (OK);
}

int xodtemplate_recombobulate_hostgroup_subgroups(
      xodtemplate_hostgroup* temp_hostgroup,
      char** members) {


  if (temp_hostgroup == NULL)
    return (ERROR);

  /* resolve subgroup memberships first */
  if (temp_hostgroup->hostgroup_members != NULL) {

    /* save members, null pointer so we don't recurse into infinite hell */
    char* orig_hgmembers(temp_hostgroup->hostgroup_members);
    temp_hostgroup->hostgroup_members = NULL;

    /* make new working copy of members */
    char* hgmembers(string::dup(orig_hgmembers));

    char* buf(NULL);
    char* ptr(hgmembers);
    while ((buf = ptr) != NULL) {

      /* get next member for next run */
      ptr = strchr(ptr, ',');
      if (ptr) {
        ptr[0] = '\x0';
        ptr++;
      }

      strip(buf);

      /* find subgroup and recurse */
      xodtemplate_hostgroup* sub_group(NULL);
      if ((sub_group = xodtemplate_find_real_hostgroup(buf)) == NULL) {
        logger(log_config_error, basic)
          << "Error: Could not find member group '" << buf
          << "' specified in hostgroup (config file '"
          << xodtemplate_config_file_name(temp_hostgroup->_config_file)
          << "', starting on line " << temp_hostgroup->_start_line
          << ")";
        return (ERROR);
      }

      char* newmembers(NULL);
      xodtemplate_recombobulate_hostgroup_subgroups(
        sub_group,
        &newmembers);

      /* add new (sub) members */
      if (newmembers != NULL) {
        if (temp_hostgroup->members == NULL)
          temp_hostgroup->members = string::dup(newmembers);
        else {
          temp_hostgroup->members = resize_string(
                                      temp_hostgroup->members,
                                      strlen(temp_hostgroup->members)
                                      + strlen(newmembers) + 2);
          strcat(temp_hostgroup->members, ",");
          strcat(temp_hostgroup->members, newmembers);
        }
      }
    }

    /* free memory */
    delete[] hgmembers;
    hgmembers = NULL;

    /* restore group members */
    temp_hostgroup->hostgroup_members = orig_hgmembers;
  }

  /* return host members */
  if (members != NULL)
    *members = temp_hostgroup->members;

  return (OK);
}

/* recombobulates servicegroup definitions */
/***** THIS NEEDS TO BE CALLED AFTER OBJECTS (SERVICES) ARE RESOLVED AND DUPLICATED *****/
int xodtemplate_recombobulate_servicegroups() {
  xodtemplate_service* temp_service = NULL;
  xodtemplate_servicegroup* temp_servicegroup = NULL;
  xodtemplate_memberlist* temp_memberlist = NULL;
  xodtemplate_memberlist* this_memberlist = NULL;
  char* servicegroup_names = NULL;
  char* member_names = NULL;
  char* host_name = NULL;
  char* service_description = NULL;
  char* temp_ptr = NULL;
  char* temp_ptr2 = NULL;
  char* new_members = NULL;

  /* This should happen before we expand servicegroup members, to avoid duplicate service memberships 01/07/2006 EG */
  /* process all services that have servicegroup directives */
  for (temp_service = xodtemplate_service_list;
       temp_service != NULL;
       temp_service = temp_service->next) {

    /* skip services without servicegroup directives or service names */
    if (temp_service->service_groups == NULL
        || temp_service->host_name == NULL
        || temp_service->service_description == NULL)
      continue;

    /* skip services that shouldn't be registered */
    if (temp_service->register_object == false)
      continue;

    /* preprocess the servicegroup list, to change "grp1,grp2,grp3,!grp2" into "grp1,grp3" */
    /* 10/19/07 EG an empry return value means an error occured */
    if ((servicegroup_names = xodtemplate_process_servicegroup_names(
                                temp_service->service_groups,
                                temp_service->_config_file,
                                temp_service->_start_line)) == NULL)
      return (ERROR);

    /* process the list of servicegroups */
    for (temp_ptr = strtok(servicegroup_names, ",");
	 temp_ptr != NULL;
         temp_ptr = strtok(NULL, ",")) {

      /* strip trailing spaces */
      strip(temp_ptr);

      /* find the servicegroup */
      temp_servicegroup = xodtemplate_find_real_servicegroup(temp_ptr);
      if (temp_servicegroup == NULL) {
        logger(log_config_error, basic)
          << "Error: Could not find servicegroup '" << temp_ptr
          << "' specified in service '"
          << temp_service->service_description << "' on host '"
          << temp_service->host_name << "' definition (config file '"
          << xodtemplate_config_file_name(temp_service->_config_file)
          << "', starting on line " << temp_service->_start_line << ")";
        delete[] servicegroup_names;
        return (ERROR);
      }

      /* add this list to the servicegroup members directive */
      if (temp_servicegroup->members == NULL) {
        temp_servicegroup->members
          = new char[strlen(temp_service->host_name)
                     + strlen(temp_service->service_description) + 2];
        strcpy(temp_servicegroup->members, temp_service->host_name);
        strcat(temp_servicegroup->members, ",");
        strcat(temp_servicegroup->members, temp_service->service_description);
      }
      else {
        new_members = resize_string(
                        temp_servicegroup->members,
                        strlen(temp_servicegroup->members)
                        + strlen(temp_service->host_name)
                        + strlen(temp_service->service_description) + 3);
        temp_servicegroup->members = new_members;
        strcat(temp_servicegroup->members, ",");
        strcat(temp_servicegroup->members, temp_service->host_name);
        strcat(temp_servicegroup->members, ",");
        strcat(
          temp_servicegroup->members,
          temp_service->service_description);
      }
    }

    /* free servicegroup names */
    delete[] servicegroup_names;
    servicegroup_names = NULL;
  }

  /* expand subgroup membership recursively */
  for (temp_servicegroup = xodtemplate_servicegroup_list;
       temp_servicegroup != NULL;
       temp_servicegroup = temp_servicegroup->next)
    xodtemplate_recombobulate_servicegroup_subgroups(
      temp_servicegroup,
      NULL);

  /* expand members of all servicegroups - this could be done in xodtemplate_register_servicegroup(), but we can save the CGIs some work if we do it here */
  for (temp_servicegroup = xodtemplate_servicegroup_list;
       temp_servicegroup != NULL;
       temp_servicegroup = temp_servicegroup->next) {

    if (temp_servicegroup->members == NULL)
      continue;

    /* skip servicegroups that shouldn't be registered */
    if (temp_servicegroup->register_object == false)
      continue;

    member_names = temp_servicegroup->members;
    temp_servicegroup->members = NULL;

    for (temp_ptr = member_names;
	 temp_ptr != NULL;
         temp_ptr = strchr(temp_ptr + 1, ',')) {

      /* this is the host name */
      if (host_name == NULL)
        host_name
          = string::dup((temp_ptr[0] == ',') ? temp_ptr + 1 : temp_ptr);

      /* this is the service description */
      else {
        service_description = string::dup(temp_ptr + 1);

        /* strsep and strtok cannot be used, as they're used in expand_servicegroups...() */
        temp_ptr2 = strchr(host_name, ',');
        if (temp_ptr2)
          temp_ptr2[0] = '\x0';
        temp_ptr2 = strchr(service_description, ',');
        if (temp_ptr2)
          temp_ptr2[0] = '\x0';

        /* strip trailing spaces */
        strip(host_name);
        strip(service_description);

        /* get list of services in the servicegroup */
        temp_memberlist = xodtemplate_expand_servicegroups_and_services(
                            NULL,
                            host_name,
                            service_description,
                            temp_servicegroup->_config_file,
                            temp_servicegroup->_start_line);

        /* add all members to the service group */
        if (temp_memberlist == NULL) {
          logger(log_config_error, basic)
            << "Error: Could not expand member services specified in "
            "servicegroup (config file '"
            << xodtemplate_config_file_name(temp_servicegroup->_config_file)
            << "', starting on line " << temp_servicegroup->_start_line << ")";
          delete[] member_names;
          delete[] host_name;
          delete[] service_description;
          return (ERROR);
        }

        for (this_memberlist = temp_memberlist;
	     this_memberlist != NULL;
             this_memberlist = this_memberlist->next) {

          /* add this service to the servicegroup members directive */
          if (temp_servicegroup->members == NULL) {
            temp_servicegroup->members
              = new char[strlen(this_memberlist->name1)
                         + strlen(this_memberlist->name2) + 2];
            strcpy(temp_servicegroup->members, this_memberlist->name1);
            strcat(temp_servicegroup->members, ",");
            strcat(temp_servicegroup->members, this_memberlist->name2);
          }
          else {
            new_members = resize_string(
                            temp_servicegroup->members,
                            strlen(temp_servicegroup->members)
                            + strlen(this_memberlist->name1)
                            + strlen(this_memberlist->name2) + 3);
            temp_servicegroup->members = new_members;
            strcat(temp_servicegroup->members, ",");
            strcat(temp_servicegroup->members, this_memberlist->name1);
            strcat(temp_servicegroup->members, ",");
            strcat(temp_servicegroup->members, this_memberlist->name2);
          }
        }
        xodtemplate_free_memberlist(&temp_memberlist);

        delete[] host_name;
        delete[] service_description;

        host_name = NULL;
        service_description = NULL;
      }
    }

    delete[] member_names;
    member_names = NULL;

    /* error if there were an odd number of items specified (unmatched host/service pair) */
    if (host_name != NULL) {
      logger(log_config_error, basic)
        << "Error: Servicegroup members must be specified in "
        "<host_name>,<service_description> pairs (config file '"
        << xodtemplate_config_file_name(temp_servicegroup->_config_file)
        << "', starting on line " << temp_servicegroup->_start_line
        << ")";
      delete[] host_name;
      return (ERROR);
    }
  }

  return (OK);
}

int xodtemplate_recombobulate_servicegroup_subgroups(
      xodtemplate_servicegroup* temp_servicegroup,
      char** members) {
  if (temp_servicegroup == NULL)
    return (ERROR);

  /* resolve subgroup memberships first */
  if (temp_servicegroup->servicegroup_members != NULL) {

    /* save members, null pointer so we don't recurse into infinite hell */
    char* orig_sgmembers(temp_servicegroup->servicegroup_members);
    temp_servicegroup->servicegroup_members = NULL;

    /* make new working copy of members */
    char* sgmembers(string::dup(orig_sgmembers));

    char* buf(NULL);
    char* ptr(sgmembers);
    while ((buf = ptr) != NULL) {

      /* get next member for next run */
      ptr = strchr(ptr, ',');
      if (ptr) {
        ptr[0] = '\x0';
        ptr++;
      }

      strip(buf);

      /* find subgroup and recurse */
      xodtemplate_servicegroup* sub_group(NULL);
      if ((sub_group = xodtemplate_find_real_servicegroup(buf)) == NULL) {
        logger(log_config_error, basic)
          << "Error: Could not find member group '" << buf
          << "' specified in servicegroup (config file '"
          << xodtemplate_config_file_name(temp_servicegroup->_config_file)
          << "', starting on line " << temp_servicegroup->_start_line
          << ")";
        return (ERROR);
      }
      char* newmembers(NULL);
      xodtemplate_recombobulate_servicegroup_subgroups(
        sub_group,
        &newmembers);

      /* add new (sub) members */
      if (newmembers != NULL) {
        if (temp_servicegroup->members == NULL)
          temp_servicegroup->members = string::dup(newmembers);
        else {
          temp_servicegroup->members = resize_string(
                                         temp_servicegroup->members,
                                         strlen(temp_servicegroup->members)
                                         + strlen(newmembers) + 2);
          strcat(temp_servicegroup->members, ",");
          strcat(temp_servicegroup->members, newmembers);
        }
      }
    }

    /* free memory */
    delete[] sgmembers;
    sgmembers = NULL;

    /* restore group members */
    temp_servicegroup->servicegroup_members = orig_sgmembers;
  }

  /* return service members */
  if (members != NULL)
    *members = temp_servicegroup->members;

  return (OK);
}

/******************************************************************/
/******************* OBJECT SEARCH FUNCTIONS **********************/
/******************************************************************/

/* finds a specific timeperiod object */
xodtemplate_timeperiod* xodtemplate_find_timeperiod(char* name) {
  if (name == NULL)
    return (NULL);

  xodtemplate_timeperiod temp_timeperiod;
  temp_timeperiod.name = name;
  return ((xodtemplate_timeperiod*)skiplist_find_first(
                                     xobject_template_skiplists[X_TIMEPERIOD_SKIPLIST],
                                     &temp_timeperiod,
                                     NULL));
}

/* finds a specific command object */
xodtemplate_command* xodtemplate_find_command(char* name) {
  if (name == NULL)
    return (NULL);

  xodtemplate_command temp_command;
  temp_command.name = name;
  return ((xodtemplate_command*)skiplist_find_first(
                                  xobject_template_skiplists[X_COMMAND_SKIPLIST],
                                  &temp_command,
                                  NULL));
}

/* finds a specific hostgroup object */
xodtemplate_hostgroup* xodtemplate_find_hostgroup(char* name) {
  if (name == NULL)
    return (NULL);

  xodtemplate_hostgroup temp_hostgroup;
  temp_hostgroup.name = name;
  return ((xodtemplate_hostgroup*)skiplist_find_first(
                                    xobject_template_skiplists[X_HOSTGROUP_SKIPLIST],
                                    &temp_hostgroup,
                                    NULL));
}

/* finds a specific hostgroup object by its REAL name, not its TEMPLATE name */
xodtemplate_hostgroup* xodtemplate_find_real_hostgroup(char* name) {
  if (name == NULL)
    return (NULL);

  xodtemplate_hostgroup temp_hostgroup;
  temp_hostgroup.hostgroup_name = name;
  return ((xodtemplate_hostgroup*)skiplist_find_first(
                                    xobject_skiplists[X_HOSTGROUP_SKIPLIST],
                                    &temp_hostgroup,
                                    NULL));
}

/* finds a specific servicegroup object */
xodtemplate_servicegroup* xodtemplate_find_servicegroup(char* name) {
  if (name == NULL)
    return (NULL);

  xodtemplate_servicegroup temp_servicegroup;
  temp_servicegroup.name = name;
  return ((xodtemplate_servicegroup*)skiplist_find_first(
                                       xobject_template_skiplists[X_SERVICEGROUP_SKIPLIST],
                                       &temp_servicegroup,
                                       NULL));
}

/* finds a specific servicegroup object by its REAL name, not its TEMPLATE name */
xodtemplate_servicegroup* xodtemplate_find_real_servicegroup(char* name) {
  if (name == NULL)
    return (NULL);

  xodtemplate_servicegroup temp_servicegroup;
  temp_servicegroup.servicegroup_name = name;
  return ((xodtemplate_servicegroup*)skiplist_find_first(
                                       xobject_skiplists[X_SERVICEGROUP_SKIPLIST],
                                       &temp_servicegroup,
                                       NULL));
}

/* finds a specific servicedependency object */
xodtemplate_servicedependency* xodtemplate_find_servicedependency(
                                 char* name) {
  if (name == NULL)
    return (NULL);

  xodtemplate_servicedependency temp_servicedependency;
  temp_servicedependency.name = name;
  return ((xodtemplate_servicedependency*)skiplist_find_first(
                                            xobject_template_skiplists[X_SERVICEDEPENDENCY_SKIPLIST],
                                            &temp_servicedependency,
                                            NULL));
}

/* finds a specific host object */
xodtemplate_host* xodtemplate_find_host(char* name) {
  if (name == NULL)
    return (NULL);

  xodtemplate_host temp_host;
  temp_host.name = name;
  return ((xodtemplate_host*)skiplist_find_first(
                               xobject_template_skiplists[X_HOST_SKIPLIST],
                               &temp_host,
                               NULL));
}

/* finds a specific host object by its REAL name, not its TEMPLATE name */
xodtemplate_host* xodtemplate_find_real_host(char* name) {
  if (name == NULL)
    return (NULL);

  xodtemplate_host temp_host;
  temp_host.host_name = name;
  return ((xodtemplate_host*)skiplist_find_first(
                               xobject_skiplists[X_HOST_SKIPLIST],
                               &temp_host,
                               NULL));
}

/* finds a specific hostdependency object */
xodtemplate_hostdependency* xodtemplate_find_hostdependency(char* name) {
  if (name == NULL)
    return (NULL);

  xodtemplate_hostdependency temp_hostdependency;
  temp_hostdependency.name = name;
  return ((xodtemplate_hostdependency*)skiplist_find_first(
                                         xobject_template_skiplists[X_HOSTDEPENDENCY_SKIPLIST],
                                         &temp_hostdependency,
                                         NULL));
}

/* finds a specific service object */
xodtemplate_service* xodtemplate_find_service(char* name) {
  if (name == NULL)
    return (NULL);

  xodtemplate_service temp_service;
  temp_service.name = name;
  return ((xodtemplate_service*)skiplist_find_first(
                                  xobject_template_skiplists[X_SERVICE_SKIPLIST],
                                  &temp_service,
                                  NULL));
}

/* finds a specific service object by its REAL name, not its TEMPLATE name */
xodtemplate_service* xodtemplate_find_real_service(
                       char* host_name,
                       char* service_description) {
  if (host_name == NULL || service_description == NULL)
    return (NULL);

  xodtemplate_service temp_service;
  temp_service.host_name = host_name;
  temp_service.service_description = service_description;
  return ((xodtemplate_service*)skiplist_find_first(
                                  xobject_skiplists[X_SERVICE_SKIPLIST],
                                  &temp_service,
                                  NULL));
}

/******************************************************************/
/**************** OBJECT REGISTRATION FUNCTIONS *******************/
/******************************************************************/

/* registers object definitions */
int xodtemplate_register_objects() {
  void* ptr(NULL);

  // Register timeperiods.
  ptr = NULL;
  xodtemplate_timeperiod* temp_timeperiod = NULL;
  for (temp_timeperiod = (xodtemplate_timeperiod*)skiplist_get_first(xobject_skiplists[X_TIMEPERIOD_SKIPLIST], &ptr);
       temp_timeperiod;
       temp_timeperiod = (xodtemplate_timeperiod*)skiplist_get_next(&ptr)) {
    // Insert timeperiod object in lists.
    if (xodtemplate_register_timeperiod(temp_timeperiod) == ERROR)
      return (ERROR);

    // Retrieve timeperiod object.
    timeperiod* t(find_timeperiod(temp_timeperiod->timeperiod_name));

    // Fill timeperiod with its content.
    if (xodtemplate_fill_timeperiod(temp_timeperiod, t) == ERROR)
      return (ERROR);
  }

  /* register connectors */
  ptr = NULL;
  xodtemplate_connector* temp_connector(NULL);
  for (temp_connector = (xodtemplate_connector*)skiplist_get_first(xobject_skiplists[X_CONNECTOR_SKIPLIST], &ptr);
       temp_connector;
       temp_connector = (xodtemplate_connector*)skiplist_get_next(&ptr)) {
    if (xodtemplate_register_connector(temp_connector) == ERROR)
      return (ERROR);
  }

  /* register commands */
  ptr = NULL;
  xodtemplate_command* temp_command(NULL);
  for (temp_command = (xodtemplate_command*)skiplist_get_first(xobject_skiplists[X_COMMAND_SKIPLIST], &ptr);
       temp_command;
       temp_command = (xodtemplate_command*)skiplist_get_next(&ptr)) {
    if (xodtemplate_register_command(temp_command) == ERROR)
      return (ERROR);
  }

  /* register hostgroups */
  ptr = NULL;
  xodtemplate_hostgroup* temp_hostgroup(NULL);
  for (temp_hostgroup = (xodtemplate_hostgroup*)skiplist_get_first(xobject_skiplists[X_HOSTGROUP_SKIPLIST], &ptr);
       temp_hostgroup;
       temp_hostgroup = (xodtemplate_hostgroup*)skiplist_get_next(&ptr)) {
    if (xodtemplate_register_hostgroup(temp_hostgroup) == ERROR)
      return (ERROR);
  }

  /* register servicegroups */
  ptr = NULL;
  xodtemplate_servicegroup* temp_servicegroup(NULL);
  for (temp_servicegroup = (xodtemplate_servicegroup*)skiplist_get_first(xobject_skiplists[X_SERVICEGROUP_SKIPLIST], &ptr);
       temp_servicegroup;
       temp_servicegroup = (xodtemplate_servicegroup*)skiplist_get_next(&ptr)) {
    if (xodtemplate_register_servicegroup(temp_servicegroup) == ERROR)
      return (ERROR);
  }

  /* register hosts */
  ptr = NULL;
  xodtemplate_host* temp_host(NULL);
  for (temp_host = (xodtemplate_host*)skiplist_get_first(xobject_skiplists[X_HOST_SKIPLIST], &ptr);
       temp_host;
       temp_host = (xodtemplate_host*)skiplist_get_next(&ptr)) {
    if (xodtemplate_register_host(temp_host) == ERROR)
      return (ERROR);
  }

  /* register services */
  ptr = NULL;
  xodtemplate_service* temp_service(NULL);
  for (temp_service = (xodtemplate_service*)skiplist_get_first(xobject_skiplists[X_SERVICE_SKIPLIST], &ptr);
       temp_service;
       temp_service = (xodtemplate_service*)skiplist_get_next(&ptr)) {

    if (xodtemplate_register_service(temp_service) == ERROR)
      return (ERROR);
  }

  /* register service dependencies */
  ptr = NULL;
  xodtemplate_servicedependency* temp_servicedependency(NULL);
  for (temp_servicedependency = (xodtemplate_servicedependency*)skiplist_get_first(xobject_skiplists[X_SERVICEDEPENDENCY_SKIPLIST], &ptr);
       temp_servicedependency;
       temp_servicedependency = (xodtemplate_servicedependency*)skiplist_get_next(&ptr)) {
    if (xodtemplate_register_servicedependency(temp_servicedependency) == ERROR)
      return (ERROR);
  }

  /* register host dependencies */
  ptr = NULL;
  xodtemplate_hostdependency* temp_hostdependency(NULL);
  for (temp_hostdependency = (xodtemplate_hostdependency*)skiplist_get_first(xobject_skiplists[X_HOSTDEPENDENCY_SKIPLIST], &ptr);
       temp_hostdependency;
       temp_hostdependency = (xodtemplate_hostdependency*)skiplist_get_next(&ptr)) {
    if (xodtemplate_register_hostdependency(temp_hostdependency) == ERROR)
      return (ERROR);
  }

  return (OK);
}

/**
 *  Fill timeperiod with its content.
 *
 *  @param[in]  this_timeperiod Template timeperiod.
 *  @param[out] new_timeperiod  Real timeperiod.
 *
 *  @return OK on success.
 */
int xodtemplate_fill_timeperiod(
      xodtemplate_timeperiod* this_timeperiod,
      timeperiod* new_timeperiod) {
  xodtemplate_daterange* temp_daterange = NULL;
  daterange* new_daterange = NULL;
  timerange* new_timerange = NULL;
  timeperiodexclusion* new_timeperiodexclusion = NULL;
  int day = 0;
  int range = 0;
  int x = 0;
  char* day_range_ptr = NULL;
  char* day_range_start_buffer = NULL;
  char* temp_ptr = NULL;
  unsigned long range_start_time = 0L;
  unsigned long range_end_time = 0L;

  /* add all exceptions to timeperiod */
  for (x = 0; x < DATERANGE_TYPES; x++) {
    for (temp_daterange = this_timeperiod->exceptions[x];
         temp_daterange != NULL;
         temp_daterange = temp_daterange->next) {

      /* skip null entries */
      if (temp_daterange->timeranges == NULL
          || !strcmp(temp_daterange->timeranges, XODTEMPLATE_NULL))
        continue;

      /* add new exception to timeperiod */
      new_daterange = add_exception_to_timeperiod(
                        new_timeperiod,
                        temp_daterange->type,
                        temp_daterange->syear,
                        temp_daterange->smon,
                        temp_daterange->smday,
                        temp_daterange->swday,
                        temp_daterange->swday_offset,
                        temp_daterange->eyear,
                        temp_daterange->emon,
                        temp_daterange->emday,
                        temp_daterange->ewday,
                        temp_daterange->ewday_offset,
                        temp_daterange->skip_interval);
      if (new_daterange == NULL) {
        logger(log_config_error, basic)
          << "Error: Could not add date exception to timeperiod "
          "(config file '"
          << xodtemplate_config_file_name(this_timeperiod->_config_file)
          << "', starting on line " << this_timeperiod->_start_line
          << ")";
        return (ERROR);
      }

      /* add timeranges to exception */
      day_range_ptr = temp_daterange->timeranges;
      range = 0;
      for (day_range_start_buffer = my_strsep(&day_range_ptr, ", ");
           day_range_start_buffer != NULL;
           day_range_start_buffer = my_strsep(&day_range_ptr, ", ")) {

        range++;

        /* get time ranges */
        if (xodtemplate_get_time_ranges(
              day_range_start_buffer,
              &range_start_time,
              &range_end_time) == ERROR) {
          logger(log_config_error, basic)
            << "Error: Could not parse timerange #" << range
            << " of timeperiod (config file '"
            << xodtemplate_config_file_name(this_timeperiod->_config_file)
            << "', starting on line " << this_timeperiod->_start_line
            << ")";
          return (ERROR);
        }

        /* add the new time range to the date range */
        new_timerange = add_timerange_to_daterange(
                          new_daterange,
                          range_start_time / (60 * 60),
                          (range_start_time / 60) % 60,
                          range_end_time / (60 * 60),
                          (range_end_time / 60) % 60);
        if (new_timerange == NULL) {
          logger(log_config_error, basic)
            << "Error: Could not add timerange #" << range
            << " to timeperiod (config file '"
            << xodtemplate_config_file_name(this_timeperiod->_config_file)
            << "', starting on line " << this_timeperiod->_start_line
            << ")";
          return (ERROR);
        }
      }
    }
  }

  /* add all necessary timeranges to timeperiod */
  for (day = 0; day < 7; day++) {

    /* skip null entries */
    if (this_timeperiod->timeranges[day] == NULL
        || !strcmp(this_timeperiod->timeranges[day], XODTEMPLATE_NULL))
      continue;

    day_range_ptr = this_timeperiod->timeranges[day];
    range = 0;
    for (day_range_start_buffer = my_strsep(&day_range_ptr, ", ");
         day_range_start_buffer != NULL;
         day_range_start_buffer = my_strsep(&day_range_ptr, ", ")) {

      range++;

      /* get time ranges */
      if (xodtemplate_get_time_ranges(
            day_range_start_buffer,
            &range_start_time,
            &range_end_time) == ERROR) {
        logger(log_config_error, basic)
          << "Error: Could not parse timerange #" << range
          <<" for day " << day << " of timeperiod (config file '"
          << xodtemplate_config_file_name(this_timeperiod->_config_file)
          << "', starting on line " << this_timeperiod->_start_line
          << ")";
        return (ERROR);
      }

      /* add the new time range to the time period */
      new_timerange = add_timerange_to_timeperiod(
                        new_timeperiod,
                        day,
                        range_start_time / (60 * 60),
                        (range_start_time / 60) % 60,
                        range_end_time / (60 * 60),
                        (range_end_time / 60) % 60);
      if (new_timerange == NULL) {
        logger(log_config_error, basic)
          << "Error: Could not add timerange #" << range
          << " for day " << day << " to timeperiod (config file '"
          << xodtemplate_config_file_name(this_timeperiod->_config_file)
          << "', starting on line " << this_timeperiod->_start_line
          << ")";
        return (ERROR);
      }
    }
  }

  /* add timeperiod exclusions */
  if (this_timeperiod->exclusions) {
    for (temp_ptr = strtok(this_timeperiod->exclusions, ",");
         temp_ptr != NULL;
	 temp_ptr = strtok(NULL, ",")) {
      strip(temp_ptr);
      new_timeperiodexclusion = add_exclusion_to_timeperiod(
                                  new_timeperiod,
                                  temp_ptr);
      if (new_timeperiodexclusion == NULL) {
        logger(log_config_error, basic)
          << "Error: Could not add excluded timeperiod '" << temp_ptr
          << "' to timeperiod (config file '"
          << xodtemplate_config_file_name(this_timeperiod->_config_file)
          << "', starting on line " << this_timeperiod->_start_line
          << ")";
        return (ERROR);
      }
    }
  }

  return (OK);
}

/**
 *  @brief Registers a timeperiod definition.
 *
 *  Timeperiod will be inserted in lists but not filled with its
 *  content.
 *
 *  @param[in] this_timeperiod Template timeperiod object.
 *
 *  @return OK on success.
 *
 *  @see xodtemplate_fill_timeperiod
 */
int xodtemplate_register_timeperiod(
      xodtemplate_timeperiod* this_timeperiod) {
  // Bail out if we shouldn't register this object.
  if (this_timeperiod->register_object == false)
    return (OK);

  // Add the timeperiod.
  timeperiod* new_timeperiod(add_timeperiod(
                               this_timeperiod->timeperiod_name,
                               this_timeperiod->alias));

  // Return with an error if we couldn't add the timeperiod.
  if (!new_timeperiod) {
    logger(log_config_error, basic)
      << "Error: Could not register timeperiod (config file '"
      << xodtemplate_config_file_name(this_timeperiod->_config_file)
      << "', starting on line " << this_timeperiod->_start_line << ")";
    return (ERROR);
  }

  return (OK);
}

/* parses timerange string into start and end minutes */
int xodtemplate_get_time_ranges(
      char* buf,
      unsigned long* range_start,
      unsigned long* range_end) {
  char* range_ptr = NULL;
  char* range_buffer = NULL;
  char* time_ptr = NULL;
  char* time_buffer = NULL;
  int hours = 0;
  int minutes = 0;

  if (buf == NULL || range_start == NULL || range_end == NULL)
    return (ERROR);

  range_ptr = buf;
  range_buffer = my_strsep(&range_ptr, "-");
  if (range_buffer == NULL)
    return (ERROR);

  time_ptr = range_buffer;
  time_buffer = my_strsep(&time_ptr, ":");
  if (time_buffer == NULL)
    return (ERROR);
  hours = atoi(time_buffer);

  time_buffer = my_strsep(&time_ptr, ":");
  if (time_buffer == NULL)
    return (ERROR);
  minutes = atoi(time_buffer);

  /* calculate the range start time in seconds */
  *range_start = (unsigned long)((minutes * 60) + (hours * 60 * 60));

  range_buffer = my_strsep(&range_ptr, "-");
  if (range_buffer == NULL)
    return (ERROR);

  time_ptr = range_buffer;
  time_buffer = my_strsep(&time_ptr, ":");
  if (time_buffer == NULL)
    return (ERROR);
  hours = atoi(time_buffer);

  time_buffer = my_strsep(&time_ptr, ":");
  if (time_buffer == NULL)
    return (ERROR);
  minutes = atoi(time_buffer);

  /* calculate the range end time in seconds */
  *range_end = (unsigned long)((minutes * 60) + (hours * 3600));

  return (OK);
}

/* registers a command definition */
int xodtemplate_register_command(xodtemplate_command* this_command) {
  /* bail out if we shouldn't register this object */
  if (this_command->register_object == false)
    return (OK);

  // Initialize command executon system.
  try {
    using namespace com::centreon::engine;
    commands::set& cmd_set(commands::set::instance());
    if (this_command->connector_name == NULL) {
      com::centreon::shared_ptr<commands::command>
        cmd(new commands::raw(
                            this_command->command_name,
                            this_command->command_line,
                            &checks::checker::instance()));
      cmd_set.add_command(cmd);
    }
    else {
      com::centreon::shared_ptr<commands::command>
        cmd_forward(cmd_set.get_command(this_command->connector_name));

      com::centreon::shared_ptr<commands::command>
        cmd(new commands::forward(
                            this_command->command_name,
                            this_command->command_line,
                            *cmd_forward));
      cmd_set.add_command(cmd);
    }
  }
  catch (std::exception const& e) {
    logger(log_config_error, basic)
      << "Error: Could not register command (config file '"
      << xodtemplate_config_file_name(this_command->_config_file)
      << "', starting on line " << this_command->_start_line << "): "
      << e.what();
    return (ERROR);
  }

  /* add the command */
  command* new_command(add_command(
                         this_command->command_name,
                         this_command->command_line));

  /* return with an error if we couldn't add the command */
  if (new_command == NULL) {
    logger(log_config_error, basic)
      << "Error: Could not register command (config file '"
      << xodtemplate_config_file_name(this_command->_config_file)
      << "', starting on line " << this_command->_start_line << ")";
    return (ERROR);
  }

  return (OK);
}

/* registers a connector definition */
int xodtemplate_register_connector(xodtemplate_connector* this_connector) {
  /* bail out if we shouldn't register this object */
  if (this_connector->register_object == false)
    return (OK);

  // Initialize command executon system.
  try {
    using namespace com::centreon::engine;

    nagios_macros* macros(get_global_macros());
    char* command_line(NULL);
    process_macros_r(
      macros,
      this_connector->connector_line,
      &command_line,
      0);
    std::string processed_cmd(command_line);
    delete[] command_line;

    com::centreon::shared_ptr<commands::command>
      cmd(new commands::connector(
                          this_connector->connector_name,
                          processed_cmd,
                          &checks::checker::instance()));
    commands::set::instance().add_command(cmd);
  }
  catch (std::exception const& e) {
    logger(log_config_error, basic)
      << "Error: Could not register connector (config file '"
      << xodtemplate_config_file_name(this_connector->_config_file)
      << "', starting on line " << this_connector->_start_line << "): "
      << e.what();
    return (ERROR);
  }

  return (OK);
}

/* registers a hostgroup definition */
int xodtemplate_register_hostgroup(
      xodtemplate_hostgroup* this_hostgroup) {
  /* bail out if we shouldn't register this object */
  if (this_hostgroup->register_object == false)
    return (OK);

  /* add the  host group */
  hostgroup* new_hostgroup
    = add_hostgroup(
        this_hostgroup->hostgroup_name,
        this_hostgroup->alias);

  /* return with an error if we couldn't add the hostgroup */
  if (new_hostgroup == NULL) {
    logger(log_config_error, basic)
      << "Error: Could not register hostgroup (config file '"
      << xodtemplate_config_file_name(this_hostgroup->_config_file)
      << "', starting on line " << this_hostgroup->_start_line
      << ")";
    return (ERROR);
  }

  if (this_hostgroup->members != NULL) {
    for (char* host_name(strtok(this_hostgroup->members, ","));
         host_name != NULL;
	 host_name = strtok(NULL, ",")) {
      strip(host_name);
      hostsmember* new_hostsmember
        = add_host_to_hostgroup(new_hostgroup, host_name);
      if (new_hostsmember == NULL) {
        logger(log_config_error, basic)
          << "Error: Could not add host '" << host_name
          << "' to hostgroup (config file '"
          << xodtemplate_config_file_name(this_hostgroup->_config_file)
          << "', starting on line " << this_hostgroup->_start_line
          << ")";
        return (ERROR);
      }
    }
  }

  return (OK);
}

/* registers a servicegroup definition */
int xodtemplate_register_servicegroup(
      xodtemplate_servicegroup* this_servicegroup) {
  /* bail out if we shouldn't register this object */
  if (this_servicegroup->register_object == false)
    return (OK);

  /* add the  service group */
  servicegroup* new_servicegroup
    = add_servicegroup(
        this_servicegroup->servicegroup_name,
        this_servicegroup->alias);

  /* return with an error if we couldn't add the servicegroup */
  if (new_servicegroup == NULL) {
    logger(log_config_error, basic)
      << "Error: Could not register servicegroup (config file '"
      << xodtemplate_config_file_name(this_servicegroup->_config_file)
      << "', starting on line " << this_servicegroup->_start_line
      << ")";
    return (ERROR);
  }

  if (this_servicegroup->members != NULL) {
    for (char* host_name(strtok(this_servicegroup->members, ","));
         host_name != NULL;
	 host_name = strtok(NULL, ",")) {
      strip(host_name);
      char* svc_description(strtok(NULL, ","));
      if (svc_description == NULL) {
        logger(log_config_error, basic)
          << "Error: Missing service name in servicegroup definition "
          "(config file '"
          << xodtemplate_config_file_name(this_servicegroup->_config_file)
          << "', starting on line " << this_servicegroup->_start_line
          << ")";
        return (ERROR);
      }
      strip(svc_description);

      servicesmember* new_servicesmember
        = add_service_to_servicegroup(
            new_servicegroup,
            host_name,
            svc_description);
      if (new_servicesmember == NULL) {
        logger(log_config_error, basic)
          << "Error: Could not add service '" << svc_description
          << "' on host '" << host_name << "' to servicegroup "
          "(config file '"
          << xodtemplate_config_file_name(this_servicegroup->_config_file)
          << "', starting on line " << this_servicegroup->_start_line
          << ")";
        return (ERROR);
      }
    }
  }

  return (OK);
}

/* registers a servicedependency definition */
int xodtemplate_register_servicedependency(
      xodtemplate_servicedependency* this_servicedependency) {
  servicedependency* new_servicedependency = NULL;

  /* bail out if we shouldn't register this object */
  if (this_servicedependency->register_object == false)
    return (OK);

  /* throw a warning on servicedeps that have no options */
  if (this_servicedependency->have_dependency_options == false) {
    logger(log_config_warning, basic)
      << "Warning: Ignoring lame service dependency (config file '"
      << xodtemplate_config_file_name(this_servicedependency->_config_file)
      << "', line " << this_servicedependency->_start_line << ")";
    return (OK);
  }

  /* add the servicedependency */
  if (this_servicedependency->have_dependency_options == true) {
    new_servicedependency = add_service_dependency(
                              this_servicedependency->dependent_host_name,
                              this_servicedependency->dependent_service_description,
                              this_servicedependency->host_name,
                              this_servicedependency->service_description,
                              this_servicedependency->inherits_parent,
                              this_servicedependency->fail_on_ok,
                              this_servicedependency->fail_on_warning,
                              this_servicedependency->fail_on_unknown,
                              this_servicedependency->fail_on_critical,
                              this_servicedependency->fail_on_pending,
                              this_servicedependency->dependency_period);

    /* return with an error if we couldn't add the servicedependency */
    if (new_servicedependency == NULL) {
      logger(log_config_error, basic)
        << "Error: Could not register service execution dependency "
        "(config file '"
        << xodtemplate_config_file_name(this_servicedependency->_config_file)
        << "', starting on line " << this_servicedependency->_start_line
        << ")";
      return (ERROR);
    }
  }

  return (OK);
}

/* registers a host definition */
int xodtemplate_register_host(xodtemplate_host* this_host) {
  host* new_host = NULL;
  char* parent_host = NULL;
  hostsmember* new_hostsmember = NULL;
  xodtemplate_customvariablesmember* temp_customvariablesmember = NULL;

  /* bail out if we shouldn't register this object */
  if (this_host->register_object == false)
    return (OK);

  /* if host has no alias or address, use host name - added 3/11/05 */
  if (this_host->alias == NULL && this_host->host_name != NULL)
    this_host->alias = string::dup(this_host->host_name);
  if (this_host->address == NULL && this_host->host_name != NULL)
    this_host->address = string::dup(this_host->host_name);

  /* add the host definition */
  new_host = add_host(
               this_host->host_name,
               this_host->alias,
               (this_host->address == NULL) ? this_host->host_name : this_host->address,
               this_host->check_period,
               this_host->initial_state,
               this_host->check_interval,
               this_host->retry_interval,
               this_host->max_check_attempts,
               this_host->check_timeout,
               this_host->check_command,
               this_host->active_checks_enabled,
               this_host->event_handler,
               this_host->event_handler_enabled,
               this_host->flap_detection_enabled,
               this_host->low_flap_threshold,
               this_host->high_flap_threshold,
               this_host->flap_detection_on_up,
               this_host->flap_detection_on_down,
               this_host->flap_detection_on_unreachable,
               this_host->check_freshness,
               this_host->freshness_threshold,
               true,
               this_host->obsess_over_host,
               this_host->timezone);

  /* return with an error if we couldn't add the host */
  if (new_host == NULL) {
    logger(log_config_error, basic)
      << "Error: Could not register host (config file '"
      << xodtemplate_config_file_name(this_host->_config_file)
      << "', starting on line " << this_host->_start_line << ")";
    return (ERROR);
  }

  /* add the parent hosts */
  if (this_host->parents != NULL) {

    for (parent_host = strtok(this_host->parents, ",");
         parent_host != NULL;
	 parent_host = strtok(NULL, ",")) {
      strip(parent_host);
      new_hostsmember = add_parent_host_to_host(new_host, parent_host);
      if (new_hostsmember == NULL) {
        logger(log_config_error, basic)
          << "Error: Could not add parent host '" << parent_host
          << "' to host (config file '"
          << xodtemplate_config_file_name(this_host->_config_file)
          << "', starting on line " << this_host->_start_line << ")";
        return (ERROR);
      }
    }
  }

  /* add all custom variables */
  for (temp_customvariablesmember = this_host->custom_variables;
       temp_customvariablesmember != NULL;
       temp_customvariablesmember = temp_customvariablesmember->next) {
    if ((add_custom_variable_to_host(
           new_host,
           temp_customvariablesmember->variable_name,
           temp_customvariablesmember->variable_value)) == NULL) {
      logger(log_config_error, basic)
        << "Error: Could not custom variable to host (config file '"
        << xodtemplate_config_file_name(this_host->_config_file)
        << "', starting on line " << this_host->_start_line << ")";
      return (ERROR);
    }
  }

  return (OK);
}

/* registers a service definition */
int xodtemplate_register_service(xodtemplate_service* this_service) {
  service* new_service = NULL;
  xodtemplate_customvariablesmember* temp_customvariablesmember = NULL;

  /* bail out if we shouldn't register this object */
  if (this_service->register_object == false)
    return (OK);

  /* add the service */
  new_service = add_service(
                  this_service->host_name,
                  this_service->service_description,
                  this_service->check_period,
                  this_service->initial_state,
                  this_service->max_check_attempts,
                  this_service->check_timeout,
                  this_service->check_interval,
                  this_service->retry_interval,
                  this_service->is_volatile,
                  this_service->event_handler,
                  this_service->event_handler_enabled,
                  this_service->check_command,
                  this_service->active_checks_enabled,
                  this_service->flap_detection_enabled,
                  this_service->low_flap_threshold,
                  this_service->high_flap_threshold,
                  this_service->flap_detection_on_ok,
                  this_service->flap_detection_on_warning,
                  this_service->flap_detection_on_unknown,
                  this_service->flap_detection_on_critical,
                  this_service->check_freshness,
                  this_service->freshness_threshold,
                  this_service->obsess_over_service,
                  this_service->timezone);

  /* return with an error if we couldn't add the service */
  if (new_service == NULL) {
    logger(log_config_error, basic)
      << "Error: Could not register service (config file '"
      << xodtemplate_config_file_name(this_service->_config_file)
      << "', starting on line " << this_service->_start_line << ")";
    return (ERROR);
  }

  /* add all custom variables */
  for (temp_customvariablesmember = this_service->custom_variables;
       temp_customvariablesmember != NULL;
       temp_customvariablesmember = temp_customvariablesmember->next) {
    if ((add_custom_variable_to_service(
           new_service,
           temp_customvariablesmember->variable_name,
           temp_customvariablesmember->variable_value)) == NULL) {
      logger(log_config_error, basic)
        << "Error: Could not custom variable to service (config file '"
        << xodtemplate_config_file_name(this_service->_config_file)
        << "', starting on line " << this_service->_start_line << ")";
      return (ERROR);
    }
  }

  return (OK);
}

/* registers a hostdependency definition */
int xodtemplate_register_hostdependency(
      xodtemplate_hostdependency* this_hostdependency) {
  hostdependency* new_hostdependency = NULL;

  /* bail out if we shouldn't register this object */
  if (this_hostdependency->register_object == false)
    return (OK);

  /* add the host execution dependency */
  new_hostdependency = add_host_dependency(
                         this_hostdependency->dependent_host_name,
                         this_hostdependency->host_name,
                         this_hostdependency->inherits_parent,
                         this_hostdependency->fail_on_up,
                         this_hostdependency->fail_on_down,
                         this_hostdependency->fail_on_unreachable,
                         this_hostdependency->fail_on_pending,
                         this_hostdependency->dependency_period);

  /* return with an error if we couldn't add the hostdependency */
  if (new_hostdependency == NULL) {
    logger(log_config_error, basic)
      << "Error: Could not register host execution dependency "
      "(config file '"
      << xodtemplate_config_file_name(this_hostdependency->_config_file)
      << "', starting on line " << this_hostdependency->_start_line
      << ")";
    return (ERROR);
  }

  return (OK);
}

/******************************************************************/
/********************** SORTING FUNCTIONS *************************/
/******************************************************************/

/* sorts all objects by name */
int xodtemplate_sort_objects() {

  /* NOTE: with skiplists, we no longer need to sort things manually... */
  return (OK);

  /* sort timeperiods */
  if (xodtemplate_sort_timeperiods() == ERROR)
    return (ERROR);

  /* sort commands */
  if (xodtemplate_sort_commands() == ERROR)
    return (ERROR);

  /* sort connectors */
  if (xodtemplate_sort_connectors() == ERROR)
    return (ERROR);

  /* sort hostgroups */
  if (xodtemplate_sort_hostgroups() == ERROR)
    return (ERROR);

  /* sort servicegroups */
  if (xodtemplate_sort_servicegroups() == ERROR)
    return (ERROR);

  /* sort hosts */
  if (xodtemplate_sort_hosts() == ERROR)
    return (ERROR);

  /* sort services */
  if (xodtemplate_sort_services() == ERROR)
    return (ERROR);

  /* sort service dependencies */
  if (xodtemplate_sort_servicedependencies() == ERROR)
    return (ERROR);

  /* sort host dependencies */
  if (xodtemplate_sort_hostdependencies() == ERROR)
    return (ERROR);

  return (OK);
}


/* used to compare two strings (object names) */
int xodtemplate_compare_strings1(char* string1, char* string2) {

  if (string1 == NULL && string2 == NULL)
    return (0);
  else if (string1 == NULL)
    return (-1);
  else if (string2 == NULL)
    return (1);
  else
    return (strcmp(string1, string2));
}


/* used to compare two sets of strings (dually-named objects, i.e. services) */
int xodtemplate_compare_strings2(
      char* string1a,
      char* string1b,
      char* string2a,
      char* string2b) {
  int result;
  if ((result = xodtemplate_compare_strings1(string1a, string2a)) == 0)
    result = xodtemplate_compare_strings1(string1b, string2b);
  return (result);
}

/* sort timeperiods by name */
int xodtemplate_sort_timeperiods() {
  xodtemplate_timeperiod* new_timeperiod_list = NULL;
  xodtemplate_timeperiod* temp_timeperiod = NULL;
  xodtemplate_timeperiod* last_timeperiod = NULL;
  xodtemplate_timeperiod* temp_timeperiod_orig = NULL;
  xodtemplate_timeperiod* next_timeperiod_orig = NULL;

  /* sort all existing timeperiods */
  for (temp_timeperiod_orig = xodtemplate_timeperiod_list;
       temp_timeperiod_orig != NULL;
       temp_timeperiod_orig = next_timeperiod_orig) {

    next_timeperiod_orig = temp_timeperiod_orig->next;

    /* add timeperiod to new list, sorted by timeperiod name */
    last_timeperiod = new_timeperiod_list;
    for (temp_timeperiod = new_timeperiod_list;
	 temp_timeperiod != NULL;
         temp_timeperiod = temp_timeperiod->next) {

      if (xodtemplate_compare_strings1(
            temp_timeperiod_orig->timeperiod_name,
            temp_timeperiod->timeperiod_name) <= 0)
        break;
      else
        last_timeperiod = temp_timeperiod;
    }

    /* first item added to new sorted list */
    if (new_timeperiod_list == NULL) {
      temp_timeperiod_orig->next = NULL;
      new_timeperiod_list = temp_timeperiod_orig;
    }

    /* item goes at head of new sorted list */
    else if (temp_timeperiod == new_timeperiod_list) {
      temp_timeperiod_orig->next = new_timeperiod_list;
      new_timeperiod_list = temp_timeperiod_orig;
    }

    /* item goes in middle or at end of new sorted list */
    else {
      temp_timeperiod_orig->next = temp_timeperiod;
      last_timeperiod->next = temp_timeperiod_orig;
    }
  }

  /* list is now sorted */
  xodtemplate_timeperiod_list = new_timeperiod_list;

  return (OK);
}

/* sort commands by name */
int xodtemplate_sort_commands() {
  xodtemplate_command* new_command_list = NULL;
  xodtemplate_command* temp_command = NULL;
  xodtemplate_command* last_command = NULL;
  xodtemplate_command* temp_command_orig = NULL;
  xodtemplate_command* next_command_orig = NULL;

  /* sort all existing commands */
  for (temp_command_orig = xodtemplate_command_list;
       temp_command_orig != NULL;
       temp_command_orig = next_command_orig) {

    next_command_orig = temp_command_orig->next;

    /* add command to new list, sorted by command name */
    last_command = new_command_list;
    for (temp_command = new_command_list;
         temp_command != NULL;
         temp_command = temp_command->next) {
      if (xodtemplate_compare_strings1(
            temp_command_orig->command_name,
            temp_command->command_name) <= 0)
        break;
      else
        last_command = temp_command;
    }

    /* first item added to new sorted list */
    if (new_command_list == NULL) {
      temp_command_orig->next = NULL;
      new_command_list = temp_command_orig;
    }

    /* item goes at head of new sorted list */
    else if (temp_command == new_command_list) {
      temp_command_orig->next = new_command_list;
      new_command_list = temp_command_orig;
    }

    /* item goes in middle or at end of new sorted list */
    else {
      temp_command_orig->next = temp_command;
      last_command->next = temp_command_orig;
    }
  }

  /* list is now sorted */
  xodtemplate_command_list = new_command_list;

  return (OK);
}

/* sort connectors by name */
int xodtemplate_sort_connectors() {
  xodtemplate_connector* new_connector_list = NULL;
  xodtemplate_connector* temp_connector = NULL;
  xodtemplate_connector* last_connector = NULL;
  xodtemplate_connector* temp_connector_orig = NULL;
  xodtemplate_connector* next_connector_orig = NULL;

  /* sort all existing connectors */
  for (temp_connector_orig = xodtemplate_connector_list;
       temp_connector_orig != NULL;
       temp_connector_orig = next_connector_orig) {

    next_connector_orig = temp_connector_orig->next;

    /* add connector to new list, sorted by connector name */
    last_connector = new_connector_list;
    for (temp_connector = new_connector_list;
	 temp_connector != NULL;
	 temp_connector = temp_connector->next) {
      if (xodtemplate_compare_strings1(
            temp_connector_orig->connector_name,
            temp_connector->connector_name) <= 0)
        break;
      else
        last_connector = temp_connector;
    }

    /* first item added to new sorted list */
    if (new_connector_list == NULL) {
      temp_connector_orig->next = NULL;
      new_connector_list = temp_connector_orig;
    }

    /* item goes at head of new sorted list */
    else if (temp_connector == new_connector_list) {
      temp_connector_orig->next = new_connector_list;
      new_connector_list = temp_connector_orig;
    }

    /* item goes in middle or at end of new sorted list */
    else {
      temp_connector_orig->next = temp_connector;
      last_connector->next = temp_connector_orig;
    }
  }

  /* list is now sorted */
  xodtemplate_connector_list = new_connector_list;

  return (OK);
}

/* sort hostgroups by name */
int xodtemplate_sort_hostgroups() {
  xodtemplate_hostgroup* new_hostgroup_list = NULL;
  xodtemplate_hostgroup* temp_hostgroup = NULL;
  xodtemplate_hostgroup* last_hostgroup = NULL;
  xodtemplate_hostgroup* temp_hostgroup_orig = NULL;
  xodtemplate_hostgroup* next_hostgroup_orig = NULL;

  /* sort all existing hostgroups */
  for (temp_hostgroup_orig = xodtemplate_hostgroup_list;
       temp_hostgroup_orig != NULL;
       temp_hostgroup_orig = next_hostgroup_orig) {

    next_hostgroup_orig = temp_hostgroup_orig->next;

    /* add hostgroup to new list, sorted by hostgroup name */
    last_hostgroup = new_hostgroup_list;
    for (temp_hostgroup = new_hostgroup_list;
	 temp_hostgroup != NULL;
         temp_hostgroup = temp_hostgroup->next) {

      if (xodtemplate_compare_strings1(
            temp_hostgroup_orig->hostgroup_name,
            temp_hostgroup->hostgroup_name) <= 0)
        break;
      else
        last_hostgroup = temp_hostgroup;
    }

    /* first item added to new sorted list */
    if (new_hostgroup_list == NULL) {
      temp_hostgroup_orig->next = NULL;
      new_hostgroup_list = temp_hostgroup_orig;
    }

    /* item goes at head of new sorted list */
    else if (temp_hostgroup == new_hostgroup_list) {
      temp_hostgroup_orig->next = new_hostgroup_list;
      new_hostgroup_list = temp_hostgroup_orig;
    }

    /* item goes in middle or at end of new sorted list */
    else {
      temp_hostgroup_orig->next = temp_hostgroup;
      last_hostgroup->next = temp_hostgroup_orig;
    }
  }

  /* list is now sorted */
  xodtemplate_hostgroup_list = new_hostgroup_list;

  return (OK);
}

/* sort servicegroups by name */
int xodtemplate_sort_servicegroups() {
  xodtemplate_servicegroup* new_servicegroup_list = NULL;
  xodtemplate_servicegroup* temp_servicegroup = NULL;
  xodtemplate_servicegroup* last_servicegroup = NULL;
  xodtemplate_servicegroup* temp_servicegroup_orig = NULL;
  xodtemplate_servicegroup* next_servicegroup_orig = NULL;

  /* sort all existing servicegroups */
  for (temp_servicegroup_orig = xodtemplate_servicegroup_list;
       temp_servicegroup_orig != NULL;
       temp_servicegroup_orig = next_servicegroup_orig) {

    next_servicegroup_orig = temp_servicegroup_orig->next;

    /* add servicegroup to new list, sorted by servicegroup name */
    last_servicegroup = new_servicegroup_list;
    for (temp_servicegroup = new_servicegroup_list;
         temp_servicegroup != NULL;
         temp_servicegroup = temp_servicegroup->next) {

      if (xodtemplate_compare_strings1(
            temp_servicegroup_orig->servicegroup_name,
            temp_servicegroup->servicegroup_name) <= 0)
        break;
      else
        last_servicegroup = temp_servicegroup;
    }

    /* first item added to new sorted list */
    if (new_servicegroup_list == NULL) {
      temp_servicegroup_orig->next = NULL;
      new_servicegroup_list = temp_servicegroup_orig;
    }

    /* item goes at head of new sorted list */
    else if (temp_servicegroup == new_servicegroup_list) {
      temp_servicegroup_orig->next = new_servicegroup_list;
      new_servicegroup_list = temp_servicegroup_orig;
    }

    /* item goes in middle or at end of new sorted list */
    else {
      temp_servicegroup_orig->next = temp_servicegroup;
      last_servicegroup->next = temp_servicegroup_orig;
    }
  }

  /* list is now sorted */
  xodtemplate_servicegroup_list = new_servicegroup_list;

  return (OK);
}

int xodtemplate_compare_host(void* arg1, void* arg2) {
  xodtemplate_host* h1 = NULL;
  xodtemplate_host* h2 = NULL;
  int x = 0;

  h1 = (xodtemplate_host*)arg1;
  h2 = (xodtemplate_host*)arg2;

  if (h1 == NULL && h2 == NULL)
    return (0);
  if (h1 == NULL)
    return (1);
  if (h2 == NULL)
    return (-1);

  x = strcmp(
        (h1->host_name == NULL) ? "" : h1->host_name,
        (h2->host_name == NULL) ? "" : h2->host_name);

  return (x);
}

/* sort hosts by name */
int xodtemplate_sort_hosts() {
#ifdef NEWSTUFF
  xodtemplate_host* temp_host = NULL;

  /* initialize a new skip list */
  if ((xodtemplate_host_skiplist = skiplist_new(
                                     15,
                                     0.5,
                                     false,
                                     xodtemplate_compare_host)) == NULL)
    return (ERROR);

  /* add all hosts to skip list */
  for (temp_host = xodtemplate_host_list;
       temp_host != NULL;
       temp_host = temp_host->next)
    skiplist_insert(xodtemplate_host_skiplist, temp_host);
  /*printf("SKIPLIST ITEMS: %lu\n",xodtemplate_host_skiplist->items); */

  /* now move items from skiplist to linked list... */
  /* TODO */
#endif

  xodtemplate_host* new_host_list = NULL;
  xodtemplate_host* temp_host = NULL;
  xodtemplate_host* last_host = NULL;
  xodtemplate_host* temp_host_orig = NULL;
  xodtemplate_host* next_host_orig = NULL;

  /* sort all existing hosts */
  for (temp_host_orig = xodtemplate_host_list;
       temp_host_orig != NULL;
       temp_host_orig = next_host_orig) {

    next_host_orig = temp_host_orig->next;

    /* add host to new list, sorted by host name */
    last_host = new_host_list;
    for (temp_host = new_host_list;
	 temp_host != NULL;
         temp_host = temp_host->next) {

      if (xodtemplate_compare_strings1(
            temp_host_orig->host_name,
            temp_host->host_name) <= 0)
        break;
      else
        last_host = temp_host;
    }

    /* first item added to new sorted list */
    if (new_host_list == NULL) {
      temp_host_orig->next = NULL;
      new_host_list = temp_host_orig;
    }

    /* item goes at head of new sorted list */
    else if (temp_host == new_host_list) {
      temp_host_orig->next = new_host_list;
      new_host_list = temp_host_orig;
    }

    /* item goes in middle or at end of new sorted list */
    else {
      temp_host_orig->next = temp_host;
      last_host->next = temp_host_orig;
    }
  }

  /* list is now sorted */
  xodtemplate_host_list = new_host_list;

  return (OK);
}

int xodtemplate_compare_service(void* arg1, void* arg2) {
  xodtemplate_service* s1 = NULL;
  xodtemplate_service* s2 = NULL;
  int x = 0;

  s1 = (xodtemplate_service*)arg1;
  s2 = (xodtemplate_service*)arg2;

  if (s1 == NULL && s2 == NULL)
    return (0);
  if (s1 == NULL)
    return (1);
  if (s2 == NULL)
    return (-1);

  x = strcmp(
        (s1->host_name == NULL) ? "" : s1->host_name,
        (s2->host_name == NULL) ? "" : s2->host_name);
  if (x == 0)
    x = strcmp(
          (s1->service_description == NULL) ? "" : s1->service_description,
          (s2->service_description == NULL) ? "" : s2->service_description);

  return (x);
}

/* sort services by name */
int xodtemplate_sort_services() {
#ifdef NEWSTUFF
  xodtemplate_service* temp_service = NULL;

  /* initialize a new skip list */
  if ((xodtemplate_service_skiplist
       = skiplist_new(
           15,
           0.5,
           false,
           xodtemplate_compare_service)) == NULL)
    return (ERROR);

  /* add all services to skip list */
  for (temp_service = xodtemplate_service_list;
       temp_service != NULL;
       temp_service = temp_service->next)
    skiplist_insert(xodtemplate_service_skiplist, temp_service);
  /*printf("SKIPLIST ITEMS: %lu\n",xodtemplate_service_skiplist->items); */

  /* now move items to linked list... */
  /* TODO */
#endif

  xodtemplate_service* new_service_list = NULL;
  xodtemplate_service* temp_service = NULL;
  xodtemplate_service* last_service = NULL;
  xodtemplate_service* temp_service_orig = NULL;
  xodtemplate_service* next_service_orig = NULL;

  /* sort all existing services */
  for (temp_service_orig = xodtemplate_service_list;
       temp_service_orig != NULL;
       temp_service_orig = next_service_orig) {

    next_service_orig = temp_service_orig->next;

    /* add service to new list, sorted by host name then service description */
    last_service = new_service_list;
    for (temp_service = new_service_list;
	 temp_service != NULL;
         temp_service = temp_service->next) {

      if (xodtemplate_compare_strings2(
            temp_service_orig->host_name,
            temp_service_orig->service_description,
            temp_service->host_name,
            temp_service->service_description) <= 0)
        break;
      else
        last_service = temp_service;
    }

    /* first item added to new sorted list */
    if (new_service_list == NULL) {
      temp_service_orig->next = NULL;
      new_service_list = temp_service_orig;
    }

    /* item goes at head of new sorted list */
    else if (temp_service == new_service_list) {
      temp_service_orig->next = new_service_list;
      new_service_list = temp_service_orig;
    }

    /* item goes in middle or at end of new sorted list */
    else {
      temp_service_orig->next = temp_service;
      last_service->next = temp_service_orig;
    }
  }

  /* list is now sorted */
  xodtemplate_service_list = new_service_list;

  return (OK);
}

/* sort servicedependencies by name */
int xodtemplate_sort_servicedependencies() {
  xodtemplate_servicedependency* new_servicedependency_list = NULL;
  xodtemplate_servicedependency* temp_servicedependency = NULL;
  xodtemplate_servicedependency* last_servicedependency = NULL;
  xodtemplate_servicedependency* temp_servicedependency_orig = NULL;
  xodtemplate_servicedependency* next_servicedependency_orig = NULL;

  /* sort all existing servicedependencies */
  for (temp_servicedependency_orig = xodtemplate_servicedependency_list;
       temp_servicedependency_orig != NULL;
       temp_servicedependency_orig = next_servicedependency_orig) {

    next_servicedependency_orig = temp_servicedependency_orig->next;

    /* add servicedependency to new list, sorted by host name then service description */
    last_servicedependency = new_servicedependency_list;
    for (temp_servicedependency = new_servicedependency_list;
         temp_servicedependency != NULL;
         temp_servicedependency = temp_servicedependency->next) {

      if (xodtemplate_compare_strings2(
            temp_servicedependency_orig->host_name,
            temp_servicedependency_orig->service_description,
            temp_servicedependency->host_name,
            temp_servicedependency->service_description) <= 0)
        break;
      else
        last_servicedependency = temp_servicedependency;
    }

    /* first item added to new sorted list */
    if (new_servicedependency_list == NULL) {
      temp_servicedependency_orig->next = NULL;
      new_servicedependency_list = temp_servicedependency_orig;
    }

    /* item goes at head of new sorted list */
    else if (temp_servicedependency == new_servicedependency_list) {
      temp_servicedependency_orig->next = new_servicedependency_list;
      new_servicedependency_list = temp_servicedependency_orig;
    }

    /* item goes in middle or at end of new sorted list */
    else {
      temp_servicedependency_orig->next = temp_servicedependency;
      last_servicedependency->next = temp_servicedependency_orig;
    }
  }

  /* list is now sorted */
  xodtemplate_servicedependency_list = new_servicedependency_list;

  return (OK);
}

/* sort hostdependencies by name */
int xodtemplate_sort_hostdependencies() {
  xodtemplate_hostdependency* new_hostdependency_list = NULL;
  xodtemplate_hostdependency* temp_hostdependency = NULL;
  xodtemplate_hostdependency* last_hostdependency = NULL;
  xodtemplate_hostdependency* temp_hostdependency_orig = NULL;
  xodtemplate_hostdependency* next_hostdependency_orig = NULL;

  /* sort all existing hostdependencys */
  for (temp_hostdependency_orig = xodtemplate_hostdependency_list;
       temp_hostdependency_orig != NULL;
       temp_hostdependency_orig = next_hostdependency_orig) {

    next_hostdependency_orig = temp_hostdependency_orig->next;

    /* add hostdependency to new list, sorted by host name then hostdependency description */
    last_hostdependency = new_hostdependency_list;
    for (temp_hostdependency = new_hostdependency_list;
         temp_hostdependency != NULL;
         temp_hostdependency = temp_hostdependency->next) {

      if (xodtemplate_compare_strings1(
            temp_hostdependency_orig->host_name,
            temp_hostdependency->host_name) <= 0)
        break;
      else
        last_hostdependency = temp_hostdependency;
    }

    /* first item added to new sorted list */
    if (new_hostdependency_list == NULL) {
      temp_hostdependency_orig->next = NULL;
      new_hostdependency_list = temp_hostdependency_orig;
    }

    /* item goes at head of new sorted list */
    else if (temp_hostdependency == new_hostdependency_list) {
      temp_hostdependency_orig->next = new_hostdependency_list;
      new_hostdependency_list = temp_hostdependency_orig;
    }

    /* item goes in middle or at end of new sorted list */
    else {
      temp_hostdependency_orig->next = temp_hostdependency;
      last_hostdependency->next = temp_hostdependency_orig;
    }
  }

  /* list is now sorted */
  xodtemplate_hostdependency_list = new_hostdependency_list;

  return (OK);
}

/******************************************************************/
/******************** SKIPLIST FUNCTIONS **************************/
/******************************************************************/

int xodtemplate_init_xobject_skiplists() {
  int x = 0;

  for (x = 0; x < NUM_XOBJECT_SKIPLISTS; x++) {
    xobject_template_skiplists[x] = NULL;
    xobject_skiplists[x] = NULL;
  }

  xobject_template_skiplists[X_HOST_SKIPLIST]
    = skiplist_new(
        16,
        0.5,
        false,
        false,
        xodtemplate_skiplist_compare_host_template);
  xobject_template_skiplists[X_SERVICE_SKIPLIST]
    = skiplist_new(
        16,
        0.5,
        false,
        false,
        xodtemplate_skiplist_compare_service_template);
  xobject_template_skiplists[X_COMMAND_SKIPLIST]
    = skiplist_new(
        10,
        0.5,
        false,
        false,
        xodtemplate_skiplist_compare_command_template);
  xobject_template_skiplists[X_TIMEPERIOD_SKIPLIST]
    = skiplist_new(
        10,
        0.5,
        false,
        false,
        xodtemplate_skiplist_compare_timeperiod_template);
  xobject_template_skiplists[X_HOSTGROUP_SKIPLIST]
    = skiplist_new(
        10,
        0.5,
        false,
        false,
        xodtemplate_skiplist_compare_hostgroup_template);
  xobject_template_skiplists[X_SERVICEGROUP_SKIPLIST]
    = skiplist_new(
        10,
        0.5,
        false,
        false,
        xodtemplate_skiplist_compare_servicegroup_template);
  xobject_template_skiplists[X_HOSTDEPENDENCY_SKIPLIST]
    = skiplist_new(
        16,
        0.5,
        false,
        false,
        xodtemplate_skiplist_compare_hostdependency_template);
  xobject_template_skiplists[X_SERVICEDEPENDENCY_SKIPLIST]
    = skiplist_new(
        16,
        0.5,
        false,
        false,
        xodtemplate_skiplist_compare_servicedependency_template);
  xobject_skiplists[X_HOST_SKIPLIST]
    = skiplist_new(
        16,
        0.5,
        false,
        false,
        xodtemplate_skiplist_compare_host);
  xobject_skiplists[X_SERVICE_SKIPLIST]
    = skiplist_new(
        16,
        0.5,
        false,
        false,
        xodtemplate_skiplist_compare_service);
  xobject_skiplists[X_COMMAND_SKIPLIST]
    = skiplist_new(
        16,
        0.5,
        false,
        false,
        xodtemplate_skiplist_compare_command);
  xobject_skiplists[X_CONNECTOR_SKIPLIST]
    = skiplist_new(
        16,
        0.5,
        false,
        false,
        xodtemplate_skiplist_compare_connector);
  xobject_skiplists[X_TIMEPERIOD_SKIPLIST]
    = skiplist_new(
        16,
        0.5,
        false,
        false,
        xodtemplate_skiplist_compare_timeperiod);
  xobject_skiplists[X_HOSTGROUP_SKIPLIST]
    = skiplist_new(
        10,
        0.5,
        false,
        false,
        xodtemplate_skiplist_compare_hostgroup);
  xobject_skiplists[X_SERVICEGROUP_SKIPLIST]
    = skiplist_new(
        10,
        0.5,
        false,
        false,
        xodtemplate_skiplist_compare_servicegroup);
  /* allow dups in the following lists... */
  xobject_skiplists[X_HOSTDEPENDENCY_SKIPLIST]
    = skiplist_new(
        16,
        0.5,
        true,
        false,
        xodtemplate_skiplist_compare_hostdependency);
  xobject_skiplists[X_SERVICEDEPENDENCY_SKIPLIST]
    = skiplist_new(
        16,
        0.5,
        true,
        false,
        xodtemplate_skiplist_compare_servicedependency);

  return (OK);
}

int xodtemplate_free_xobject_skiplists() {
  int x = 0;

  for (x = 0; x < NUM_XOBJECT_SKIPLISTS; x++) {
    skiplist_free(&xobject_template_skiplists[x]);
    skiplist_free(&xobject_skiplists[x]);
  }

  return (OK);
}

int xodtemplate_skiplist_compare_text(
      const char* val1a,
      const char* val1b,
      const char* val2a,
      const char* val2b) {
  int result = 0;

  /* check first name */
  if (val1a == NULL && val2a == NULL)
    result = 0;
  else if (val1a == NULL)
    result = 1;
  else if (val2a == NULL)
    result = -1;
  else
    result = strcmp(val1a, val2a);

  /* check second name if necessary */
  if (result == 0) {
    if (val1b == NULL && val2b == NULL)
      result = 0;
    else if (val1b == NULL)
      result = 1;
    else if (val2b == NULL)
      result = -1;
    else
      result = strcmp(val1b, val2b);
  }

  return (result);
}

int xodtemplate_skiplist_compare_host_template(
      void const* a,
      void const* b) {
  xodtemplate_host const* oa = static_cast<xodtemplate_host const*>(a);
  xodtemplate_host const* ob = static_cast<xodtemplate_host const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(oa->name, NULL, ob->name, NULL));
}

int xodtemplate_skiplist_compare_host(void const* a, void const* b) {
  xodtemplate_host const* oa = static_cast<xodtemplate_host const*>(a);
  xodtemplate_host const* ob = static_cast<xodtemplate_host const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(oa->host_name, NULL, ob->host_name, NULL));
}

int xodtemplate_skiplist_compare_service_template(
      void const* a,
      void const* b) {
  xodtemplate_service const* oa = static_cast<xodtemplate_service const*>(a);
  xodtemplate_service const* ob = static_cast<xodtemplate_service const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(oa->name, NULL, ob->name, NULL));
}

int xodtemplate_skiplist_compare_service(void const* a, void const* b) {
  xodtemplate_service const* oa = static_cast<xodtemplate_service const*>(a);
  xodtemplate_service const* ob = static_cast<xodtemplate_service const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(oa->host_name,
				oa->service_description,
				ob->host_name,
				ob->service_description));
}

int xodtemplate_skiplist_compare_timeperiod_template(
      void const* a,
      void const* b) {
  xodtemplate_timeperiod const* oa
    = static_cast<xodtemplate_timeperiod const*>(a);
  xodtemplate_timeperiod const* ob
    = static_cast<xodtemplate_timeperiod const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(oa->name, NULL, ob->name, NULL));
}

int xodtemplate_skiplist_compare_timeperiod(
      void const* a,
      void const* b) {
  xodtemplate_timeperiod const* oa
    = static_cast<xodtemplate_timeperiod const*>(a);
  xodtemplate_timeperiod const* ob
    = static_cast<xodtemplate_timeperiod const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(
            oa->timeperiod_name,
            NULL,
            ob->timeperiod_name,
            NULL));
}

int xodtemplate_skiplist_compare_command_template(
      void const* a,
      void const* b) {
  xodtemplate_command const* oa
    = static_cast<xodtemplate_command const*>(a);
  xodtemplate_command const* ob
    = static_cast<xodtemplate_command const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(oa->name, NULL, ob->name, NULL));
}

int xodtemplate_skiplist_compare_connector_template(
      void const* a,
      void const* b) {
  xodtemplate_connector const* oa
    = static_cast<xodtemplate_connector const*>(a);
  xodtemplate_connector const* ob
    = static_cast<xodtemplate_connector const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(oa->name, NULL, ob->name, NULL));
}

int xodtemplate_skiplist_compare_command(void const* a, void const* b) {
  xodtemplate_command const* oa
    = static_cast<xodtemplate_command const*>(a);
  xodtemplate_command const* ob
    = static_cast<xodtemplate_command const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(
            oa->command_name,
            NULL,
            ob->command_name,
            NULL));
}

int xodtemplate_skiplist_compare_connector(
      void const* a,
      void const* b) {
  xodtemplate_connector const* oa
    = static_cast<xodtemplate_connector const*>(a);
  xodtemplate_connector const* ob
    = static_cast<xodtemplate_connector const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(
            oa->connector_name,
            NULL,
            ob->connector_name,
            NULL));
}

int xodtemplate_skiplist_compare_hostgroup_template(
      void const* a,
      void const* b) {
  xodtemplate_hostgroup const* oa
    = static_cast<xodtemplate_hostgroup const*>(a);
  xodtemplate_hostgroup const* ob
    = static_cast<xodtemplate_hostgroup const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(oa->name, NULL, ob->name, NULL));
}

int xodtemplate_skiplist_compare_hostgroup(
      void const* a,
      void const* b) {
  xodtemplate_hostgroup const* oa
    = static_cast<xodtemplate_hostgroup const*>(a);
  xodtemplate_hostgroup const* ob
    = static_cast<xodtemplate_hostgroup const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(
            oa->hostgroup_name,
            NULL,
            ob->hostgroup_name,
            NULL));
}

int xodtemplate_skiplist_compare_servicegroup_template(
      void const* a,
      void const* b) {
  xodtemplate_servicegroup const* oa
    = static_cast<xodtemplate_servicegroup const*>(a);
  xodtemplate_servicegroup const* ob
    = static_cast<xodtemplate_servicegroup const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(oa->name, NULL, ob->name, NULL));
}

int xodtemplate_skiplist_compare_servicegroup(
      void const* a,
      void const* b) {
  xodtemplate_servicegroup const* oa
    = static_cast<xodtemplate_servicegroup const*>(a);
  xodtemplate_servicegroup const* ob
    = static_cast<xodtemplate_servicegroup const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(
            oa->servicegroup_name,
            NULL,
            ob->servicegroup_name,
            NULL));
}

int xodtemplate_skiplist_compare_hostdependency_template(
      void const* a,
      void const* b) {
  xodtemplate_hostdependency const* oa
    = static_cast<xodtemplate_hostdependency const*>(a);
  xodtemplate_hostdependency const* ob
    = static_cast<xodtemplate_hostdependency const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(oa->name, NULL, ob->name, NULL));
}

int xodtemplate_skiplist_compare_hostdependency(
      void const* a,
      void const* b) {
  xodtemplate_hostdependency const* oa
    = static_cast<xodtemplate_hostdependency const*>(a);
  xodtemplate_hostdependency const* ob
    = static_cast<xodtemplate_hostdependency const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(
            oa->dependent_host_name,
            NULL,
            ob->dependent_host_name,
            NULL));
}

int xodtemplate_skiplist_compare_servicedependency_template(
      void const* a,
      void const* b) {
  xodtemplate_servicedependency const* oa
    = static_cast<xodtemplate_servicedependency const*>(a);
  xodtemplate_servicedependency const* ob
    = static_cast<xodtemplate_servicedependency const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(oa->name, NULL, ob->name, NULL));
}

int xodtemplate_skiplist_compare_servicedependency(
      void const* a,
      void const* b) {
  xodtemplate_servicedependency const* oa
    = static_cast<xodtemplate_servicedependency const*>(a);
  xodtemplate_servicedependency const* ob
    = static_cast<xodtemplate_servicedependency const*>(b);

  if (oa == NULL && ob == NULL)
    return (0);
  if (oa == NULL)
    return (1);
  if (ob == NULL)
    return (-1);

  return (skiplist_compare_text(
            oa->dependent_host_name,
            oa->dependent_service_description,
            ob->dependent_host_name,
            ob->dependent_service_description));
}

/******************************************************************/
/********************** CLEANUP FUNCTIONS *************************/
/******************************************************************/

void xodtemplate_free_timeperiod(
       xodtemplate_timeperiod const* tperiod) {
  delete[] tperiod->tmpl;
  delete[] tperiod->name;
  delete[] tperiod->timeperiod_name;
  delete[] tperiod->alias;
  for (int x = 0; x < 7; x++)
    delete[] tperiod->timeranges[x];
  for (int x = 0; x < DATERANGE_TYPES; x++) {
    xodtemplate_daterange* this_daterange = tperiod->exceptions[x];
    while (this_daterange != NULL) {
      xodtemplate_daterange* next_daterange = this_daterange->next;
      delete[] this_daterange->timeranges;
      delete this_daterange;
      this_daterange = next_daterange;
    }
  }
  delete[] tperiod->exclusions;
  delete tperiod;
  return;
}

/* frees memory */
int xodtemplate_free_memory() {
  xodtemplate_timeperiod* this_timeperiod = NULL;
  xodtemplate_timeperiod* next_timeperiod = NULL;
  xodtemplate_command* this_command = NULL;
  xodtemplate_command* next_command = NULL;
  xodtemplate_connector* this_connector = NULL;
  xodtemplate_connector* next_connector = NULL;
  xodtemplate_hostgroup* this_hostgroup = NULL;
  xodtemplate_hostgroup* next_hostgroup = NULL;
  xodtemplate_servicegroup* this_servicegroup = NULL;
  xodtemplate_servicegroup* next_servicegroup = NULL;
  xodtemplate_servicedependency* this_servicedependency = NULL;
  xodtemplate_servicedependency* next_servicedependency = NULL;
  xodtemplate_host* this_host = NULL;
  xodtemplate_host* next_host = NULL;
  xodtemplate_service* this_service = NULL;
  xodtemplate_service* next_service = NULL;
  xodtemplate_hostdependency* this_hostdependency = NULL;
  xodtemplate_hostdependency* next_hostdependency = NULL;
  xodtemplate_customvariablesmember* this_customvariablesmember = NULL;
  xodtemplate_customvariablesmember* next_customvariablesmember = NULL;
  int x = 0;

  /* free memory allocated to timeperiod list */
  for (this_timeperiod = xodtemplate_timeperiod_list;
       this_timeperiod != NULL;
       this_timeperiod = next_timeperiod) {
    next_timeperiod = this_timeperiod->next;
    xodtemplate_free_timeperiod(this_timeperiod);
  }
  xodtemplate_timeperiod_list = NULL;
  xodtemplate_timeperiod_list_tail = NULL;

  /* free memory allocated to command list */
  for (this_command = xodtemplate_command_list;
       this_command != NULL;
       this_command = next_command) {
    next_command = this_command->next;
    delete[] this_command->tmpl;
    delete[] this_command->name;
    delete[] this_command->command_name;
    delete[] this_command->command_line;
    delete[] this_command->connector_name;
    delete this_command;
  }
  xodtemplate_command_list = NULL;
  xodtemplate_command_list_tail = NULL;

  /* free memory allocated to connector list */
  for (this_connector = xodtemplate_connector_list;
       this_connector != NULL;
       this_connector = next_connector) {
    next_connector = this_connector->next;
    delete[] this_connector->tmpl;
    delete[] this_connector->name;
    delete[] this_connector->connector_name;
    delete[] this_connector->connector_line;
    delete this_connector;
  }
  xodtemplate_connector_list = NULL;
  xodtemplate_connector_list_tail = NULL;

  /* free memory allocated to hostgroup list */
  for (this_hostgroup = xodtemplate_hostgroup_list;
       this_hostgroup != NULL;
       this_hostgroup = next_hostgroup) {
    next_hostgroup = this_hostgroup->next;
    delete[] this_hostgroup->tmpl;
    delete[] this_hostgroup->name;
    delete[] this_hostgroup->hostgroup_name;
    delete[] this_hostgroup->alias;
    delete[] this_hostgroup->members;
    delete[] this_hostgroup->hostgroup_members;
    delete this_hostgroup;
  }
  xodtemplate_hostgroup_list = NULL;
  xodtemplate_hostgroup_list_tail = NULL;

  /* free memory allocated to servicegroup list */
  for (this_servicegroup = xodtemplate_servicegroup_list;
       this_servicegroup != NULL;
       this_servicegroup = next_servicegroup) {
    next_servicegroup = this_servicegroup->next;
    delete[] this_servicegroup->tmpl;
    delete[] this_servicegroup->name;
    delete[] this_servicegroup->servicegroup_name;
    delete[] this_servicegroup->alias;
    delete[] this_servicegroup->members;
    delete[] this_servicegroup->servicegroup_members;
    delete this_servicegroup;
  }
  xodtemplate_servicegroup_list = NULL;
  xodtemplate_servicegroup_list_tail = NULL;

  /* free memory allocated to servicedependency list */
  for (this_servicedependency = xodtemplate_servicedependency_list;
       this_servicedependency != NULL;
       this_servicedependency = next_servicedependency) {
    next_servicedependency = this_servicedependency->next;
    delete[] this_servicedependency->tmpl;
    delete[] this_servicedependency->name;
    delete[] this_servicedependency->servicegroup_name;
    delete[] this_servicedependency->hostgroup_name;
    delete[] this_servicedependency->host_name;
    delete[] this_servicedependency->service_description;
    delete[] this_servicedependency->dependent_servicegroup_name;
    delete[] this_servicedependency->dependent_hostgroup_name;
    delete[] this_servicedependency->dependent_host_name;
    delete[] this_servicedependency->dependent_service_description;
    delete[] this_servicedependency->dependency_period;
    delete this_servicedependency;
  }
  xodtemplate_servicedependency_list = NULL;
  xodtemplate_servicedependency_list_tail = NULL;

  /* free memory allocated to host list */
  for (this_host = xodtemplate_host_list;
       this_host != NULL;
       this_host = next_host) {

    /* free custom variables */
    this_customvariablesmember = this_host->custom_variables;
    while (this_customvariablesmember != NULL) {
      next_customvariablesmember = this_customvariablesmember->next;
      delete[] this_customvariablesmember->variable_name;
      delete[] this_customvariablesmember->variable_value;
      delete this_customvariablesmember;
      this_customvariablesmember = next_customvariablesmember;
    }

    next_host = this_host->next;
    delete[] this_host->tmpl;
    delete[] this_host->name;
    delete[] this_host->host_name;
    delete[] this_host->alias;
    delete[] this_host->address;
    delete[] this_host->parents;
    delete[] this_host->host_groups;
    delete[] this_host->check_command;
    delete[] this_host->check_period;
    delete[] this_host->event_handler;
    delete[] this_host->timezone;
    delete this_host;
  }
  xodtemplate_host_list = NULL;
  xodtemplate_host_list_tail = NULL;

  /* free memory allocated to service list */
  for (this_service = xodtemplate_service_list;
       this_service != NULL;
       this_service = next_service) {

    /* free custom variables */
    this_customvariablesmember = this_service->custom_variables;
    while (this_customvariablesmember != NULL) {
      next_customvariablesmember = this_customvariablesmember->next;
      delete[] this_customvariablesmember->variable_name;
      delete[] this_customvariablesmember->variable_value;
      delete this_customvariablesmember;
      this_customvariablesmember = next_customvariablesmember;
    }

    next_service = this_service->next;
    delete[] this_service->tmpl;
    delete[] this_service->name;
    delete[] this_service->hostgroup_name;
    delete[] this_service->host_name;
    delete[] this_service->service_description;
    delete[] this_service->service_groups;
    delete[] this_service->check_command;
    delete[] this_service->check_period;
    delete[] this_service->event_handler;
    delete[] this_service->timezone;
    delete this_service;
  }
  xodtemplate_service_list = NULL;
  xodtemplate_service_list_tail = NULL;

  /* free memory allocated to hostdependency list */
  for (this_hostdependency = xodtemplate_hostdependency_list;
       this_hostdependency != NULL;
       this_hostdependency = next_hostdependency) {
    next_hostdependency = this_hostdependency->next;
    delete[] this_hostdependency->tmpl;
    delete[] this_hostdependency->name;
    delete[] this_hostdependency->hostgroup_name;
    delete[] this_hostdependency->dependent_hostgroup_name;
    delete[] this_hostdependency->host_name;
    delete[] this_hostdependency->dependent_host_name;
    delete[] this_hostdependency->dependency_period;
    delete this_hostdependency;
  }
  xodtemplate_hostdependency_list = NULL;
  xodtemplate_hostdependency_list_tail = NULL;

  /* free memory for the config file names */
  for (x = 0; x < xodtemplate_current_config_file; x++) {
    delete[] xodtemplate_config_files[x];
    xodtemplate_config_files[x] = NULL;
  }
  delete[] xodtemplate_config_files;
  xodtemplate_config_files = NULL;
  xodtemplate_current_config_file = 0;

  /* free skiplists */
  xodtemplate_free_xobject_skiplists();
  delete[] log_file;
  log_file = NULL;
  return (OK);
}

/* adds a member to a list */
int xodtemplate_add_member_to_memberlist(
      xodtemplate_memberlist** list,
      char* name1,
      char* name2) {
  xodtemplate_memberlist* temp_item = NULL;
  xodtemplate_memberlist* new_item = NULL;

  if (list == NULL)
    return (ERROR);
  if (name1 == NULL)
    return (ERROR);

  /* skip this member if its already in the list */
  for (temp_item = *list; temp_item; temp_item = temp_item->next) {
    if (!strcmp(temp_item->name1, name1)) {
      if (temp_item->name2 == NULL) {
        if (name2 == NULL)
          break;
      }
      else if (name2 != NULL && !strcmp(temp_item->name2, name2))
        break;
    }
  }
  if (temp_item)
    return (OK);

  /* allocate memory for a new list item */
  new_item = new xodtemplate_memberlist;

  /* save the member name(s) */
  new_item->name1 = NULL;
  new_item->name2 = NULL;
  if (name1)
    new_item->name1 = string::dup(name1);
  if (name2)
    new_item->name2 = string::dup(name2);

  /* add new item to head of list */
  new_item->next = *list;
  *list = new_item;

  return (OK);
}

/* frees memory allocated to a temporary member list */
int xodtemplate_free_memberlist(xodtemplate_memberlist** temp_list) {
  xodtemplate_memberlist* this_memberlist = NULL;
  xodtemplate_memberlist* next_memberlist = NULL;

  /* free memory allocated to member name list */
  for (this_memberlist = *temp_list;
       this_memberlist != NULL;
       this_memberlist = next_memberlist) {
    next_memberlist = this_memberlist->next;
    delete[] this_memberlist->name1;
    delete[] this_memberlist->name2;
    delete this_memberlist;
  }

  *temp_list = NULL;
  return (OK);
}

/* remove an entry from the member list */
void xodtemplate_remove_memberlist_item(
       xodtemplate_memberlist* item,
       xodtemplate_memberlist** list) {
  xodtemplate_memberlist* temp_item = NULL;

  if (item == NULL || list == NULL)
    return;

  if (*list == NULL)
    return;

  if (*list == item)
    *list = item->next;
  else {
    for (temp_item = *list;
	 temp_item != NULL;
         temp_item = temp_item->next) {
      if (temp_item->next == item) {
        temp_item->next = item->next;
        break;
      }
    }
  }

  delete[] item->name1;
  delete[] item->name2;
  delete item;
  return;
}

/******************************************************************/
/********************** UTILITY FUNCTIONS *************************/
/******************************************************************/

/* expands a comma-delimited list of hostgroups and/or hosts to member host names */
xodtemplate_memberlist* xodtemplate_expand_hostgroups_and_hosts(
                          char* hostgroups,
                          char* hosts,
                          int _config_file,
                          int _start_line) {
  xodtemplate_memberlist* temp_list = NULL;
  xodtemplate_memberlist* reject_list = NULL;
  xodtemplate_memberlist* list_ptr = NULL;
  xodtemplate_memberlist* reject_ptr = NULL;
  int result = OK;

  /* process list of hostgroups... */
  if (hostgroups != NULL) {

    /* expand host */
    result = xodtemplate_expand_hostgroups(
               &temp_list,
               &reject_list,
               hostgroups,
               _config_file,
               _start_line);
    if (result != OK) {
      xodtemplate_free_memberlist(&temp_list);
      xodtemplate_free_memberlist(&reject_list);
      return (NULL);
    }
  }

  /* process host names */
  if (hosts != NULL) {
    /* expand hosts */
    result = xodtemplate_expand_hosts(
               &temp_list,
               &reject_list,
               hosts,
               _config_file,
               _start_line);
    if (result != OK) {
      xodtemplate_free_memberlist(&temp_list);
      xodtemplate_free_memberlist(&reject_list);
      return (NULL);
    }
  }

#ifdef TESTING
  printf("->PRIOR TO CLEANUP\n");
  printf("   REJECT LIST:\n");
  for (list_ptr = reject_list;
       list_ptr != NULL;
       list_ptr = list_ptr->next)
    printf("      '%s'\n", list_ptr->name1);
  printf("   ACCEPT LIST:\n");
  for (list_ptr = temp_list;
       list_ptr != NULL;
       list_ptr = list_ptr->next)
    printf("      '%s'\n", list_ptr->name1);
#endif

  /* remove rejects (if any) from the list (no duplicate entries exist in either list) */
  /* NOTE: rejects from this list also affect hosts generated from processing hostgroup names (see above) */
  for (reject_ptr = reject_list;
       reject_ptr != NULL;
       reject_ptr = reject_ptr->next) {
    for (list_ptr = temp_list;
         list_ptr != NULL;
         list_ptr = list_ptr->next) {
      if (!strcmp(reject_ptr->name1, list_ptr->name1)) {
        xodtemplate_remove_memberlist_item(list_ptr, &temp_list);
        break;
      }
    }
  }
  xodtemplate_free_memberlist(&reject_list);
  reject_list = NULL;
  return (temp_list);
}

/* expands hostgroups */
int xodtemplate_expand_hostgroups(
      xodtemplate_memberlist** list,
      xodtemplate_memberlist** reject_list,
      char* hostgroups,
      int _config_file,
      int _start_line) {
  char* hostgroup_names = NULL;
  char* temp_ptr = NULL;
  xodtemplate_hostgroup* temp_hostgroup = NULL;
  int found_match = true;
  int reject_item = false;

  if (list == NULL || hostgroups == NULL)
    return (ERROR);

  /* allocate memory for hostgroup name list */
  hostgroup_names = string::dup(hostgroups);

  for (temp_ptr = strtok(hostgroup_names, ",");
       temp_ptr != NULL;
       temp_ptr = strtok(NULL, ",")) {
    found_match = false;
    reject_item = false;

    /* strip trailing spaces */
    strip(temp_ptr);

    /* return a list of all hostgroups */
    if (!strcmp(temp_ptr, "*")) {
      found_match = true;

      for (temp_hostgroup = xodtemplate_hostgroup_list;
           temp_hostgroup != NULL;
           temp_hostgroup = temp_hostgroup->next) {

        /* dont' add hostgroups that shouldn't be registered */
        if (temp_hostgroup->register_object == false)
          continue;

        /* add hostgroup to list */
        xodtemplate_add_hostgroup_members_to_memberlist(
          list,
          temp_hostgroup,
          _config_file,
          _start_line);
      }
    }
    /* else this is just a single hostgroup... */
    else {
      /* this hostgroup should be excluded (rejected) */
      if (temp_ptr[0] == '!') {
        reject_item = true;
        temp_ptr++;
      }

      /* find the hostgroup */
      temp_hostgroup = xodtemplate_find_real_hostgroup(temp_ptr);
      if (temp_hostgroup != NULL) {
        found_match = true;

        /* add hostgroup members to proper list */
        xodtemplate_add_hostgroup_members_to_memberlist(
          (reject_item == true ? reject_list : list),
          temp_hostgroup,
          _config_file,
          _start_line);
      }
    }

    if (found_match == false) {
      logger(log_config_error, basic)
        << "Error: Could not find any hostgroup matching '"
        << temp_ptr << "' (config file '"
        << xodtemplate_config_file_name(_config_file)
        << "', starting on line " << _start_line << ")";
      break;
    }
  }

  /* free memory */
  delete[] hostgroup_names;

  if (found_match == false)
    return (ERROR);

  return (OK);
}

/* expands hosts */
int xodtemplate_expand_hosts(
      xodtemplate_memberlist** list,
      xodtemplate_memberlist** reject_list,
      char* hosts,
      int _config_file,
      int _start_line) {
  char* host_names = NULL;
  char* temp_ptr = NULL;
  xodtemplate_host* temp_host = NULL;
  int found_match = true;
  int reject_item = false;

  if (list == NULL || hosts == NULL)
    return (ERROR);

  host_names = string::dup(hosts);

  /* expand each host name */
  for (temp_ptr = strtok(host_names, ",");
       temp_ptr;
       temp_ptr = strtok(NULL, ",")) {
    found_match = false;
    reject_item = false;

    /* strip trailing spaces */
    strip(temp_ptr);

    /* return a list of all hosts */
    if (!strcmp(temp_ptr, "*")) {
      found_match = true;

      for (temp_host = xodtemplate_host_list;
           temp_host != NULL;
           temp_host = temp_host->next) {
        if (temp_host->host_name == NULL)
          continue;

        /* dont' add hosts that shouldn't be registered */
        if (temp_host->register_object == false)
          continue;

        /* add host to list */
        xodtemplate_add_member_to_memberlist(
          list,
          temp_host->host_name,
          NULL);
      }
    }

    /* else this is just a single host... */
    else {

      /* this host should be excluded (rejected) */
      if (temp_ptr[0] == '!') {
        reject_item = true;
        temp_ptr++;
      }

      /* find the host */
      temp_host = xodtemplate_find_real_host(temp_ptr);
      if (temp_host != NULL) {
        found_match = true;

        /* add host to list */
        xodtemplate_add_member_to_memberlist(
          (reject_item == true ? reject_list : list),
          temp_ptr,
          NULL);
      }
    }

    if (found_match == false) {
      logger(log_config_error, basic)
        << "Error: Could not find any host matching '"
        << temp_ptr << "' (config file '"
        << xodtemplate_config_file_name(_config_file)
        << "', starting on line " << _start_line << ")";
      break;
    }
  }

  /* free memory */
  delete[] host_names;

  if (found_match == false)
    return (ERROR);

  return (OK);
}

/* adds members of a hostgroups to the list of expanded (accepted) or rejected hosts */
int xodtemplate_add_hostgroup_members_to_memberlist(
      xodtemplate_memberlist** list,
      xodtemplate_hostgroup* temp_hostgroup,
      int _config_file,
      int _start_line) {
  char* group_members = NULL;
  char* member_name = NULL;
  char* member_ptr = NULL;

  (void)_config_file;
  (void)_start_line;

  if (list == NULL || temp_hostgroup == NULL)
    return (ERROR);

  /* if we have no members, just return. Empty hostgroups are ok */
  if (temp_hostgroup->members == NULL) {
    return (OK);
  }

  /* save a copy of the members */
  group_members = string::dup(temp_hostgroup->members);

  /* process all hosts that belong to the hostgroup */
  /* NOTE: members of the group have already have been expanded by xodtemplate_recombobulate_hostgroups(), so we don't need to do it here */
  member_ptr = group_members;
  for (member_name = my_strsep(&member_ptr, ",");
       member_name != NULL;
       member_name = my_strsep(&member_ptr, ",")) {

    /* strip trailing spaces from member name */
    strip(member_name);

    /* add host to the list */
    xodtemplate_add_member_to_memberlist(list, member_name, NULL);
  }

  delete[] group_members;

  return (OK);
}

/* expands a comma-delimited list of servicegroups and/or service descriptions */
xodtemplate_memberlist* xodtemplate_expand_servicegroups_and_services(
                          char* servicegroups,
                          char* host_name,
                          char* services,
                          int _config_file,
                          int _start_line) {
  xodtemplate_memberlist* temp_list = NULL;
  xodtemplate_memberlist* reject_list = NULL;
  xodtemplate_memberlist* list_ptr = NULL;
  xodtemplate_memberlist* reject_ptr = NULL;
  int result = OK;

  /* process list of servicegroups... */
  if (servicegroups != NULL) {

    /* expand servicegroups */
    result = xodtemplate_expand_servicegroups(
               &temp_list,
               &reject_list,
               servicegroups,
               _config_file,
               _start_line);
    if (result != OK) {
      xodtemplate_free_memberlist(&temp_list);
      xodtemplate_free_memberlist(&reject_list);
      return (NULL);
    }
  }

  /* process service names */
  if (host_name != NULL && services != NULL) {

    /* expand services */
    result = xodtemplate_expand_services(
               &temp_list,
               &reject_list,
               host_name,
               services,
               _config_file,
               _start_line);
    if (result != OK) {
      xodtemplate_free_memberlist(&temp_list);
      xodtemplate_free_memberlist(&reject_list);
      return (NULL);
    }
  }

  /* remove rejects (if any) from the list (no duplicate entries exist in either list) */
  /* NOTE: rejects from this list also affect hosts generated from processing hostgroup names (see above) */
  for (reject_ptr = reject_list;
       reject_ptr != NULL;
       reject_ptr = reject_ptr->next) {
    for (list_ptr = temp_list;
         list_ptr != NULL;
         list_ptr = list_ptr->next) {
      if (!strcmp(reject_ptr->name1, list_ptr->name1)
          && !strcmp(reject_ptr->name2, list_ptr->name2)) {
        xodtemplate_remove_memberlist_item(list_ptr, &temp_list);
        break;
      }
    }
  }
  xodtemplate_free_memberlist(&reject_list);
  reject_list = NULL;

  return (temp_list);
}

/* expands servicegroups */
int xodtemplate_expand_servicegroups(
      xodtemplate_memberlist** list,
      xodtemplate_memberlist** reject_list,
      char* servicegroups,
      int _config_file,
      int _start_line) {
  xodtemplate_servicegroup* temp_servicegroup = NULL;
  char* servicegroup_names = NULL;
  char* temp_ptr = NULL;
  int found_match = true;
  int reject_item = false;

  if (list == NULL)
    return (ERROR);
  if (servicegroups == NULL)
    return (OK);

  /* allocate memory for servicegroup name list */
  servicegroup_names = string::dup(servicegroups);

  /* expand each servicegroup */
  for (temp_ptr = strtok(servicegroup_names, ",");
       temp_ptr != NULL;
       temp_ptr = strtok(NULL, ",")) {

    found_match = false;
    reject_item = false;

    /* strip trailing spaces */
    strip(temp_ptr);

    /* return a list of all servicegroups */
    if (!strcmp(temp_ptr, "*")) {

      found_match = true;

      for (temp_servicegroup = xodtemplate_servicegroup_list;
           temp_servicegroup != NULL;
           temp_servicegroup = temp_servicegroup->next) {

        /* dont' add servicegroups that shouldn't be registered */
        if (temp_servicegroup->register_object == false)
          continue;

        /* add servicegroup to list */
        xodtemplate_add_servicegroup_members_to_memberlist(
          list,
          temp_servicegroup,
          _config_file,
          _start_line);
      }
    }

    /* else this is just a single servicegroup... */
    else {

      /* this servicegroup should be excluded (rejected) */
      if (temp_ptr[0] == '!') {
        reject_item = true;
        temp_ptr++;
      }

      /* find the servicegroup */
      if ((temp_servicegroup = xodtemplate_find_real_servicegroup(temp_ptr)) != NULL) {

        found_match = true;

        /* add servicegroup members to list */
        xodtemplate_add_servicegroup_members_to_memberlist(
          (reject_item == true ? reject_list : list),
          temp_servicegroup,
          _config_file,
          _start_line);
      }
    }

    /* we didn't find a matching servicegroup */
    if (found_match == false) {
      logger(log_config_error, basic)
        << "Error: Could not find any servicegroup matching '"
        << temp_ptr << "' (config file '"
        << xodtemplate_config_file_name(_config_file)
        << "', starting on line " << _start_line << ")";
      break;
    }
  }

  /* free memory */
  delete[] servicegroup_names;

  if (found_match == false)
    return (ERROR);

  return (OK);
}

/* expands services (host name is not expanded) */
int xodtemplate_expand_services(
      xodtemplate_memberlist** list,
      xodtemplate_memberlist** reject_list,
      char* host_name,
      char* services,
      int _config_file,
      int _start_line) {
  char* service_names = NULL;
  char* temp_ptr = NULL;
  xodtemplate_service* temp_service = NULL;
  int found_match = true;
  int reject_item = false;

  if (list == NULL)
    return (ERROR);
  if (host_name == NULL || services == NULL)
    return (OK);

  if ((service_names = string::dup(services)) == NULL)
    return (ERROR);

  /* expand each service description */
  for (temp_ptr = strtok(service_names, ",");
       temp_ptr != NULL;
       temp_ptr = strtok(NULL, ",")) {

    found_match = false;
    reject_item = false;

    /* strip trailing spaces */
    strip(temp_ptr);

    /* return a list of all services on the host */
    if (!strcmp(temp_ptr, "*")) {

      found_match = true;

      for (temp_service = xodtemplate_service_list;
           temp_service != NULL;
           temp_service = temp_service->next) {

        if (temp_service->host_name == NULL
            || temp_service->service_description == NULL)
          continue;

        if (strcmp(temp_service->host_name, host_name))
          continue;

        /* dont' add services that shouldn't be registered */
        if (temp_service->register_object == false)
          continue;

        /* add service to the list */
        xodtemplate_add_member_to_memberlist(
          list,
          host_name,
          temp_service->service_description);
      }
    }

    /* else this is just a single service... */
    else {

      /* this service should be excluded (rejected) */
      if (temp_ptr[0] == '!') {
        reject_item = true;
        temp_ptr++;
      }

      /* find the service */
      if ((temp_service = xodtemplate_find_real_service(
                            host_name,
                            temp_ptr)) != NULL) {

        found_match = true;

        /* add service to the list */
        xodtemplate_add_member_to_memberlist(
          (reject_item == true ? reject_list : list),
          host_name,
          temp_service->service_description);
      }
    }

    /* we didn't find a match */
    if (found_match == false && reject_item == false) {
      logger(log_config_error, basic)
        << "Error: Could not find a service matching host name '"
        << host_name << "' and description '" << temp_ptr
        << "' (config file '"
        << xodtemplate_config_file_name(_config_file)
        << "', starting on line " << _start_line << ")";
      break;
    }
  }

  if (found_match == false && reject_item == false)
    return (ERROR);

  return (OK);
}

/* adds members of a servicegroups to the list of expanded services */
int xodtemplate_add_servicegroup_members_to_memberlist(
      xodtemplate_memberlist** list,
      xodtemplate_servicegroup* temp_servicegroup,
      int _config_file,
      int _start_line) {
  char* group_members = NULL;
  char* member_name = NULL;
  char* host_name = NULL;
  char* member_ptr = NULL;

  (void)_config_file;
  (void)_start_line;

  if (list == NULL || temp_servicegroup == NULL)
    return (ERROR);

  /* if we have no members, just return. Empty servicegroups are ok */
  if (temp_servicegroup->members == NULL) {
    return (OK);
  }

  /* save a copy of the members */
  group_members = string::dup(temp_servicegroup->members);

  /* process all services that belong to the servicegroup */
  /* NOTE: members of the group have already have been expanded by xodtemplate_recombobulate_servicegroups(), so we don't need to do it here */
  member_ptr = group_members;
  for (member_name = my_strsep(&member_ptr, ",");
       member_name != NULL;
       member_name = my_strsep(&member_ptr, ",")) {

    /* strip trailing spaces from member name */
    strip(member_name);

    /* host name */
    if (host_name == NULL) {
      host_name = string::dup(member_name);
    }

    /* service description */
    else {

      /* add service to the list */
      xodtemplate_add_member_to_memberlist(
        list,
        host_name,
        member_name);

      delete[] host_name;
      host_name = NULL;
    }
  }

  delete[] group_members;

  return (OK);
}

/* returns a comma-delimited list of hostgroup names */
char* xodtemplate_process_hostgroup_names(
        char* hostgroups,
        int _config_file,
        int _start_line) {
  xodtemplate_memberlist* temp_list = NULL;
  xodtemplate_memberlist* reject_list = NULL;
  xodtemplate_memberlist* list_ptr = NULL;
  xodtemplate_memberlist* reject_ptr = NULL;
  xodtemplate_memberlist* this_list = NULL;
  char* buf = NULL;
  int result = OK;

  /* process list of hostgroups... */
  if (hostgroups != NULL) {

    /* split group names into two lists */
    result = xodtemplate_get_hostgroup_names(
               &temp_list,
               &reject_list,
               hostgroups,
               _config_file,
               _start_line);
    if (result != OK) {
      xodtemplate_free_memberlist(&temp_list);
      xodtemplate_free_memberlist(&reject_list);
      return (NULL);
    }

    /* remove rejects (if any) from the list (no duplicate entries exist in either list) */
    for (reject_ptr = reject_list;
	 reject_ptr != NULL;
         reject_ptr = reject_ptr->next) {
      for (list_ptr = temp_list;
           list_ptr != NULL;
           list_ptr = list_ptr->next) {
        if (!strcmp(reject_ptr->name1, list_ptr->name1)) {
          xodtemplate_remove_memberlist_item(list_ptr, &temp_list);
          break;
        }
      }
    }

    xodtemplate_free_memberlist(&reject_list);
    reject_list = NULL;
  }

  /* generate the list of group members */
  for (this_list = temp_list;
       this_list != NULL;
       this_list = this_list->next) {
    if (buf == NULL) {
      buf = new char[strlen(this_list->name1) + 1];
      strcpy(buf, this_list->name1);
    }
    else {
      buf = resize_string(
              buf,
              strlen(buf) + strlen(this_list->name1) + 2);
      strcat(buf, ",");
      strcat(buf, this_list->name1);
    }
  }

  xodtemplate_free_memberlist(&temp_list);

  return (buf);
}

/* return a list of hostgroup names */
int xodtemplate_get_hostgroup_names(
      xodtemplate_memberlist** list,
      xodtemplate_memberlist** reject_list,
      char* hostgroups,
      int _config_file,
      int _start_line) {
  char* hostgroup_names = NULL;
  char* temp_ptr = NULL;
  xodtemplate_hostgroup* temp_hostgroup = NULL;
  int found_match = true;
  int reject_item = false;

  if (list == NULL || hostgroups == NULL)
    return (ERROR);

  /* allocate memory for hostgroup name list */
  hostgroup_names = string::dup(hostgroups);

  for (temp_ptr = strtok(hostgroup_names, ",");
       temp_ptr != NULL;
       temp_ptr = strtok(NULL, ",")) {
    found_match = false;
    reject_item = false;

    /* strip trailing spaces */
    strip(temp_ptr);

    /* return a list of all hostgroups */
    if (!strcmp(temp_ptr, "*")) {

      found_match = true;

      for (temp_hostgroup = xodtemplate_hostgroup_list;
           temp_hostgroup != NULL;
           temp_hostgroup = temp_hostgroup->next) {

        /* dont' add hostgroups that shouldn't be registered */
        if (temp_hostgroup->register_object == false)
          continue;

        /* add hostgroup to list */
        xodtemplate_add_member_to_memberlist(
          list,
          temp_hostgroup->hostgroup_name,
          NULL);
      }
    }

    /* else this is just a single hostgroup... */
    else {

      /* this hostgroup should be excluded (rejected) */
      if (temp_ptr[0] == '!') {
        reject_item = true;
        temp_ptr++;
      }

      /* find the hostgroup */
      temp_hostgroup = xodtemplate_find_real_hostgroup(temp_ptr);
      if (temp_hostgroup != NULL) {
        found_match = true;

        /* add hostgroup members to proper list */
        xodtemplate_add_member_to_memberlist(
          (reject_item == true ? reject_list : list),
          temp_hostgroup->hostgroup_name,
          NULL);
      }
    }

    if (found_match == false) {
      logger(log_config_error, basic)
        << "Error: Could not find any hostgroup matching '"
        << temp_ptr << "' (config file '"
        << xodtemplate_config_file_name(_config_file)
        << "', starting on line " << _start_line << ")";
      break;
    }
  }

  /* free memory */
  delete[] hostgroup_names;

  if (found_match == false)
    return (ERROR);

  return (OK);
}

/* returns a comma-delimited list of servicegroup names */
char* xodtemplate_process_servicegroup_names(
        char* servicegroups,
        int _config_file,
        int _start_line) {
  xodtemplate_memberlist* temp_list = NULL;
  xodtemplate_memberlist* reject_list = NULL;
  xodtemplate_memberlist* list_ptr = NULL;
  xodtemplate_memberlist* reject_ptr = NULL;
  xodtemplate_memberlist* this_list = NULL;
  char* buf = NULL;
  int result = OK;

  /* process list of servicegroups... */
  if (servicegroups != NULL) {

    /* split group names into two lists */
    result = xodtemplate_get_servicegroup_names(
               &temp_list,
               &reject_list,
               servicegroups,
               _config_file,
               _start_line);
    if (result != OK) {
      xodtemplate_free_memberlist(&temp_list);
      xodtemplate_free_memberlist(&reject_list);
      return (NULL);
    }

    /* remove rejects (if any) from the list (no duplicate entries exist in either list) */
    for (reject_ptr = reject_list;
         reject_ptr != NULL;
         reject_ptr = reject_ptr->next) {
      for (list_ptr = temp_list;
           list_ptr != NULL;
           list_ptr = list_ptr->next) {
        if (!strcmp(reject_ptr->name1, list_ptr->name1)) {
          xodtemplate_remove_memberlist_item(list_ptr, &temp_list);
          break;
        }
      }
    }

    xodtemplate_free_memberlist(&reject_list);
    reject_list = NULL;
  }

  /* generate the list of group members */
  for (this_list = temp_list;
       this_list != NULL;
       this_list = this_list->next) {
    if (buf == NULL) {
      buf = new char[strlen(this_list->name1) + 1];
      strcpy(buf, this_list->name1);
    }
    else {
      buf = resize_string(
              buf,
              strlen(buf) + strlen(this_list->name1) + 2);
      strcat(buf, ",");
      strcat(buf, this_list->name1);
    }
  }

  xodtemplate_free_memberlist(&temp_list);

  return (buf);
}

/* return a list of servicegroup names */
int xodtemplate_get_servicegroup_names(
      xodtemplate_memberlist** list,
      xodtemplate_memberlist** reject_list,
      char* servicegroups,
      int _config_file,
      int _start_line) {
  char* servicegroup_names = NULL;
  char* temp_ptr = NULL;
  xodtemplate_servicegroup* temp_servicegroup = NULL;
  int found_match = true;
  int reject_item = false;

  if (list == NULL || servicegroups == NULL)
    return (ERROR);

  /* allocate memory for servicegroup name list */
  servicegroup_names = string::dup(servicegroups);

  for (temp_ptr = strtok(servicegroup_names, ",");
       temp_ptr != NULL;
       temp_ptr = strtok(NULL, ",")) {

    found_match = false;
    reject_item = false;

    /* strip trailing spaces */
    strip(temp_ptr);

    /* return a list of all servicegroups */
    if (!strcmp(temp_ptr, "*")) {

      found_match = true;

      for (temp_servicegroup = xodtemplate_servicegroup_list;
           temp_servicegroup != NULL;
           temp_servicegroup = temp_servicegroup->next) {

        /* dont' add servicegroups that shouldn't be registered */
        if (temp_servicegroup->register_object == false)
          continue;

        /* add servicegroup to list */
        xodtemplate_add_member_to_memberlist(
          list,
          temp_servicegroup->servicegroup_name,
          NULL);
      }
    }

    /* else this is just a single servicegroup... */
    else {

      /* this servicegroup should be excluded (rejected) */
      if (temp_ptr[0] == '!') {
        reject_item = true;
        temp_ptr++;
      }

      /* find the servicegroup */
      temp_servicegroup = xodtemplate_find_real_servicegroup(temp_ptr);
      if (temp_servicegroup != NULL) {

        found_match = true;

        /* add servicegroup members to proper list */
        xodtemplate_add_member_to_memberlist(
          (reject_item == true ? reject_list : list),
          temp_servicegroup->servicegroup_name,
          NULL);
      }
    }

    if (found_match == false) {
      logger(log_config_error, basic)
        << "Error: Could not find any servicegroup matching '"
        << temp_ptr << "' (config file '"
        << xodtemplate_config_file_name(_config_file)
        << "', starting on line " << _start_line << ")";
      break;
    }
  }

  /* free memory */
  delete[] servicegroup_names;

  if (found_match == false)
    return (ERROR);

  return (OK);
}

/******************************************************************/
/****************** ADDITIVE INHERITANCE STUFF ********************/
/******************************************************************/

/* determines the value of an inherited string */
int xodtemplate_get_inherited_string(
      int* have_template_value,
      char** template_value,
      int* have_this_value,
      char** this_value) {
  char* buf = NULL;

  /* template has a value we should use */
  if (*have_template_value == true) {

    /* template has a non-NULL value */
    if (*template_value != NULL) {

      /* we have no value... */
      if (*this_value == NULL) {

        /* use the template value only if we need a value - otherwise stay NULL */
        if (*have_this_value == false) {
          /* NOTE: leave leading + sign if present, as it needed during object resolution and will get stripped later */
          *this_value = string::dup(*template_value);
        }
      }

      /* we already have a value... */
      else {
        /* our value should be added to the template value */
        if (*this_value[0] == '+') {
          buf = new char[strlen(*template_value) + strlen(*this_value) + 1];
          strcpy(buf, *template_value);
          strcat(buf, ",");
          strcat(buf, *this_value + 1);
          delete[] *this_value;
          *this_value = buf;
        }

        /* otherwise our value overrides/replaces the template value */
      }
    }

    /* template has a NULL value.... */

    *have_this_value = true;
  }

  return (OK);
}

/* removes leading + sign from various directives */
int xodtemplate_clean_additive_string(char** str) {
  char* buf = NULL;

  /* remove the additive symbol if present */
  if (*str != NULL && *str[0] == '+') {
    buf = string::dup(*str + 1);
    delete[] *str;
    *str = buf;
  }

  return (OK);
}

/* cleans strings which may contain additive inheritance directives */
/* NOTE: this must be done after objects are resolved */
int xodtemplate_clean_additive_strings() {
  xodtemplate_hostgroup* temp_hostgroup = NULL;
  xodtemplate_servicegroup* temp_servicegroup = NULL;
  xodtemplate_servicedependency* temp_servicedependency = NULL;
  xodtemplate_host* temp_host = NULL;
  xodtemplate_service* temp_service = NULL;
  xodtemplate_hostdependency* temp_hostdependency = NULL;

  /* resolve all hostgroup objects */
  for (temp_hostgroup = xodtemplate_hostgroup_list;
       temp_hostgroup != NULL;
       temp_hostgroup = temp_hostgroup->next) {
    xodtemplate_clean_additive_string(&temp_hostgroup->members);
    xodtemplate_clean_additive_string(
      &temp_hostgroup->hostgroup_members);
  }

  /* resolve all servicegroup objects */
  for (temp_servicegroup = xodtemplate_servicegroup_list;
       temp_servicegroup != NULL;
       temp_servicegroup = temp_servicegroup->next) {
    xodtemplate_clean_additive_string(&temp_servicegroup->members);
    xodtemplate_clean_additive_string(
      &temp_servicegroup->servicegroup_members);
  }

  /* resolve all servicedependency objects */
  for (temp_servicedependency = xodtemplate_servicedependency_list;
       temp_servicedependency != NULL;
       temp_servicedependency = temp_servicedependency->next) {
    xodtemplate_clean_additive_string(
      &temp_servicedependency->servicegroup_name);
    xodtemplate_clean_additive_string(
      &temp_servicedependency->hostgroup_name);
    xodtemplate_clean_additive_string(
      &temp_servicedependency->host_name);
    xodtemplate_clean_additive_string(
      &temp_servicedependency->service_description);
    xodtemplate_clean_additive_string(
      &temp_servicedependency->dependent_servicegroup_name);
    xodtemplate_clean_additive_string(
      &temp_servicedependency->dependent_hostgroup_name);
    xodtemplate_clean_additive_string(
      &temp_servicedependency->dependent_host_name);
    xodtemplate_clean_additive_string(
      &temp_servicedependency->dependent_service_description);
  }

  /* clean all host objects */
  for (temp_host = xodtemplate_host_list;
       temp_host != NULL;
       temp_host = temp_host->next) {
    xodtemplate_clean_additive_string(&temp_host->parents);
    xodtemplate_clean_additive_string(&temp_host->host_groups);
  }

  /* clean all service objects */
  for (temp_service = xodtemplate_service_list;
       temp_service != NULL;
       temp_service = temp_service->next) {
    xodtemplate_clean_additive_string(&temp_service->host_name);
    xodtemplate_clean_additive_string(&temp_service->hostgroup_name);
    xodtemplate_clean_additive_string(&temp_service->service_groups);
  }

  /* resolve all hostdependency objects */
  for (temp_hostdependency = xodtemplate_hostdependency_list;
       temp_hostdependency != NULL;
       temp_hostdependency = temp_hostdependency->next) {
    xodtemplate_clean_additive_string(&temp_hostdependency->host_name);
    xodtemplate_clean_additive_string(
      &temp_hostdependency->dependent_host_name);
    xodtemplate_clean_additive_string(
      &temp_hostdependency->hostgroup_name);
    xodtemplate_clean_additive_string(
      &temp_hostdependency->dependent_hostgroup_name);
  }

  return (OK);
}

int read_main_config_file(char const* main_config_file) {
  char *input=NULL;
  char *variable=NULL;
  char *value=NULL;
  char *error_message=NULL;
  char *temp_ptr=NULL;
  mmapfile *thefile=NULL;
  int current_line=0;
  int error=false;
  int command_check_interval_is_seconds=false;
  char *modptr=NULL;
  char *argptr=NULL;
  DIR *tmpdir=NULL;


  /* open the config file for reading */
  if((thefile=mmap_fopen(main_config_file))==NULL){
    logit(NSLOG_CONFIG_ERROR,true,"Error: Cannot open main configuration file '%s' for reading!",main_config_file);
    return ERROR;
  }

  /* save the main config file macro */
  delete[] macro_x[MACRO_MAINCONFIGFILE];
  if((macro_x[MACRO_MAINCONFIGFILE]=(char *)string::dup(main_config_file)))
    strip(macro_x[MACRO_MAINCONFIGFILE]);

  /* process all lines in the config file */
  while(1){

    /* free memory */
    delete[] input;
    input = NULL;
    delete[] variable;
    variable = NULL;
    delete[] value;
    value = NULL;

    /* read the next line */
    if((input=mmap_fgets_multiline(thefile))==NULL)
      break;

    current_line=thefile->current_line;

    strip(input);

    /* skip blank lines and comments */
    if(input[0]=='\x0' || input[0]=='#')
      continue;

    /* get the variable name */
    if((temp_ptr=my_strtok(input,"="))==NULL){
      if (asprintf(&error_message,"NULL variable")) {}
      error=true;
      break;
    }
    if((variable=(char *)string::dup(temp_ptr))==NULL){
      if (asprintf(&error_message,"malloc() error")) {}
      error=true;
      break;
    }

    /* get the value */
    if((temp_ptr=my_strtok(NULL,"\n"))==NULL){
      if (asprintf(&error_message,"NULL value")) {}
      error=true;
      break;
    }
    if((value=(char *)string::dup(temp_ptr))==NULL){
      if (asprintf(&error_message,"malloc() error")) {}
      error=true;
      break;
    }
    strip(variable);
    strip(value);

    /* process the variable/value */

    if(!strcmp(variable,"resource_file")){

      /* save the macro */
      delete[] macro_x[MACRO_RESOURCEFILE];
      macro_x[MACRO_RESOURCEFILE]=(char *)string::dup(value);

      /* process the resource file */
      // read_resource_file(value);
    }

    else if(!strcmp(variable,"log_file")){

      if(strlen(value)>MAX_FILENAME_LENGTH-1){
        if (asprintf(&error_message,"Log file is too long")) {}
        error=true;
        break;
      }

      delete[] log_file;
      log_file=(char *)string::dup(value);

      /* save the macro */
      delete[] macro_x[MACRO_LOGFILE];
      macro_x[MACRO_LOGFILE]=(char *)string::dup(log_file);
    }

    else if(!strcmp(variable,"debug_level"))
      debug_level=atoi(value);

    else if(!strcmp(variable,"debug_verbosity"))
      debug_verbosity=atoi(value);

    else if(!strcmp(variable,"debug_file")){

      if(strlen(value)>MAX_FILENAME_LENGTH-1){
        if (asprintf(&error_message,"Debug log file is too long")) {}
        error=true;
        break;
      }

      delete[] debug_file;
      debug_file=(char *)string::dup(value);
    }

    else if(!strcmp(variable,"max_debug_file_size"))
      max_debug_file_size=strtoul(value,NULL,0);

    else if(!strcmp(variable,"command_file")){

      if(strlen(value)>MAX_FILENAME_LENGTH-1){
        if (asprintf(&error_message,"Command file is too long")) {}
        error=true;
        break;
      }

      delete[] command_file;
      command_file=(char *)string::dup(value);

      /* save the macro */
      delete[] macro_x[MACRO_COMMANDFILE];
      macro_x[MACRO_COMMANDFILE]=(char *)string::dup(value);
    }

    else if(!strcmp(variable,"global_host_event_handler")){
      delete[] global_host_event_handler;
      global_host_event_handler=(char *)string::dup(value);
    }

    else if(!strcmp(variable,"global_service_event_handler")){
      delete[] global_service_event_handler;
      global_service_event_handler=(char *)string::dup(value);
    }

    else if(!strcmp(variable,"ocsp_command")){
      delete[] ocsp_command;
      ocsp_command=(char *)string::dup(value);
    }

    else if(!strcmp(variable,"ochp_command")){
      delete[] ochp_command;
      ochp_command=(char *)string::dup(value);
    }

    else if(!strcmp(variable,"use_syslog")){

      if(strlen(value)!=1||value[0]<'0'||value[0]>'1'){
        if (asprintf(&error_message,"Illegal value for use_syslog")) {}
        error=true;
        break;
      }

      use_syslog=(atoi(value)>0)?true:false;
    }

    else if(!strcmp(variable,"log_service_retries")){

      if(strlen(value)!=1||value[0]<'0'||value[0]>'1'){
        if (asprintf(&error_message,"Illegal value for log_service_retries")) {}
        error=true;
        break;
      }

      log_service_retries=(atoi(value)>0)?true:false;
    }

    else if(!strcmp(variable,"log_host_retries")){

      if(strlen(value)!=1||value[0]<'0'||value[0]>'1'){
        if (asprintf(&error_message,"Illegal value for log_host_retries")) {}
        error=true;
        break;
      }

      log_host_retries=(atoi(value)>0)?true:false;
    }

    else if(!strcmp(variable,"log_event_handlers")){

      if(strlen(value)!=1||value[0]<'0'||value[0]>'1'){
        if (asprintf(&error_message,"Illegal value for log_event_handlers")) {}
        error=true;
        break;
      }

      log_event_handlers=(atoi(value)>0)?true:false;
    }

    else if(!strcmp(variable,"log_external_commands")){

      if(strlen(value)!=1||value[0]<'0'||value[0]>'1'){
        if (asprintf(&error_message,"Illegal value for log_external_commands")) {}
        error=true;
        break;
      }

      log_external_commands=(atoi(value)>0)?true:false;
    }

    else if(!strcmp(variable,"log_passive_checks")){

      if(strlen(value)!=1||value[0]<'0'||value[0]>'1'){
        if (asprintf(&error_message,"Illegal value for log_passive_checks")) {}
        error=true;
        break;
      }

      log_passive_checks=(atoi(value)>0)?true:false;
    }

    else if(!strcmp(variable,"log_initial_states")){

      if(strlen(value)!=1||value[0]<'0'||value[0]>'1'){
        if (asprintf(&error_message,"Illegal value for log_initial_states")) {}
        error=true;
        break;
      }

      log_initial_states=(atoi(value)>0)?true:false;
    }

    else if(!strcmp(variable,"retention_update_interval")){
      int tmp(atoi(value));
      if(tmp<0){
        if (asprintf(&error_message,"Illegal value for retention_update_interval")) {}
        error=true;
        break;
      }
      retention_update_interval=tmp;
    }

    else if(!strcmp(variable,"additional_freshness_latency"))
      additional_freshness_latency=atoi(value);

    else if(!strcmp(variable,"obsess_over_services")){

      if(strlen(value)!=1||value[0]<'0'||value[0]>'1'){
        if (asprintf(&error_message,"Illegal value for obsess_over_services")) {}
        error=true;
        break;
      }

      obsess_over_services=(atoi(value)>0)?true:false;
    }

    else if(!strcmp(variable,"obsess_over_hosts")){

      if(strlen(value)!=1||value[0]<'0'||value[0]>'1'){
        if (asprintf(&error_message,"Illegal value for obsess_over_hosts")) {}
        error=true;
        break;
      }

      obsess_over_hosts=(atoi(value)>0)?true:false;
    }

    else if(!strcmp(variable,"translate_passive_host_checks")){

      if(strlen(value)!=1||value[0]<'0'||value[0]>'1'){
        if (asprintf(&error_message,"Illegal value for translate_passive_host_checks")) {}
        error=true;
        break;
      }

      translate_passive_host_checks=(atoi(value)>0)?true:false;
    }

    else if(!strcmp(variable,"passive_host_checks_are_soft"))
      passive_host_checks_are_soft=(atoi(value)>0)?true:false;

    else if(!strcmp(variable,"service_check_timeout")){

      service_check_timeout=atoi(value);

      if(service_check_timeout<=0){
        if (asprintf(&error_message,"Illegal value for service_check_timeout")) {}
        error=true;
        break;
      }
    }

    else if(!strcmp(variable,"host_check_timeout")){

      host_check_timeout=atoi(value);

      if(host_check_timeout<=0){
        if (asprintf(&error_message,"Illegal value for host_check_timeout")) {}
        error=true;
        break;
      }
    }

    else if(!strcmp(variable,"event_handler_timeout")){

      event_handler_timeout=atoi(value);

      if(event_handler_timeout<=0){
        if (asprintf(&error_message,"Illegal value for event_handler_timeout")) {}
        error=true;
        break;
      }
    }

    else if(!strcmp(variable,"ocsp_timeout")){

      ocsp_timeout=atoi(value);

      if(ocsp_timeout<=0){
        if (asprintf(&error_message,"Illegal value for ocsp_timeout")) {}
        error=true;
        break;
      }
    }

    else if(!strcmp(variable,"ochp_timeout")){

      ochp_timeout=atoi(value);

      if(ochp_timeout<=0){
        if (asprintf(&error_message,"Illegal value for ochp_timeout")) {}
        error=true;
        break;
      }
    }

    else if(!strcmp(variable,"cached_host_check_horizon"))
      cached_host_check_horizon=strtoul(value,NULL,0);

    else if(!strcmp(variable,"enable_predictive_host_dependency_checks"))
      enable_predictive_host_dependency_checks=(atoi(value)>0)?true:false;

    else if(!strcmp(variable,"cached_service_check_horizon"))
      cached_service_check_horizon=strtoul(value,NULL,0);

    else if(!strcmp(variable,"enable_predictive_service_dependency_checks"))
      enable_predictive_service_dependency_checks=(atoi(value)>0)?true:false;

    else if(!strcmp(variable,"soft_state_dependencies")){
      if(strlen(value)!=1||value[0]<'0'||value[0]>'1'){
        if (asprintf(&error_message,"Illegal value for soft_state_dependencies")) {}
        error=true;
        break;
      }

      soft_state_dependencies=(atoi(value)>0)?true:false;
    }

    else if(!strcmp(variable,"enable_event_handlers"))
      enable_event_handlers=(atoi(value)>0)?true:false;

    else if(!strcmp(variable,"service_inter_check_delay_method")){
      if(!strcmp(value,"n"))
        service_inter_check_delay_method=ICD_NONE;
      else if(!strcmp(value,"d"))
        service_inter_check_delay_method=ICD_DUMB;
      else if(!strcmp(value,"s"))
        service_inter_check_delay_method=ICD_SMART;
      else{
        service_inter_check_delay_method=ICD_USER;
        scheduling_info.service_inter_check_delay=strtod(value,NULL);
        if(scheduling_info.service_inter_check_delay<=0.0){
          if (asprintf(&error_message,"Illegal value for service_inter_check_delay_method")) {}
          error=true;
          break;
        }
      }
    }

    else if(!strcmp(variable,"max_service_check_spread")){
      strip(value);
      max_service_check_spread=atoi(value);
      if(max_service_check_spread<1){
        if (asprintf(&error_message,"Illegal value for max_service_check_spread")) {}
        error=true;
        break;
      }
    }

    else if(!strcmp(variable,"host_inter_check_delay_method")){

      if(!strcmp(value,"n"))
        host_inter_check_delay_method=ICD_NONE;
      else if(!strcmp(value,"d"))
        host_inter_check_delay_method=ICD_DUMB;
      else if(!strcmp(value,"s"))
        host_inter_check_delay_method=ICD_SMART;
      else{
        host_inter_check_delay_method=ICD_USER;
        scheduling_info.host_inter_check_delay=strtod(value,NULL);
        if(scheduling_info.host_inter_check_delay<=0.0){
          if (asprintf(&error_message,"Illegal value for host_inter_check_delay_method")) {}
          error=true;
          break;
        }
      }
    }

    else if(!strcmp(variable,"max_host_check_spread")){

      max_host_check_spread=atoi(value);
      if(max_host_check_spread<1){
        if (asprintf(&error_message,"Illegal value for max_host_check_spread")) {}
        error=true;
        break;
      }
    }

    else if(!strcmp(variable,"service_interleave_factor")){
      if(!strcmp(value,"s"))
        service_interleave_factor_method=ILF_SMART;
      else{
        service_interleave_factor_method=ILF_USER;
        scheduling_info.service_interleave_factor=atoi(value);
        if(scheduling_info.service_interleave_factor<1)
          scheduling_info.service_interleave_factor=1;
      }
    }

    else if(!strcmp(variable,"max_concurrent_checks")){
      int tmp(atoi(value));
      if(tmp<0){
        if (asprintf(&error_message,"Illegal value for max_concurrent_checks")) {}
        error=true;
        break;
      }
      max_parallel_service_checks=tmp;
    }

    else if(!strcmp(variable,"check_result_reaper_frequency") || !strcmp(variable,"service_reaper_frequency")){

      check_reaper_interval=atoi(value);
      if(check_reaper_interval<1){
        if (asprintf(&error_message,"Illegal value for check_result_reaper_frequency")) {}
        error=true;
        break;
      }
    }

    else if(!strcmp(variable,"max_check_result_reaper_time")){

      max_check_reaper_time=atoi(value);
      if(max_check_reaper_time<1){
        if (asprintf(&error_message,"Illegal value for max_check_result_reaper_time")) {}
        error=true;
        break;
      }
    }

    else if(!strcmp(variable,"sleep_time")){

      sleep_time=atof(value);
      if(sleep_time<=0.0){
        if (asprintf(&error_message,"Illegal value for sleep_time")) {}
        error=true;
        break;
      }
    }

    else if(!strcmp(variable,"interval_length")){

      interval_length=atoi(value);
      if(interval_length<1){
        if (asprintf(&error_message,"Illegal value for interval_length")) {}
        error=true;
        break;
      }
    }

    else if(!strcmp(variable,"command_check_interval")){

      command_check_interval_is_seconds=(strstr(value,"s"))?true:false;
      command_check_interval=atoi(value);
      if(command_check_interval<-1 || command_check_interval==0){
        if (asprintf(&error_message,"Illegal value for command_check_interval")) {}
        error=true;
        break;
      }
    }

    else if(!strcmp(variable,"check_service_freshness")){

      if(strlen(value)!=1||value[0]<'0'||value[0]>'1'){
        if (asprintf(&error_message,"Illegal value for check_service_freshness")) {}
        error=true;
        break;
      }

      check_service_freshness=(atoi(value)>0)?true:false;
    }

    else if(!strcmp(variable,"check_host_freshness")){

      if(strlen(value)!=1||value[0]<'0'||value[0]>'1'){
        if (asprintf(&error_message,"Illegal value for check_host_freshness")) {}
        error=true;
        break;
      }

      check_host_freshness=(atoi(value)>0)?true:false;
    }

    else if(!strcmp(variable,"service_freshness_check_interval") || !strcmp(variable,"freshness_check_interval")){

      service_freshness_check_interval=atoi(value);
      if(service_freshness_check_interval<=0){
        if (asprintf(&error_message,"Illegal value for service_freshness_check_interval")) {}
        error=true;
        break;
      }
    }

    else if(!strcmp(variable,"host_freshness_check_interval")){

      host_freshness_check_interval=atoi(value);
      if(host_freshness_check_interval<=0){
        if (asprintf(&error_message,"Illegal value for host_freshness_check_interval")) {}
        error=true;
        break;
      }
    }
    else if(!strcmp(variable,"auto_reschedule_checks")){

      if(strlen(value)!=1||value[0]<'0'||value[0]>'1'){
        if (asprintf(&error_message,"Illegal value for auto_reschedule_checks")) {}
        error=true;
        break;
      }

      auto_reschedule_checks=(atoi(value)>0)?true:false;
    }

    else if(!strcmp(variable,"auto_rescheduling_interval")){

      auto_rescheduling_interval=atoi(value);
      if(auto_rescheduling_interval<=0){
        if (asprintf(&error_message,"Illegal value for auto_rescheduling_interval")) {}
        error=true;
        break;
      }
    }

    else if(!strcmp(variable,"auto_rescheduling_window")){

      auto_rescheduling_window=atoi(value);
      if(auto_rescheduling_window<=0){
        if (asprintf(&error_message,"Illegal value for auto_rescheduling_window")) {}
        error=true;
        break;
      }
    }

    else if(!strcmp(variable,"status_update_interval")){

      status_update_interval=atoi(value);
      if(status_update_interval<=1){
        if (asprintf(&error_message,"Illegal value for status_update_interval")) {}
        error=true;
        break;
      }
    }

    else if(!strcmp(variable,"time_change_threshold")){

      time_change_threshold=atoi(value);

      if(time_change_threshold<=5){
        if (asprintf(&error_message,"Illegal value for time_change_threshold")) {}
        error=true;
        break;
      }
    }

    else if(!strcmp(variable,"enable_flap_detection"))
      enable_flap_detection=(atoi(value)>0)?true:false;

    else if(!strcmp(variable,"low_service_flap_threshold")){

      low_service_flap_threshold=strtod(value,NULL);
      if(low_service_flap_threshold<=0.0 || low_service_flap_threshold>=100.0){
        if (asprintf(&error_message,"Illegal value for low_service_flap_threshold")) {}
        error=true;
        break;
      }
    }

    else if(!strcmp(variable,"high_service_flap_threshold")){

      high_service_flap_threshold=strtod(value,NULL);
      if(high_service_flap_threshold<=0.0 ||  high_service_flap_threshold>100.0){
        if (asprintf(&error_message,"Illegal value for high_service_flap_threshold")) {}
        error=true;
        break;
      }
    }

    else if(!strcmp(variable,"low_host_flap_threshold")){

      low_host_flap_threshold=strtod(value,NULL);
      if(low_host_flap_threshold<=0.0 || low_host_flap_threshold>=100.0){
        if (asprintf(&error_message,"Illegal value for low_host_flap_threshold")) {}
        error=true;
        break;
      }
    }

    else if(!strcmp(variable,"high_host_flap_threshold")){

      high_host_flap_threshold=strtod(value,NULL);
      if(high_host_flap_threshold<=0.0 || high_host_flap_threshold>100.0){
        if (asprintf(&error_message,"Illegal value for high_host_flap_threshold")) {}
        error=true;
        break;
      }
    }

    else if(!strcmp(variable,"date_format")){

      if(!strcmp(value,"euro"))
        date_format=DATE_FORMAT_EURO;
      else if(!strcmp(value,"iso8601"))
        date_format=DATE_FORMAT_ISO8601;
      else if(!strcmp(value,"strict-iso8601"))
        date_format=DATE_FORMAT_STRICT_ISO8601;
      else
        date_format=DATE_FORMAT_US;
    }

    else if(!strcmp(variable,"use_timezone")){
      delete[] use_timezone;
      use_timezone=(char *)string::dup(value);
    }

    else if(!strcmp(variable,"event_broker_options")){

      if(!strcmp(value,"-1"))
        event_broker_options=BROKER_EVERYTHING;
      else
        event_broker_options=strtoul(value,NULL,0);
    }

    else if(!strcmp(variable,"illegal_object_name_chars"))
      illegal_object_chars=(char *)string::dup(value);

    else if(!strcmp(variable,"illegal_macro_output_chars"))
      illegal_output_chars=(char *)string::dup(value);


    else if(!strcmp(variable,"broker_module")){
      modptr=strtok(value," \n");
      argptr=strtok(NULL,"\n");
#ifdef USE_EVENT_BROKER
      neb_add_module(modptr,argptr,true);
#endif
    }

    else if(!strcmp(variable,"external_command_buffer_slots"))
      external_command_buffer_slots=atoi(value);

    /* skip external data directives */
    else if(strstr(input,"x")==input)
      continue;

    /* ignore external variables */
    else if(!strcmp(variable,"status_file"))
      continue;
    else if(strstr(input,"cfg_file=")==input || strstr(input,"cfg_dir=")==input)
      continue;
    else if(strstr(input,"state_retention_file=")==input)
      continue;

    /* we don't know what this variable is... */
    else{
      if (asprintf(&error_message,"UNKNOWN VARIABLE")) {}
      error=true;
      break;
    }

  }

  /* adjust timezone values */
  if(use_timezone!=NULL)
    set_environment_var("TZ",use_timezone,1);
  tzset();

  /* adjust command check interval */
  if(command_check_interval_is_seconds==false && command_check_interval!=-1)
    command_check_interval*=interval_length;

  /* handle errors */
  if(error==true){
    logit(NSLOG_CONFIG_ERROR,true,"Error in configuration file '%s' - Line %d (%s)",main_config_file,current_line,(error_message==NULL)?"NULL":error_message);
    return ERROR;
  }

  /* free leftover memory and close the file */
  delete[] input;
  mmap_fclose(thefile);

  /* free memory */
  delete[] error_message;
  error_message = NULL;
  delete[] input;
  input = NULL;
  delete[] variable;
  variable = NULL;
  delete[] value;
  value = NULL;

  /* make sure a log file has been specified */
  strip(log_file);
  // if(!strcmp(log_file,"")){
  //   if(daemon_mode==false)
  //     printf("Error: Log file is not specified anywhere in main config file '%s'!\n",main_config_file);
  //   return ERROR;
  // }

  return OK;
}
