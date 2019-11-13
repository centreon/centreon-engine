/*
** Copyright 2001-2009 Ethan Galstad
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

#include "xodtemplate.hh"
#include <dirent.h>
#include <libgen.h>
#include <regex.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
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
#include "nagios.h"
#include "skiplist.h"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

static xodtemplate_timeperiod* xodtemplate_timeperiod_list = NULL;
static xodtemplate_command* xodtemplate_command_list = NULL;
static xodtemplate_connector* xodtemplate_connector_list = NULL;
static xodtemplate_contactgroup* xodtemplate_contactgroup_list = NULL;
static xodtemplate_hostgroup* xodtemplate_hostgroup_list = NULL;
static xodtemplate_servicegroup* xodtemplate_servicegroup_list = NULL;
static xodtemplate_servicedependency* xodtemplate_servicedependency_list = NULL;
static xodtemplate_serviceescalation* xodtemplate_serviceescalation_list = NULL;
static xodtemplate_contact* xodtemplate_contact_list = NULL;
static xodtemplate_host* xodtemplate_host_list = NULL;
static xodtemplate_service* xodtemplate_service_list = NULL;
static xodtemplate_hostdependency* xodtemplate_hostdependency_list = NULL;
static xodtemplate_hostescalation* xodtemplate_hostescalation_list = NULL;
static xodtemplate_hostextinfo* xodtemplate_hostextinfo_list = NULL;
static xodtemplate_serviceextinfo* xodtemplate_serviceextinfo_list = NULL;

static xodtemplate_timeperiod* xodtemplate_timeperiod_list_tail = NULL;
static xodtemplate_command* xodtemplate_command_list_tail = NULL;
static xodtemplate_connector* xodtemplate_connector_list_tail = NULL;
static xodtemplate_contactgroup* xodtemplate_contactgroup_list_tail = NULL;
static xodtemplate_hostgroup* xodtemplate_hostgroup_list_tail = NULL;
static xodtemplate_servicegroup* xodtemplate_servicegroup_list_tail = NULL;
static xodtemplate_servicedependency* xodtemplate_servicedependency_list_tail =
    NULL;
static xodtemplate_serviceescalation* xodtemplate_serviceescalation_list_tail =
    NULL;
static xodtemplate_contact* xodtemplate_contact_list_tail = NULL;
static xodtemplate_host* xodtemplate_host_list_tail = NULL;
static xodtemplate_service* xodtemplate_service_list_tail = NULL;
static xodtemplate_hostdependency* xodtemplate_hostdependency_list_tail = NULL;
static xodtemplate_hostescalation* xodtemplate_hostescalation_list_tail = NULL;
static xodtemplate_hostextinfo* xodtemplate_hostextinfo_list_tail = NULL;
static xodtemplate_serviceextinfo* xodtemplate_serviceextinfo_list_tail = NULL;

static skiplist* xobject_template_skiplists[NUM_XOBJECT_SKIPLISTS];
static skiplist* xobject_skiplists[NUM_XOBJECT_SKIPLISTS];

static void* xodtemplate_current_object = NULL;
static int xodtemplate_current_object_type = XODTEMPLATE_NONE;

static int xodtemplate_current_config_file = 0;
static char** xodtemplate_config_files = NULL;

static char* xodtemplate_cache_file = NULL;
static char* xodtemplate_precache_file = NULL;

static int presorted_objects = false;

/*
 * Macro magic used to determine if a service is assigned
 * via hostgroup_name or host_name. Those assigned via host_name
 * take precedence.
 */
#define X_SERVICE_IS_FROM_HOSTGROUP                                       \
  (1 << 1) /* flag to know if service come from a hostgroup def, apply on \
              srv->have_initial_state */
#define xodtemplate_set_service_is_from_hostgroup(srv) \
  srv->have_initial_state |= X_SERVICE_IS_FROM_HOSTGROUP
#define xodtemplate_unset_service_is_from_hostgroup(srv) \
  srv->have_initial_state &= ~X_SERVICE_IS_FROM_HOSTGROUP
#define xodtemplate_is_service_is_from_hostgroup(srv) \
  ((srv->have_initial_state & X_SERVICE_IS_FROM_HOSTGROUP) != 0)

/* returns the name of a numbered config file */
static char const* xodtemplate_config_file_name(int config_file) {
  if (config_file <= xodtemplate_current_config_file)
    return xodtemplate_config_files[config_file - 1];
  return "?";
}

/******************************************************************/
/************* TOP-LEVEL CONFIG DATA INPUT FUNCTION ***************/
/******************************************************************/

/* process all config files - both core and CGIs pass in name of main config
 * file */
int xodtemplate_read_config_data(char const* main_config_file,
                                 int options,
                                 int cache,
                                 int precache) {
  struct timeval tv[14];
  int result = OK;

  if (main_config_file == NULL) {
    printf("Error: No main config file passed to object routines!\n");
    return ERROR;
  }

  /* get variables from main config file */
  xodtemplate_grab_config_info(main_config_file);

  /* initialize variables */
  xodtemplate_timeperiod_list = NULL;
  xodtemplate_command_list = NULL;
  xodtemplate_contactgroup_list = NULL;
  xodtemplate_hostgroup_list = NULL;
  xodtemplate_servicegroup_list = NULL;
  xodtemplate_servicedependency_list = NULL;
  xodtemplate_serviceescalation_list = NULL;
  xodtemplate_contact_list = NULL;
  xodtemplate_host_list = NULL;
  xodtemplate_service_list = NULL;
  xodtemplate_hostdependency_list = NULL;
  xodtemplate_hostescalation_list = NULL;
  xodtemplate_hostextinfo_list = NULL;
  xodtemplate_serviceextinfo_list = NULL;

  /* initialize skiplists */
  xodtemplate_init_xobject_skiplists();

  xodtemplate_current_object = NULL;
  xodtemplate_current_object_type = XODTEMPLATE_NONE;

  /* allocate memory for 256 config files (increased dynamically) */
  xodtemplate_current_config_file = 0;
  xodtemplate_config_files = new char*[256];

  /* are the objects we're reading already pre-sorted? */
  presorted_objects = false;
  presorted_objects = (use_precached_objects == true) ? true : false;
  if (test_scheduling == true)
    gettimeofday(&tv[0], NULL);

  /* only process the precached object file as long as we're not regenerating it
   * and we're not verifying the config */
  if (use_precached_objects == true)
    result =
        xodtemplate_process_config_file(xodtemplate_precache_file, options);
  /* process object config files normally... */
  else {
    /* determine the directory of the main config file */
    char* config_file(string::dup(main_config_file));
    char* config_base_dir(string::dup(dirname(config_file)));
    delete[] config_file;

    /* open the main config file for reading (we need to find all the config
     * files to read) */
    mmapfile* thefile(mmap_fopen(main_config_file));
    if (!thefile) {
      delete[] config_base_dir;
      delete[] xodtemplate_config_files;
      xodtemplate_config_files = NULL;
      printf("Unable to open main config file '%s'\n", main_config_file);
      return ERROR;
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
      if (!strcmp(var, "xodtemplate_config_file") || !strcmp(var, "cfg_file")) {
        char* temp_buffer(string::dup(val));
        if (config_base_dir != NULL && val[0] != '/') {
          std::ostringstream oss;
          oss << config_base_dir << '/' << temp_buffer;
          config_file = string::dup(oss.str());
          delete[] temp_buffer;
        } else
          config_file = temp_buffer;

        /* process the config file... */
        result = xodtemplate_process_config_file(config_file, options);

        delete[] config_file;

        /* if there was an error processing the config file, break out of loop
         */
        if (result == ERROR)
          break;
      }
      /* process all files in a config directory */
      else if (!strcmp(var, "xodtemplate_config_dir") ||
               !strcmp(var, "cfg_dir")) {
        char* temp_buffer(string::dup(val));
        if (config_base_dir != NULL && val[0] != '/') {
          std::ostringstream oss;
          oss << config_base_dir << '/' << temp_buffer;
          config_file = string::dup(oss.str());
          delete[] temp_buffer;
        } else
          config_file = temp_buffer;

        /* strip trailing / if necessary */
        if (config_file != NULL && config_file[strlen(config_file) - 1] == '/')
          config_file[strlen(config_file) - 1] = '\x0';

        /* process the config directory... */
        result = xodtemplate_process_config_dir(config_file, options);

        delete[] config_file;

        /* if there was an error processing the config file, break out of loop
         */
        if (result == ERROR)
          break;
      }
    }

    /* free memory and close the file */
    delete[] config_base_dir;
    delete[] input;
    mmap_fclose(thefile);
  }

  if (test_scheduling == true)
    gettimeofday(&tv[1], NULL);

  /* only perform intensive operations if we're not using the precached object
   * file */
  if (use_precached_objects == false) {
    /* resolve objects definitions */
    if (result == OK)
      result = xodtemplate_resolve_objects();
    if (test_scheduling == true)
      gettimeofday(&tv[2], NULL);

    /* cleanup some additive inheritance stuff... */
    xodtemplate_clean_additive_strings();

    /* do the meat and potatoes stuff... */
    if (result == OK)
      result = xodtemplate_recombobulate_contactgroups();
    if (test_scheduling == true)
      gettimeofday(&tv[3], NULL);

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

    /* NOTE: some missing defaults (notification options, etc.) are also applied
     * here */
    if (result == OK)
      result = xodtemplate_inherit_object_properties();
    if (test_scheduling == true)
      gettimeofday(&tv[8], NULL);

    if (result == OK)
      result = xodtemplate_recombobulate_object_contacts();
    if (test_scheduling == true)
      gettimeofday(&tv[9], NULL);

    /* sort objects */
    if (result == OK)
      result = xodtemplate_sort_objects();
    if (test_scheduling == true)
      gettimeofday(&tv[10], NULL);
  }

  if (result == OK) {
    /* merge host/service extinfo definitions with host/service definitions */
    /* this will be removed in next major release */
    xodtemplate_merge_extinfo_ojects();

    /* cache object definitions for the CGIs and external apps */
    if (cache == true)
      xodtemplate_cache_objects(xodtemplate_cache_file);

    /* precache object definitions for future runs */
    if (precache == true)
      xodtemplate_cache_objects(xodtemplate_precache_file);
  }

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

  /* free memory */
  delete[] xodtemplate_cache_file;
  delete[] xodtemplate_precache_file;

  xodtemplate_cache_file = NULL;
  xodtemplate_precache_file = NULL;

  if (test_scheduling == true) {
    double runtime[14];
    runtime[0] =
        (double)((double)(tv[1].tv_sec - tv[0].tv_sec) +
                 (double)((tv[1].tv_usec - tv[0].tv_usec) / 1000.0) / 1000.0);
    if (use_precached_objects == false) {
      runtime[1] =
          (double)((double)(tv[2].tv_sec - tv[1].tv_sec) +
                   (double)((tv[2].tv_usec - tv[1].tv_usec) / 1000.0) / 1000.0);
      runtime[2] =
          (double)((double)(tv[3].tv_sec - tv[2].tv_sec) +
                   (double)((tv[3].tv_usec - tv[2].tv_usec) / 1000.0) / 1000.0);
      runtime[3] =
          (double)((double)(tv[4].tv_sec - tv[3].tv_sec) +
                   (double)((tv[4].tv_usec - tv[3].tv_usec) / 1000.0) / 1000.0);
      runtime[4] =
          (double)((double)(tv[5].tv_sec - tv[4].tv_sec) +
                   (double)((tv[5].tv_usec - tv[4].tv_usec) / 1000.0) / 1000.0);
      runtime[5] =
          (double)((double)(tv[6].tv_sec - tv[5].tv_sec) +
                   (double)((tv[6].tv_usec - tv[5].tv_usec) / 1000.0) / 1000.0);
      runtime[6] =
          (double)((double)(tv[7].tv_sec - tv[6].tv_sec) +
                   (double)((tv[7].tv_usec - tv[6].tv_usec) / 1000.0) / 1000.0);
      runtime[7] =
          (double)((double)(tv[8].tv_sec - tv[7].tv_sec) +
                   (double)((tv[8].tv_usec - tv[7].tv_usec) / 1000.0) / 1000.0);
      runtime[8] =
          (double)((double)(tv[9].tv_sec - tv[8].tv_sec) +
                   (double)((tv[9].tv_usec - tv[8].tv_usec) / 1000.0) / 1000.0);
      runtime[9] =
          (double)((double)(tv[10].tv_sec - tv[9].tv_sec) +
                   (double)((tv[10].tv_usec - tv[9].tv_usec) / 1000.0) /
                       1000.0);
      runtime[10] =
          (double)((double)(tv[11].tv_sec - tv[10].tv_sec) +
                   (double)((tv[11].tv_usec - tv[10].tv_usec) / 1000.0) /
                       1000.0);
      runtime[11] =
          (double)((double)(tv[12].tv_sec - tv[11].tv_sec) +
                   (double)((tv[12].tv_usec - tv[11].tv_usec) / 1000.0) /
                       1000.0);
    } else {
      runtime[1] = 0.0;
      runtime[2] = 0.0;
      runtime[3] = 0.0;
      runtime[4] = 0.0;
      runtime[5] = 0.0;
      runtime[6] = 0.0;
      runtime[7] = 0.0;
      runtime[8] = 0.0;
      runtime[9] = 0.0;
      runtime[10] = 0.0;
      runtime[11] =
          (double)((double)(tv[12].tv_sec - tv[1].tv_sec) +
                   (double)((tv[12].tv_usec - tv[1].tv_usec) / 1000.0) /
                       1000.0);
    }
    runtime[12] =
        (double)((double)(tv[13].tv_sec - tv[12].tv_sec) +
                 (double)((tv[13].tv_usec - tv[12].tv_usec) / 1000.0) / 1000.0);
    runtime[13] =
        (double)((double)(tv[13].tv_sec - tv[0].tv_sec) +
                 (double)((tv[13].tv_usec - tv[0].tv_usec) / 1000.0) / 1000.0);

    printf(
        "Timing information on object configuration processing is listed\n"
        "below.  You can use this information to see if precaching your\n"
        "object configuration would be useful.\n\n"
        "Object Config Source: %s\n\n",
        (use_precached_objects == true ? "Pre-cached config file"
                                       : "Config files (uncached)"));

    printf(
        "OBJECT CONFIG PROCESSING TIMES      (* = "
        "Potential for precache savings with -u option)\n");
    printf("----------------------------------\n");
    printf("Read:                 %.6f sec\n", runtime[0]);
    printf("Resolve:              %.6f sec  *\n", runtime[1]);
    printf("Recomb Contactgroups: %.6f sec  *\n", runtime[2]);
    printf("Recomb Hostgroups:    %.6f sec  *\n", runtime[3]);
    printf("Dup Services:         %.6f sec  *\n", runtime[4]);
    printf("Recomb Servicegroups: %.6f sec  *\n", runtime[5]);
    printf("Duplicate:            %.6f sec  *\n", runtime[6]);
    printf("Inherit:              %.6f sec  *\n", runtime[7]);
    printf("Recomb Contacts:      %.6f sec  *\n", runtime[8]);
    printf("Sort:                 %.6f sec  *\n", runtime[9]);
    /*		printf("Cache:                %.6f sec\n",runtime[10]);*/
    printf("Register:             %.6f sec\n", runtime[11]);
    printf("Free:                 %.6f sec\n", runtime[12]);
    printf("                      ============\n");
    printf("TOTAL:                %.6f sec  ", runtime[13]);
    if (use_precached_objects == false)
      printf("* = %.6f sec (%.2f%%) estimated savings",
             runtime[13] - runtime[12] - runtime[11] - runtime[0],
             ((runtime[13] - runtime[12] - runtime[11] - runtime[0]) /
              runtime[13]) *
                 100.0);
    printf("\n");
    printf("\n\n");
  }

  return result;
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
    return ERROR;

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

    /* cached object file definition (overrides default location) */
    if (!strcmp(var, "object_cache_file"))
      xodtemplate_cache_file = string::dup(val);

    /* pre-cached object file definition */
    if (!strcmp(var, "precached_object_file"))
      xodtemplate_precache_file = string::dup(val);
  }

  /* close the file */
  mmap_fclose(thefile);

  /* default locations */
  if (xodtemplate_cache_file == NULL)
    xodtemplate_cache_file = string::dup(DEFAULT_OBJECT_CACHE_FILE);
  if (xodtemplate_precache_file == NULL)
    xodtemplate_precache_file = string::dup(DEFAULT_PRECACHED_OBJECT_FILE);

  mac = get_global_macros();
  /* save the object cache file macro */
  delete[] mac->x[MACRO_OBJECTCACHEFILE];
  mac->x[MACRO_OBJECTCACHEFILE] = string::dup(xodtemplate_cache_file);
  strip(mac->x[MACRO_OBJECTCACHEFILE]);

  return OK;
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
        << "Error: Could not open config directory '" << dirname
        << "' for reading.";
    return ERROR;
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
      return ERROR;
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
          return ERROR;
        }
        break;

      case S_IFDIR:
        /* recurse into subdirectories... */
        result = xodtemplate_process_config_dir(file, options);

        if (result == ERROR) {
          closedir(dirp);
          return ERROR;
        }
        break;

      default:
        /* everything else we ignore */
        break;
    }
  }

  closedir(dirp);
  return result;
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
  xodtemplate_config_files[xodtemplate_current_config_file++] =
      string::dup(filename);

  /* reallocate memory for config files */
  if (!(xodtemplate_current_config_file % 256)) {
    char** new_tab = new char*[xodtemplate_current_config_file + 256];
    memcpy(new_tab, xodtemplate_config_files,
           sizeof(*new_tab) * xodtemplate_current_config_file);
    delete[] xodtemplate_config_files;
    xodtemplate_config_files = new_tab;
  }

  /* open the config file for reading */
  if ((thefile = mmap_fopen(filename)) == NULL) {
    logger(log_config_error, basic)
        << "Error: Cannot open config file '" << filename
        << "' for reading: " << strerror(errno);
    return ERROR;
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

    /* grab data before comment delimiter - faster than a strtok() and
     * strncpy()... */
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
            << "Error: No object type specified in file '" << filename
            << "' on line " << current_line << ".";
        result = ERROR;
        break;
      }

      /* check validity of object type */
      if (strcmp(input, "timeperiod") && strcmp(input, "command") &&
          strcmp(input, "contact") && strcmp(input, "contactgroup") &&
          strcmp(input, "host") && strcmp(input, "hostgroup") &&
          strcmp(input, "servicegroup") && strcmp(input, "service") &&
          strcmp(input, "servicedependency") &&
          strcmp(input, "serviceescalation") &&
          strcmp(input, "hostgroupescalation") &&
          strcmp(input, "hostdependency") && strcmp(input, "hostescalation") &&
          strcmp(input, "hostextinfo") && strcmp(input, "serviceextinfo") &&
          strcmp(input, "connector")) {
        logger(log_config_error, basic)
            << "Error: Invalid object definition type '" << input
            << "' in file '" << filename << "' on line " << current_line << ".";
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
      if (xodtemplate_begin_object_definition(input, options,
                                              xodtemplate_current_config_file,
                                              current_line) == ERROR) {
        logger(log_config_error, basic)
            << "Error: Could not add object definition in file '" << filename
            << "' on line " << current_line << ".";
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
              << "Error: Could not add object property in file '" << filename
              << "' on line " << current_line << ".";
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
          << "Error: Unexpected token or statement in file '" << filename
          << "' on line " << current_line << ".";
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
        << "Error: Unexpected EOF in file '" << filename << "' on line "
        << current_line << " - check for a missing closing bracket.",
        result = ERROR;
  }

  return result;
}

/******************************************************************/
/***************** OBJECT DEFINITION FUNCTIONS ********************/
/******************************************************************/

/*
 * all objects start the same way, so we can get rid of quite
 * a lot of code with this struct-offset-insensitive macro
 */
#define xod_begin_def(type)                                         \
  do {                                                              \
    new_##type = new xodtemplate_##type;                            \
    memset(new_##type, 0, sizeof(*new_##type));                     \
                                                                    \
    new_##type->register_object = true;                             \
    new_##type->_config_file = config_file;                         \
    new_##type->_start_line = start_line;                           \
                                                                    \
    /* precached object files are already sorted, so add to tail */ \
    if (presorted_objects) {                                        \
      if (xodtemplate_##type##_list == NULL) {                      \
        xodtemplate_##type##_list = new_##type;                     \
        xodtemplate_##type##_list_tail = xodtemplate_##type##_list; \
      } else {                                                      \
        xodtemplate_##type##_list_tail->next = new_##type;          \
        xodtemplate_##type##_list_tail = new_##type;                \
      }                                                             \
                                                                    \
      /* update current object pointer */                           \
      xodtemplate_current_object = xodtemplate_##type##_list_tail;  \
    } else {                                                        \
      /* add new object to head of list in memory */                \
      new_##type->next = xodtemplate_##type##_list;                 \
      xodtemplate_##type##_list = new_##type;                       \
                                                                    \
      /* update current object pointer */                           \
      xodtemplate_current_object = xodtemplate_##type##_list;       \
    }                                                               \
  } while (0)

/* starts a new object definition */
int xodtemplate_begin_object_definition(char* input,
                                        int options,
                                        int config_file,
                                        int start_line) {
  (void)options;

  int result = OK;
  xodtemplate_timeperiod* new_timeperiod = NULL;
  xodtemplate_command* new_command = NULL;
  xodtemplate_connector* new_connector = NULL;
  xodtemplate_contactgroup* new_contactgroup = NULL;
  xodtemplate_hostgroup* new_hostgroup = NULL;
  xodtemplate_servicegroup* new_servicegroup = NULL;
  xodtemplate_servicedependency* new_servicedependency = NULL;
  xodtemplate_serviceescalation* new_serviceescalation = NULL;
  xodtemplate_contact* new_contact = NULL;
  xodtemplate_host* new_host = NULL;
  xodtemplate_service* new_service = NULL;
  xodtemplate_hostdependency* new_hostdependency = NULL;
  xodtemplate_hostescalation* new_hostescalation = NULL;
  xodtemplate_hostextinfo* new_hostextinfo = NULL;
  xodtemplate_serviceextinfo* new_serviceextinfo = NULL;

  if (!strcmp(input, "service")) {
    xodtemplate_current_object_type = XODTEMPLATE_SERVICE;
    xod_begin_def(service);

    new_service->initial_state = STATE_OK;
    new_service->max_check_attempts = -2;
    new_service->check_interval = 5.0;
    new_service->retry_interval = 1.0;
    new_service->active_checks_enabled = true;
    new_service->passive_checks_enabled = true;
    new_service->parallelize_check = true;
    new_service->obsess_over_service = true;
    new_service->event_handler_enabled = true;
    new_service->flap_detection_enabled = true;
    new_service->flap_detection_on_ok = true;
    new_service->flap_detection_on_warning = true;
    new_service->flap_detection_on_unknown = true;
    new_service->flap_detection_on_critical = true;
    new_service->notifications_enabled = true;
    new_service->notification_interval = 30.0;
    new_service->process_perf_data = true;
    new_service->retain_status_information = true;
    new_service->retain_nonstatus_information = true;

    /* true service, so is not from host group, must be set AFTER
     * have_initial_state */
    xodtemplate_unset_service_is_from_hostgroup(new_service);
  } else if (!strcmp(input, "host")) {
    xodtemplate_current_object_type = XODTEMPLATE_HOST;
    xod_begin_def(host);

    new_host->check_interval = 5.0;
    new_host->retry_interval = 1.0;
    new_host->active_checks_enabled = true;
    new_host->passive_checks_enabled = true;
    new_host->obsess_over_host = true;
    new_host->max_check_attempts = -2;
    new_host->event_handler_enabled = true;
    new_host->flap_detection_enabled = true;
    new_host->flap_detection_on_up = true;
    new_host->flap_detection_on_down = true;
    new_host->flap_detection_on_unreachable = true;
    new_host->notifications_enabled = true;
    new_host->notification_interval = 30.0;
    new_host->process_perf_data = true;
    new_host->x_2d = -1;
    new_host->y_2d = -1;
    new_host->retain_status_information = true;
    new_host->retain_nonstatus_information = true;
  } else if (!strcmp(input, "command")) {
    xodtemplate_current_object_type = XODTEMPLATE_COMMAND;
    xod_begin_def(command);
  } else if (!strcmp(input, "contact")) {
    xodtemplate_current_object_type = XODTEMPLATE_CONTACT;
    xod_begin_def(contact);

    new_contact->host_notifications_enabled = true;
    new_contact->service_notifications_enabled = true;
    new_contact->can_submit_commands = true;
    new_contact->retain_status_information = true;
    new_contact->retain_nonstatus_information = true;
  } else if (!strcmp(input, "timeperiod")) {
    xodtemplate_current_object_type = XODTEMPLATE_TIMEPERIOD;
    xod_begin_def(timeperiod);
  } else if (!strcmp(input, "contactgroup")) {
    xodtemplate_current_object_type = XODTEMPLATE_CONTACTGROUP;
    xod_begin_def(contactgroup);
  } else if (!strcmp(input, "hostgroup")) {
    xodtemplate_current_object_type = XODTEMPLATE_HOSTGROUP;
    xod_begin_def(hostgroup);
  } else if (!strcmp(input, "servicegroup")) {
    xodtemplate_current_object_type = XODTEMPLATE_SERVICEGROUP;
    xod_begin_def(servicegroup);
  } else if (!strcmp(input, "servicedependency")) {
    xodtemplate_current_object_type = XODTEMPLATE_SERVICEDEPENDENCY;
    xod_begin_def(servicedependency);
  } else if (!strcmp(input, "serviceescalation")) {
    xodtemplate_current_object_type = XODTEMPLATE_SERVICEESCALATION;
    xod_begin_def(serviceescalation);

    new_serviceescalation->first_notification = -2;
    new_serviceescalation->last_notification = -2;
  } else if (!strcmp(input, "hostdependency")) {
    xodtemplate_current_object_type = XODTEMPLATE_HOSTDEPENDENCY;
    xod_begin_def(hostdependency);
  } else if (!strcmp(input, "hostescalation")) {
    xodtemplate_current_object_type = XODTEMPLATE_HOSTESCALATION;
    xod_begin_def(hostescalation);

    new_hostescalation->first_notification = -2;
    new_hostescalation->last_notification = -2;
  } else if (!strcmp(input, "serviceextinfo")) {
    xodtemplate_current_object_type = XODTEMPLATE_SERVICEEXTINFO;
    xod_begin_def(serviceextinfo);
  } else if (!strcmp(input, "hostextinfo")) {
    xodtemplate_current_object_type = XODTEMPLATE_HOSTEXTINFO;
    xod_begin_def(hostextinfo);

    new_hostextinfo->x_2d = -1;
    new_hostextinfo->y_2d = -1;
  } else if (!strcmp(input, "connector")) {
    xodtemplate_current_object_type = XODTEMPLATE_CONNECTOR;
    xod_begin_def(connector);
  } else
    return ERROR;

  return result;
}

#undef xod_begin_def /* we don't need this anymore */

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
  xodtemplate_contactgroup* temp_contactgroup = NULL;
  xodtemplate_hostgroup* temp_hostgroup = NULL;
  xodtemplate_servicegroup* temp_servicegroup = NULL;
  xodtemplate_servicedependency* temp_servicedependency = NULL;
  xodtemplate_serviceescalation* temp_serviceescalation = NULL;
  xodtemplate_contact* temp_contact = NULL;
  xodtemplate_host* temp_host = NULL;
  xodtemplate_service* temp_service = NULL;
  xodtemplate_hostdependency* temp_hostdependency = NULL;
  xodtemplate_hostescalation* temp_hostescalation = NULL;
  xodtemplate_hostextinfo* temp_hostextinfo = NULL;
  xodtemplate_serviceextinfo* temp_serviceextinfo = NULL;
  int x = 0;
  int y = 0;
  int force_skiplists = false;

  /* should some object definitions be added to skiplists immediately? */
  if (use_precached_objects == true)
    force_skiplists = true;

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
    return ERROR;
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
        result = skiplist_insert(xobject_template_skiplists[X_SERVICE_SKIPLIST],
                                 (void*)temp_service);
        switch (result) {
          case SKIPLIST_OK:
            result = OK;
            break;

          case SKIPLIST_ERROR_DUPLICATE:
            logger(log_config_warning, basic)
                << "Warning: Duplicate definition found for service '" << value
                << "' (config file '"
                << xodtemplate_config_file_name(temp_service->_config_file)
                << "', starting on line " << temp_service->_start_line << ")";
            result = ERROR;
            break;

          default:
            result = ERROR;
            break;
        }
      } else if (!strcmp(variable, "host") || !strcmp(variable, "hosts") ||
                 !strcmp(variable, "host_name")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_service->host_name = string::dup(value);
        temp_service->have_host_name = true;

        /* NOTE: services are added to the skiplist in
         * xodtemplate_duplicate_services(), except if daemon is using precached
         * config */
        if (result == OK && force_skiplists == true &&
            temp_service->host_name != NULL &&
            temp_service->service_description != NULL) {
          /* add service to template skiplist for fast searches */
          result = skiplist_insert(xobject_skiplists[X_SERVICE_SKIPLIST],
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
      } else if (!strcmp(variable, "service_description") ||
                 !strcmp(variable, "description")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_service->service_description = string::dup(value);
        temp_service->have_service_description = true;

        /* NOTE: services are added to the skiplist in
         * xodtemplate_duplicate_services(), except if daemon is using precached
         * config */
        if (result == OK && force_skiplists == true &&
            temp_service->host_name != NULL &&
            temp_service->service_description != NULL) {
          /* add service to template skiplist for fast searches */
          result = skiplist_insert(xobject_skiplists[X_SERVICE_SKIPLIST],
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
      } else if (!strcmp(variable, "display_name")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_service->display_name = string::dup(value);
        temp_service->have_display_name = true;
      } else if (!strcmp(variable, "hostgroup") ||
                 !strcmp(variable, "hostgroups") ||
                 !strcmp(variable, "hostgroup_name")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_service->hostgroup_name = string::dup(value);
        temp_service->have_hostgroup_name = true;
      } else if (!strcmp(variable, "service_groups") ||
                 !strcmp(variable, "servicegroups")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_service->service_groups = string::dup(value);
        temp_service->have_service_groups = true;
      } else if (!strcmp(variable, "check_command")) {
        if (strcmp(value, XODTEMPLATE_NULL)) {
          if (value[0] == '!') {
            temp_service->have_important_check_command = true;
            temp_ptr = value + 1;
          } else
            temp_ptr = value;
          temp_service->check_command = string::dup(temp_ptr);
        }
        temp_service->have_check_command = true;
      } else if (!strcmp(variable, "check_period")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_service->check_period = string::dup(value);
        temp_service->have_check_period = true;
      } else if (!strcmp(variable, "event_handler")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_service->event_handler = string::dup(value);
        temp_service->have_event_handler = true;
      } else if (!strcmp(variable, "notification_period")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_service->notification_period = string::dup(value);
        temp_service->have_notification_period = true;
      } else if (!strcmp(variable, "contact_groups")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_service->contact_groups = string::dup(value);
        temp_service->have_contact_groups = true;
      } else if (!strcmp(variable, "contacts")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_service->contacts = string::dup(value);
        temp_service->have_contacts = true;
      } else if (!strcmp(variable, "notes")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_service->notes = string::dup(value);
        temp_service->have_notes = true;
      } else if (!strcmp(variable, "notes_url")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_service->notes_url = string::dup(value);
        temp_service->have_notes_url = true;
      } else if (!strcmp(variable, "action_url")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_service->action_url = string::dup(value);
        temp_service->have_action_url = true;
      } else if (!strcmp(variable, "icon_image")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_service->icon_image = string::dup(value);
        temp_service->have_icon_image = true;
      } else if (!strcmp(variable, "icon_image_alt")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_service->icon_image_alt = string::dup(value);
        temp_service->have_icon_image_alt = true;
      } else if (!strcmp(variable, "initial_state")) {
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
      } else if (!strcmp(variable, "max_check_attempts")) {
        temp_service->max_check_attempts = atoi(value);
        temp_service->have_max_check_attempts = true;
      } else if (!strcmp(variable, "check_interval") ||
                 !strcmp(variable, "normal_check_interval")) {
        temp_service->check_interval = strtod(value, NULL);
        temp_service->have_check_interval = true;
      } else if (!strcmp(variable, "retry_interval") ||
                 !strcmp(variable, "retry_check_interval")) {
        temp_service->retry_interval = strtod(value, NULL);
        temp_service->have_retry_interval = true;
      } else if (!strcmp(variable, "active_checks_enabled")) {
        temp_service->active_checks_enabled = (atoi(value) > 0) ? true : false;
        temp_service->have_active_checks_enabled = true;
      } else if (!strcmp(variable, "passive_checks_enabled")) {
        temp_service->passive_checks_enabled = (atoi(value) > 0) ? true : false;
        temp_service->have_passive_checks_enabled = true;
      } else if (!strcmp(variable, "parallelize_check")) {
        temp_service->parallelize_check = atoi(value);
        temp_service->have_parallelize_check = true;
      } else if (!strcmp(variable, "is_volatile")) {
        temp_service->is_volatile = (atoi(value) > 0) ? true : false;
        temp_service->have_is_volatile = true;
      } else if (!strcmp(variable, "obsess_over_service")) {
        temp_service->obsess_over_service = (atoi(value) > 0) ? true : false;
        temp_service->have_obsess_over_service = true;
      } else if (!strcmp(variable, "event_handler_enabled")) {
        temp_service->event_handler_enabled = (atoi(value) > 0) ? true : false;
        temp_service->have_event_handler_enabled = true;
      } else if (!strcmp(variable, "check_freshness")) {
        temp_service->check_freshness = (atoi(value) > 0) ? true : false;
        temp_service->have_check_freshness = true;
      } else if (!strcmp(variable, "freshness_threshold")) {
        temp_service->freshness_threshold = atoi(value);
        temp_service->have_freshness_threshold = true;
      } else if (!strcmp(variable, "low_flap_threshold")) {
        temp_service->low_flap_threshold = strtod(value, NULL);
        temp_service->have_low_flap_threshold = true;
      } else if (!strcmp(variable, "high_flap_threshold")) {
        temp_service->high_flap_threshold = strtod(value, NULL);
        temp_service->have_high_flap_threshold = true;
      } else if (!strcmp(variable, "flap_detection_enabled")) {
        temp_service->flap_detection_enabled = (atoi(value) > 0) ? true : false;
        temp_service->have_flap_detection_enabled = true;
      } else if (!strcmp(variable, "flap_detection_options")) {
        /* user is specifying something, so discard defaults... */
        temp_service->flap_detection_on_ok = false;
        temp_service->flap_detection_on_warning = false;
        temp_service->flap_detection_on_unknown = false;
        temp_service->flap_detection_on_critical = false;

        for (temp_ptr = strtok(value, ", "); temp_ptr != NULL;
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
          } else if (!strcmp(temp_ptr, "a") || !strcmp(temp_ptr, "all")) {
            temp_service->flap_detection_on_ok = true;
            temp_service->flap_detection_on_warning = true;
            temp_service->flap_detection_on_unknown = true;
            temp_service->flap_detection_on_critical = true;
          } else {
            logger(log_config_error, basic)
                << "Error: Invalid flap detection option '" << temp_ptr
                << "' in service definition.";
            return ERROR;
          }
        }
        temp_service->have_flap_detection_options = true;
      } else if (!strcmp(variable, "notification_options")) {
        for (temp_ptr = strtok(value, ", "); temp_ptr != NULL;
             temp_ptr = strtok(NULL, ", ")) {
          if (!strcmp(temp_ptr, "u") || !strcmp(temp_ptr, "unknown"))
            temp_service->notify_on_unknown = true;
          else if (!strcmp(temp_ptr, "w") || !strcmp(temp_ptr, "warning"))
            temp_service->notify_on_warning = true;
          else if (!strcmp(temp_ptr, "c") || !strcmp(temp_ptr, "critical"))
            temp_service->notify_on_critical = true;
          else if (!strcmp(temp_ptr, "r") || !strcmp(temp_ptr, "recovery"))
            temp_service->notify_on_recovery = true;
          else if (!strcmp(temp_ptr, "f") || !strcmp(temp_ptr, "flapping"))
            temp_service->notify_on_flapping = true;
          else if (!strcmp(temp_ptr, "s") || !strcmp(temp_ptr, "downtime"))
            temp_service->notify_on_downtime = true;
          else if (!strcmp(temp_ptr, "n") || !strcmp(temp_ptr, "none")) {
            temp_service->notify_on_unknown = false;
            temp_service->notify_on_warning = false;
            temp_service->notify_on_critical = false;
            temp_service->notify_on_recovery = false;
            temp_service->notify_on_flapping = false;
            temp_service->notify_on_downtime = false;
          } else if (!strcmp(temp_ptr, "a") || !strcmp(temp_ptr, "all")) {
            temp_service->notify_on_unknown = true;
            temp_service->notify_on_warning = true;
            temp_service->notify_on_critical = true;
            temp_service->notify_on_recovery = true;
            temp_service->notify_on_flapping = true;
            temp_service->notify_on_downtime = true;
          } else {
            logger(log_config_error, basic)
                << "Error: Invalid notification option '" << temp_ptr
                << "' in service definition.";
            return ERROR;
          }
        }
        temp_service->have_notification_options = true;
      } else if (!strcmp(variable, "notifications_enabled")) {
        temp_service->notifications_enabled = (atoi(value) > 0) ? true : false;
        temp_service->have_notifications_enabled = true;
      } else if (!strcmp(variable, "notification_interval")) {
        temp_service->notification_interval = strtod(value, NULL);
        temp_service->have_notification_interval = true;
      } else if (!strcmp(variable, "first_notification_delay")) {
        temp_service->first_notification_delay = strtod(value, NULL);
        temp_service->have_first_notification_delay = true;
      } else if (!strcmp(variable, "stalking_options")) {
        for (temp_ptr = strtok(value, ", "); temp_ptr != NULL;
             temp_ptr = strtok(NULL, ", ")) {
          if (!strcmp(temp_ptr, "o") || !strcmp(temp_ptr, "ok"))
            temp_service->stalk_on_ok = true;
          else if (!strcmp(temp_ptr, "w") || !strcmp(temp_ptr, "warning"))
            temp_service->stalk_on_warning = true;
          else if (!strcmp(temp_ptr, "u") || !strcmp(temp_ptr, "unknown"))
            temp_service->stalk_on_unknown = true;
          else if (!strcmp(temp_ptr, "c") || !strcmp(temp_ptr, "critical"))
            temp_service->stalk_on_critical = true;
          else if (!strcmp(temp_ptr, "n") || !strcmp(temp_ptr, "none")) {
            temp_service->stalk_on_ok = false;
            temp_service->stalk_on_warning = false;
            temp_service->stalk_on_unknown = false;
            temp_service->stalk_on_critical = false;
          } else if (!strcmp(temp_ptr, "a") || !strcmp(temp_ptr, "all")) {
            temp_service->stalk_on_ok = true;
            temp_service->stalk_on_warning = true;
            temp_service->stalk_on_unknown = true;
            temp_service->stalk_on_critical = true;
          } else {
            logger(log_config_error, basic)
                << "Error: Invalid stalking option '" << temp_ptr
                << "' in service definition.";
            return ERROR;
          }
        }
        temp_service->have_stalking_options = true;
      } else if (!strcmp(variable, "process_perf_data")) {
        temp_service->process_perf_data = (atoi(value) > 0) ? true : false;
        temp_service->have_process_perf_data = true;
      } else if (!strcmp(variable, "retain_status_information")) {
        temp_service->retain_status_information =
            (atoi(value) > 0) ? true : false;
        temp_service->have_retain_status_information = true;
      } else if (!strcmp(variable, "retain_nonstatus_information")) {
        temp_service->retain_nonstatus_information =
            (atoi(value) > 0) ? true : false;
        temp_service->have_retain_nonstatus_information = true;
      } else if (!strcmp(variable, "register"))
        temp_service->register_object = (atoi(value) > 0) ? true : false;
      else if (variable[0] == '_') {
        /* get the variable name */
        customvarname = string::dup(variable + 1);

        /* make sure we have a variable name */
        if (!strcmp(customvarname, "")) {
          logger(log_config_error, basic)
              << "Error: Null custom variable name.";
          delete[] customvarname;
          return ERROR;
        }

        /* get the variable value */
        if (strcmp(value, XODTEMPLATE_NULL))
          customvarvalue = string::dup(value);
        else
          customvarvalue = NULL;

        /* add the custom variable */
        if (xodtemplate_add_custom_variable_to_service(
                temp_service, customvarname, customvarvalue) == NULL) {
          delete[] customvarname;
          delete[] customvarvalue;
          return ERROR;
        }

        /* free memory */
        delete[] customvarname;
        delete[] customvarvalue;
      } else {
        logger(log_config_error, basic)
            << "Error: Invalid service object directive '" << variable << "'.";
        return ERROR;
      }
      break;

    case XODTEMPLATE_HOST:
      temp_host = (xodtemplate_host*)xodtemplate_current_object;

      if (!strcmp(variable, "use"))
        temp_host->tmpl = string::dup(value);
      else if (!strcmp(variable, "name")) {
        temp_host->name = string::dup(value);

        /* add host to template skiplist for fast searches */
        result = skiplist_insert(xobject_template_skiplists[X_HOST_SKIPLIST],
                                 (void*)temp_host);
        switch (result) {
          case SKIPLIST_OK:
            result = OK;
            break;

          case SKIPLIST_ERROR_DUPLICATE:
            logger(log_config_warning, basic)
                << "Warning: Duplicate definition found for host '" << value
                << "' (config file '"
                << xodtemplate_config_file_name(temp_host->_config_file)
                << "', starting on line " << temp_host->_start_line << ")";
            result = ERROR;
            break;

          default:
            result = ERROR;
            break;
        }
      } else if (!strcmp(variable, "host_name")) {
        temp_host->host_name = string::dup(value);

        /* add host to template skiplist for fast searches */
        result = skiplist_insert(xobject_skiplists[X_HOST_SKIPLIST],
                                 (void*)temp_host);
        switch (result) {
          case SKIPLIST_OK:
            result = OK;
            break;

          case SKIPLIST_ERROR_DUPLICATE:
            logger(log_config_warning, basic)
                << "Warning: Duplicate definition found for host '" << value
                << "' (config file '"
                << xodtemplate_config_file_name(temp_host->_config_file)
                << "', starting on line " << temp_host->_start_line << ")";
            result = ERROR;
            break;

          default:
            result = ERROR;
            break;
        }
      } else if (!strcmp(variable, "display_name")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_host->display_name = string::dup(value);
        temp_host->have_display_name = true;
      } else if (!strcmp(variable, "alias"))
        temp_host->alias = string::dup(value);
      else if (!strcmp(variable, "address"))
        temp_host->address = string::dup(value);
      else if (!strcmp(variable, "parents")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_host->parents = string::dup(value);
        temp_host->have_parents = true;
      } else if (!strcmp(variable, "host_groups") ||
                 !strcmp(variable, "hostgroups")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_host->host_groups = string::dup(value);
        temp_host->have_host_groups = true;
      } else if (!strcmp(variable, "contact_groups")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_host->contact_groups = string::dup(value);
        temp_host->have_contact_groups = true;
      } else if (!strcmp(variable, "contacts")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_host->contacts = string::dup(value);
        temp_host->have_contacts = true;
      } else if (!strcmp(variable, "notification_period")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_host->notification_period = string::dup(value);
        temp_host->have_notification_period = true;
      } else if (!strcmp(variable, "check_command")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_host->check_command = string::dup(value);
        temp_host->have_check_command = true;
      } else if (!strcmp(variable, "check_period")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_host->check_period = string::dup(value);
        temp_host->have_check_period = true;
      } else if (!strcmp(variable, "event_handler")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_host->event_handler = string::dup(value);
        temp_host->have_event_handler = true;
      } else if (!strcmp(variable, "notes")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_host->notes = string::dup(value);
        temp_host->have_notes = true;
      } else if (!strcmp(variable, "notes_url")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_host->notes_url = string::dup(value);
        temp_host->have_notes_url = true;
      } else if (!strcmp(variable, "action_url")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_host->action_url = string::dup(value);
        temp_host->have_action_url = true;
      } else if (!strcmp(variable, "icon_image")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_host->icon_image = string::dup(value);
        temp_host->have_icon_image = true;
      } else if (!strcmp(variable, "icon_image_alt")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_host->icon_image_alt = string::dup(value);
        temp_host->have_icon_image_alt = true;
      } else if (!strcmp(variable, "vrml_image")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_host->vrml_image = string::dup(value);
        temp_host->have_vrml_image = true;
      } else if (!strcmp(variable, "gd2_image") ||
                 !strcmp(variable, "statusmap_image")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_host->statusmap_image = string::dup(value);
        temp_host->have_statusmap_image = true;
      } else if (!strcmp(variable, "initial_state")) {
        if (!strcmp(value, "o") || !strcmp(value, "up"))
          temp_host->initial_state = 0; /* HOST_UP */
        else if (!strcmp(value, "d") || !strcmp(value, "down"))
          temp_host->initial_state = 1; /* HOST_DOWN */
        else if (!strcmp(value, "u") || !strcmp(value, "unreachable"))
          temp_host->initial_state = 2; /* HOST_UNREACHABLE */
        else {
          logger(log_config_error, basic) << "Error: Invalid initial state '"
                                          << value << "' in host definition.";
          result = ERROR;
        }
        temp_host->have_initial_state = true;
      } else if (!strcmp(variable, "check_interval") ||
                 !strcmp(variable, "normal_check_interval")) {
        temp_host->check_interval = strtod(value, NULL);
        temp_host->have_check_interval = true;
      } else if (!strcmp(variable, "retry_interval") ||
                 !strcmp(variable, "retry_check_interval")) {
        temp_host->retry_interval = strtod(value, NULL);
        temp_host->have_retry_interval = true;
      } else if (!strcmp(variable, "max_check_attempts")) {
        temp_host->max_check_attempts = atoi(value);
        temp_host->have_max_check_attempts = true;
      } else if (!strcmp(variable, "checks_enabled") ||
                 !strcmp(variable, "active_checks_enabled")) {
        temp_host->active_checks_enabled = (atoi(value) > 0) ? true : false;
        temp_host->have_active_checks_enabled = true;
      } else if (!strcmp(variable, "passive_checks_enabled")) {
        temp_host->passive_checks_enabled = (atoi(value) > 0) ? true : false;
        temp_host->have_passive_checks_enabled = true;
      } else if (!strcmp(variable, "event_handler_enabled")) {
        temp_host->event_handler_enabled = (atoi(value) > 0) ? true : false;
        temp_host->have_event_handler_enabled = true;
      } else if (!strcmp(variable, "check_freshness")) {
        temp_host->check_freshness = (atoi(value) > 0) ? true : false;
        temp_host->have_check_freshness = true;
      } else if (!strcmp(variable, "freshness_threshold")) {
        temp_host->freshness_threshold = atoi(value);
        temp_host->have_freshness_threshold = true;
      } else if (!strcmp(variable, "low_flap_threshold")) {
        temp_host->low_flap_threshold = strtod(value, NULL);
        temp_host->have_low_flap_threshold = true;
      } else if (!strcmp(variable, "high_flap_threshold")) {
        temp_host->high_flap_threshold = strtod(value, NULL);
        temp_host->have_high_flap_threshold = true;
      } else if (!strcmp(variable, "flap_detection_enabled")) {
        temp_host->flap_detection_enabled = (atoi(value) > 0) ? true : false;
        temp_host->have_flap_detection_enabled = true;
      } else if (!strcmp(variable, "flap_detection_options")) {
        /* user is specifying something, so discard defaults... */
        temp_host->flap_detection_on_up = false;
        temp_host->flap_detection_on_down = false;
        temp_host->flap_detection_on_unreachable = false;

        for (temp_ptr = strtok(value, ", "); temp_ptr != NULL;
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
          } else if (!strcmp(temp_ptr, "a") || !strcmp(temp_ptr, "all")) {
            temp_host->flap_detection_on_up = true;
            temp_host->flap_detection_on_down = true;
            temp_host->flap_detection_on_unreachable = true;
          } else {
            logger(log_config_error, basic)
                << "Error: Invalid flap detection option '" << temp_ptr
                << "' in host definition.";
            result = ERROR;
          }
        }
        temp_host->have_flap_detection_options = true;
      } else if (!strcmp(variable, "notification_options")) {
        for (temp_ptr = strtok(value, ", "); temp_ptr != NULL;
             temp_ptr = strtok(NULL, ", ")) {
          if (!strcmp(temp_ptr, "d") || !strcmp(temp_ptr, "down"))
            temp_host->notify_on_down = true;
          else if (!strcmp(temp_ptr, "u") || !strcmp(temp_ptr, "unreachable"))
            temp_host->notify_on_unreachable = true;
          else if (!strcmp(temp_ptr, "r") || !strcmp(temp_ptr, "recovery"))
            temp_host->notify_on_recovery = true;
          else if (!strcmp(temp_ptr, "f") || !strcmp(temp_ptr, "flapping"))
            temp_host->notify_on_flapping = true;
          else if (!strcmp(temp_ptr, "s") || !strcmp(temp_ptr, "downtime"))
            temp_host->notify_on_downtime = true;
          else if (!strcmp(temp_ptr, "n") || !strcmp(temp_ptr, "none")) {
            temp_host->notify_on_down = false;
            temp_host->notify_on_unreachable = false;
            temp_host->notify_on_recovery = false;
            temp_host->notify_on_flapping = false;
            temp_host->notify_on_downtime = false;
          } else if (!strcmp(temp_ptr, "a") || !strcmp(temp_ptr, "all")) {
            temp_host->notify_on_down = true;
            temp_host->notify_on_unreachable = true;
            temp_host->notify_on_recovery = true;
            temp_host->notify_on_flapping = true;
            temp_host->notify_on_downtime = true;
          } else {
            logger(log_config_error, basic)
                << "Error: Invalid notification option '" << temp_ptr
                << "' in host definition.";
            result = ERROR;
          }
        }
        temp_host->have_notification_options = true;
      } else if (!strcmp(variable, "notifications_enabled")) {
        temp_host->notifications_enabled = (atoi(value) > 0) ? true : false;
        temp_host->have_notifications_enabled = true;
      } else if (!strcmp(variable, "notification_interval")) {
        temp_host->notification_interval = strtod(value, NULL);
        temp_host->have_notification_interval = true;
      } else if (!strcmp(variable, "first_notification_delay")) {
        temp_host->first_notification_delay = strtod(value, NULL);
        temp_host->have_first_notification_delay = true;
      } else if (!strcmp(variable, "stalking_options")) {
        for (temp_ptr = strtok(value, ", "); temp_ptr != NULL;
             temp_ptr = strtok(NULL, ", ")) {
          if (!strcmp(temp_ptr, "o") || !strcmp(temp_ptr, "up"))
            temp_host->stalk_on_up = true;
          else if (!strcmp(temp_ptr, "d") || !strcmp(temp_ptr, "down"))
            temp_host->stalk_on_down = true;
          else if (!strcmp(temp_ptr, "u") || !strcmp(temp_ptr, "unreachable"))
            temp_host->stalk_on_unreachable = true;
          else if (!strcmp(temp_ptr, "n") || !strcmp(temp_ptr, "none")) {
            temp_host->stalk_on_up = false;
            temp_host->stalk_on_down = false;
            temp_host->stalk_on_unreachable = false;
          } else if (!strcmp(temp_ptr, "a") || !strcmp(temp_ptr, "all")) {
            temp_host->stalk_on_up = true;
            temp_host->stalk_on_down = true;
            temp_host->stalk_on_unreachable = true;
          } else {
            logger(log_config_error, basic)
                << "Error: Invalid stalking option '" << temp_ptr
                << "' in host definition.";
            result = ERROR;
          }
        }
        temp_host->have_stalking_options = true;
      } else if (!strcmp(variable, "process_perf_data")) {
        temp_host->process_perf_data = (atoi(value) > 0) ? true : false;
        temp_host->have_process_perf_data = true;
      } else if (!strcmp(variable, "2d_coords")) {
        if ((temp_ptr = strtok(value, ", ")) == NULL) {
          logger(log_config_error, basic)
              << "Error: Invalid 2d_coords value '" << temp_ptr
              << "' in host definition.";
          return ERROR;
        }
        temp_host->x_2d = atoi(temp_ptr);
        if ((temp_ptr = strtok(NULL, ", ")) == NULL) {
          logger(log_config_error, basic)
              << "Error: Invalid 2d_coords value '" << temp_ptr
              << "' in host definition.";
          return ERROR;
        }
        temp_host->y_2d = atoi(temp_ptr);
        temp_host->have_2d_coords = true;
      } else if (!strcmp(variable, "3d_coords")) {
        if ((temp_ptr = strtok(value, ", ")) == NULL) {
          logger(log_config_error, basic)
              << "Error: Invalid 3d_coords value '" << temp_ptr
              << "' in host definition.";
          return ERROR;
        }
        temp_host->x_3d = strtod(temp_ptr, NULL);
        if ((temp_ptr = strtok(NULL, ", ")) == NULL) {
          logger(log_config_error, basic)
              << "Error: Invalid 3d_coords value '" << temp_ptr
              << "' in host definition.";
          return ERROR;
        }
        temp_host->y_3d = strtod(temp_ptr, NULL);
        if ((temp_ptr = strtok(NULL, ", ")) == NULL) {
          logger(log_config_error, basic)
              << "Error: Invalid 3d_coords value '" << temp_ptr
              << "' in host definition.";
          return ERROR;
        }
        temp_host->z_3d = strtod(temp_ptr, NULL);
        temp_host->have_3d_coords = true;
      } else if (!strcmp(variable, "obsess_over_host")) {
        temp_host->obsess_over_host = (atoi(value) > 0) ? true : false;
        temp_host->have_obsess_over_host = true;
      } else if (!strcmp(variable, "retain_status_information")) {
        temp_host->retain_status_information = (atoi(value) > 0) ? true : false;
        temp_host->have_retain_status_information = true;
      } else if (!strcmp(variable, "retain_nonstatus_information")) {
        temp_host->retain_nonstatus_information =
            (atoi(value) > 0) ? true : false;
        temp_host->have_retain_nonstatus_information = true;
      } else if (!strcmp(variable, "register"))
        temp_host->register_object = (atoi(value) > 0) ? true : false;
      else if (variable[0] == '_') {
        /* get the variable name */
        customvarname = string::dup(variable + 1);

        /* make sure we have a variable name */
        if (!strcmp(customvarname, "")) {
          logger(log_config_error, basic)
              << "Error: Null custom variable name.";
          delete[] customvarname;
          return ERROR;
        }

        /* get the variable value */
        customvarvalue = NULL;
        if (strcmp(value, XODTEMPLATE_NULL))
          customvarvalue = string::dup(value);

        /* add the custom variable */
        if (xodtemplate_add_custom_variable_to_host(temp_host, customvarname,
                                                    customvarvalue) == NULL) {
          delete[] customvarname;
          delete[] customvarvalue;
          return ERROR;
        }

        /* free memory */
        delete[] customvarname;
        delete[] customvarvalue;
      } else {
        logger(log_config_error, basic)
            << "Error: Invalid host object directive '" << variable << "'.";
        return ERROR;
      }
      break;

    case XODTEMPLATE_TIMEPERIOD:
      temp_timeperiod = (xodtemplate_timeperiod*)xodtemplate_current_object;

      if (!strcmp(variable, "use"))
        temp_timeperiod->tmpl = string::dup(value);
      else if (!strcmp(variable, "name")) {
        temp_timeperiod->name = string::dup(value);

        /* add timeperiod to template skiplist for fast searches */
        result =
            skiplist_insert(xobject_template_skiplists[X_TIMEPERIOD_SKIPLIST],
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
                << "', starting on line " << temp_timeperiod->_start_line
                << ")";
            result = ERROR;
            break;

          default:
            result = ERROR;
            break;
        }
      } else if (!strcmp(variable, "timeperiod_name")) {
        temp_timeperiod->timeperiod_name = string::dup(value);

        result = skiplist_insert(xobject_skiplists[X_TIMEPERIOD_SKIPLIST],
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
                << "', starting on line " << temp_timeperiod->_start_line
                << ")";
            result = ERROR;
            break;

          default:
            result = ERROR;
            break;
        }
      } else if (!strcmp(variable, "alias"))
        temp_timeperiod->alias = string::dup(value);
      else if (!strcmp(variable, "exclude"))
        temp_timeperiod->exclusions = string::dup(value);
      else if (!strcmp(variable, "register"))
        temp_timeperiod->register_object = (atoi(value) > 0) ? true : false;
      else if (xodtemplate_parse_timeperiod_directive(temp_timeperiod, variable,
                                                      value) == OK)
        result = OK;
      else {
        logger(log_config_error, basic)
            << "Error: Invalid timeperiod object directive '" << variable
            << "'.";
        return ERROR;
      }
      break;

    case XODTEMPLATE_CONTACT:
      temp_contact = (xodtemplate_contact*)xodtemplate_current_object;

      if (!strcmp(variable, "use"))
        temp_contact->tmpl = string::dup(value);
      else if (!strcmp(variable, "name")) {
        temp_contact->name = string::dup(value);

        /* add contact to template skiplist for fast searches */
        result = skiplist_insert(xobject_template_skiplists[X_CONTACT_SKIPLIST],
                                 (void*)temp_contact);
        switch (result) {
          case SKIPLIST_OK:
            result = OK;
            break;

          case SKIPLIST_ERROR_DUPLICATE:
            logger(log_config_warning, basic)
                << "Warning: Duplicate definition found for contact '" << value
                << "' (config file '"
                << xodtemplate_config_file_name(temp_contact->_config_file)
                << "', starting on line " << temp_contact->_start_line << ")";
            result = ERROR;
            break;

          default:
            result = ERROR;
            break;
        }
      } else if (!strcmp(variable, "contact_name")) {
        temp_contact->contact_name = string::dup(value);

        /* add contact to template skiplist for fast searches */
        result = skiplist_insert(xobject_skiplists[X_CONTACT_SKIPLIST],
                                 (void*)temp_contact);
        switch (result) {
          case SKIPLIST_OK:
            result = OK;
            break;

          case SKIPLIST_ERROR_DUPLICATE:
            logger(log_config_warning, basic)
                << "Warning: Duplicate definition found for contact '" << value
                << "' (config file '"
                << xodtemplate_config_file_name(temp_contact->_config_file)
                << "', starting on line " << temp_contact->_start_line << ")";
            result = ERROR;
            break;

          default:
            result = ERROR;
            break;
        }
      } else if (!strcmp(variable, "alias"))
        temp_contact->alias = string::dup(value);
      else if (!strcmp(variable, "contact_groups") ||
               !strcmp(variable, "contactgroups")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_contact->contact_groups = string::dup(value);
        temp_contact->have_contact_groups = true;
      } else if (!strcmp(variable, "email")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_contact->email = string::dup(value);
        temp_contact->have_email = true;
      } else if (!strcmp(variable, "pager")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_contact->pager = string::dup(value);
        temp_contact->have_pager = true;
      } else if (strstr(variable, "address") == variable) {
        x = atoi(variable + 7);
        if (x < 1 || x > MAX_XODTEMPLATE_CONTACT_ADDRESSES)
          result = ERROR;
        else if (strcmp(value, XODTEMPLATE_NULL))
          temp_contact->address[x - 1] = string::dup(value);
        if (result == OK)
          temp_contact->have_address[x - 1] = true;
      } else if (!strcmp(variable, "host_notification_period")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_contact->host_notification_period = string::dup(value);
        temp_contact->have_host_notification_period = true;
      } else if (!strcmp(variable, "host_notification_commands")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_contact->host_notification_commands = string::dup(value);
        temp_contact->have_host_notification_commands = true;
      } else if (!strcmp(variable, "service_notification_period")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_contact->service_notification_period = string::dup(value);
        temp_contact->have_service_notification_period = true;
      } else if (!strcmp(variable, "service_notification_commands")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_contact->service_notification_commands = string::dup(value);
        temp_contact->have_service_notification_commands = true;
      } else if (!strcmp(variable, "host_notification_options")) {
        for (temp_ptr = strtok(value, ", "); temp_ptr != NULL;
             temp_ptr = strtok(NULL, ", ")) {
          if (!strcmp(temp_ptr, "d") || !strcmp(temp_ptr, "down"))
            temp_contact->notify_on_host_down = true;
          else if (!strcmp(temp_ptr, "u") || !strcmp(temp_ptr, "unreachable"))
            temp_contact->notify_on_host_unreachable = true;
          else if (!strcmp(temp_ptr, "r") || !strcmp(temp_ptr, "recovery"))
            temp_contact->notify_on_host_recovery = true;
          else if (!strcmp(temp_ptr, "f") || !strcmp(temp_ptr, "flapping"))
            temp_contact->notify_on_host_flapping = true;
          else if (!strcmp(temp_ptr, "s") || !strcmp(temp_ptr, "downtime"))
            temp_contact->notify_on_host_downtime = true;
          else if (!strcmp(temp_ptr, "n") || !strcmp(temp_ptr, "none")) {
            temp_contact->notify_on_host_down = false;
            temp_contact->notify_on_host_unreachable = false;
            temp_contact->notify_on_host_recovery = false;
            temp_contact->notify_on_host_flapping = false;
            temp_contact->notify_on_host_downtime = false;
          } else if (!strcmp(temp_ptr, "a") || !strcmp(temp_ptr, "all")) {
            temp_contact->notify_on_host_down = true;
            temp_contact->notify_on_host_unreachable = true;
            temp_contact->notify_on_host_recovery = true;
            temp_contact->notify_on_host_flapping = true;
            temp_contact->notify_on_host_downtime = true;
          } else {
            logger(log_config_error, basic)
                << "Error: Invalid host notification option '" << temp_ptr
                << "' in contact definition.";
            return ERROR;
          }
        }
        temp_contact->have_host_notification_options = true;
      } else if (!strcmp(variable, "service_notification_options")) {
        for (temp_ptr = strtok(value, ", "); temp_ptr != NULL;
             temp_ptr = strtok(NULL, ", ")) {
          if (!strcmp(temp_ptr, "u") || !strcmp(temp_ptr, "unknown"))
            temp_contact->notify_on_service_unknown = true;
          else if (!strcmp(temp_ptr, "w") || !strcmp(temp_ptr, "warning"))
            temp_contact->notify_on_service_warning = true;
          else if (!strcmp(temp_ptr, "c") || !strcmp(temp_ptr, "critical"))
            temp_contact->notify_on_service_critical = true;
          else if (!strcmp(temp_ptr, "r") || !strcmp(temp_ptr, "recovery"))
            temp_contact->notify_on_service_recovery = true;
          else if (!strcmp(temp_ptr, "f") || !strcmp(temp_ptr, "flapping"))
            temp_contact->notify_on_service_flapping = true;
          else if (!strcmp(temp_ptr, "s") || !strcmp(temp_ptr, "downtime"))
            temp_contact->notify_on_service_downtime = true;
          else if (!strcmp(temp_ptr, "n") || !strcmp(temp_ptr, "none")) {
            temp_contact->notify_on_service_unknown = false;
            temp_contact->notify_on_service_warning = false;
            temp_contact->notify_on_service_critical = false;
            temp_contact->notify_on_service_recovery = false;
            temp_contact->notify_on_service_flapping = false;
            temp_contact->notify_on_service_downtime = false;
          } else if (!strcmp(temp_ptr, "a") || !strcmp(temp_ptr, "all")) {
            temp_contact->notify_on_service_unknown = true;
            temp_contact->notify_on_service_warning = true;
            temp_contact->notify_on_service_critical = true;
            temp_contact->notify_on_service_recovery = true;
            temp_contact->notify_on_service_flapping = true;
            temp_contact->notify_on_service_downtime = true;
          } else {
            logger(log_config_error, basic)
                << "Error: Invalid service notification option '" << temp_ptr
                << "' in contact definition.";
            return ERROR;
          }
        }
        temp_contact->have_service_notification_options = true;
      } else if (!strcmp(variable, "host_notifications_enabled")) {
        temp_contact->host_notifications_enabled =
            (atoi(value) > 0) ? true : false;
        temp_contact->have_host_notifications_enabled = true;
      } else if (!strcmp(variable, "service_notifications_enabled")) {
        temp_contact->service_notifications_enabled =
            (atoi(value) > 0) ? true : false;
        temp_contact->have_service_notifications_enabled = true;
      } else if (!strcmp(variable, "can_submit_commands")) {
        temp_contact->can_submit_commands = (atoi(value) > 0) ? true : false;
        temp_contact->have_can_submit_commands = true;
      } else if (!strcmp(variable, "retain_status_information")) {
        temp_contact->retain_status_information =
            (atoi(value) > 0) ? true : false;
        temp_contact->have_retain_status_information = true;
      } else if (!strcmp(variable, "retain_nonstatus_information")) {
        temp_contact->retain_nonstatus_information =
            (atoi(value) > 0) ? true : false;
        temp_contact->have_retain_nonstatus_information = true;
      } else if (!strcmp(variable, "register"))
        temp_contact->register_object = (atoi(value) > 0) ? true : false;
      else if (variable[0] == '_') {
        /* get the variable name */
        customvarname = string::dup(variable + 1);

        /* make sure we have a variable name */
        if (customvarname == NULL || !strcmp(customvarname, "")) {
          logger(log_config_error, basic)
              << "Error: Null custom variable name.";
          delete[] customvarname;
          return ERROR;
        }

        /* get the variable value */
        if (strcmp(value, XODTEMPLATE_NULL))
          customvarvalue = string::dup(value);
        else
          customvarvalue = NULL;

        /* add the custom variable */
        if (xodtemplate_add_custom_variable_to_contact(
                temp_contact, customvarname, customvarvalue) == NULL) {
          delete[] customvarname;
          delete[] customvarvalue;
          return ERROR;
        }

        /* free memory */
        delete[] customvarname;
        delete[] customvarvalue;
      } else {
        logger(log_config_error, basic)
            << "Error: Invalid contact object directive '" << variable << "'.";
        return ERROR;
      }

      break;

    case XODTEMPLATE_COMMAND:
      temp_command = (xodtemplate_command*)xodtemplate_current_object;

      if (!strcmp(variable, "use"))
        temp_command->tmpl = string::dup(value);
      else if (!strcmp(variable, "name")) {
        temp_command->name = string::dup(value);

        /* add command to template skiplist for fast searches */
        result = skiplist_insert(xobject_template_skiplists[X_COMMAND_SKIPLIST],
                                 (void*)temp_command);
        switch (result) {
          case SKIPLIST_OK:
            result = OK;
            break;

          case SKIPLIST_ERROR_DUPLICATE:
            logger(log_config_warning, basic)
                << "Warning: Duplicate definition found for command '" << value
                << "' (config file '"
                << xodtemplate_config_file_name(temp_command->_config_file)
                << "', starting on line " << temp_command->_start_line << ")";
            result = ERROR;
            break;

          default:
            result = ERROR;
            break;
        }
      } else if (!strcmp(variable, "command_name")) {
        temp_command->command_name = string::dup(value);

        /* add command to template skiplist for fast searches */
        result = skiplist_insert(xobject_skiplists[X_COMMAND_SKIPLIST],
                                 (void*)temp_command);
        switch (result) {
          case SKIPLIST_OK:
            result = OK;
            break;

          case SKIPLIST_ERROR_DUPLICATE:
            logger(log_config_warning, basic)
                << "Warning: Duplicate definition found for command '" << value
                << "' (config file '"
                << xodtemplate_config_file_name(temp_command->_config_file)
                << "', starting on line " << temp_command->_start_line << ")";
            result = ERROR;
            break;

          default:
            result = ERROR;
            break;
        }
      } else if (!strcmp(variable, "command_line"))
        temp_command->command_line = string::dup(value);
      else if (!strcmp(variable, "register"))
        temp_command->register_object = (atoi(value) > 0) ? true : false;
      else if (!strcmp(variable, "connector"))
        temp_command->connector_name = string::dup(value);
      else {
        logger(log_config_error, basic)
            << "Error: Invalid command object directive '" << variable << "'.";
        return ERROR;
      }
      break;

    case XODTEMPLATE_CONNECTOR:
      temp_connector = (xodtemplate_connector*)xodtemplate_current_object;

      if (!strcmp(variable, "connector_line"))
        temp_connector->connector_line = string::dup(value);
      else if (!strcmp(variable, "connector_name")) {
        temp_connector->connector_name = string::dup(value);

        /* add command to template skiplist for fast searches */
        result = skiplist_insert(xobject_skiplists[X_CONNECTOR_SKIPLIST],
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
      } else {
        logger(log_config_error, basic)
            << "Error: Invalid command object directive '" << variable << "'.";
        return ERROR;
      }
      break;

    case XODTEMPLATE_CONTACTGROUP:
      temp_contactgroup = (xodtemplate_contactgroup*)xodtemplate_current_object;
      if (!strcmp(variable, "use"))
        temp_contactgroup->tmpl = string::dup(value);
      else if (!strcmp(variable, "name")) {
        temp_contactgroup->name = string::dup(value);

        /* add contactgroup to template skiplist for fast searches */
        result =
            skiplist_insert(xobject_template_skiplists[X_CONTACTGROUP_SKIPLIST],
                            (void*)temp_contactgroup);
        switch (result) {
          case SKIPLIST_OK:
            result = OK;
            break;

          case SKIPLIST_ERROR_DUPLICATE:
            logger(log_config_warning, basic)
                << "Warning: Duplicate definition found for contactgroup '"
                << value << "' (config file '"
                << xodtemplate_config_file_name(temp_contactgroup->_config_file)
                << "', starting on line " << temp_contactgroup->_start_line
                << ")";
            result = ERROR;
            break;

          default:
            result = ERROR;
            break;
        }
      } else if (!strcmp(variable, "contactgroup_name")) {
        temp_contactgroup->contactgroup_name = string::dup(value);

        /* add contactgroup to template skiplist for fast searches */
        result = skiplist_insert(xobject_skiplists[X_CONTACTGROUP_SKIPLIST],
                                 (void*)temp_contactgroup);
        switch (result) {
          case SKIPLIST_OK:
            result = OK;
            break;

          case SKIPLIST_ERROR_DUPLICATE:
            logger(log_config_warning, basic)
                << "Warning: Duplicate definition found for contactgroup '"
                << value << "' (config file '"
                << xodtemplate_config_file_name(temp_contactgroup->_config_file)
                << "', starting on line " << temp_contactgroup->_start_line
                << ")";
            result = ERROR;
            break;

          default:
            result = ERROR;
            break;
        }
      } else if (!strcmp(variable, "alias"))
        temp_contactgroup->alias = string::dup(value);
      else if (!strcmp(variable, "members")) {
        if (strcmp(value, XODTEMPLATE_NULL)) {
          if (temp_contactgroup->members == NULL)
            temp_contactgroup->members = string::dup(value);
          else {
            temp_contactgroup->members = resize_string(
                temp_contactgroup->members,
                strlen(temp_contactgroup->members) + strlen(value) + 2);
            strcat(temp_contactgroup->members, ",");
            strcat(temp_contactgroup->members, value);
          }
          if (temp_contactgroup->members == NULL)
            result = ERROR;
        }
        temp_contactgroup->have_members = true;
      } else if (!strcmp(variable, "contactgroup_members")) {
        if (strcmp(value, XODTEMPLATE_NULL)) {
          if (temp_contactgroup->contactgroup_members == NULL)
            temp_contactgroup->contactgroup_members = string::dup(value);
          else {
            temp_contactgroup->contactgroup_members =
                resize_string(temp_contactgroup->contactgroup_members,
                              strlen(temp_contactgroup->contactgroup_members) +
                                  strlen(value) + 2);
            strcat(temp_contactgroup->contactgroup_members, ",");
            strcat(temp_contactgroup->contactgroup_members, value);
          }
          if (temp_contactgroup->contactgroup_members == NULL)
            result = ERROR;
        }
        temp_contactgroup->have_contactgroup_members = true;
      } else if (!strcmp(variable, "register"))
        temp_contactgroup->register_object = (atoi(value) > 0) ? true : false;
      else {
        logger(log_config_error, basic)
            << "Error: Invalid contactgroup object directive '" << variable
            << "'.";
        return ERROR;
      }

      break;

    case XODTEMPLATE_HOSTGROUP:
      temp_hostgroup = (xodtemplate_hostgroup*)xodtemplate_current_object;

      if (!strcmp(variable, "use"))
        temp_hostgroup->tmpl = string::dup(value);
      else if (!strcmp(variable, "name")) {
        temp_hostgroup->name = string::dup(value);

        /* add hostgroup to template skiplist for fast searches */
        result =
            skiplist_insert(xobject_template_skiplists[X_HOSTGROUP_SKIPLIST],
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
      } else if (!strcmp(variable, "hostgroup_name")) {
        temp_hostgroup->hostgroup_name = string::dup(value);

        /* add hostgroup to template skiplist for fast searches */
        result = skiplist_insert(xobject_skiplists[X_HOSTGROUP_SKIPLIST],
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
      } else if (!strcmp(variable, "alias"))
        temp_hostgroup->alias = string::dup(value);
      else if (!strcmp(variable, "members")) {
        if (strcmp(value, XODTEMPLATE_NULL)) {
          if (temp_hostgroup->members == NULL)
            temp_hostgroup->members = string::dup(value);
          else {
            temp_hostgroup->members = resize_string(
                temp_hostgroup->members,
                strlen(temp_hostgroup->members) + strlen(value) + 2);
            strcat(temp_hostgroup->members, ",");
            strcat(temp_hostgroup->members, value);
          }
          if (temp_hostgroup->members == NULL)
            result = ERROR;
        }
        temp_hostgroup->have_members = true;
      } else if (!strcmp(variable, "hostgroup_members")) {
        if (strcmp(value, XODTEMPLATE_NULL)) {
          if (temp_hostgroup->hostgroup_members == NULL)
            temp_hostgroup->hostgroup_members = string::dup(value);
          else {
            temp_hostgroup->hostgroup_members = resize_string(
                temp_hostgroup->hostgroup_members,
                strlen(temp_hostgroup->hostgroup_members) + strlen(value) + 2);
            strcat(temp_hostgroup->hostgroup_members, ",");
            strcat(temp_hostgroup->hostgroup_members, value);
          }
          if (temp_hostgroup->hostgroup_members == NULL)
            result = ERROR;
        }
        temp_hostgroup->have_hostgroup_members = true;
      } else if (!strcmp(variable, "notes")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_hostgroup->notes = string::dup(value);
        temp_hostgroup->have_notes = true;
      } else if (!strcmp(variable, "notes_url")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_hostgroup->notes_url = string::dup(value);
        temp_hostgroup->have_notes_url = true;
      } else if (!strcmp(variable, "action_url")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_hostgroup->action_url = string::dup(value);
        temp_hostgroup->have_action_url = true;
      } else if (!strcmp(variable, "register"))
        temp_hostgroup->register_object = (atoi(value) > 0) ? true : false;
      else {
        logger(log_config_error, basic)
            << "Error: Invalid hostgroup object directive '" << variable
            << "'.";
        return ERROR;
      }

      break;

    case XODTEMPLATE_SERVICEGROUP:
      temp_servicegroup = (xodtemplate_servicegroup*)xodtemplate_current_object;

      if (!strcmp(variable, "use"))
        temp_servicegroup->tmpl = string::dup(value);
      else if (!strcmp(variable, "name")) {
        temp_servicegroup->name = string::dup(value);

        /* add servicegroup to template skiplist for fast searches */
        result =
            skiplist_insert(xobject_template_skiplists[X_SERVICEGROUP_SKIPLIST],
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
                << "', starting on line " << temp_servicegroup->_start_line
                << ")";
            result = ERROR;
            break;

          default:
            result = ERROR;
            break;
        }
      } else if (!strcmp(variable, "servicegroup_name")) {
        temp_servicegroup->servicegroup_name = string::dup(value);

        /* add servicegroup to template skiplist for fast searches */
        result = skiplist_insert(xobject_skiplists[X_SERVICEGROUP_SKIPLIST],
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
                << "', starting on line " << temp_servicegroup->_start_line
                << ")";
            result = ERROR;
            break;

          default:
            result = ERROR;
            break;
        }
      } else if (!strcmp(variable, "alias"))
        temp_servicegroup->alias = string::dup(value);
      else if (!strcmp(variable, "members")) {
        if (strcmp(value, XODTEMPLATE_NULL)) {
          if (temp_servicegroup->members == NULL)
            temp_servicegroup->members = string::dup(value);
          else {
            temp_servicegroup->members = resize_string(
                temp_servicegroup->members,
                strlen(temp_servicegroup->members) + strlen(value) + 2);
            strcat(temp_servicegroup->members, ",");
            strcat(temp_servicegroup->members, value);
          }
          if (temp_servicegroup->members == NULL)
            result = ERROR;
        }
        temp_servicegroup->have_members = true;
      } else if (!strcmp(variable, "servicegroup_members")) {
        if (strcmp(value, XODTEMPLATE_NULL)) {
          if (temp_servicegroup->servicegroup_members == NULL)
            temp_servicegroup->servicegroup_members = string::dup(value);
          else {
            temp_servicegroup->servicegroup_members =
                resize_string(temp_servicegroup->servicegroup_members,
                              strlen(temp_servicegroup->servicegroup_members) +
                                  strlen(value) + 2);
            strcat(temp_servicegroup->servicegroup_members, ",");
            strcat(temp_servicegroup->servicegroup_members, value);
          }
          if (temp_servicegroup->servicegroup_members == NULL)
            result = ERROR;
        }
        temp_servicegroup->have_servicegroup_members = true;
      } else if (!strcmp(variable, "notes")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_servicegroup->notes = string::dup(value);
        temp_servicegroup->have_notes = true;
      } else if (!strcmp(variable, "notes_url")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_servicegroup->notes_url = string::dup(value);
        temp_servicegroup->have_notes_url = true;
      } else if (!strcmp(variable, "action_url")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_servicegroup->action_url = string::dup(value);
        temp_servicegroup->have_action_url = true;
      } else if (!strcmp(variable, "register"))
        temp_servicegroup->register_object = (atoi(value) > 0) ? true : false;
      else {
        logger(log_config_error, basic)
            << "Error: Invalid servicegroup object directive '" << variable
            << "'.";
        return ERROR;
      }

      break;

    case XODTEMPLATE_SERVICEDEPENDENCY:
      temp_servicedependency =
          (xodtemplate_servicedependency*)xodtemplate_current_object;

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
                << "Warning: Duplicate definition found for service dependency "
                   "'"
                << variable << "' (config file '"
                << xodtemplate_config_file_name(
                       temp_servicedependency->_config_file)
                << "', starting on line " << temp_servicedependency->_start_line
                << ")";
            result = ERROR;
            break;

          default:
            result = ERROR;
            break;
        }
      } else if (!strcmp(variable, "servicegroup") ||
                 !strcmp(variable, "servicegroups") ||
                 !strcmp(variable, "servicegroup_name")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_servicedependency->servicegroup_name = string::dup(value);
        temp_servicedependency->have_servicegroup_name = true;
      } else if (!strcmp(variable, "hostgroup") ||
                 !strcmp(variable, "hostgroups") ||
                 !strcmp(variable, "hostgroup_name")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_servicedependency->hostgroup_name = string::dup(value);
        temp_servicedependency->have_hostgroup_name = true;
      } else if (!strcmp(variable, "host") || !strcmp(variable, "host_name") ||
                 !strcmp(variable, "master_host") ||
                 !strcmp(variable, "master_host_name")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_servicedependency->host_name = string::dup(value);
        temp_servicedependency->have_host_name = true;
      } else if (!strcmp(variable, "description") ||
                 !strcmp(variable, "service_description") ||
                 !strcmp(variable, "master_description") ||
                 !strcmp(variable, "master_service_description")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_servicedependency->service_description = string::dup(value);
        temp_servicedependency->have_service_description = true;
      } else if (!strcmp(variable, "dependent_servicegroup") ||
                 !strcmp(variable, "dependent_servicegroups") ||
                 !strcmp(variable, "dependent_servicegroup_name")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_servicedependency->dependent_servicegroup_name =
              string::dup(value);
        temp_servicedependency->have_dependent_servicegroup_name = true;
      } else if (!strcmp(variable, "dependent_hostgroup") ||
                 !strcmp(variable, "dependent_hostgroups") ||
                 !strcmp(variable, "dependent_hostgroup_name")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_servicedependency->dependent_hostgroup_name = string::dup(value);
        temp_servicedependency->have_dependent_hostgroup_name = true;
      } else if (!strcmp(variable, "dependent_host") ||
                 !strcmp(variable, "dependent_host_name")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_servicedependency->dependent_host_name = string::dup(value);
        temp_servicedependency->have_dependent_host_name = true;

        /* NOTE: dependencies are added to the skiplist in
         * xodtemplate_duplicate_objects(), except if daemon is using precached
         * config */
        if (result == OK && force_skiplists == true &&
            temp_servicedependency->dependent_host_name != NULL &&
            temp_servicedependency->dependent_service_description != NULL) {
          /* add servicedependency to template skiplist for fast searches */
          result =
              skiplist_insert(xobject_skiplists[X_SERVICEDEPENDENCY_SKIPLIST],
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
      } else if (!strcmp(variable, "dependent_description") ||
                 !strcmp(variable, "dependent_service_description")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_servicedependency->dependent_service_description =
              string::dup(value);
        temp_servicedependency->have_dependent_service_description = true;

        /* NOTE: dependencies are added to the skiplist in
         * xodtemplate_duplicate_objects(), except if daemon is using precached
         * config */
        if (result == OK && force_skiplists == true &&
            temp_servicedependency->dependent_host_name != NULL &&
            temp_servicedependency->dependent_service_description != NULL) {
          /* add servicedependency to template skiplist for fast searches */
          result =
              skiplist_insert(xobject_skiplists[X_SERVICEDEPENDENCY_SKIPLIST],
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
      } else if (!strcmp(variable, "dependency_period")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_servicedependency->dependency_period = string::dup(value);
        temp_servicedependency->have_dependency_period = true;
      } else if (!strcmp(variable, "inherits_parent")) {
        temp_servicedependency->inherits_parent =
            (atoi(value) > 0) ? true : false;
        temp_servicedependency->have_inherits_parent = true;
      } else if (!strcmp(variable, "execution_failure_options") ||
                 !strcmp(variable, "execution_failure_criteria")) {
        for (temp_ptr = strtok(value, ", "); temp_ptr != NULL;
             temp_ptr = strtok(NULL, ", ")) {
          if (!strcmp(temp_ptr, "o") || !strcmp(temp_ptr, "ok"))
            temp_servicedependency->fail_execute_on_ok = true;
          else if (!strcmp(temp_ptr, "u") || !strcmp(temp_ptr, "unknown"))
            temp_servicedependency->fail_execute_on_unknown = true;
          else if (!strcmp(temp_ptr, "w") || !strcmp(temp_ptr, "warning"))
            temp_servicedependency->fail_execute_on_warning = true;
          else if (!strcmp(temp_ptr, "c") || !strcmp(temp_ptr, "critical"))
            temp_servicedependency->fail_execute_on_critical = true;
          else if (!strcmp(temp_ptr, "p") || !strcmp(temp_ptr, "pending"))
            temp_servicedependency->fail_execute_on_pending = true;
          else if (!strcmp(temp_ptr, "n") || !strcmp(temp_ptr, "none")) {
            temp_servicedependency->fail_execute_on_ok = false;
            temp_servicedependency->fail_execute_on_unknown = false;
            temp_servicedependency->fail_execute_on_warning = false;
            temp_servicedependency->fail_execute_on_critical = false;
            temp_servicedependency->fail_execute_on_pending = false;
          } else if (!strcmp(temp_ptr, "a") || !strcmp(temp_ptr, "all")) {
            temp_servicedependency->fail_execute_on_ok = true;
            temp_servicedependency->fail_execute_on_unknown = true;
            temp_servicedependency->fail_execute_on_warning = true;
            temp_servicedependency->fail_execute_on_critical = true;
            temp_servicedependency->fail_execute_on_pending = true;
          } else {
            logger(log_config_error, basic)
                << "Error: Invalid execution dependency option '" << temp_ptr
                << "' in servicedependency definition.";
            return ERROR;
          }
        }
        temp_servicedependency->have_execution_dependency_options = true;
      } else if (!strcmp(variable, "notification_failure_options") ||
                 !strcmp(variable, "notification_failure_criteria")) {
        for (temp_ptr = strtok(value, ", "); temp_ptr != NULL;
             temp_ptr = strtok(NULL, ", ")) {
          if (!strcmp(temp_ptr, "o") || !strcmp(temp_ptr, "ok"))
            temp_servicedependency->fail_notify_on_ok = true;
          else if (!strcmp(temp_ptr, "u") || !strcmp(temp_ptr, "unknown"))
            temp_servicedependency->fail_notify_on_unknown = true;
          else if (!strcmp(temp_ptr, "w") || !strcmp(temp_ptr, "warning"))
            temp_servicedependency->fail_notify_on_warning = true;
          else if (!strcmp(temp_ptr, "c") || !strcmp(temp_ptr, "critical"))
            temp_servicedependency->fail_notify_on_critical = true;
          else if (!strcmp(temp_ptr, "p") || !strcmp(temp_ptr, "pending"))
            temp_servicedependency->fail_notify_on_pending = true;
          else if (!strcmp(temp_ptr, "n") || !strcmp(temp_ptr, "none")) {
            temp_servicedependency->fail_notify_on_ok = false;
            temp_servicedependency->fail_notify_on_unknown = false;
            temp_servicedependency->fail_notify_on_warning = false;
            temp_servicedependency->fail_notify_on_critical = false;
            temp_servicedependency->fail_notify_on_pending = false;
          } else if (!strcmp(temp_ptr, "a") || !strcmp(temp_ptr, "all")) {
            temp_servicedependency->fail_notify_on_ok = true;
            temp_servicedependency->fail_notify_on_unknown = true;
            temp_servicedependency->fail_notify_on_warning = true;
            temp_servicedependency->fail_notify_on_critical = true;
            temp_servicedependency->fail_notify_on_pending = true;
          } else {
            logger(log_config_error, basic)
                << "Error: Invalid notification dependency option '" << temp_ptr
                << "' in servicedependency definition.";
            return ERROR;
          }
        }
        temp_servicedependency->have_notification_dependency_options = true;
      } else if (!strcmp(variable, "register"))
        temp_servicedependency->register_object =
            (atoi(value) > 0) ? true : false;
      else {
        logger(log_config_error, basic)
            << "Error: Invalid servicedependency object directive '" << variable
            << "'.";
        return ERROR;
      }
      break;

    case XODTEMPLATE_SERVICEESCALATION:
      temp_serviceescalation =
          (xodtemplate_serviceescalation*)xodtemplate_current_object;

      if (!strcmp(variable, "use"))
        temp_serviceescalation->tmpl = string::dup(value);
      else if (!strcmp(variable, "name")) {
        temp_serviceescalation->name = string::dup(value);

        /* add escalation to template skiplist for fast searches */
        result = skiplist_insert(
            xobject_template_skiplists[X_SERVICEESCALATION_SKIPLIST],
            (void*)temp_serviceescalation);
        switch (result) {
          case SKIPLIST_OK:
            result = OK;
            break;

          case SKIPLIST_ERROR_DUPLICATE:
            logger(log_config_warning, basic)
                << "Warning: Duplicate definition found for service escalation "
                   "'"
                << value << "' (config file '"
                << xodtemplate_config_file_name(
                       temp_serviceescalation->_config_file)
                << "', starting on line " << temp_serviceescalation->_start_line
                << ")";
            result = ERROR;
            break;

          default:
            result = ERROR;
            break;
        }
      } else if (!strcmp(variable, "host") || !strcmp(variable, "host_name")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_serviceescalation->host_name = string::dup(value);
        temp_serviceescalation->have_host_name = true;

        /* NOTE: escalations are added to the skiplist in
         * xodtemplate_duplicate_objects(), except if daemon is using precached
         * config */
        if (result == OK && force_skiplists == true &&
            temp_serviceescalation->host_name != NULL &&
            temp_serviceescalation->service_description != NULL) {
          /* add serviceescalation to template skiplist for fast searches */
          result =
              skiplist_insert(xobject_skiplists[X_SERVICEESCALATION_SKIPLIST],
                              (void*)temp_serviceescalation);
          switch (result) {
            case SKIPLIST_OK:
              result = OK;
              break;

            default:
              result = ERROR;
              break;
          }
        }
      } else if (!strcmp(variable, "description") ||
                 !strcmp(variable, "service_description")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_serviceescalation->service_description = string::dup(value);
        temp_serviceescalation->have_service_description = true;

        /* NOTE: escalations are added to the skiplist in
         * xodtemplate_duplicate_objects(), except if daemon is using precached
         * config */
        if (result == OK && force_skiplists == true &&
            temp_serviceescalation->host_name != NULL &&
            temp_serviceescalation->service_description != NULL) {
          /* add serviceescalation to template skiplist for fast searches */
          result =
              skiplist_insert(xobject_skiplists[X_SERVICEESCALATION_SKIPLIST],
                              (void*)temp_serviceescalation);
          switch (result) {
            case SKIPLIST_OK:
              result = OK;
              break;

            default:
              result = ERROR;
              break;
          }
        }
      } else if (!strcmp(variable, "servicegroup") ||
                 !strcmp(variable, "servicegroups") ||
                 !strcmp(variable, "servicegroup_name")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_serviceescalation->servicegroup_name = string::dup(value);
        temp_serviceescalation->have_servicegroup_name = true;
      } else if (!strcmp(variable, "hostgroup") ||
                 !strcmp(variable, "hostgroups") ||
                 !strcmp(variable, "hostgroup_name")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_serviceescalation->hostgroup_name = string::dup(value);
        temp_serviceescalation->have_hostgroup_name = true;
      } else if (!strcmp(variable, "contact_groups")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_serviceescalation->contact_groups = string::dup(value);
        temp_serviceescalation->have_contact_groups = true;
      } else if (!strcmp(variable, "contacts")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_serviceescalation->contacts = string::dup(value);
        temp_serviceescalation->have_contacts = true;
      } else if (!strcmp(variable, "escalation_period")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_serviceescalation->escalation_period = string::dup(value);
        temp_serviceescalation->have_escalation_period = true;
      } else if (!strcmp(variable, "first_notification")) {
        temp_serviceescalation->first_notification = atoi(value);
        temp_serviceescalation->have_first_notification = true;
      } else if (!strcmp(variable, "last_notification")) {
        temp_serviceescalation->last_notification = atoi(value);
        temp_serviceescalation->have_last_notification = true;
      } else if (!strcmp(variable, "notification_interval")) {
        temp_serviceescalation->notification_interval = strtod(value, NULL);
        temp_serviceescalation->have_notification_interval = true;
      } else if (!strcmp(variable, "escalation_options")) {
        for (temp_ptr = strtok(value, ", "); temp_ptr != NULL;
             temp_ptr = strtok(NULL, ", ")) {
          if (!strcmp(temp_ptr, "w") || !strcmp(temp_ptr, "warning"))
            temp_serviceescalation->escalate_on_warning = true;
          else if (!strcmp(temp_ptr, "u") || !strcmp(temp_ptr, "unknown"))
            temp_serviceescalation->escalate_on_unknown = true;
          else if (!strcmp(temp_ptr, "c") || !strcmp(temp_ptr, "critical"))
            temp_serviceescalation->escalate_on_critical = true;
          else if (!strcmp(temp_ptr, "r") || !strcmp(temp_ptr, "recovery"))
            temp_serviceescalation->escalate_on_recovery = true;
          else if (!strcmp(temp_ptr, "n") || !strcmp(temp_ptr, "none")) {
            temp_serviceescalation->escalate_on_warning = false;
            temp_serviceescalation->escalate_on_unknown = false;
            temp_serviceescalation->escalate_on_critical = false;
            temp_serviceescalation->escalate_on_recovery = false;
          } else if (!strcmp(temp_ptr, "a") || !strcmp(temp_ptr, "all")) {
            temp_serviceescalation->escalate_on_warning = true;
            temp_serviceescalation->escalate_on_unknown = true;
            temp_serviceescalation->escalate_on_critical = true;
            temp_serviceescalation->escalate_on_recovery = true;
          } else {
            logger(log_config_error, basic)
                << "Error: Invalid escalation option '" << temp_ptr
                << "' in serviceescalation definition.";
            return ERROR;
          }
        }
        temp_serviceescalation->have_escalation_options = true;
      } else if (!strcmp(variable, "register"))
        temp_serviceescalation->register_object =
            (atoi(value) > 0) ? true : false;
      else {
        logger(log_config_error, basic)
            << "Error: Invalid serviceescalation object directive '" << variable
            << "'.";
        return ERROR;
      }

      break;

    case XODTEMPLATE_HOSTDEPENDENCY:
      temp_hostdependency =
          (xodtemplate_hostdependency*)xodtemplate_current_object;

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
                << xodtemplate_config_file_name(
                       temp_hostdependency->_config_file)
                << "', starting on line " << temp_hostdependency->_start_line
                << ")";
            result = ERROR;
            break;

          default:
            result = ERROR;
            break;
        }
      } else if (!strcmp(variable, "hostgroup") ||
                 !strcmp(variable, "hostgroups") ||
                 !strcmp(variable, "hostgroup_name")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_hostdependency->hostgroup_name = string::dup(value);
        temp_hostdependency->have_hostgroup_name = true;
      } else if (!strcmp(variable, "host") || !strcmp(variable, "host_name") ||
                 !strcmp(variable, "master_host") ||
                 !strcmp(variable, "master_host_name")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_hostdependency->host_name = string::dup(value);
        temp_hostdependency->have_host_name = true;
      } else if (!strcmp(variable, "dependent_hostgroup") ||
                 !strcmp(variable, "dependent_hostgroups") ||
                 !strcmp(variable, "dependent_hostgroup_name")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_hostdependency->dependent_hostgroup_name = string::dup(value);
        temp_hostdependency->have_dependent_hostgroup_name = true;
      } else if (!strcmp(variable, "dependent_host") ||
                 !strcmp(variable, "dependent_host_name")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_hostdependency->dependent_host_name = string::dup(value);
        temp_hostdependency->have_dependent_host_name = true;

        /* NOTE: dependencies are added to the skiplist in
         * xodtemplate_duplicate_objects(), except if daemon is using precached
         * config */
        if (result == OK && force_skiplists == true) {
          /* add hostdependency to template skiplist for fast searches */
          result = skiplist_insert(xobject_skiplists[X_HOSTDEPENDENCY_SKIPLIST],
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
      } else if (!strcmp(variable, "dependency_period")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_hostdependency->dependency_period = string::dup(value);
        temp_hostdependency->have_dependency_period = true;
      } else if (!strcmp(variable, "inherits_parent")) {
        temp_hostdependency->inherits_parent = (atoi(value) > 0) ? true : false;
        temp_hostdependency->have_inherits_parent = true;
      } else if (!strcmp(variable, "notification_failure_options") ||
                 !strcmp(variable, "notification_failure_criteria")) {
        for (temp_ptr = strtok(value, ", "); temp_ptr != NULL;
             temp_ptr = strtok(NULL, ", ")) {
          if (!strcmp(temp_ptr, "o") || !strcmp(temp_ptr, "up"))
            temp_hostdependency->fail_notify_on_up = true;
          else if (!strcmp(temp_ptr, "d") || !strcmp(temp_ptr, "down"))
            temp_hostdependency->fail_notify_on_down = true;
          else if (!strcmp(temp_ptr, "u") || !strcmp(temp_ptr, "unreachable"))
            temp_hostdependency->fail_notify_on_unreachable = true;
          else if (!strcmp(temp_ptr, "p") || !strcmp(temp_ptr, "pending"))
            temp_hostdependency->fail_notify_on_pending = true;
          else if (!strcmp(temp_ptr, "n") || !strcmp(temp_ptr, "none")) {
            temp_hostdependency->fail_notify_on_up = false;
            temp_hostdependency->fail_notify_on_down = false;
            temp_hostdependency->fail_notify_on_unreachable = false;
            temp_hostdependency->fail_notify_on_pending = false;
          } else if (!strcmp(temp_ptr, "a") || !strcmp(temp_ptr, "all")) {
            temp_hostdependency->fail_notify_on_up = true;
            temp_hostdependency->fail_notify_on_down = true;
            temp_hostdependency->fail_notify_on_unreachable = true;
            temp_hostdependency->fail_notify_on_pending = true;
          } else {
            logger(log_config_error, basic)
                << "Error: Invalid notification dependency option '" << temp_ptr
                << "' in hostdependency definition.";
            return ERROR;
          }
        }
        temp_hostdependency->have_notification_dependency_options = true;
      } else if (!strcmp(variable, "execution_failure_options") ||
                 !strcmp(variable, "execution_failure_criteria")) {
        for (temp_ptr = strtok(value, ", "); temp_ptr != NULL;
             temp_ptr = strtok(NULL, ", ")) {
          if (!strcmp(temp_ptr, "o") || !strcmp(temp_ptr, "up"))
            temp_hostdependency->fail_execute_on_up = true;
          else if (!strcmp(temp_ptr, "d") || !strcmp(temp_ptr, "down"))
            temp_hostdependency->fail_execute_on_down = true;
          else if (!strcmp(temp_ptr, "u") || !strcmp(temp_ptr, "unreachable"))
            temp_hostdependency->fail_execute_on_unreachable = true;
          else if (!strcmp(temp_ptr, "p") || !strcmp(temp_ptr, "pending"))
            temp_hostdependency->fail_execute_on_pending = true;
          else if (!strcmp(temp_ptr, "n") || !strcmp(temp_ptr, "none")) {
            temp_hostdependency->fail_execute_on_up = false;
            temp_hostdependency->fail_execute_on_down = false;
            temp_hostdependency->fail_execute_on_unreachable = false;
            temp_hostdependency->fail_execute_on_pending = false;
          } else if (!strcmp(temp_ptr, "a") || !strcmp(temp_ptr, "all")) {
            temp_hostdependency->fail_execute_on_up = true;
            temp_hostdependency->fail_execute_on_down = true;
            temp_hostdependency->fail_execute_on_unreachable = true;
            temp_hostdependency->fail_execute_on_pending = true;
          } else {
            logger(log_config_error, basic)
                << "Error: Invalid execution dependency option '" << temp_ptr
                << "' in hostdependency definition.";
            return ERROR;
          }
        }
        temp_hostdependency->have_execution_dependency_options = true;
      } else if (!strcmp(variable, "register"))
        temp_hostdependency->register_object = (atoi(value) > 0) ? true : false;
      else {
        logger(log_config_error, basic)
            << "Error: Invalid hostdependency object directive '" << variable
            << "'.";
        return ERROR;
      }
      break;

    case XODTEMPLATE_HOSTESCALATION:
      temp_hostescalation =
          (xodtemplate_hostescalation*)xodtemplate_current_object;

      if (!strcmp(variable, "use"))
        temp_hostescalation->tmpl = string::dup(value);
      else if (!strcmp(variable, "name")) {
        temp_hostescalation->name = string::dup(value);

        /* add escalation to template skiplist for fast searches */
        result = skiplist_insert(
            xobject_template_skiplists[X_HOSTESCALATION_SKIPLIST],
            (void*)temp_hostescalation);
        switch (result) {
          case SKIPLIST_OK:
            result = OK;
            break;

          case SKIPLIST_ERROR_DUPLICATE:
            logger(log_config_warning, basic)
                << "Warning: Duplicate definition found for host escalation '"
                << value << "' (config file '"
                << xodtemplate_config_file_name(
                       temp_hostescalation->_config_file)
                << "', starting on line " << temp_hostescalation->_start_line
                << ")";
            result = ERROR;
            break;

          default:
            result = ERROR;
            break;
        }
      } else if (!strcmp(variable, "hostgroup") ||
                 !strcmp(variable, "hostgroups") ||
                 !strcmp(variable, "hostgroup_name")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_hostescalation->hostgroup_name = string::dup(value);
        temp_hostescalation->have_hostgroup_name = true;
      } else if (!strcmp(variable, "host") || !strcmp(variable, "host_name")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_hostescalation->host_name = string::dup(value);
        temp_hostescalation->have_host_name = true;

        /* NOTE: escalations are added to the skiplist in
         * xodtemplate_duplicate_objects(), except if daemon is using precached
         * config */
        if (result == OK && force_skiplists == true) {
          /* add hostescalation to template skiplist for fast searches */
          result = skiplist_insert(xobject_skiplists[X_HOSTESCALATION_SKIPLIST],
                                   (void*)temp_hostescalation);
          switch (result) {
            case SKIPLIST_OK:
              result = OK;
              break;

            default:
              result = ERROR;
              break;
          }
        }
      } else if (!strcmp(variable, "contact_groups")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_hostescalation->contact_groups = string::dup(value);
        temp_hostescalation->have_contact_groups = true;
      } else if (!strcmp(variable, "contacts")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_hostescalation->contacts = string::dup(value);
        temp_hostescalation->have_contacts = true;
      } else if (!strcmp(variable, "escalation_period")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_hostescalation->escalation_period = string::dup(value);
        temp_hostescalation->have_escalation_period = true;
      } else if (!strcmp(variable, "first_notification")) {
        temp_hostescalation->first_notification = atoi(value);
        temp_hostescalation->have_first_notification = true;
      } else if (!strcmp(variable, "last_notification")) {
        temp_hostescalation->last_notification = atoi(value);
        temp_hostescalation->have_last_notification = true;
      } else if (!strcmp(variable, "notification_interval")) {
        temp_hostescalation->notification_interval = strtod(value, NULL);
        temp_hostescalation->have_notification_interval = true;
      } else if (!strcmp(variable, "escalation_options")) {
        for (temp_ptr = strtok(value, ", "); temp_ptr != NULL;
             temp_ptr = strtok(NULL, ", ")) {
          if (!strcmp(temp_ptr, "d") || !strcmp(temp_ptr, "down"))
            temp_hostescalation->escalate_on_down = true;
          else if (!strcmp(temp_ptr, "u") || !strcmp(temp_ptr, "unreachable"))
            temp_hostescalation->escalate_on_unreachable = true;
          else if (!strcmp(temp_ptr, "r") || !strcmp(temp_ptr, "recovery"))
            temp_hostescalation->escalate_on_recovery = true;
          else if (!strcmp(temp_ptr, "n") || !strcmp(temp_ptr, "none")) {
            temp_hostescalation->escalate_on_down = false;
            temp_hostescalation->escalate_on_unreachable = false;
            temp_hostescalation->escalate_on_recovery = false;
          } else if (!strcmp(temp_ptr, "a") || !strcmp(temp_ptr, "all")) {
            temp_hostescalation->escalate_on_down = true;
            temp_hostescalation->escalate_on_unreachable = true;
            temp_hostescalation->escalate_on_recovery = true;
          } else {
            logger(log_config_error, basic)
                << "Error: Invalid escalation option '" << temp_ptr
                << "' in hostescalation definition.";
            return ERROR;
          }
        }
        temp_hostescalation->have_escalation_options = true;
      } else if (!strcmp(variable, "register"))
        temp_hostescalation->register_object = (atoi(value) > 0) ? true : false;
      else {
        logger(log_config_error, basic)
            << "Error: Invalid hostescalation object directive '" << variable
            << "'.";
        return ERROR;
      }
      break;

    case XODTEMPLATE_HOSTEXTINFO:
      temp_hostextinfo = xodtemplate_hostextinfo_list;

      if (!strcmp(variable, "use"))
        temp_hostextinfo->tmpl = string::dup(value);
      else if (!strcmp(variable, "name")) {
        temp_hostextinfo->name = string::dup(value);

        /* add to template skiplist for fast searches */
        result =
            skiplist_insert(xobject_template_skiplists[X_HOSTEXTINFO_SKIPLIST],
                            (void*)temp_hostextinfo);
        switch (result) {
          case SKIPLIST_OK:
            result = OK;
            break;

          case SKIPLIST_ERROR_DUPLICATE:
            logger(log_config_warning, basic)
                << "Warning: Duplicate definition found for extended host info "
                   "'"
                << value << "' (config file '"
                << xodtemplate_config_file_name(temp_hostextinfo->_config_file)
                << "', starting on line " << temp_hostextinfo->_start_line
                << ")";
            result = ERROR;
            break;

          default:
            result = ERROR;
            break;
        }
      } else if (!strcmp(variable, "host_name")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_hostextinfo->host_name = string::dup(value);
        temp_hostextinfo->have_host_name = true;
      } else if (!strcmp(variable, "hostgroup") ||
                 !strcmp(variable, "hostgroup_name")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_hostextinfo->hostgroup_name = string::dup(value);
        temp_hostextinfo->have_hostgroup_name = true;
      } else if (!strcmp(variable, "notes")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_hostextinfo->notes = string::dup(value);
        temp_hostextinfo->have_notes = true;
      } else if (!strcmp(variable, "notes_url")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_hostextinfo->notes_url = string::dup(value);
        temp_hostextinfo->have_notes_url = true;
      } else if (!strcmp(variable, "action_url")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_hostextinfo->action_url = string::dup(value);
        temp_hostextinfo->have_action_url = true;
      } else if (!strcmp(variable, "icon_image")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_hostextinfo->icon_image = string::dup(value);
        temp_hostextinfo->have_icon_image = true;
      } else if (!strcmp(variable, "icon_image_alt")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_hostextinfo->icon_image_alt = string::dup(value);
        temp_hostextinfo->have_icon_image_alt = true;
      } else if (!strcmp(variable, "vrml_image")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_hostextinfo->vrml_image = string::dup(value);
        temp_hostextinfo->have_vrml_image = true;
      } else if (!strcmp(variable, "gd2_image") ||
                 !strcmp(variable, "statusmap_image")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_hostextinfo->statusmap_image = string::dup(value);
        temp_hostextinfo->have_statusmap_image = true;
      } else if (!strcmp(variable, "2d_coords")) {
        temp_ptr = strtok(value, ", ");
        if (temp_ptr == NULL) {
          logger(log_config_error, basic)
              << "Error: Invalid 2d_coords value '" << temp_ptr
              << "' in extended host info definition.";
          return ERROR;
        }
        temp_hostextinfo->x_2d = atoi(temp_ptr);
        temp_ptr = strtok(NULL, ", ");
        if (temp_ptr == NULL) {
          logger(log_config_error, basic)
              << "Error: Invalid 2d_coords value '" << temp_ptr
              << "' in extended host info definition.";
          return ERROR;
        }
        temp_hostextinfo->y_2d = atoi(temp_ptr);
        temp_hostextinfo->have_2d_coords = true;
      } else if (!strcmp(variable, "3d_coords")) {
        temp_ptr = strtok(value, ", ");
        if (temp_ptr == NULL) {
          logger(log_config_error, basic)
              << "Error: Invalid 3d_coords value '" << temp_ptr
              << "' in extended host info definition.";
          return ERROR;
        }
        temp_hostextinfo->x_3d = strtod(temp_ptr, NULL);
        temp_ptr = strtok(NULL, ", ");
        if (temp_ptr == NULL) {
          logger(log_config_error, basic)
              << "Error: Invalid 3d_coords value '" << temp_ptr
              << "' in extended host info definition.";
          return ERROR;
        }
        temp_hostextinfo->y_3d = strtod(temp_ptr, NULL);
        temp_ptr = strtok(NULL, ", ");
        if (temp_ptr == NULL) {
          logger(log_config_error, basic)
              << "Error: Invalid 3d_coords value '" << temp_ptr
              << "' in extended host info definition.";
          return ERROR;
        }
        temp_hostextinfo->z_3d = strtod(temp_ptr, NULL);
        temp_hostextinfo->have_3d_coords = true;
      } else if (!strcmp(variable, "register"))
        temp_hostextinfo->register_object = (atoi(value) > 0) ? true : false;
      else {
        logger(log_config_error, basic)
            << "Error: Invalid hostextinfo object directive '" << variable
            << "'.";
        return ERROR;
      }
      break;

    case XODTEMPLATE_SERVICEEXTINFO:
      temp_serviceextinfo = xodtemplate_serviceextinfo_list;

      if (!strcmp(variable, "use"))
        temp_serviceextinfo->tmpl = string::dup(value);
      else if (!strcmp(variable, "name")) {
        temp_serviceextinfo->name = string::dup(value);

        /* add to template skiplist for fast searches */
        result = skiplist_insert(
            xobject_template_skiplists[X_SERVICEEXTINFO_SKIPLIST],
            (void*)temp_serviceextinfo);
        switch (result) {
          case SKIPLIST_OK:
            result = OK;
            break;

          case SKIPLIST_ERROR_DUPLICATE:
            logger(log_config_warning, basic)
                << "Warning: Duplicate definition found for extended service "
                   "info '"
                << value << "' (config file '"
                << xodtemplate_config_file_name(
                       temp_serviceextinfo->_config_file)
                << "', starting on line " << temp_serviceextinfo->_start_line
                << ")";
            result = ERROR;
            break;

          default:
            result = ERROR;
            break;
        }
      } else if (!strcmp(variable, "host_name")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_serviceextinfo->host_name = string::dup(value);
        temp_serviceextinfo->have_host_name = true;
      } else if (!strcmp(variable, "hostgroup") ||
                 !strcmp(variable, "hostgroup_name")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_serviceextinfo->hostgroup_name = string::dup(value);
        temp_serviceextinfo->have_hostgroup_name = true;
      } else if (!strcmp(variable, "service_description")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_serviceextinfo->service_description = string::dup(value);
        temp_serviceextinfo->have_service_description = true;
      } else if (!strcmp(variable, "notes")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_serviceextinfo->notes = string::dup(value);
        temp_serviceextinfo->have_notes = true;
      } else if (!strcmp(variable, "notes_url")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_serviceextinfo->notes_url = string::dup(value);
        temp_serviceextinfo->have_notes_url = true;
      } else if (!strcmp(variable, "action_url")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_serviceextinfo->action_url = string::dup(value);
        temp_serviceextinfo->have_action_url = true;
      } else if (!strcmp(variable, "icon_image")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_serviceextinfo->icon_image = string::dup(value);
        temp_serviceextinfo->have_icon_image = true;
      } else if (!strcmp(variable, "icon_image_alt")) {
        if (strcmp(value, XODTEMPLATE_NULL))
          temp_serviceextinfo->icon_image_alt = string::dup(value);
        temp_serviceextinfo->have_icon_image_alt = true;
      } else if (!strcmp(variable, "register"))
        temp_serviceextinfo->register_object = (atoi(value) > 0) ? true : false;
      else {
        logger(log_config_error, basic)
            << "Error: Invalid serviceextinfo object directive '" << variable
            << "'.";
        return ERROR;
      }
      break;

    default:
      return ERROR;
      break;
  }

  /* free memory */
  delete[] variable;
  delete[] value;

  return result;
}

/* completes an object definition */
int xodtemplate_end_object_definition(int options) {
  (void)options;

  xodtemplate_current_object = NULL;
  xodtemplate_current_object_type = XODTEMPLATE_NONE;

  return OK;
}

/* adds a custom variable to a host */
xodtemplate_customvariablesmember* xodtemplate_add_custom_variable_to_host(
    xodtemplate_host* hst,
    char* varname,
    char* varvalue) {
  return (xodtemplate_add_custom_variable_to_object(&hst->custom_variables,
                                                    varname, varvalue));
}

/* adds a custom variable to a service */
xodtemplate_customvariablesmember* xodtemplate_add_custom_variable_to_service(
    xodtemplate_service* svc,
    char* varname,
    char* varvalue) {
  return (xodtemplate_add_custom_variable_to_object(&svc->custom_variables,
                                                    varname, varvalue));
}

/* adds a custom variable to a contact */
xodtemplate_customvariablesmember* xodtemplate_add_custom_variable_to_contact(
    xodtemplate_contact* cntct,
    char* varname,
    char* varvalue) {
  return (xodtemplate_add_custom_variable_to_object(&cntct->custom_variables,
                                                    varname, varvalue));
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
    return NULL;

  if (varname == NULL || !strcmp(varname, ""))
    return NULL;

  /* allocate memory for a new member */
  new_customvariablesmember = new xodtemplate_customvariablesmember;
  new_customvariablesmember->variable_name = string::dup(varname);
  if (varvalue)
    new_customvariablesmember->variable_value = string::dup(varvalue);
  else
    new_customvariablesmember->variable_value = NULL;

  /* convert varname to all uppercase (saves CPU time during macro functions) */
  for (x = 0; new_customvariablesmember->variable_name[x] != '\x0'; x++)
    new_customvariablesmember->variable_name[x] =
        toupper(new_customvariablesmember->variable_name[x]);

  /* add the new member to the head of the member list */
  new_customvariablesmember->next = *object_ptr;
  *object_ptr = new_customvariablesmember;

  return new_customvariablesmember;
}

/* parses a timeperod directive... :-) */
int xodtemplate_parse_timeperiod_directive(xodtemplate_timeperiod* tperiod,
                                           char const* var,
                                           char const* val) {
  char* input = NULL;
  char temp_buffer[5][MAX_INPUT_BUFFER] = {"", "", "", "", ""};
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
    return ERROR;

  /* we'll need the full (unsplit) input later */
  input = new char[strlen(var) + strlen(val) + 2];

  strcpy(input, var);
  strcat(input, " ");
  strcat(input, val);

  if (0)
    return OK;
  /* calendar dates */
  else if (sscanf(input, "%4d-%2d-%2d - %4d-%2d-%2d / %d %[0-9:, -]", &syear,
                  &smon, &smday, &eyear, &emon, &emday, &skip_interval,
                  temp_buffer[0]) == 8) {
    /* add timerange exception */
    if (xodtemplate_add_exception_to_timeperiod(
            tperiod, DATERANGE_CALENDAR_DATE, syear, smon - 1, smday, 0, 0,
            eyear, emon - 1, emday, 0, 0, skip_interval,
            temp_buffer[0]) == NULL)
      result = ERROR;
  } else if (sscanf(input, "%4d-%2d-%2d / %d %[0-9:, -]", &syear, &smon, &smday,
                    &skip_interval, temp_buffer[0]) == 5) {
    eyear = syear;
    emon = smon;
    emday = smday;
    /* add timerange exception */
    if (xodtemplate_add_exception_to_timeperiod(
            tperiod, DATERANGE_CALENDAR_DATE, syear, smon - 1, smday, 0, 0,
            eyear, emon - 1, emday, 0, 0, skip_interval,
            temp_buffer[0]) == NULL)
      result = ERROR;
  } else if (sscanf(input, "%4d-%2d-%2d - %4d-%2d-%2d %[0-9:, -]", &syear,
                    &smon, &smday, &eyear, &emon, &emday,
                    temp_buffer[0]) == 7) {
    /* add timerange exception */
    if (xodtemplate_add_exception_to_timeperiod(
            tperiod, DATERANGE_CALENDAR_DATE, syear, smon - 1, smday, 0, 0,
            eyear, emon - 1, emday, 0, 0, 0, temp_buffer[0]) == NULL)
      result = ERROR;
  } else if (sscanf(input, "%4d-%2d-%2d %[0-9:, -]", &syear, &smon, &smday,
                    temp_buffer[0]) == 4) {
    eyear = syear;
    emon = smon;
    emday = smday;
    /* add timerange exception */
    if (xodtemplate_add_exception_to_timeperiod(
            tperiod, DATERANGE_CALENDAR_DATE, syear, smon - 1, smday, 0, 0,
            eyear, emon - 1, emday, 0, 0, 0, temp_buffer[0]) == NULL)
      result = ERROR;
  }
  /* other types... */
  else if (sscanf(input, "%[a-z] %d %[a-z] - %[a-z] %d %[a-z] / %d %[0-9:, -]",
                  temp_buffer[0], &swday_offset, temp_buffer[1], temp_buffer[2],
                  &ewday_offset, temp_buffer[3], &skip_interval,
                  temp_buffer[4]) == 8) {
    /* wednesday 1 january - thursday 2 july / 3 */
    if ((result = xodtemplate_get_weekday_from_string(temp_buffer[0],
                                                      &swday)) == OK &&
        (result = xodtemplate_get_month_from_string(temp_buffer[1], &smon)) ==
            OK &&
        (result = xodtemplate_get_weekday_from_string(temp_buffer[2],
                                                      &ewday)) == OK &&
        (result = xodtemplate_get_month_from_string(temp_buffer[3], &emon)) ==
            OK) {
      /* add timeperiod exception */
      if (xodtemplate_add_exception_to_timeperiod(
              tperiod, DATERANGE_MONTH_WEEK_DAY, 0, smon, 0, swday,
              swday_offset, 0, emon, 0, ewday, ewday_offset, skip_interval,
              temp_buffer[4]) == NULL)
        result = ERROR;
    }
  } else if (sscanf(input, "%[a-z] %d - %[a-z] %d / %d %[0-9:, -]",
                    temp_buffer[0], &smday, temp_buffer[1], &emday,
                    &skip_interval, temp_buffer[2]) == 6) {
    /* february 1 - march 15 / 3 */
    /* monday 2 - thursday 3 / 2 */
    /* day 4 - day 6 / 2 */
    if ((result = xodtemplate_get_weekday_from_string(temp_buffer[0],
                                                      &swday)) == OK &&
        (result = xodtemplate_get_weekday_from_string(temp_buffer[1],
                                                      &ewday)) == OK) {
      /* monday 2 - thursday 3 / 2 */
      swday_offset = smday;
      ewday_offset = emday;
      /* add timeperiod exception */
      if (xodtemplate_add_exception_to_timeperiod(
              tperiod, DATERANGE_WEEK_DAY, 0, 0, 0, swday, swday_offset, 0, 0,
              0, ewday, ewday_offset, skip_interval, temp_buffer[2]) == NULL)
        result = ERROR;
    } else if ((result = xodtemplate_get_month_from_string(temp_buffer[0],
                                                           &smon)) == OK &&
               (result = xodtemplate_get_month_from_string(temp_buffer[1],
                                                           &emon)) == OK) {
      /* february 1 - march 15 / 3 */
      /* add timeperiod exception */
      if (xodtemplate_add_exception_to_timeperiod(
              tperiod, DATERANGE_MONTH_DATE, 0, smon, smday, 0, 0, 0, emon,
              emday, 0, 0, skip_interval, temp_buffer[2]) == NULL)
        result = ERROR;
    } else if (!strcmp(temp_buffer[0], "day") &&
               !strcmp(temp_buffer[1], "day")) {
      /* day 4 - 6 / 2 */
      /* add timeperiod exception */
      result = OK;
      if (xodtemplate_add_exception_to_timeperiod(
              tperiod, DATERANGE_MONTH_DAY, 0, 0, smday, 0, 0, 0, 0, emday, 0,
              0, skip_interval, temp_buffer[2]) == NULL)
        result = ERROR;
    }
  } else if (sscanf(input, "%[a-z] %d - %d / %d %[0-9:, -]", temp_buffer[0],
                    &smday, &emday, &skip_interval, temp_buffer[1]) == 5) {
    /* february 1 - 15 / 3 */
    /* monday 2 - 3 / 2 */
    /* day 1 - 25 / 4 */
    if ((result = xodtemplate_get_weekday_from_string(temp_buffer[0],
                                                      &swday)) == OK) {
      /* thursday 2 - 4 */
      swday_offset = smday;
      ewday = swday;
      ewday_offset = emday;
      /* add timeperiod exception */
      if (xodtemplate_add_exception_to_timeperiod(
              tperiod, DATERANGE_WEEK_DAY, 0, 0, 0, swday, swday_offset, 0, 0,
              0, ewday, ewday_offset, skip_interval, temp_buffer[1]) == NULL)
        result = ERROR;
    } else if ((result = xodtemplate_get_month_from_string(temp_buffer[0],
                                                           &smon)) == OK) {
      /* february 3 - 5 */
      emon = smon;
      /* add timeperiod exception */
      if (xodtemplate_add_exception_to_timeperiod(
              tperiod, DATERANGE_MONTH_DATE, 0, smon, smday, 0, 0, 0, emon,
              emday, 0, 0, skip_interval, temp_buffer[1]) == NULL)
        result = ERROR;
    } else if (!strcmp(temp_buffer[0], "day")) {
      /* day 1 - 4 */
      /* add timeperiod exception */
      result = OK;
      if (xodtemplate_add_exception_to_timeperiod(
              tperiod, DATERANGE_MONTH_DAY, 0, 0, smday, 0, 0, 0, 0, emday, 0,
              0, skip_interval, temp_buffer[1]) == NULL)
        result = ERROR;
    }
  } else if (sscanf(input, "%[a-z] %d %[a-z] - %[a-z] %d %[a-z] %[0-9:, -]",
                    temp_buffer[0], &swday_offset, temp_buffer[1],
                    temp_buffer[2], &ewday_offset, temp_buffer[3],
                    temp_buffer[4]) == 7) {
    /* wednesday 1 january - thursday 2 july */
    if ((result = xodtemplate_get_weekday_from_string(temp_buffer[0],
                                                      &swday)) == OK &&
        (result = xodtemplate_get_month_from_string(temp_buffer[1], &smon)) ==
            OK &&
        (result = xodtemplate_get_weekday_from_string(temp_buffer[2],
                                                      &ewday)) == OK &&
        (result = xodtemplate_get_month_from_string(temp_buffer[3], &emon)) ==
            OK) {
      /* add timeperiod exception */
      if (xodtemplate_add_exception_to_timeperiod(
              tperiod, DATERANGE_MONTH_WEEK_DAY, 0, smon, 0, swday,
              swday_offset, 0, emon, 0, ewday, ewday_offset, 0,
              temp_buffer[4]) == NULL)
        result = ERROR;
    }
  } else if (sscanf(input, "%[a-z] %d - %d %[0-9:, -]", temp_buffer[0], &smday,
                    &emday, temp_buffer[1]) == 4) {
    /* february 3 - 5 */
    /* thursday 2 - 4 */
    /* day 1 - 4 */
    if ((result = xodtemplate_get_weekday_from_string(temp_buffer[0],
                                                      &swday)) == OK) {
      /* thursday 2 - 4 */
      swday_offset = smday;
      ewday = swday;
      ewday_offset = emday;
      /* add timeperiod exception */
      if (xodtemplate_add_exception_to_timeperiod(
              tperiod, DATERANGE_WEEK_DAY, 0, 0, 0, swday, swday_offset, 0, 0,
              0, ewday, ewday_offset, 0, temp_buffer[1]) == NULL)
        result = ERROR;
    } else if ((result = xodtemplate_get_month_from_string(temp_buffer[0],
                                                           &smon)) == OK) {
      /* february 3 - 5 */
      emon = smon;
      /* add timeperiod exception */
      if (xodtemplate_add_exception_to_timeperiod(
              tperiod, DATERANGE_MONTH_DATE, 0, smon, smday, 0, 0, 0, emon,
              emday, 0, 0, 0, temp_buffer[1]) == NULL)
        result = ERROR;
    } else if (!strcmp(temp_buffer[0], "day")) {
      /* day 1 - 4 */
      /* add timeperiod exception */
      result = OK;
      if (xodtemplate_add_exception_to_timeperiod(
              tperiod, DATERANGE_MONTH_DAY, 0, 0, smday, 0, 0, 0, 0, emday, 0,
              0, 0, temp_buffer[1]) == NULL)
        result = ERROR;
    }
  } else if (sscanf(input, "%[a-z] %d - %[a-z] %d %[0-9:, -]", temp_buffer[0],
                    &smday, temp_buffer[1], &emday, temp_buffer[2]) == 5) {
    /* february 1 - march 15 */
    /* monday 2 - thursday 3 */
    /* day 1 - day 5 */
    if ((result = xodtemplate_get_weekday_from_string(temp_buffer[0],
                                                      &swday)) == OK &&
        (result = xodtemplate_get_weekday_from_string(temp_buffer[1],
                                                      &ewday)) == OK) {
      /* monday 2 - thursday 3 */
      swday_offset = smday;
      ewday_offset = emday;
      /* add timeperiod exception */
      if (xodtemplate_add_exception_to_timeperiod(
              tperiod, DATERANGE_WEEK_DAY, 0, 0, 0, swday, swday_offset, 0, 0,
              0, ewday, ewday_offset, 0, temp_buffer[2]) == NULL)
        result = ERROR;
    } else if ((result = xodtemplate_get_month_from_string(temp_buffer[0],
                                                           &smon)) == OK &&
               (result = xodtemplate_get_month_from_string(temp_buffer[1],
                                                           &emon)) == OK) {
      /* february 1 - march 15 */
      /* add timeperiod exception */
      if (xodtemplate_add_exception_to_timeperiod(
              tperiod, DATERANGE_MONTH_DATE, 0, smon, smday, 0, 0, 0, emon,
              emday, 0, 0, 0, temp_buffer[2]) == NULL)
        result = ERROR;
    } else if (!strcmp(temp_buffer[0], "day") &&
               !strcmp(temp_buffer[1], "day")) {
      /* day 1 - day 5 */
      /* add timeperiod exception */
      result = OK;
      if (xodtemplate_add_exception_to_timeperiod(
              tperiod, DATERANGE_MONTH_DAY, 0, 0, smday, 0, 0, 0, 0, emday, 0,
              0, 0, temp_buffer[2]) == NULL)
        result = ERROR;
    }
  } else if (sscanf(input, "%[a-z] %d%*[ \t]%[0-9:, -]", temp_buffer[0], &smday,
                    temp_buffer[1]) == 3) {
    /* february 3 */
    /* thursday 2 */
    /* day 1 */
    if ((result = xodtemplate_get_weekday_from_string(temp_buffer[0],
                                                      &swday)) == OK) {
      /* thursday 2 */
      swday_offset = smday;
      ewday = swday;
      ewday_offset = swday_offset;
      /* add timeperiod exception */
      if (xodtemplate_add_exception_to_timeperiod(
              tperiod, DATERANGE_WEEK_DAY, 0, 0, 0, swday, swday_offset, 0, 0,
              0, ewday, ewday_offset, 0, temp_buffer[1]) == NULL)
        result = ERROR;
    } else if ((result = xodtemplate_get_month_from_string(temp_buffer[0],
                                                           &smon)) == OK) {
      /* february 3 */
      emon = smon;
      emday = smday;
      /* add timeperiod exception */
      if (xodtemplate_add_exception_to_timeperiod(
              tperiod, DATERANGE_MONTH_DATE, 0, smon, smday, 0, 0, 0, emon,
              emday, 0, 0, 0, temp_buffer[1]) == NULL)
        result = ERROR;
    } else if (!strcmp(temp_buffer[0], "day")) {
      /* day 1 */
      emday = smday;
      /* add timeperiod exception */
      result = OK;
      if (xodtemplate_add_exception_to_timeperiod(
              tperiod, DATERANGE_MONTH_DAY, 0, 0, smday, 0, 0, 0, 0, emday, 0,
              0, 0, temp_buffer[1]) == NULL)
        result = ERROR;
    }
  } else if (sscanf(input, "%[a-z] %d %[a-z] %[0-9:, -]", temp_buffer[0],
                    &swday_offset, temp_buffer[1], temp_buffer[2]) == 4) {
    /* thursday 3 february */
    if ((result = xodtemplate_get_weekday_from_string(temp_buffer[0],
                                                      &swday)) == OK &&
        (result = xodtemplate_get_month_from_string(temp_buffer[1], &smon)) ==
            OK) {
      emon = smon;
      ewday = swday;
      ewday_offset = swday_offset;
      /* add timeperiod exception */
      if (xodtemplate_add_exception_to_timeperiod(
              tperiod, DATERANGE_MONTH_WEEK_DAY, 0, smon, 0, swday,
              swday_offset, 0, emon, 0, ewday, ewday_offset, 0,
              temp_buffer[2]) == NULL)
        result = ERROR;
    }
  } else if (sscanf(input, "%[a-z] %[0-9:, -]", temp_buffer[0],
                    temp_buffer[1]) == 2) {
    /* monday */
    if ((result = xodtemplate_get_weekday_from_string(temp_buffer[0],
                                                      &swday)) == OK) {
      /* add normal weekday timerange */
      tperiod->timeranges[swday] = string::dup(temp_buffer[1]);
    }
  } else
    result = ERROR;

  /* free memory */
  delete[] input;

  if (result == ERROR) {
    printf("Error: Could not parse timeperiod directive '%s'!", input);
    return ERROR;
  }

  return OK;
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
  xodtemplate_daterange* new_daterange = NULL;

  /* make sure we have the data we need */
  if (period == NULL || timeranges == NULL)
    return NULL;

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

  /* add the new date range to the head of the range list for this exception
   * type */
  new_daterange->next = period->exceptions[type];
  period->exceptions[type] = new_daterange;
  return new_daterange;
}

int xodtemplate_get_month_from_string(char* str, int* month) {
  static char const* months[12] = {
      "january", "february", "march",     "april",   "may",      "june",
      "july",    "august",   "september", "october", "november", "december"};

  if (str == NULL || month == NULL)
    return ERROR;

  for (int x = 0; x < 12; x++) {
    if (!strcmp(str, months[x])) {
      *month = x;
      return OK;
    }
  }
  return ERROR;
}

int xodtemplate_get_weekday_from_string(char* str, int* weekday) {
  static char const* days[7] = {"sunday",   "monday", "tuesday", "wednesday",
                                "thursday", "friday", "saturday"};

  if (str == NULL || weekday == NULL)
    return ERROR;

  for (int x = 0; x < 7; x++) {
    if (!strcmp(str, days[x])) {
      *weekday = x;
      return OK;
    }
  }
  return ERROR;
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

  /****** DUPLICATE SERVICE DEFINITIONS WITH ONE OR MORE HOSTGROUP AND/OR HOST
   * NAMES ******/
  for (temp_service = xodtemplate_service_list; temp_service != NULL;
       temp_service = temp_service->next) {
    /* skip service definitions without enough data */
    if (temp_service->hostgroup_name == NULL && temp_service->host_name == NULL)
      continue;

    /* If hostgroup is not null and hostgroup has no members, check to see if */
    /* allow_empty_hostgroup_assignment is set to 1 - if it is, continue without
     * error  */
    if (temp_service->hostgroup_name != NULL) {
      if (xodtemplate_expand_hostgroups(
              &temp_memberlist, &temp_rejectlist, temp_service->hostgroup_name,
              temp_service->_config_file, temp_service->_start_line) == ERROR) {
        return ERROR;
      } else {
        xodtemplate_free_memberlist(&temp_rejectlist);
        if (temp_memberlist != NULL)
          xodtemplate_free_memberlist(&temp_memberlist);
        else {
          /* User is ok with hostgroup -> service mappings with no hosts */
          if (config->allow_empty_hostgroup_assignment() == 1) {
            continue;
          }
        }
      }
    }

    /* skip services that shouldn't be registered */
    if (temp_service->register_object == false)
      continue;

    /* get list of hosts */
    temp_memberlist = xodtemplate_expand_hostgroups_and_hosts(
        temp_service->hostgroup_name, temp_service->host_name,
        temp_service->_config_file, temp_service->_start_line);
    if (temp_memberlist == NULL) {
      logger(log_config_error, basic)
          << "Error: Could not expand hostgroups and/or hosts specified in "
             "service (config file '"
          << xodtemplate_config_file_name(temp_service->_config_file)
          << "', starting on line " << temp_service->_start_line << ")";
      return ERROR;
    }

    /* add a copy of the service for every host in the hostgroup/host name list
     */
    first_item = true;
    for (this_memberlist = temp_memberlist; this_memberlist != NULL;
         this_memberlist = this_memberlist->next) {
      /* if this is the first duplication, use the existing entry */
      if (first_item == true) {
        delete[] temp_service->host_name;
        temp_service->host_name = string::dup(this_memberlist->name1);

        first_item = false;
        continue;
      }

      /* duplicate service definition */
      result =
          xodtemplate_duplicate_service(temp_service, this_memberlist->name1);

      /* exit on error */
      if (result == ERROR) {
        delete[] host_name;
        return ERROR;
      }
    }

    /* free memory we used for host list */
    xodtemplate_free_memberlist(&temp_memberlist);
  }

  /***************************************/
  /* SKIPLIST STUFF FOR FAST SORT/SEARCH */
  /***************************************/

  /* First loop for single host service definition */
  for (temp_service = xodtemplate_service_list; temp_service != NULL;
       temp_service = temp_service->next) {
    /* skip services that shouldn't be registered */
    if (temp_service->register_object == false)
      continue;

    /* skip service definitions without enough data */
    if (temp_service->host_name == NULL ||
        temp_service->service_description == NULL)
      continue;

    if (xodtemplate_is_service_is_from_hostgroup(temp_service)) {
      continue;
    }

    result = skiplist_insert(xobject_skiplists[X_SERVICE_SKIPLIST],
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
  for (temp_service = xodtemplate_service_list; temp_service != NULL;
       temp_service = temp_service->next) {
    /* skip services that shouldn't be registered */
    if (temp_service->register_object == false)
      continue;

    /* skip service definitions without enough data */
    if (temp_service->host_name == NULL ||
        temp_service->service_description == NULL)
      continue;

    if (!xodtemplate_is_service_is_from_hostgroup(temp_service)) {
      continue;
    }
    /*The flag X_SERVICE_IS_FROM_HOSTGROUP is set, unset it */
    xodtemplate_unset_service_is_from_hostgroup(temp_service);

    result = skiplist_insert(xobject_skiplists[X_SERVICE_SKIPLIST],
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

  return OK;
}

/* duplicates object definitions */
int xodtemplate_duplicate_objects() {
  int result = OK;
  xodtemplate_hostescalation* temp_hostescalation = NULL;
  xodtemplate_serviceescalation* temp_serviceescalation = NULL;
  xodtemplate_hostdependency* temp_hostdependency = NULL;
  xodtemplate_servicedependency* temp_servicedependency = NULL;
  xodtemplate_hostextinfo* temp_hostextinfo = NULL;
  xodtemplate_serviceextinfo* temp_serviceextinfo = NULL;
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

  /****** DUPLICATE HOST ESCALATION DEFINITIONS WITH ONE OR MORE HOSTGROUP
   * AND/OR HOST NAMES ******/
  for (temp_hostescalation = xodtemplate_hostescalation_list;
       temp_hostescalation != NULL;
       temp_hostescalation = temp_hostescalation->next) {
    /* skip host escalation definitions without enough data */
    if (temp_hostescalation->hostgroup_name == NULL &&
        temp_hostescalation->host_name == NULL)
      continue;

    /* get list of hosts */
    master_hostlist = xodtemplate_expand_hostgroups_and_hosts(
        temp_hostescalation->hostgroup_name, temp_hostescalation->host_name,
        temp_hostescalation->_config_file, temp_hostescalation->_start_line);
    if (master_hostlist == NULL) {
      logger(log_config_error, basic)
          << "Error: Could not expand hostgroups and/or hosts specified "
             "in host escalation (config file '"
          << xodtemplate_config_file_name(temp_hostescalation->_config_file)
          << "', starting on line " << temp_hostescalation->_start_line << ")";
      return ERROR;
    }

    /* add a copy of the hostescalation for every host in the hostgroup/host
     * name list */
    first_item = true;
    for (temp_masterhost = master_hostlist; temp_masterhost != NULL;
         temp_masterhost = temp_masterhost->next) {
      /* if this is the first duplication, use the existing entry */
      if (first_item == true) {
        delete[] temp_hostescalation->host_name;
        temp_hostescalation->host_name = string::dup(temp_masterhost->name1);
        first_item = false;
        continue;
      }

      /* duplicate hostescalation definition */
      result = xodtemplate_duplicate_hostescalation(temp_hostescalation,
                                                    temp_masterhost->name1);
      /* exit on error */
      if (result == ERROR) {
        xodtemplate_free_memberlist(&master_hostlist);
        return ERROR;
      }
    }

    /* free memory we used for host list */
    xodtemplate_free_memberlist(&master_hostlist);
  }

  /****** DUPLICATE SERVICE ESCALATION DEFINITIONS WITH ONE OR MORE HOSTGROUP
   * AND/OR HOST NAMES ******/
  for (temp_serviceescalation = xodtemplate_serviceescalation_list;
       temp_serviceescalation != NULL;
       temp_serviceescalation = temp_serviceescalation->next) {
    /* skip service escalation definitions without enough data */
    if (temp_serviceescalation->hostgroup_name == NULL &&
        temp_serviceescalation->host_name == NULL)
      continue;

    /* get list of hosts */
    master_hostlist = xodtemplate_expand_hostgroups_and_hosts(
        temp_serviceescalation->hostgroup_name,
        temp_serviceescalation->host_name, temp_serviceescalation->_config_file,
        temp_serviceescalation->_start_line);
    if (master_hostlist == NULL) {
      logger(log_config_error, basic)
          << "Error: Could not expand hostgroups and/or hosts specified "
             "in service escalation (config file '"
          << xodtemplate_config_file_name(temp_serviceescalation->_config_file)
          << "', starting on line " << temp_serviceescalation->_start_line
          << ")";
      return ERROR;
    }

    /* duplicate service escalation entries */
    first_item = true;
    for (temp_masterhost = master_hostlist; temp_masterhost != NULL;
         temp_masterhost = temp_masterhost->next) {
      /* if this is the first duplication,use the existing entry */
      if (first_item == true) {
        delete[] temp_serviceescalation->host_name;
        temp_serviceescalation->host_name = string::dup(temp_masterhost->name1);
        first_item = false;
        continue;
      }

      /* duplicate service escalation definition */
      result = xodtemplate_duplicate_serviceescalation(
          temp_serviceescalation, temp_masterhost->name1,
          temp_serviceescalation->service_description);
      /* exit on error */
      if (result == ERROR) {
        xodtemplate_free_memberlist(&master_hostlist);
        return ERROR;
      }
    }

    /* free memory we used for host list */
    xodtemplate_free_memberlist(&master_hostlist);
  }

  /****** DUPLICATE SERVICE ESCALATION DEFINITIONS WITH MULTIPLE DESCRIPTIONS
   * ******/
  /* THIS MUST BE DONE AFTER DUPLICATING FOR MULTIPLE HOST NAMES (SEE ABOVE) */
  for (temp_serviceescalation = xodtemplate_serviceescalation_list;
       temp_serviceescalation != NULL;
       temp_serviceescalation = temp_serviceescalation->next) {
    /* skip serviceescalations without enough data */
    if (temp_serviceescalation->service_description == NULL ||
        temp_serviceescalation->host_name == NULL)
      continue;

    /* get list of services */
    master_servicelist = xodtemplate_expand_servicegroups_and_services(
        NULL, temp_serviceescalation->host_name,
        temp_serviceescalation->service_description,
        temp_serviceescalation->_config_file,
        temp_serviceescalation->_start_line);
    if (master_servicelist == NULL) {
      logger(log_config_error, basic)
          << "Error: Could not expand services specified in service "
             "escalation (config file '"
          << xodtemplate_config_file_name(temp_serviceescalation->_config_file)
          << "', starting on line " << temp_serviceescalation->_start_line
          << ")";
      return ERROR;
    }

    /* duplicate service escalation entries */
    first_item = true;
    for (temp_masterservice = master_servicelist; temp_masterservice != NULL;
         temp_masterservice = temp_masterservice->next) {
      /* if this is the first duplication, use the existing entry */
      if (first_item == true) {
        delete[] temp_serviceescalation->service_description;
        temp_serviceescalation->service_description =
            string::dup(temp_masterservice->name2);
        first_item = false;
        continue;
      }

      /* duplicate service escalation definition */
      result = xodtemplate_duplicate_serviceescalation(
          temp_serviceescalation, temp_serviceescalation->host_name,
          temp_masterservice->name2);
      /* exit on error */
      if (result == ERROR) {
        xodtemplate_free_memberlist(&master_servicelist);
        return ERROR;
      }
    }

    /* free memory we used for service list */
    xodtemplate_free_memberlist(&master_servicelist);
  }

  /****** DUPLICATE SERVICE ESCALATION DEFINITIONS WITH SERVICEGROUPS ******/
  /* THIS MUST BE DONE AFTER DUPLICATING FOR MULTIPLE HOST NAMES (SEE ABOVE) */
  for (temp_serviceescalation = xodtemplate_serviceescalation_list;
       temp_serviceescalation != NULL;
       temp_serviceescalation = temp_serviceescalation->next) {
    /* skip serviceescalations without enough data */
    if (temp_serviceescalation->servicegroup_name == NULL)
      continue;

    /* get list of services */
    master_servicelist = xodtemplate_expand_servicegroups_and_services(
        temp_serviceescalation->servicegroup_name, NULL, NULL,
        temp_serviceescalation->_config_file,
        temp_serviceescalation->_start_line);
    if (master_servicelist == NULL) {
      logger(log_config_error, basic)
          << "Error: Could not expand servicegroups specified in service "
             "escalation (config file '"
          << xodtemplate_config_file_name(temp_serviceescalation->_config_file)
          << "', starting on line " << temp_serviceescalation->_start_line
          << ")";
      return ERROR;
    }

    /* duplicate service escalation entries */
    first_item = true;
    for (temp_masterservice = master_servicelist; temp_masterservice != NULL;
         temp_masterservice = temp_masterservice->next) {
      /* if this is the first duplication, use the existing entry if possible */
      if (first_item == true && temp_serviceescalation->host_name == NULL &&
          temp_serviceescalation->service_description == NULL) {
        delete[] temp_serviceescalation->host_name;
        temp_serviceescalation->host_name =
            string::dup(temp_masterservice->name1);

        delete[] temp_serviceescalation->service_description;
        temp_serviceescalation->service_description =
            string::dup(temp_masterservice->name2);

        first_item = false;
        continue;
      }

      /* duplicate service escalation definition */
      result = xodtemplate_duplicate_serviceescalation(
          temp_serviceescalation, temp_masterservice->name1,
          temp_masterservice->name2);
      /* exit on error */
      if (result == ERROR) {
        xodtemplate_free_memberlist(&master_servicelist);
        return ERROR;
      }
    }

    /* free memory we used for service list */
    xodtemplate_free_memberlist(&master_servicelist);
  }

  /****** DUPLICATE HOST DEPENDENCY DEFINITIONS WITH MULTIPLE HOSTGROUP AND/OR
   * HOST NAMES (MASTER AND DEPENDENT) ******/
  for (temp_hostdependency = xodtemplate_hostdependency_list;
       temp_hostdependency != NULL;
       temp_hostdependency = temp_hostdependency->next) {
    /* skip host dependencies without enough data */
    if (temp_hostdependency->hostgroup_name == NULL &&
        temp_hostdependency->dependent_hostgroup_name == NULL &&
        temp_hostdependency->host_name == NULL &&
        temp_hostdependency->dependent_host_name == NULL)
      continue;

    /* get list of master host names */
    master_hostlist = xodtemplate_expand_hostgroups_and_hosts(
        temp_hostdependency->hostgroup_name, temp_hostdependency->host_name,
        temp_hostdependency->_config_file, temp_hostdependency->_start_line);
    if (master_hostlist == NULL) {
      logger(log_config_error, basic)
          << "Error: Could not expand master hostgroups and/or hosts "
             "specified in host dependency (config file '"
          << xodtemplate_config_file_name(temp_hostdependency->_config_file)
          << "', starting on line " << temp_hostdependency->_start_line << ")";
      return ERROR;
    }

    /* get list of dependent host names */
    dependent_hostlist = xodtemplate_expand_hostgroups_and_hosts(
        temp_hostdependency->dependent_hostgroup_name,
        temp_hostdependency->dependent_host_name,
        temp_hostdependency->_config_file, temp_hostdependency->_start_line);
    if (dependent_hostlist == NULL) {
      logger(log_config_error, basic)
          << "Error: Could not expand dependent hostgroups and/or hosts "
             "specified in host dependency (config file '"
          << xodtemplate_config_file_name(temp_hostdependency->_config_file)
          << "', starting on line " << temp_hostdependency->_start_line << ")";
      xodtemplate_free_memberlist(&master_hostlist);
      return ERROR;
    }

    /* duplicate the dependency definitions */
    first_item = true;
    for (temp_masterhost = master_hostlist; temp_masterhost != NULL;
         temp_masterhost = temp_masterhost->next) {
      for (temp_dependenthost = dependent_hostlist; temp_dependenthost != NULL;
           temp_dependenthost = temp_dependenthost->next) {
        /* temp=master, this=dep */

        /* existing definition gets first names */
        if (first_item == true) {
          delete[] temp_hostdependency->host_name;
          delete[] temp_hostdependency->dependent_host_name;
          temp_hostdependency->host_name = string::dup(temp_masterhost->name1);
          temp_hostdependency->dependent_host_name =
              string::dup(temp_dependenthost->name1);
          first_item = false;
          continue;
        } else
          result = xodtemplate_duplicate_hostdependency(
              temp_hostdependency, temp_masterhost->name1,
              temp_dependenthost->name1);
        /* exit on error */
        if (result == ERROR) {
          xodtemplate_free_memberlist(&master_hostlist);
          xodtemplate_free_memberlist(&dependent_hostlist);
          return ERROR;
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
          temp_servicedependency->servicegroup_name, NULL, NULL,
          temp_servicedependency->_config_file,
          temp_servicedependency->_start_line);
      if (master_servicelist == NULL) {
        logger(log_config_error, basic)
            << "Error: Could not expand master servicegroups "
               "specified in service dependency (config file '"
            << xodtemplate_config_file_name(
                   temp_servicedependency->_config_file)
            << "', starting on line " << temp_servicedependency->_start_line
            << ")";
        return ERROR;
      }

      /* if dependency also has master host, hostgroup, and/or service, we must
       * split that off to another definition */
      if (temp_servicedependency->host_name != NULL ||
          temp_servicedependency->hostgroup_name != NULL ||
          temp_servicedependency->service_description != NULL) {
        /* duplicate everything except master servicegroup */
        xodtemplate_duplicate_servicedependency(
            temp_servicedependency, temp_servicedependency->host_name,
            temp_servicedependency->service_description,
            temp_servicedependency->hostgroup_name, NULL,
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
      for (temp_masterservice = master_servicelist; temp_masterservice != NULL;
           temp_masterservice = temp_masterservice->next) {
        /* just in case */
        if (temp_masterservice->name1 == NULL ||
            temp_masterservice->name2 == NULL)
          continue;

        /* if this is the first duplication, use the existing entry */
        if (first_item == true) {
          delete[] temp_servicedependency->host_name;
          temp_servicedependency->host_name =
              string::dup(temp_masterservice->name1);

          delete[] temp_servicedependency->service_description;
          temp_servicedependency->service_description =
              string::dup(temp_masterservice->name2);

          /* clear the master servicegroup */
          temp_servicedependency->have_servicegroup_name = false;
          delete[] temp_servicedependency->servicegroup_name;
          temp_servicedependency->servicegroup_name = NULL;

          first_item = false;
          continue;
        }

        /* duplicate service dependency definition */
        result = xodtemplate_duplicate_servicedependency(
            temp_servicedependency, temp_masterservice->name1,
            temp_masterservice->name2, NULL, NULL,
            temp_servicedependency->dependent_host_name,
            temp_servicedependency->dependent_service_description,
            temp_servicedependency->dependent_hostgroup_name,
            temp_servicedependency->dependent_servicegroup_name);
        /* exit on error */
        if (result == ERROR) {
          xodtemplate_free_memberlist(&master_servicelist);
          return ERROR;
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
    if (temp_servicedependency->host_name != NULL ||
        temp_servicedependency->hostgroup_name != NULL) {
#ifdef DEBUG_SERVICE_DEPENDENCIES
      printf("1a) H: %s  HG: %s  SD: %s\n", temp_servicedependency->host_name,
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
            << "Error: Could not expand master hostgroups and/or hosts "
               "specified "
               "in service dependency (config file '"
            << xodtemplate_config_file_name(
                   temp_servicedependency->_config_file)
            << "', starting on line " << temp_servicedependency->_start_line
            << ")";
        return ERROR;
      }

      /* save service descriptions for later */
      if (temp_servicedependency->service_description)
        service_descriptions =
            string::dup(temp_servicedependency->service_description);

      /* for each host, expand master services */
      first_item = true;
      for (temp_masterhost = master_hostlist; temp_masterhost != NULL;
           temp_masterhost = temp_masterhost->next) {
        master_servicelist = xodtemplate_expand_servicegroups_and_services(
            NULL, temp_masterhost->name1, service_descriptions,
            temp_servicedependency->_config_file,
            temp_servicedependency->_start_line);
        if (master_servicelist == NULL) {
          logger(log_config_error, basic)
              << "Error: Could not expand master services specified in "
                 "service dependency (config file '"
              << xodtemplate_config_file_name(
                     temp_servicedependency->_config_file)
              << "', starting on line " << temp_servicedependency->_start_line
              << ")";
          return ERROR;
        }

        /* duplicate service dependency entries */
        for (temp_masterservice = master_servicelist;
             temp_masterservice != NULL;
             temp_masterservice = temp_masterservice->next) {
          /* just in case */
          if (temp_masterservice->name1 == NULL ||
              temp_masterservice->name2 == NULL)
            continue;

          /* if this is the first duplication, use the existing entry */
          if (first_item == true) {
            delete[] temp_servicedependency->host_name;
            temp_servicedependency->host_name =
                string::dup(temp_masterhost->name1);

            delete[] temp_servicedependency->service_description;
            temp_servicedependency->service_description =
                string::dup(temp_masterservice->name2);

            first_item = false;
            continue;
          }

          /* duplicate service dependency definition */
          result = xodtemplate_duplicate_servicedependency(
              temp_servicedependency, temp_masterhost->name1,
              temp_masterservice->name2, NULL, NULL,
              temp_servicedependency->dependent_host_name,
              temp_servicedependency->dependent_service_description,
              temp_servicedependency->dependent_hostgroup_name,
              temp_servicedependency->dependent_servicegroup_name);
          /* exit on error */
          if (result == ERROR) {
            xodtemplate_free_memberlist(&master_hostlist);
            xodtemplate_free_memberlist(&master_servicelist);
            return ERROR;
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
        "1**) H: %s  HG: %s  SG: %s  SD: %s  DH: %s  DHG: %s  DSG: %s  DSD: "
        "%s\n",
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
      dependent_servicelist = xodtemplate_expand_servicegroups_and_services(
          temp_servicedependency->dependent_servicegroup_name, NULL, NULL,
          temp_servicedependency->_config_file,
          temp_servicedependency->_start_line);
      if (dependent_servicelist == NULL) {
        logger(log_config_error, basic)
            << "Error: Could not expand dependent servicegroups "
               "specified in service dependency (config file '"
            << xodtemplate_config_file_name(
                   temp_servicedependency->_config_file)
            << "', starting on line " << temp_servicedependency->_start_line
            << ")";
        return ERROR;
      }

      /* if dependency also has dependent host, hostgroup, and/or service, we
       * must split that off to another definition */
      if (temp_servicedependency->dependent_host_name != NULL ||
          temp_servicedependency->dependent_hostgroup_name != NULL ||
          temp_servicedependency->dependent_service_description != NULL) {
        /* duplicate everything except dependent servicegroup */
        xodtemplate_duplicate_servicedependency(
            temp_servicedependency, temp_servicedependency->host_name,
            temp_servicedependency->service_description,
            temp_servicedependency->hostgroup_name,
            temp_servicedependency->servicegroup_name,
            temp_servicedependency->dependent_host_name,
            temp_servicedependency->dependent_service_description,
            temp_servicedependency->dependent_hostgroup_name, NULL);

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
      if (temp_servicedependency->host_name == NULL &&
          temp_servicedependency->hostgroup_name == NULL)
        same_host_servicedependency = true;

      /* duplicate service dependency entries */
      first_item = true;
      for (temp_dependentservice = dependent_servicelist;
           temp_dependentservice != NULL;
           temp_dependentservice = temp_dependentservice->next) {
        /* just in case */
        if (temp_dependentservice->name1 == NULL ||
            temp_dependentservice->name2 == NULL)
          continue;

        /* if this is the first duplication, use the existing entry */
        if (first_item == true) {
          delete[] temp_servicedependency->dependent_host_name;
          temp_servicedependency->dependent_host_name =
              string::dup(temp_dependentservice->name1);

          delete[] temp_servicedependency->dependent_service_description;
          temp_servicedependency->dependent_service_description =
              string::dup(temp_dependentservice->name2);

          /* Same host servicegroups dependencies: Use dependentservice
           * host_name for master host_name */
          if (same_host_servicedependency == true)
            temp_servicedependency->host_name =
                string::dup(temp_dependentservice->name1);

          /* clear the dependent servicegroup */
          temp_servicedependency->have_dependent_servicegroup_name = false;
          delete[] temp_servicedependency->dependent_servicegroup_name;
          temp_servicedependency->dependent_servicegroup_name = NULL;

          first_item = false;
          continue;
        }

        /* duplicate service dependency definition */
        /* Same host servicegroups dependencies: Use dependentservice host_name
         * for master host_name instead of undefined (not yet) master host_name
         */
        if (same_host_servicedependency == true)
          result = xodtemplate_duplicate_servicedependency(
              temp_servicedependency, temp_dependentservice->name1,
              temp_servicedependency->service_description, NULL, NULL,
              temp_dependentservice->name1, temp_dependentservice->name2, NULL,
              NULL);
        else
          result = xodtemplate_duplicate_servicedependency(
              temp_servicedependency, temp_servicedependency->host_name,
              temp_servicedependency->service_description, NULL, NULL,
              temp_dependentservice->name1, temp_dependentservice->name2, NULL,
              NULL);
        /* exit on error */
        if (result == ERROR) {
          xodtemplate_free_memberlist(&dependent_servicelist);
          return ERROR;
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
        "2**) H: %s  HG: %s  SG: %s  SD: %s  DH: %s  DHG: %s  DSG: %s  DSD: "
        "%s\n",
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
    if (temp_servicedependency->dependent_host_name == NULL &&
        temp_servicedependency->dependent_hostgroup_name == NULL) {
      if (temp_servicedependency->host_name)
        temp_servicedependency->dependent_host_name =
            string::dup(temp_servicedependency->host_name);
    }

    /* expand dependent hosts/hostgroups into a list of host names */
    if (temp_servicedependency->dependent_host_name != NULL ||
        temp_servicedependency->dependent_hostgroup_name != NULL) {
      dependent_hostlist = xodtemplate_expand_hostgroups_and_hosts(
          temp_servicedependency->dependent_hostgroup_name,
          temp_servicedependency->dependent_host_name,
          temp_servicedependency->_config_file,
          temp_servicedependency->_start_line);
      if (dependent_hostlist == NULL) {
        logger(log_config_error, basic)
            << "Error: Could not expand dependent hostgroups and/or "
               "hosts specified in service dependency (config file '"
            << xodtemplate_config_file_name(
                   temp_servicedependency->_config_file)
            << "', starting on line " << temp_servicedependency->_start_line
            << ")";
        return ERROR;
      }

      /* save service descriptions for later */
      if (temp_servicedependency->dependent_service_description)
        service_descriptions =
            string::dup(temp_servicedependency->dependent_service_description);

      /* for each host, expand dependent services */
      first_item = true;
      for (temp_dependenthost = dependent_hostlist; temp_dependenthost != NULL;
           temp_dependenthost = temp_dependenthost->next) {
        dependent_servicelist = xodtemplate_expand_servicegroups_and_services(
            NULL, temp_dependenthost->name1, service_descriptions,
            temp_servicedependency->_config_file,
            temp_servicedependency->_start_line);
        if (dependent_servicelist == NULL) {
          logger(log_config_error, basic)
              << "Error: Could not expand dependent services "
                 "specified in service dependency (config file '"
              << xodtemplate_config_file_name(
                     temp_servicedependency->_config_file)
              << "', starting on line " << temp_servicedependency->_start_line
              << ")";
          return ERROR;
        }

        /* duplicate service dependency entries */
        for (temp_dependentservice = dependent_servicelist;
             temp_dependentservice != NULL;
             temp_dependentservice = temp_dependentservice->next) {
          /* just in case */
          if (temp_dependentservice->name1 == NULL ||
              temp_dependentservice->name2 == NULL)
            continue;

          /* if this is the first duplication, use the existing entry */
          if (first_item == true) {
            delete[] temp_servicedependency->dependent_host_name;
            temp_servicedependency->dependent_host_name =
                string::dup(temp_dependentservice->name1);

            delete[] temp_servicedependency->dependent_service_description;
            temp_servicedependency->dependent_service_description =
                string::dup(temp_dependentservice->name2);

            first_item = false;
            continue;
          }

          /* duplicate service dependency definition */
          result = xodtemplate_duplicate_servicedependency(
              temp_servicedependency, temp_servicedependency->host_name,
              temp_servicedependency->service_description, NULL, NULL,
              temp_dependentservice->name1, temp_dependentservice->name2, NULL,
              NULL);
          /* exit on error */
          if (result == ERROR) {
            xodtemplate_free_memberlist(&dependent_servicelist);
            xodtemplate_free_memberlist(&dependent_hostlist);
            return ERROR;
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
    printf("3**)  MAS: %s/%s  DEP: %s/%s\n", temp_servicedependency->host_name,
           temp_servicedependency->service_description,
           temp_servicedependency->dependent_host_name,
           temp_servicedependency->dependent_service_description);
  }
#endif

  /****** DUPLICATE HOSTEXTINFO DEFINITIONS WITH ONE OR MORE HOSTGROUP AND/OR
   * HOST NAMES ******/
  for (temp_hostextinfo = xodtemplate_hostextinfo_list;
       temp_hostextinfo != NULL; temp_hostextinfo = temp_hostextinfo->next) {
    /* skip definitions without enough data */
    if (temp_hostextinfo->hostgroup_name == NULL &&
        temp_hostextinfo->host_name == NULL)
      continue;

    /* get list of hosts */
    master_hostlist = xodtemplate_expand_hostgroups_and_hosts(
        temp_hostextinfo->hostgroup_name, temp_hostextinfo->host_name,
        temp_hostextinfo->_config_file, temp_hostextinfo->_start_line);
    if (master_hostlist == NULL) {
      logger(log_config_error, basic)
          << "Error: Could not expand hostgroups and/or hosts "
             "specified in extended host info (config file '"
          << xodtemplate_config_file_name(temp_hostextinfo->_config_file)
          << "', starting on line " << temp_hostextinfo->_start_line << ")";
      return ERROR;
    }

    /* add a copy of the definition for every host in the hostgroup/host name
     * list */
    first_item = true;
    for (temp_masterhost = master_hostlist; temp_masterhost != NULL;
         temp_masterhost = temp_masterhost->next) {
      /* if this is the first duplication, use the existing entry */
      if (first_item == true) {
        delete[] temp_hostextinfo->host_name;
        temp_hostextinfo->host_name = string::dup(temp_masterhost->name1);

        first_item = false;
        continue;
      }

      /* duplicate hostextinfo definition */
      result = xodtemplate_duplicate_hostextinfo(temp_hostextinfo,
                                                 temp_masterhost->name1);

      /* exit on error */
      if (result == ERROR) {
        xodtemplate_free_memberlist(&master_hostlist);
        return ERROR;
      }
    }

    /* free memory we used for host list */
    xodtemplate_free_memberlist(&master_hostlist);
  }

  /****** DUPLICATE SERVICEEXTINFO DEFINITIONS WITH ONE OR MORE HOSTGROUP AND/OR
   * HOST NAMES ******/
  for (temp_serviceextinfo = xodtemplate_serviceextinfo_list;
       temp_serviceextinfo != NULL;
       temp_serviceextinfo = temp_serviceextinfo->next) {
    /* skip definitions without enough data */
    if (temp_serviceextinfo->hostgroup_name == NULL &&
        temp_serviceextinfo->host_name == NULL)
      continue;

    /* get list of hosts */
    master_hostlist = xodtemplate_expand_hostgroups_and_hosts(
        temp_serviceextinfo->hostgroup_name, temp_serviceextinfo->host_name,
        temp_serviceextinfo->_config_file, temp_serviceextinfo->_start_line);
    if (master_hostlist == NULL) {
      logger(log_config_error, basic)
          << "Error: Could not expand hostgroups and/or hosts "
             "specified in extended service info (config file '"
          << xodtemplate_config_file_name(temp_serviceextinfo->_config_file)
          << "', starting on line " << temp_serviceextinfo->_start_line << ")";
      return ERROR;
    }

    /* add a copy of the definition for every host in the hostgroup/host name
     * list */
    first_item = true;
    for (temp_masterhost = master_hostlist; temp_masterhost != NULL;
         temp_masterhost = temp_masterhost->next) {
      /* existing definition gets first host name */
      if (first_item == true) {
        delete[] temp_serviceextinfo->host_name;
        temp_serviceextinfo->host_name = string::dup(temp_masterhost->name1);

        first_item = false;
        continue;
      }

      /* duplicate serviceextinfo definition */
      result = xodtemplate_duplicate_serviceextinfo(temp_serviceextinfo,
                                                    temp_masterhost->name1);

      /* exit on error */
      if (result == ERROR) {
        xodtemplate_free_memberlist(&master_hostlist);
        return ERROR;
      }
    }

    /* free memory we used for host list */
    xodtemplate_free_memberlist(&master_hostlist);
  }

  /***************************************/
  /* SKIPLIST STUFF FOR FAST SORT/SEARCH */
  /***************************************/

  /* host escalations */
  for (temp_hostescalation = xodtemplate_hostescalation_list;
       temp_hostescalation != NULL;
       temp_hostescalation = temp_hostescalation->next) {
    /* skip escalations that shouldn't be registered */
    if (temp_hostescalation->register_object == false)
      continue;

    /* skip escalation definitions without enough data */
    if (temp_hostescalation->host_name == NULL)
      continue;

    result = skiplist_insert(xobject_skiplists[X_HOSTESCALATION_SKIPLIST],
                             (void*)temp_hostescalation);
    switch (result) {
      case SKIPLIST_OK:
        result = OK;
        break;

      default:
        result = ERROR;
        break;
    }
  }

  /* service escalations */
  for (temp_serviceescalation = xodtemplate_serviceescalation_list;
       temp_serviceescalation != NULL;
       temp_serviceescalation = temp_serviceescalation->next) {
    /* skip escalations that shouldn't be registered */
    if (temp_serviceescalation->register_object == false)
      continue;

    /* skip escalation definitions without enough data */
    if (temp_serviceescalation->host_name == NULL ||
        temp_serviceescalation->service_description == NULL)
      continue;

    result = skiplist_insert(xobject_skiplists[X_SERVICEESCALATION_SKIPLIST],
                             (void*)temp_serviceescalation);
    switch (result) {
      case SKIPLIST_OK:
        result = OK;
        break;

      default:
        result = ERROR;
        break;
    }
  }

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

    result = skiplist_insert(xobject_skiplists[X_HOSTDEPENDENCY_SKIPLIST],
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
    if (temp_servicedependency->dependent_host_name == NULL ||
        temp_servicedependency->dependent_service_description == NULL)
      continue;

    result = skiplist_insert(xobject_skiplists[X_SERVICEDEPENDENCY_SKIPLIST],
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

  /* host extinfo */
  /* NOT NEEDED */

  /* service extinfo */
  /* NOT NEEDED */

  return OK;
}

/* duplicates a service definition (with a new host name) */
int xodtemplate_duplicate_service(xodtemplate_service* temp_service,
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
  new_service->have_service_description =
      temp_service->have_service_description;
  new_service->display_name = NULL;
  new_service->have_display_name = temp_service->have_display_name;
  new_service->service_groups = NULL;
  new_service->have_service_groups = temp_service->have_service_groups;
  new_service->check_command = NULL;
  new_service->have_check_command = temp_service->have_check_command;
  new_service->check_period = NULL;
  new_service->have_check_period = temp_service->have_check_period;
  new_service->event_handler = NULL;
  new_service->have_event_handler = temp_service->have_event_handler;
  new_service->notification_period = NULL;
  new_service->have_notification_period =
      temp_service->have_notification_period;
  new_service->contact_groups = NULL;
  new_service->have_contact_groups = temp_service->have_contact_groups;
  new_service->contacts = NULL;
  new_service->have_contacts = temp_service->have_contacts;
  new_service->notes = NULL;
  new_service->have_notes = temp_service->have_notes;
  new_service->notes_url = NULL;
  new_service->have_notes_url = temp_service->have_notes_url;
  new_service->action_url = NULL;
  new_service->have_action_url = temp_service->have_action_url;
  new_service->icon_image = NULL;
  new_service->have_icon_image = temp_service->have_icon_image;
  new_service->icon_image_alt = NULL;
  new_service->have_icon_image_alt = temp_service->have_icon_image_alt;
  new_service->custom_variables = NULL;

  /* make sure hostgroup member in new service definition is NULL */
  new_service->hostgroup_name = NULL;

  /* allocate memory for and copy string members of service definition (host
   * name provided, DO NOT duplicate hostgroup member!) */
  if (temp_service->host_name != NULL)
    new_service->host_name = string::dup(host_name);
  if (temp_service->tmpl != NULL)
    new_service->tmpl = string::dup(temp_service->tmpl);
  if (temp_service->name != NULL)
    new_service->name = string::dup(temp_service->name);
  if (temp_service->service_description != NULL)
    new_service->service_description =
        string::dup(temp_service->service_description);
  if (temp_service->display_name != NULL)
    new_service->display_name = string::dup(temp_service->display_name);
  if (temp_service->service_groups != NULL)
    new_service->service_groups = string::dup(temp_service->service_groups);
  if (temp_service->check_command != NULL)
    new_service->check_command = string::dup(temp_service->check_command);
  if (temp_service->check_period != NULL)
    new_service->check_period = string::dup(temp_service->check_period);
  if (temp_service->event_handler != NULL)
    new_service->event_handler = string::dup(temp_service->event_handler);
  if (temp_service->notification_period != NULL)
    new_service->notification_period =
        string::dup(temp_service->notification_period);
  if (temp_service->contact_groups != NULL)
    new_service->contact_groups = string::dup(temp_service->contact_groups);
  if (temp_service->contacts != NULL)
    new_service->contacts = string::dup(temp_service->contacts);
  if (temp_service->notes != NULL)
    new_service->notes = string::dup(temp_service->notes);
  if (temp_service->notes_url != NULL)
    new_service->notes_url = string::dup(temp_service->notes_url);
  if (temp_service->action_url != NULL)
    new_service->action_url = string::dup(temp_service->action_url);
  if (temp_service->icon_image != NULL)
    new_service->icon_image = string::dup(temp_service->icon_image);
  if (temp_service->icon_image_alt != NULL)
    new_service->icon_image_alt = string::dup(temp_service->icon_image_alt);

  /* duplicate custom variables */
  for (temp_customvariablesmember = temp_service->custom_variables;
       temp_customvariablesmember != NULL;
       temp_customvariablesmember = temp_customvariablesmember->next)
    xodtemplate_add_custom_variable_to_service(
        new_service, temp_customvariablesmember->variable_name,
        temp_customvariablesmember->variable_value);

  /* duplicate non-string members */
  new_service->initial_state = temp_service->initial_state;
  new_service->max_check_attempts = temp_service->max_check_attempts;
  new_service->have_max_check_attempts = temp_service->have_max_check_attempts;
  new_service->check_interval = temp_service->check_interval;
  new_service->have_check_interval = temp_service->have_check_interval;
  new_service->retry_interval = temp_service->retry_interval;
  new_service->have_retry_interval = temp_service->have_retry_interval;
  new_service->active_checks_enabled = temp_service->active_checks_enabled;
  new_service->have_active_checks_enabled =
      temp_service->have_active_checks_enabled;
  new_service->passive_checks_enabled = temp_service->passive_checks_enabled;
  new_service->have_passive_checks_enabled =
      temp_service->have_passive_checks_enabled;
  new_service->parallelize_check = temp_service->parallelize_check;
  new_service->have_parallelize_check = temp_service->have_parallelize_check;
  new_service->is_volatile = temp_service->is_volatile;
  new_service->have_is_volatile = temp_service->have_is_volatile;
  new_service->obsess_over_service = temp_service->obsess_over_service;
  new_service->have_obsess_over_service =
      temp_service->have_obsess_over_service;
  new_service->event_handler_enabled = temp_service->event_handler_enabled;
  new_service->have_event_handler_enabled =
      temp_service->have_event_handler_enabled;
  new_service->check_freshness = temp_service->check_freshness;
  new_service->have_check_freshness = temp_service->have_check_freshness;
  new_service->freshness_threshold = temp_service->freshness_threshold;
  new_service->have_freshness_threshold =
      temp_service->have_freshness_threshold;
  new_service->flap_detection_enabled = temp_service->flap_detection_enabled;
  new_service->have_flap_detection_enabled =
      temp_service->have_flap_detection_enabled;
  new_service->low_flap_threshold = temp_service->low_flap_threshold;
  new_service->have_low_flap_threshold = temp_service->have_low_flap_threshold;
  new_service->high_flap_threshold = temp_service->high_flap_threshold;
  new_service->have_high_flap_threshold =
      temp_service->have_high_flap_threshold;
  new_service->flap_detection_on_ok = temp_service->flap_detection_on_ok;
  new_service->flap_detection_on_warning =
      temp_service->flap_detection_on_warning;
  new_service->flap_detection_on_unknown =
      temp_service->flap_detection_on_unknown;
  new_service->flap_detection_on_critical =
      temp_service->flap_detection_on_critical;
  new_service->have_flap_detection_options =
      temp_service->have_flap_detection_options;
  new_service->notify_on_unknown = temp_service->notify_on_unknown;
  new_service->notify_on_warning = temp_service->notify_on_warning;
  new_service->notify_on_critical = temp_service->notify_on_critical;
  new_service->notify_on_recovery = temp_service->notify_on_recovery;
  new_service->notify_on_flapping = temp_service->notify_on_flapping;
  new_service->notify_on_downtime = temp_service->notify_on_downtime;
  new_service->have_notification_options =
      temp_service->have_notification_options;
  new_service->notifications_enabled = temp_service->notifications_enabled;
  new_service->have_notifications_enabled =
      temp_service->have_notifications_enabled;
  new_service->notification_interval = temp_service->notification_interval;
  new_service->have_notification_interval =
      temp_service->have_notification_interval;
  new_service->first_notification_delay =
      temp_service->first_notification_delay;
  new_service->have_first_notification_delay =
      temp_service->have_first_notification_delay;
  new_service->stalk_on_ok = temp_service->stalk_on_ok;
  new_service->stalk_on_unknown = temp_service->stalk_on_unknown;
  new_service->stalk_on_warning = temp_service->stalk_on_warning;
  new_service->stalk_on_critical = temp_service->stalk_on_critical;
  new_service->have_stalking_options = temp_service->have_stalking_options;
  new_service->process_perf_data = temp_service->process_perf_data;
  new_service->have_process_perf_data = temp_service->have_process_perf_data;
  new_service->retain_status_information =
      temp_service->retain_status_information;
  new_service->have_retain_status_information =
      temp_service->have_retain_status_information;
  new_service->retain_nonstatus_information =
      temp_service->retain_nonstatus_information;
  new_service->have_retain_nonstatus_information =
      temp_service->have_retain_nonstatus_information;

  /* add new service to head of list in memory */
  new_service->next = xodtemplate_service_list;
  xodtemplate_service_list = new_service;

  return OK;
}

/* duplicates a host escalation definition (with a new host name) */
int xodtemplate_duplicate_hostescalation(
    xodtemplate_hostescalation* temp_hostescalation,
    char* host_name) {
  xodtemplate_hostescalation* new_hostescalation = NULL;

  /* allocate memory for a new host escalation definition */
  new_hostescalation = new xodtemplate_hostescalation;

  /* standard items */
  new_hostescalation->tmpl = NULL;
  new_hostescalation->name = NULL;
  new_hostescalation->has_been_resolved =
      temp_hostescalation->has_been_resolved;
  new_hostescalation->register_object = temp_hostescalation->register_object;
  new_hostescalation->_config_file = temp_hostescalation->_config_file;
  new_hostescalation->_start_line = temp_hostescalation->_start_line;

  /* string defaults */
  new_hostescalation->hostgroup_name = NULL;
  new_hostescalation->have_hostgroup_name =
      temp_hostescalation->have_hostgroup_name;
  new_hostescalation->host_name = NULL;
  new_hostescalation->have_host_name = (host_name) ? true : false;
  new_hostescalation->contact_groups = NULL;
  new_hostescalation->have_contact_groups =
      temp_hostescalation->have_contact_groups;
  new_hostescalation->contacts = NULL;
  new_hostescalation->have_contacts = temp_hostescalation->have_contacts;
  new_hostescalation->escalation_period = NULL;
  new_hostescalation->have_escalation_period =
      temp_hostescalation->have_escalation_period;

  /* allocate memory for and copy string members of hostescalation definition */
  if (host_name != NULL)
    new_hostescalation->host_name = string::dup(host_name);
  if (temp_hostescalation->tmpl != NULL)
    new_hostescalation->tmpl = string::dup(temp_hostescalation->tmpl);
  if (temp_hostescalation->name != NULL)
    new_hostescalation->name = string::dup(temp_hostescalation->name);
  if (temp_hostescalation->contact_groups != NULL)
    new_hostescalation->contact_groups =
        string::dup(temp_hostescalation->contact_groups);
  if (temp_hostescalation->contacts != NULL)
    new_hostescalation->contacts = string::dup(temp_hostescalation->contacts);
  if (temp_hostescalation->escalation_period != NULL)
    new_hostescalation->escalation_period =
        string::dup(temp_hostescalation->escalation_period);

  /* duplicate non-string members */
  new_hostescalation->first_notification =
      temp_hostescalation->first_notification;
  new_hostescalation->last_notification =
      temp_hostescalation->last_notification;
  new_hostescalation->have_first_notification =
      temp_hostescalation->have_first_notification;
  new_hostescalation->have_last_notification =
      temp_hostescalation->have_last_notification;
  new_hostescalation->notification_interval =
      temp_hostescalation->notification_interval;
  new_hostescalation->have_notification_interval =
      temp_hostescalation->have_notification_interval;
  new_hostescalation->escalate_on_down = temp_hostescalation->escalate_on_down;
  new_hostescalation->escalate_on_unreachable =
      temp_hostescalation->escalate_on_unreachable;
  new_hostescalation->escalate_on_recovery =
      temp_hostescalation->escalate_on_recovery;
  new_hostescalation->have_escalation_options =
      temp_hostescalation->have_escalation_options;

  /* add new hostescalation to head of list in memory */
  new_hostescalation->next = xodtemplate_hostescalation_list;
  xodtemplate_hostescalation_list = new_hostescalation;

  return OK;
}

/* duplicates a service escalation definition (with a new host name and/or
 * service description) */
int xodtemplate_duplicate_serviceescalation(
    xodtemplate_serviceescalation* temp_serviceescalation,
    char* host_name,
    char* svc_description) {
  xodtemplate_serviceescalation* new_serviceescalation = NULL;

  /* allocate memory for a new service escalation definition */
  new_serviceescalation = new xodtemplate_serviceescalation;

  /* standard items */
  new_serviceescalation->tmpl = NULL;
  new_serviceescalation->name = NULL;
  new_serviceescalation->has_been_resolved =
      temp_serviceescalation->has_been_resolved;
  new_serviceescalation->register_object =
      temp_serviceescalation->register_object;
  new_serviceescalation->_config_file = temp_serviceescalation->_config_file;
  new_serviceescalation->_start_line = temp_serviceescalation->_start_line;

  /* string defaults */
  new_serviceescalation->servicegroup_name = NULL;
  new_serviceescalation->have_servicegroup_name = false;
  new_serviceescalation->hostgroup_name = NULL;
  new_serviceescalation->have_hostgroup_name = false;
  new_serviceescalation->host_name = NULL;
  new_serviceescalation->have_host_name = (host_name) ? true : false;
  new_serviceescalation->service_description = NULL;
  new_serviceescalation->have_service_description =
      (svc_description) ? true : false;
  new_serviceescalation->contact_groups = NULL;
  new_serviceescalation->have_contact_groups =
      temp_serviceescalation->have_contact_groups;
  new_serviceescalation->contacts = NULL;
  new_serviceescalation->have_contacts = temp_serviceescalation->have_contacts;
  new_serviceescalation->escalation_period = NULL;
  new_serviceescalation->have_escalation_period =
      temp_serviceescalation->have_escalation_period;

  /* allocate memory for and copy string members of serviceescalation definition
   */
  if (host_name != NULL)
    new_serviceescalation->host_name = string::dup(host_name);
  if (svc_description != NULL)
    new_serviceescalation->service_description = string::dup(svc_description);

  if (temp_serviceescalation->tmpl != NULL)
    new_serviceescalation->tmpl = string::dup(temp_serviceescalation->tmpl);
  if (temp_serviceescalation->name != NULL)
    new_serviceescalation->name = string::dup(temp_serviceescalation->name);
  if (temp_serviceescalation->contact_groups != NULL)
    new_serviceescalation->contact_groups =
        string::dup(temp_serviceescalation->contact_groups);
  if (temp_serviceescalation->contacts != NULL)
    new_serviceescalation->contacts =
        string::dup(temp_serviceescalation->contacts);
  if (temp_serviceescalation->escalation_period != NULL)
    new_serviceescalation->escalation_period =
        string::dup(temp_serviceescalation->escalation_period);

  /* duplicate non-string members */
  new_serviceescalation->first_notification =
      temp_serviceescalation->first_notification;
  new_serviceescalation->last_notification =
      temp_serviceescalation->last_notification;
  new_serviceescalation->have_first_notification =
      temp_serviceescalation->have_first_notification;
  new_serviceescalation->have_last_notification =
      temp_serviceescalation->have_last_notification;
  new_serviceescalation->notification_interval =
      temp_serviceescalation->notification_interval;
  new_serviceescalation->have_notification_interval =
      temp_serviceescalation->have_notification_interval;
  new_serviceescalation->escalate_on_warning =
      temp_serviceescalation->escalate_on_warning;
  new_serviceescalation->escalate_on_unknown =
      temp_serviceescalation->escalate_on_unknown;
  new_serviceescalation->escalate_on_critical =
      temp_serviceescalation->escalate_on_critical;
  new_serviceescalation->escalate_on_recovery =
      temp_serviceescalation->escalate_on_recovery;
  new_serviceescalation->have_escalation_options =
      temp_serviceescalation->have_escalation_options;

  /* add new serviceescalation to head of list in memory */
  new_serviceescalation->next = xodtemplate_serviceescalation_list;
  xodtemplate_serviceescalation_list = new_serviceescalation;

  return OK;
}

/* duplicates a host dependency definition (with master and dependent host
 * names) */
int xodtemplate_duplicate_hostdependency(
    xodtemplate_hostdependency* temp_hostdependency,
    char* master_host_name,
    char* dependent_host_name) {
  xodtemplate_hostdependency* new_hostdependency = NULL;

  /* allocate memory for a new host dependency definition */
  new_hostdependency = new xodtemplate_hostdependency;

  /* standard items */
  new_hostdependency->tmpl = NULL;
  new_hostdependency->name = NULL;
  new_hostdependency->has_been_resolved =
      temp_hostdependency->has_been_resolved;
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
  new_hostdependency->have_dependent_host_name =
      temp_hostdependency->have_dependent_host_name;
  new_hostdependency->dependency_period = NULL;
  new_hostdependency->have_dependency_period =
      temp_hostdependency->have_dependency_period;

  /* allocate memory for and copy string members of hostdependency definition */
  if (master_host_name != NULL)
    new_hostdependency->host_name = string::dup(master_host_name);
  if (dependent_host_name != NULL)
    new_hostdependency->dependent_host_name = string::dup(dependent_host_name);

  if (temp_hostdependency->dependency_period != NULL)
    new_hostdependency->dependency_period =
        string::dup(temp_hostdependency->dependency_period);
  if (temp_hostdependency->tmpl != NULL)
    new_hostdependency->tmpl = string::dup(temp_hostdependency->tmpl);
  if (temp_hostdependency->name != NULL)
    new_hostdependency->name = string::dup(temp_hostdependency->name);

  /* duplicate non-string members */
  new_hostdependency->fail_notify_on_up =
      temp_hostdependency->fail_notify_on_up;
  new_hostdependency->fail_notify_on_down =
      temp_hostdependency->fail_notify_on_down;
  new_hostdependency->fail_notify_on_unreachable =
      temp_hostdependency->fail_notify_on_unreachable;
  new_hostdependency->fail_notify_on_pending =
      temp_hostdependency->fail_notify_on_pending;
  new_hostdependency->have_notification_dependency_options =
      temp_hostdependency->have_notification_dependency_options;
  new_hostdependency->fail_execute_on_up =
      temp_hostdependency->fail_execute_on_up;
  new_hostdependency->fail_execute_on_down =
      temp_hostdependency->fail_execute_on_down;
  new_hostdependency->fail_execute_on_unreachable =
      temp_hostdependency->fail_execute_on_unreachable;
  new_hostdependency->fail_execute_on_pending =
      temp_hostdependency->fail_execute_on_pending;
  new_hostdependency->have_execution_dependency_options =
      temp_hostdependency->have_execution_dependency_options;
  new_hostdependency->inherits_parent = temp_hostdependency->inherits_parent;
  new_hostdependency->have_inherits_parent =
      temp_hostdependency->have_inherits_parent;

  /* add new hostdependency to head of list in memory */
  new_hostdependency->next = xodtemplate_hostdependency_list;
  xodtemplate_hostdependency_list = new_hostdependency;

  return OK;
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
  new_servicedependency->has_been_resolved =
      temp_servicedependency->has_been_resolved;
  new_servicedependency->register_object =
      temp_servicedependency->register_object;
  new_servicedependency->_config_file = temp_servicedependency->_config_file;
  new_servicedependency->_start_line = temp_servicedependency->_start_line;

  /* string defaults */
  new_servicedependency->host_name = NULL;
  new_servicedependency->have_host_name = (master_host_name) ? true : false;
  new_servicedependency->service_description = NULL;
  new_servicedependency->have_service_description =
      (master_service_description) ? true : false;
  new_servicedependency->hostgroup_name = NULL;
  new_servicedependency->have_hostgroup_name =
      (master_hostgroup_name) ? true : false;
  new_servicedependency->servicegroup_name = NULL;
  new_servicedependency->have_servicegroup_name =
      (master_servicegroup_name) ? true : false;

  new_servicedependency->dependent_host_name = NULL;
  new_servicedependency->have_dependent_host_name =
      (dependent_host_name) ? true : false;
  new_servicedependency->dependent_service_description = NULL;
  new_servicedependency->have_dependent_service_description =
      (dependent_service_description) ? true : false;
  new_servicedependency->dependent_hostgroup_name = NULL;
  new_servicedependency->have_dependent_hostgroup_name =
      (dependent_hostgroup_name) ? true : false;
  new_servicedependency->dependent_servicegroup_name = NULL;
  new_servicedependency->have_dependent_servicegroup_name =
      (dependent_servicegroup_name) ? true : false;

  new_servicedependency->dependency_period = NULL;
  new_servicedependency->have_dependency_period =
      temp_servicedependency->have_dependency_period;
  new_servicedependency->service_description = NULL;
  new_servicedependency->dependent_service_description = NULL;

  /* duplicate strings */
  if (master_host_name != NULL)
    new_servicedependency->host_name = string::dup(master_host_name);
  if (master_service_description != NULL)
    new_servicedependency->service_description =
        string::dup(master_service_description);
  if (master_hostgroup_name != NULL)
    new_servicedependency->hostgroup_name = string::dup(master_hostgroup_name);
  if (master_servicegroup_name != NULL)
    new_servicedependency->servicegroup_name =
        string::dup(master_servicegroup_name);
  if (dependent_host_name != NULL)
    new_servicedependency->dependent_host_name =
        string::dup(dependent_host_name);
  if (dependent_service_description != NULL)
    new_servicedependency->dependent_service_description =
        string::dup(dependent_service_description);
  if (dependent_hostgroup_name != NULL)
    new_servicedependency->dependent_hostgroup_name =
        string::dup(dependent_hostgroup_name);
  if (dependent_servicegroup_name != NULL)
    new_servicedependency->dependent_servicegroup_name =
        string::dup(dependent_servicegroup_name);

  if (temp_servicedependency->dependency_period != NULL)
    new_servicedependency->dependency_period =
        string::dup(temp_servicedependency->dependency_period);
  if (temp_servicedependency->tmpl != NULL)
    new_servicedependency->tmpl = string::dup(temp_servicedependency->tmpl);
  if (temp_servicedependency->name != NULL)
    new_servicedependency->name = string::dup(temp_servicedependency->name);

  /* duplicate non-string members */
  new_servicedependency->fail_notify_on_ok =
      temp_servicedependency->fail_notify_on_ok;
  new_servicedependency->fail_notify_on_unknown =
      temp_servicedependency->fail_notify_on_unknown;
  new_servicedependency->fail_notify_on_warning =
      temp_servicedependency->fail_notify_on_warning;
  new_servicedependency->fail_notify_on_critical =
      temp_servicedependency->fail_notify_on_critical;
  new_servicedependency->fail_notify_on_pending =
      temp_servicedependency->fail_notify_on_pending;
  new_servicedependency->have_notification_dependency_options =
      temp_servicedependency->have_notification_dependency_options;
  new_servicedependency->fail_execute_on_ok =
      temp_servicedependency->fail_execute_on_ok;
  new_servicedependency->fail_execute_on_unknown =
      temp_servicedependency->fail_execute_on_unknown;
  new_servicedependency->fail_execute_on_warning =
      temp_servicedependency->fail_execute_on_warning;
  new_servicedependency->fail_execute_on_critical =
      temp_servicedependency->fail_execute_on_critical;
  new_servicedependency->fail_execute_on_pending =
      temp_servicedependency->fail_execute_on_pending;
  new_servicedependency->have_execution_dependency_options =
      temp_servicedependency->have_execution_dependency_options;
  new_servicedependency->inherits_parent =
      temp_servicedependency->inherits_parent;
  new_servicedependency->have_inherits_parent =
      temp_servicedependency->have_inherits_parent;

  /* add new servicedependency to head of list in memory */
  new_servicedependency->next = xodtemplate_servicedependency_list;
  xodtemplate_servicedependency_list = new_servicedependency;

  return OK;
}

/* duplicates a hostextinfo object definition */
int xodtemplate_duplicate_hostextinfo(xodtemplate_hostextinfo* this_hostextinfo,
                                      char* host_name) {
  xodtemplate_hostextinfo* new_hostextinfo = NULL;

  new_hostextinfo = new xodtemplate_hostextinfo;

  /* standard items */
  new_hostextinfo->tmpl = NULL;
  new_hostextinfo->name = NULL;
  new_hostextinfo->has_been_resolved = this_hostextinfo->has_been_resolved;
  new_hostextinfo->register_object = this_hostextinfo->register_object;
  new_hostextinfo->_config_file = this_hostextinfo->_config_file;
  new_hostextinfo->_start_line = this_hostextinfo->_start_line;

  /* string defaults */
  new_hostextinfo->host_name = NULL;
  new_hostextinfo->have_host_name = this_hostextinfo->have_host_name;
  new_hostextinfo->hostgroup_name = NULL;
  new_hostextinfo->have_hostgroup_name = this_hostextinfo->have_hostgroup_name;
  new_hostextinfo->notes = NULL;
  new_hostextinfo->have_notes = this_hostextinfo->have_notes;
  new_hostextinfo->notes_url = NULL;
  new_hostextinfo->have_notes_url = this_hostextinfo->have_notes_url;
  new_hostextinfo->action_url = NULL;
  new_hostextinfo->have_action_url = this_hostextinfo->have_action_url;
  new_hostextinfo->icon_image = NULL;
  new_hostextinfo->have_icon_image = this_hostextinfo->have_icon_image;
  new_hostextinfo->icon_image_alt = NULL;
  new_hostextinfo->have_icon_image_alt = this_hostextinfo->have_icon_image_alt;
  new_hostextinfo->vrml_image = NULL;
  new_hostextinfo->have_vrml_image = this_hostextinfo->have_vrml_image;
  new_hostextinfo->statusmap_image = NULL;
  new_hostextinfo->have_statusmap_image =
      this_hostextinfo->have_statusmap_image;

  /* duplicate strings (host_name member is passed in) */
  if (host_name != NULL)
    new_hostextinfo->host_name = string::dup(host_name);
  if (this_hostextinfo->tmpl != NULL)
    new_hostextinfo->tmpl = string::dup(this_hostextinfo->tmpl);
  if (this_hostextinfo->name != NULL)
    new_hostextinfo->name = string::dup(this_hostextinfo->name);
  if (this_hostextinfo->notes != NULL)
    new_hostextinfo->notes = string::dup(this_hostextinfo->notes);
  if (this_hostextinfo->notes_url != NULL)
    new_hostextinfo->notes_url = string::dup(this_hostextinfo->notes_url);
  if (this_hostextinfo->action_url != NULL)
    new_hostextinfo->action_url = string::dup(this_hostextinfo->action_url);
  if (this_hostextinfo->icon_image != NULL)
    new_hostextinfo->icon_image = string::dup(this_hostextinfo->icon_image);
  if (this_hostextinfo->icon_image_alt != NULL)
    new_hostextinfo->icon_image_alt =
        string::dup(this_hostextinfo->icon_image_alt);
  if (this_hostextinfo->vrml_image != NULL)
    new_hostextinfo->vrml_image = string::dup(this_hostextinfo->vrml_image);
  if (this_hostextinfo->statusmap_image != NULL)
    new_hostextinfo->statusmap_image =
        string::dup(this_hostextinfo->statusmap_image);

  /* duplicate non-string members */
  new_hostextinfo->x_2d = this_hostextinfo->x_2d;
  new_hostextinfo->y_2d = this_hostextinfo->y_2d;
  new_hostextinfo->have_2d_coords = this_hostextinfo->have_2d_coords;
  new_hostextinfo->x_3d = this_hostextinfo->x_3d;
  new_hostextinfo->y_3d = this_hostextinfo->y_3d;
  new_hostextinfo->z_3d = this_hostextinfo->z_3d;
  new_hostextinfo->have_3d_coords = this_hostextinfo->have_3d_coords;

  /* add new object to head of list */
  new_hostextinfo->next = xodtemplate_hostextinfo_list;
  xodtemplate_hostextinfo_list = new_hostextinfo;

  return OK;
}

/* duplicates a serviceextinfo object definition */
int xodtemplate_duplicate_serviceextinfo(
    xodtemplate_serviceextinfo* this_serviceextinfo,
    char* host_name) {
  xodtemplate_serviceextinfo* new_serviceextinfo = NULL;

  new_serviceextinfo = new xodtemplate_serviceextinfo;

  /* standard items */
  new_serviceextinfo->tmpl = NULL;
  new_serviceextinfo->name = NULL;
  new_serviceextinfo->has_been_resolved =
      this_serviceextinfo->has_been_resolved;
  new_serviceextinfo->register_object = this_serviceextinfo->register_object;
  new_serviceextinfo->_config_file = this_serviceextinfo->_config_file;
  new_serviceextinfo->_start_line = this_serviceextinfo->_start_line;

  /* string defaults */
  new_serviceextinfo->host_name = NULL;
  new_serviceextinfo->have_host_name = this_serviceextinfo->have_host_name;
  new_serviceextinfo->service_description = NULL;
  new_serviceextinfo->have_service_description =
      this_serviceextinfo->have_service_description;
  new_serviceextinfo->hostgroup_name = NULL;
  new_serviceextinfo->have_hostgroup_name =
      this_serviceextinfo->have_hostgroup_name;
  new_serviceextinfo->notes = NULL;
  new_serviceextinfo->have_notes = this_serviceextinfo->have_notes;
  new_serviceextinfo->notes_url = NULL;
  new_serviceextinfo->have_notes_url = this_serviceextinfo->have_notes_url;
  new_serviceextinfo->action_url = NULL;
  new_serviceextinfo->have_action_url = this_serviceextinfo->have_action_url;
  new_serviceextinfo->icon_image = NULL;
  new_serviceextinfo->have_icon_image = this_serviceextinfo->have_icon_image;
  new_serviceextinfo->icon_image_alt = NULL;
  new_serviceextinfo->have_icon_image_alt =
      this_serviceextinfo->have_icon_image_alt;

  /* duplicate strings (host_name member is passed in) */
  if (host_name != NULL)
    new_serviceextinfo->host_name = string::dup(host_name);
  if (this_serviceextinfo->tmpl != NULL)
    new_serviceextinfo->tmpl = string::dup(this_serviceextinfo->tmpl);
  if (this_serviceextinfo->name != NULL)
    new_serviceextinfo->name = string::dup(this_serviceextinfo->name);
  if (this_serviceextinfo->service_description != NULL)
    new_serviceextinfo->service_description =
        string::dup(this_serviceextinfo->service_description);
  if (this_serviceextinfo->notes != NULL)
    new_serviceextinfo->notes = string::dup(this_serviceextinfo->notes);
  if (this_serviceextinfo->notes_url != NULL)
    new_serviceextinfo->notes_url = string::dup(this_serviceextinfo->notes_url);
  if (this_serviceextinfo->action_url != NULL)
    new_serviceextinfo->action_url =
        string::dup(this_serviceextinfo->action_url);
  if (this_serviceextinfo->icon_image != NULL)
    new_serviceextinfo->icon_image =
        string::dup(this_serviceextinfo->icon_image);
  if (this_serviceextinfo->icon_image_alt != NULL)
    new_serviceextinfo->icon_image_alt =
        string::dup(this_serviceextinfo->icon_image_alt);

  /* add new object to head of list */
  new_serviceextinfo->next = xodtemplate_serviceextinfo_list;
  xodtemplate_serviceextinfo_list = new_serviceextinfo;

  return OK;
}

/******************************************************************/
/***************** OBJECT RESOLUTION FUNCTIONS ********************/
/******************************************************************/

/* inherit object properties */
/* some missing defaults (notification options, etc.) are also applied here */
int xodtemplate_inherit_object_properties() {
  xodtemplate_host* temp_host = NULL;
  xodtemplate_service* temp_service = NULL;
  xodtemplate_serviceescalation* temp_serviceescalation = NULL;
  xodtemplate_hostescalation* temp_hostescalation = NULL;

  /* fill in missing defaults for hosts... */
  for (temp_host = xodtemplate_host_list; temp_host != NULL;
       temp_host = temp_host->next) {
    /* if notification options are missing, assume all */
    if (temp_host->have_notification_options == false) {
      temp_host->notify_on_down = true;
      temp_host->notify_on_unreachable = true;
      temp_host->notify_on_recovery = true;
      temp_host->notify_on_flapping = true;
      temp_host->notify_on_downtime = true;
      temp_host->have_notification_options = true;
    }
  }

  /* services inherit some properties from their associated host... */
  for (temp_service = xodtemplate_service_list; temp_service != NULL;
       temp_service = temp_service->next) {
    /* find the host */
    if ((temp_host = xodtemplate_find_real_host(temp_service->host_name)) ==
        NULL)
      continue;

    /* services inherit contact groups from host if not already specified */
    if (temp_service->have_contact_groups == false &&
        temp_host->have_contact_groups == true &&
        temp_host->contact_groups != NULL) {
      temp_service->contact_groups = string::dup(temp_host->contact_groups);
      temp_service->have_contact_groups = true;
    }

    /* services inherit contacts from host if not already specified */
    if (temp_service->have_contacts == false &&
        temp_host->have_contacts == true && temp_host->contacts != NULL) {
      temp_service->contacts = string::dup(temp_host->contacts);
      temp_service->have_contacts = true;
    }

    /* services inherit notification interval from host if not already specified
     */
    if (temp_service->have_notification_interval == false &&
        temp_host->have_notification_interval == true) {
      temp_service->notification_interval = temp_host->notification_interval;
      temp_service->have_notification_interval = true;
    }

    /* services inherit notification period from host if not already specified
     */
    if (temp_service->have_notification_period == false &&
        temp_host->have_notification_period == true &&
        temp_host->notification_period != NULL) {
      temp_service->notification_period =
          string::dup(temp_host->notification_period);
      temp_service->have_notification_period = true;
    }

    /* if notification options are missing, assume all */
    if (temp_service->have_notification_options == false) {
      temp_service->notify_on_unknown = true;
      temp_service->notify_on_warning = true;
      temp_service->notify_on_critical = true;
      temp_service->notify_on_recovery = true;
      temp_service->notify_on_flapping = true;
      temp_service->notify_on_downtime = true;
      temp_service->have_notification_options = true;
    }
  }

  /* service escalations inherit some properties from their associated
   * service... */
  for (temp_serviceescalation = xodtemplate_serviceescalation_list;
       temp_serviceescalation != NULL;
       temp_serviceescalation = temp_serviceescalation->next) {
    /* find the service */
    if ((temp_service = xodtemplate_find_real_service(
             temp_serviceescalation->host_name,
             temp_serviceescalation->service_description)) == NULL)
      continue;

    /* service escalations inherit contact groups from service if not already
     * specified */
    if (temp_serviceescalation->have_contact_groups == false &&
        temp_service->have_contact_groups == true &&
        temp_service->contact_groups != NULL) {
      temp_serviceescalation->contact_groups =
          string::dup(temp_service->contact_groups);
      temp_serviceescalation->have_contact_groups = true;
    }

    /* SPECIAL RULE 10/04/07 - additive inheritance from service's
     * contactgroup(s) */
    if (temp_serviceescalation->contact_groups != NULL &&
        temp_serviceescalation->contact_groups[0] == '+')
      xodtemplate_get_inherited_string(
          &temp_service->have_contact_groups, &temp_service->contact_groups,
          &temp_serviceescalation->have_contact_groups,
          &temp_serviceescalation->contact_groups);

    /* service escalations inherit contacts from service if not already
     * specified */
    if (temp_serviceescalation->have_contacts == false &&
        temp_service->have_contacts == true && temp_service->contacts != NULL) {
      temp_serviceescalation->contacts = string::dup(temp_service->contacts);
      temp_serviceescalation->have_contacts = true;
    }

    /* SPECIAL RULE 10/04/07 - additive inheritance from service's contact(s) */
    if (temp_serviceescalation->contacts != NULL &&
        temp_serviceescalation->contacts[0] == '+')
      xodtemplate_get_inherited_string(&temp_service->have_contacts,
                                       &temp_service->contacts,
                                       &temp_serviceescalation->have_contacts,
                                       &temp_serviceescalation->contacts);

    /* service escalations inherit notification interval from service if not
     * already defined */
    if (temp_serviceescalation->have_notification_interval == false &&
        temp_service->have_notification_interval == true) {
      temp_serviceescalation->notification_interval =
          temp_service->notification_interval;
      temp_serviceescalation->have_notification_interval = true;
    }

    /* service escalations inherit escalation period from service if not already
     * defined */
    if (temp_serviceescalation->have_escalation_period == false &&
        temp_service->have_notification_period == true &&
        temp_service->notification_period != NULL) {
      temp_serviceescalation->escalation_period =
          string::dup(temp_service->notification_period);
      temp_serviceescalation->have_escalation_period = true;
    }

    /* if escalation options are missing, assume all */
    if (temp_serviceescalation->have_escalation_options == false) {
      temp_serviceescalation->escalate_on_unknown = true;
      temp_serviceescalation->escalate_on_warning = true;
      temp_serviceescalation->escalate_on_critical = true;
      temp_serviceescalation->escalate_on_recovery = true;
      temp_serviceescalation->have_escalation_options = true;
    }

    /* 03/05/08 clear additive string chars - not done in
     * xodtemplate_clean_additive_strings() anymore */
    xodtemplate_clean_additive_string(&temp_serviceescalation->contact_groups);
    xodtemplate_clean_additive_string(&temp_serviceescalation->contacts);
  }

  /* host escalations inherit some properties from their associated host... */
  for (temp_hostescalation = xodtemplate_hostescalation_list;
       temp_hostescalation != NULL;
       temp_hostescalation = temp_hostescalation->next) {
    /* find the host */
    if ((temp_host = xodtemplate_find_real_host(
             temp_hostescalation->host_name)) == NULL)
      continue;

    /* host escalations inherit contact groups from service if not already
     * specified */
    if (temp_hostescalation->have_contact_groups == false &&
        temp_host->have_contact_groups == true &&
        temp_host->contact_groups != NULL) {
      temp_hostescalation->contact_groups =
          string::dup(temp_host->contact_groups);
      temp_hostescalation->have_contact_groups = true;
    }

    /* SPECIAL RULE 10/04/07 - additive inheritance from host's contactgroup(s)
     */
    if (temp_hostescalation->contact_groups != NULL &&
        temp_hostescalation->contact_groups[0] == '+')
      xodtemplate_get_inherited_string(
          &temp_host->have_contact_groups, &temp_host->contact_groups,
          &temp_hostescalation->have_contact_groups,
          &temp_hostescalation->contact_groups);

    /* host escalations inherit contacts from service if not already specified
     */
    if (temp_hostescalation->have_contacts == false &&
        temp_host->have_contacts == true && temp_host->contacts != NULL) {
      temp_hostescalation->contacts = string::dup(temp_host->contacts);
      temp_hostescalation->have_contacts = true;
    }

    /* SPECIAL RULE 10/04/07 - additive inheritance from host's contact(s) */
    if (temp_hostescalation->contacts != NULL &&
        temp_hostescalation->contacts[0] == '+')
      xodtemplate_get_inherited_string(
          &temp_host->have_contacts, &temp_host->contacts,
          &temp_hostescalation->have_contacts, &temp_hostescalation->contacts);

    /* host escalations inherit notification interval from host if not already
     * defined */
    if (temp_hostescalation->have_notification_interval == false &&
        temp_host->have_notification_interval == true) {
      temp_hostescalation->notification_interval =
          temp_host->notification_interval;
      temp_hostescalation->have_notification_interval = true;
    }

    /* host escalations inherit escalation period from host if not already
     * defined */
    if (temp_hostescalation->have_escalation_period == false &&
        temp_host->have_notification_period == true &&
        temp_host->notification_period != NULL) {
      temp_hostescalation->escalation_period =
          string::dup(temp_host->notification_period);
      temp_hostescalation->have_escalation_period = true;
    }

    /* if escalation options are missing, assume all */
    if (temp_hostescalation->have_escalation_options == false) {
      temp_hostescalation->escalate_on_down = true;
      temp_hostescalation->escalate_on_unreachable = true;
      temp_hostescalation->escalate_on_recovery = true;
      temp_hostescalation->have_escalation_options = true;
    }

    /* 03/05/08 clear additive string chars - not done in
     * xodtemplate_clean_additive_strings() anymore */
    xodtemplate_clean_additive_string(&temp_hostescalation->contact_groups);
    xodtemplate_clean_additive_string(&temp_hostescalation->contacts);
  }

  return OK;
}

/******************************************************************/
/***************** OBJECT RESOLUTION FUNCTIONS ********************/
/******************************************************************/

/* resolves object definitions */
int xodtemplate_resolve_objects() {
  xodtemplate_timeperiod* temp_timeperiod = NULL;
  xodtemplate_command* temp_command = NULL;
  xodtemplate_contactgroup* temp_contactgroup = NULL;
  xodtemplate_hostgroup* temp_hostgroup = NULL;
  xodtemplate_servicegroup* temp_servicegroup = NULL;
  xodtemplate_servicedependency* temp_servicedependency = NULL;
  xodtemplate_serviceescalation* temp_serviceescalation = NULL;
  xodtemplate_contact* temp_contact = NULL;
  xodtemplate_host* temp_host = NULL;
  xodtemplate_service* temp_service = NULL;
  xodtemplate_hostdependency* temp_hostdependency = NULL;
  xodtemplate_hostescalation* temp_hostescalation = NULL;
  xodtemplate_hostextinfo* temp_hostextinfo = NULL;
  xodtemplate_serviceextinfo* temp_serviceextinfo = NULL;

  /* resolve all timeperiod objects */
  for (temp_timeperiod = xodtemplate_timeperiod_list; temp_timeperiod != NULL;
       temp_timeperiod = temp_timeperiod->next) {
    if (xodtemplate_resolve_timeperiod(temp_timeperiod) == ERROR)
      return ERROR;
  }

  /* resolve all command objects */
  for (temp_command = xodtemplate_command_list; temp_command != NULL;
       temp_command = temp_command->next) {
    if (xodtemplate_resolve_command(temp_command) == ERROR)
      return ERROR;
  }

  /* resolve all contactgroup objects */
  for (temp_contactgroup = xodtemplate_contactgroup_list;
       temp_contactgroup != NULL; temp_contactgroup = temp_contactgroup->next) {
    if (xodtemplate_resolve_contactgroup(temp_contactgroup) == ERROR)
      return ERROR;
  }

  /* resolve all hostgroup objects */
  for (temp_hostgroup = xodtemplate_hostgroup_list; temp_hostgroup != NULL;
       temp_hostgroup = temp_hostgroup->next) {
    if (xodtemplate_resolve_hostgroup(temp_hostgroup) == ERROR)
      return ERROR;
  }

  /* resolve all servicegroup objects */
  for (temp_servicegroup = xodtemplate_servicegroup_list;
       temp_servicegroup != NULL; temp_servicegroup = temp_servicegroup->next) {
    if (xodtemplate_resolve_servicegroup(temp_servicegroup) == ERROR)
      return ERROR;
  }

  /* resolve all servicedependency objects */
  for (temp_servicedependency = xodtemplate_servicedependency_list;
       temp_servicedependency != NULL;
       temp_servicedependency = temp_servicedependency->next) {
    if (xodtemplate_resolve_servicedependency(temp_servicedependency) == ERROR)
      return ERROR;
  }

  /* resolve all serviceescalation objects */
  for (temp_serviceescalation = xodtemplate_serviceescalation_list;
       temp_serviceescalation != NULL;
       temp_serviceescalation = temp_serviceescalation->next) {
    if (xodtemplate_resolve_serviceescalation(temp_serviceescalation) == ERROR)
      return ERROR;
  }

  /* resolve all contact objects */
  for (temp_contact = xodtemplate_contact_list; temp_contact != NULL;
       temp_contact = temp_contact->next) {
    if (xodtemplate_resolve_contact(temp_contact) == ERROR)
      return ERROR;
  }

  /* resolve all host objects */
  for (temp_host = xodtemplate_host_list; temp_host != NULL;
       temp_host = temp_host->next) {
    if (xodtemplate_resolve_host(temp_host) == ERROR)
      return ERROR;
  }

  /* resolve all service objects */
  for (temp_service = xodtemplate_service_list; temp_service != NULL;
       temp_service = temp_service->next) {
    if (xodtemplate_resolve_service(temp_service) == ERROR)
      return ERROR;
  }

  /* resolve all hostdependency objects */
  for (temp_hostdependency = xodtemplate_hostdependency_list;
       temp_hostdependency != NULL;
       temp_hostdependency = temp_hostdependency->next) {
    if (xodtemplate_resolve_hostdependency(temp_hostdependency) == ERROR)
      return ERROR;
  }

  /* resolve all hostescalation objects */
  for (temp_hostescalation = xodtemplate_hostescalation_list;
       temp_hostescalation != NULL;
       temp_hostescalation = temp_hostescalation->next) {
    if (xodtemplate_resolve_hostescalation(temp_hostescalation) == ERROR)
      return ERROR;
  }

  /* resolve all hostextinfo objects */
  for (temp_hostextinfo = xodtemplate_hostextinfo_list;
       temp_hostextinfo != NULL; temp_hostextinfo = temp_hostextinfo->next) {
    if (xodtemplate_resolve_hostextinfo(temp_hostextinfo) == ERROR)
      return ERROR;
  }

  /* resolve all serviceextinfo objects */
  for (temp_serviceextinfo = xodtemplate_serviceextinfo_list;
       temp_serviceextinfo != NULL;
       temp_serviceextinfo = temp_serviceextinfo->next) {
    if (xodtemplate_resolve_serviceextinfo(temp_serviceextinfo) == ERROR)
      return ERROR;
  }

  return OK;
}

/* resolves a timeperiod object */
int xodtemplate_resolve_timeperiod(xodtemplate_timeperiod* this_timeperiod) {
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
    return OK;

  /* set the resolved flag */
  this_timeperiod->has_been_resolved = true;

  /* return if we have no template */
  if (this_timeperiod->tmpl == NULL)
    return OK;

  template_names = string::dup(this_timeperiod->tmpl);

  /* apply all templates */
  template_name_ptr = template_names;
  for (temp_ptr = my_strsep(&template_name_ptr, ","); temp_ptr != NULL;
       temp_ptr = my_strsep(&template_name_ptr, ",")) {
    template_timeperiod = xodtemplate_find_timeperiod(temp_ptr);
    if (template_timeperiod == NULL) {
      logger(log_config_error, basic)
          << "Error: Template '" << temp_ptr
          << "' specified in "
             "timeperiod definition could not be not found (config file '"
          << xodtemplate_config_file_name(this_timeperiod->_config_file)
          << "', starting on line " << this_timeperiod->_start_line << ")";
      delete[] template_names;
      return ERROR;
    }

    /* resolve the template timeperiod... */
    xodtemplate_resolve_timeperiod(template_timeperiod);

    /* apply missing properties from template timeperiod... */
    if (this_timeperiod->timeperiod_name == NULL &&
        template_timeperiod->timeperiod_name != NULL)
      this_timeperiod->timeperiod_name =
          string::dup(template_timeperiod->timeperiod_name);
    if (this_timeperiod->alias == NULL && template_timeperiod->alias != NULL)
      this_timeperiod->alias = string::dup(template_timeperiod->alias);
    if (this_timeperiod->exclusions == NULL &&
        template_timeperiod->exclusions != NULL)
      this_timeperiod->exclusions =
          string::dup(template_timeperiod->exclusions);
    for (x = 0; x < 7; x++) {
      if (this_timeperiod->timeranges[x] == NULL &&
          template_timeperiod->timeranges[x] != NULL) {
        this_timeperiod->timeranges[x] =
            string::dup(template_timeperiod->timeranges[x]);
      }
    }
    /* daterange exceptions require more work to apply missing ranges... */
    for (x = 0; x < DATERANGE_TYPES; x++) {
      for (template_daterange = template_timeperiod->exceptions[x];
           template_daterange != NULL;
           template_daterange = template_daterange->next) {
        /* see if this same daterange already exists in the timeperiod */
        for (this_daterange = this_timeperiod->exceptions[x];
             this_daterange != NULL; this_daterange = this_daterange->next) {
          if ((this_daterange->type == template_daterange->type) &&
              (this_daterange->syear == template_daterange->syear) &&
              (this_daterange->smon == template_daterange->smon) &&
              (this_daterange->smday == template_daterange->smday) &&
              (this_daterange->swday == template_daterange->swday) &&
              (this_daterange->swday_offset ==
               template_daterange->swday_offset) &&
              (this_daterange->eyear == template_daterange->eyear) &&
              (this_daterange->emon == template_daterange->emon) &&
              (this_daterange->emday == template_daterange->emday) &&
              (this_daterange->ewday == template_daterange->ewday) &&
              (this_daterange->ewday_offset ==
               template_daterange->ewday_offset) &&
              (this_daterange->skip_interval ==
               template_daterange->skip_interval))
            break;
        }

        /* this daterange already exists in the timeperiod, so don't inherit it
         */
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
          new_daterange->timeranges =
              string::dup(template_daterange->timeranges);

        /* add new daterange to head of list (should it be added to the end
         * instead?) */
        new_daterange->next = this_timeperiod->exceptions[x];
        this_timeperiod->exceptions[x] = new_daterange;
      }
    }
  }

  delete[] template_names;

  return OK;
}

/* resolves a command object */
int xodtemplate_resolve_command(xodtemplate_command* this_command) {
  char* temp_ptr = NULL;
  char* template_names = NULL;
  char* template_name_ptr = NULL;
  xodtemplate_command* template_command = NULL;

  /* return if this command has already been resolved */
  if (this_command->has_been_resolved == true)
    return OK;

  /* set the resolved flag */
  this_command->has_been_resolved = true;

  /* return if we have no template */
  if (this_command->tmpl == NULL)
    return OK;

  template_names = string::dup(this_command->tmpl);

  /* apply all templates */
  template_name_ptr = template_names;
  for (temp_ptr = my_strsep(&template_name_ptr, ","); temp_ptr != NULL;
       temp_ptr = my_strsep(&template_name_ptr, ",")) {
    template_command = xodtemplate_find_command(temp_ptr);
    if (template_command == NULL) {
      logger(log_config_error, basic)
          << "Error: Template '" << temp_ptr
          << "' specified in command "
             "definition could not be not found (config file '"
          << xodtemplate_config_file_name(this_command->_config_file)
          << "', starting on line " << this_command->_start_line << ")";
      delete[] template_names;
      return ERROR;
    }

    /* resolve the template command... */
    xodtemplate_resolve_command(template_command);

    /* apply missing properties from template command... */
    if (this_command->command_name == NULL &&
        template_command->command_name != NULL)
      this_command->command_name = string::dup(template_command->command_name);
    if (this_command->command_line == NULL &&
        template_command->command_line != NULL)
      this_command->command_line = string::dup(template_command->command_line);
  }

  delete[] template_names;

  return OK;
}

/* resolves a contactgroup object */
int xodtemplate_resolve_contactgroup(
    xodtemplate_contactgroup* this_contactgroup) {
  char* temp_ptr = NULL;
  char* template_names = NULL;
  char* template_name_ptr = NULL;
  xodtemplate_contactgroup* template_contactgroup = NULL;

  /* return if this contactgroup has already been resolved */
  if (this_contactgroup->has_been_resolved == true)
    return OK;

  /* set the resolved flag */
  this_contactgroup->has_been_resolved = true;

  /* return if we have no template */
  if (this_contactgroup->tmpl == NULL)
    return OK;

  template_names = string::dup(this_contactgroup->tmpl);

  /* apply all templates */
  template_name_ptr = template_names;
  for (temp_ptr = my_strsep(&template_name_ptr, ","); temp_ptr != NULL;
       temp_ptr = my_strsep(&template_name_ptr, ",")) {
    template_contactgroup = xodtemplate_find_contactgroup(temp_ptr);
    if (template_contactgroup == NULL) {
      logger(log_config_error, basic)
          << "Error: Template '" << temp_ptr
          << "' specified in "
             "contactgroup definition could not be not found (config file '"
          << xodtemplate_config_file_name(this_contactgroup->_config_file)
          << "', starting on line " << this_contactgroup->_start_line << ")";
      delete[] template_names;
      return ERROR;
    }

    /* resolve the template contactgroup... */
    xodtemplate_resolve_contactgroup(template_contactgroup);

    /* apply missing properties from template contactgroup... */
    if (this_contactgroup->contactgroup_name == NULL &&
        template_contactgroup->contactgroup_name != NULL)
      this_contactgroup->contactgroup_name =
          string::dup(template_contactgroup->contactgroup_name);
    if (this_contactgroup->alias == NULL &&
        template_contactgroup->alias != NULL)
      this_contactgroup->alias = string::dup(template_contactgroup->alias);

    xodtemplate_get_inherited_string(
        &template_contactgroup->have_members, &template_contactgroup->members,
        &this_contactgroup->have_members, &this_contactgroup->members);
    xodtemplate_get_inherited_string(
        &template_contactgroup->have_contactgroup_members,
        &template_contactgroup->contactgroup_members,
        &this_contactgroup->have_contactgroup_members,
        &this_contactgroup->contactgroup_members);
  }

  delete[] template_names;

  return OK;
}

/* resolves a hostgroup object */
int xodtemplate_resolve_hostgroup(xodtemplate_hostgroup* this_hostgroup) {
  char* temp_ptr = NULL;
  char* template_names = NULL;
  char* template_name_ptr = NULL;
  xodtemplate_hostgroup* template_hostgroup = NULL;

  /* return if this hostgroup has already been resolved */
  if (this_hostgroup->has_been_resolved == true)
    return OK;

  /* set the resolved flag */
  this_hostgroup->has_been_resolved = true;

  /* return if we have no template */
  if (this_hostgroup->tmpl == NULL)
    return OK;

  template_names = string::dup(this_hostgroup->tmpl);

  /* apply all templates */
  template_name_ptr = template_names;
  for (temp_ptr = my_strsep(&template_name_ptr, ","); temp_ptr != NULL;
       temp_ptr = my_strsep(&template_name_ptr, ",")) {
    template_hostgroup = xodtemplate_find_hostgroup(temp_ptr);
    if (template_hostgroup == NULL) {
      logger(log_config_error, basic)
          << "Error: Template '" << temp_ptr
          << "' specified in "
             "hostgroup definition could not be not found (config file '"
          << xodtemplate_config_file_name(this_hostgroup->_config_file)
          << "', starting on line " << this_hostgroup->_start_line << ")";
      delete[] template_names;
      return ERROR;
    }

    /* resolve the template hostgroup... */
    xodtemplate_resolve_hostgroup(template_hostgroup);

    /* apply missing properties from template hostgroup... */
    if (this_hostgroup->hostgroup_name == NULL &&
        template_hostgroup->hostgroup_name != NULL)
      this_hostgroup->hostgroup_name =
          string::dup(template_hostgroup->hostgroup_name);
    if (this_hostgroup->alias == NULL && template_hostgroup->alias != NULL)
      this_hostgroup->alias = string::dup(template_hostgroup->alias);

    xodtemplate_get_inherited_string(
        &template_hostgroup->have_members, &template_hostgroup->members,
        &this_hostgroup->have_members, &this_hostgroup->members);
    xodtemplate_get_inherited_string(
        &template_hostgroup->have_hostgroup_members,
        &template_hostgroup->hostgroup_members,
        &this_hostgroup->have_hostgroup_members,
        &this_hostgroup->hostgroup_members);

    if (this_hostgroup->have_notes == false &&
        template_hostgroup->have_notes == true) {
      if (this_hostgroup->notes == NULL && template_hostgroup->notes != NULL)
        this_hostgroup->notes = string::dup(template_hostgroup->notes);
      this_hostgroup->have_notes = true;
    }
    if (this_hostgroup->have_notes_url == false &&
        template_hostgroup->have_notes_url == true) {
      if (this_hostgroup->notes_url == NULL &&
          template_hostgroup->notes_url != NULL)
        this_hostgroup->notes_url = string::dup(template_hostgroup->notes_url);
      this_hostgroup->have_notes_url = true;
    }
    if (this_hostgroup->have_action_url == false &&
        template_hostgroup->have_action_url == true) {
      if (this_hostgroup->action_url == NULL &&
          template_hostgroup->action_url != NULL)
        this_hostgroup->action_url =
            string::dup(template_hostgroup->action_url);
      this_hostgroup->have_action_url = true;
    }
  }

  delete[] template_names;

  return OK;
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
    return OK;

  /* set the resolved flag */
  this_servicegroup->has_been_resolved = true;

  /* return if we have no template */
  if (this_servicegroup->tmpl == NULL)
    return OK;

  template_names = string::dup(this_servicegroup->tmpl);

  /* apply all templates */
  template_name_ptr = template_names;
  for (temp_ptr = my_strsep(&template_name_ptr, ","); temp_ptr != NULL;
       temp_ptr = my_strsep(&template_name_ptr, ",")) {
    template_servicegroup = xodtemplate_find_servicegroup(temp_ptr);
    if (template_servicegroup == NULL) {
      logger(log_config_error, basic)
          << "Error: Template '" << temp_ptr
          << "' specified in "
             "servicegroup definition could not be not found (config file '"
          << xodtemplate_config_file_name(this_servicegroup->_config_file)
          << "', starting on line " << this_servicegroup->_start_line << ")";
      delete[] template_names;
      return ERROR;
    }

    /* resolve the template servicegroup... */
    xodtemplate_resolve_servicegroup(template_servicegroup);

    /* apply missing properties from template servicegroup... */
    if (this_servicegroup->servicegroup_name == NULL &&
        template_servicegroup->servicegroup_name != NULL)
      this_servicegroup->servicegroup_name =
          string::dup(template_servicegroup->servicegroup_name);
    if (this_servicegroup->alias == NULL &&
        template_servicegroup->alias != NULL)
      this_servicegroup->alias = string::dup(template_servicegroup->alias);

    xodtemplate_get_inherited_string(
        &template_servicegroup->have_members, &template_servicegroup->members,
        &this_servicegroup->have_members, &this_servicegroup->members);
    xodtemplate_get_inherited_string(
        &template_servicegroup->have_servicegroup_members,
        &template_servicegroup->servicegroup_members,
        &this_servicegroup->have_servicegroup_members,
        &this_servicegroup->servicegroup_members);

    if (this_servicegroup->have_notes == false &&
        template_servicegroup->have_notes == true) {
      if (this_servicegroup->notes == NULL &&
          template_servicegroup->notes != NULL)
        this_servicegroup->notes = string::dup(template_servicegroup->notes);
      this_servicegroup->have_notes = true;
    }
    if (this_servicegroup->have_notes_url == false &&
        template_servicegroup->have_notes_url == true) {
      if (this_servicegroup->notes_url == NULL &&
          template_servicegroup->notes_url != NULL)
        this_servicegroup->notes_url =
            string::dup(template_servicegroup->notes_url);
      this_servicegroup->have_notes_url = true;
    }
    if (this_servicegroup->have_action_url == false &&
        template_servicegroup->have_action_url == true) {
      if (this_servicegroup->action_url == NULL &&
          template_servicegroup->action_url != NULL)
        this_servicegroup->action_url =
            string::dup(template_servicegroup->action_url);
      this_servicegroup->have_action_url = true;
    }
  }

  delete[] template_names;

  return OK;
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
    return OK;

  /* set the resolved flag */
  this_servicedependency->has_been_resolved = true;

  /* return if we have no template */
  if (this_servicedependency->tmpl == NULL)
    return OK;

  template_names = string::dup(this_servicedependency->tmpl);

  /* apply all templates */
  template_name_ptr = template_names;
  for (temp_ptr = my_strsep(&template_name_ptr, ","); temp_ptr != NULL;
       temp_ptr = my_strsep(&template_name_ptr, ",")) {
    template_servicedependency = xodtemplate_find_servicedependency(temp_ptr);
    if (template_servicedependency == NULL) {
      logger(log_config_error, basic)
          << "Error: Template '" << temp_ptr
          << "' specified in service "
             "dependency definition could not be not found (config file '"
          << xodtemplate_config_file_name(this_servicedependency->_config_file)
          << "', starting on line " << this_servicedependency->_start_line
          << ")";
      delete[] template_names;
      return ERROR;
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

    if (this_servicedependency->have_dependency_period == false &&
        template_servicedependency->have_dependency_period == true) {
      if (this_servicedependency->dependency_period == NULL &&
          template_servicedependency->dependency_period != NULL)
        this_servicedependency->dependency_period =
            string::dup(template_servicedependency->dependency_period);
      this_servicedependency->have_dependency_period = true;
    }
    if (this_servicedependency->have_inherits_parent == false &&
        template_servicedependency->have_inherits_parent == true) {
      this_servicedependency->inherits_parent =
          template_servicedependency->inherits_parent;
      this_servicedependency->have_inherits_parent = true;
    }
    if (this_servicedependency->have_execution_dependency_options == false &&
        template_servicedependency->have_execution_dependency_options == true) {
      this_servicedependency->fail_execute_on_ok =
          template_servicedependency->fail_execute_on_ok;
      this_servicedependency->fail_execute_on_unknown =
          template_servicedependency->fail_execute_on_unknown;
      this_servicedependency->fail_execute_on_warning =
          template_servicedependency->fail_execute_on_warning;
      this_servicedependency->fail_execute_on_critical =
          template_servicedependency->fail_execute_on_critical;
      this_servicedependency->fail_execute_on_pending =
          template_servicedependency->fail_execute_on_pending;
      this_servicedependency->have_execution_dependency_options = true;
    }
    if (this_servicedependency->have_notification_dependency_options == false &&
        template_servicedependency->have_notification_dependency_options ==
            true) {
      this_servicedependency->fail_notify_on_ok =
          template_servicedependency->fail_notify_on_ok;
      this_servicedependency->fail_notify_on_unknown =
          template_servicedependency->fail_notify_on_unknown;
      this_servicedependency->fail_notify_on_warning =
          template_servicedependency->fail_notify_on_warning;
      this_servicedependency->fail_notify_on_critical =
          template_servicedependency->fail_notify_on_critical;
      this_servicedependency->fail_notify_on_pending =
          template_servicedependency->fail_notify_on_pending;
      this_servicedependency->have_notification_dependency_options = true;
    }
  }

  delete[] template_names;

  return OK;
}

/* resolves a serviceescalation object */
int xodtemplate_resolve_serviceescalation(
    xodtemplate_serviceescalation* this_serviceescalation) {
  char* temp_ptr = NULL;
  char* template_names = NULL;
  char* template_name_ptr = NULL;
  xodtemplate_serviceescalation* template_serviceescalation = NULL;

  /* return if this serviceescalation has already been resolved */
  if (this_serviceescalation->has_been_resolved == true)
    return OK;

  /* set the resolved flag */
  this_serviceescalation->has_been_resolved = true;

  /* return if we have no template */
  if (this_serviceescalation->tmpl == NULL)
    return OK;

  template_names = string::dup(this_serviceescalation->tmpl);

  /* apply all templates */
  template_name_ptr = template_names;
  for (temp_ptr = my_strsep(&template_name_ptr, ","); temp_ptr != NULL;
       temp_ptr = my_strsep(&template_name_ptr, ",")) {
    template_serviceescalation = xodtemplate_find_serviceescalation(temp_ptr);
    if (template_serviceescalation == NULL) {
      logger(log_config_error, basic)
          << "Error: Template '" << temp_ptr
          << "' specified in service "
             "escalation definition could not be not found (config file '"
          << xodtemplate_config_file_name(this_serviceescalation->_config_file)
          << "', starting on line " << this_serviceescalation->_start_line
          << ")";
      delete[] template_names;
      return ERROR;
    }

    /* resolve the template serviceescalation... */
    xodtemplate_resolve_serviceescalation(template_serviceescalation);

    /* apply missing properties from template serviceescalation... */
    xodtemplate_get_inherited_string(
        &template_serviceescalation->have_servicegroup_name,
        &template_serviceescalation->servicegroup_name,
        &this_serviceescalation->have_servicegroup_name,
        &this_serviceescalation->servicegroup_name);
    xodtemplate_get_inherited_string(
        &template_serviceescalation->have_hostgroup_name,
        &template_serviceescalation->hostgroup_name,
        &this_serviceescalation->have_hostgroup_name,
        &this_serviceescalation->hostgroup_name);
    xodtemplate_get_inherited_string(
        &template_serviceescalation->have_host_name,
        &template_serviceescalation->host_name,
        &this_serviceescalation->have_host_name,
        &this_serviceescalation->host_name);
    xodtemplate_get_inherited_string(
        &template_serviceescalation->have_service_description,
        &template_serviceescalation->service_description,
        &this_serviceescalation->have_service_description,
        &this_serviceescalation->service_description);
    xodtemplate_get_inherited_string(
        &template_serviceescalation->have_contact_groups,
        &template_serviceescalation->contact_groups,
        &this_serviceescalation->have_contact_groups,
        &this_serviceescalation->contact_groups);
    xodtemplate_get_inherited_string(&template_serviceescalation->have_contacts,
                                     &template_serviceescalation->contacts,
                                     &this_serviceescalation->have_contacts,
                                     &this_serviceescalation->contacts);

    if (this_serviceescalation->have_escalation_period == false &&
        template_serviceescalation->have_escalation_period == true) {
      if (this_serviceescalation->escalation_period == NULL &&
          template_serviceescalation->escalation_period != NULL)
        this_serviceescalation->escalation_period =
            string::dup(template_serviceescalation->escalation_period);
      this_serviceescalation->have_escalation_period = true;
    }
    if (this_serviceescalation->have_first_notification == false &&
        template_serviceescalation->have_first_notification == true) {
      this_serviceescalation->first_notification =
          template_serviceescalation->first_notification;
      this_serviceescalation->have_first_notification = true;
    }
    if (this_serviceescalation->have_last_notification == false &&
        template_serviceescalation->have_last_notification == true) {
      this_serviceescalation->last_notification =
          template_serviceescalation->last_notification;
      this_serviceescalation->have_last_notification = true;
    }
    if (this_serviceescalation->have_notification_interval == false &&
        template_serviceescalation->have_notification_interval == true) {
      this_serviceescalation->notification_interval =
          template_serviceescalation->notification_interval;
      this_serviceescalation->have_notification_interval = true;
    }
    if (this_serviceescalation->have_escalation_options == false &&
        template_serviceescalation->have_escalation_options == true) {
      this_serviceescalation->escalate_on_warning =
          template_serviceescalation->escalate_on_warning;
      this_serviceescalation->escalate_on_unknown =
          template_serviceescalation->escalate_on_unknown;
      this_serviceescalation->escalate_on_critical =
          template_serviceescalation->escalate_on_critical;
      this_serviceescalation->escalate_on_recovery =
          template_serviceescalation->escalate_on_recovery;
      this_serviceescalation->have_escalation_options = true;
    }
  }

  delete[] template_names;

  return OK;
}

/* resolves a contact object */
int xodtemplate_resolve_contact(xodtemplate_contact* this_contact) {
  char* temp_ptr = NULL;
  char* template_names = NULL;
  char* template_name_ptr = NULL;
  xodtemplate_contact* template_contact = NULL;
  xodtemplate_customvariablesmember* this_customvariablesmember = NULL;
  xodtemplate_customvariablesmember* temp_customvariablesmember = NULL;
  int x;

  /* return if this contact has already been resolved */
  if (this_contact->has_been_resolved == true)
    return OK;

  /* set the resolved flag */
  this_contact->has_been_resolved = true;

  /* return if we have no template */
  if (this_contact->tmpl == NULL)
    return OK;

  template_names = string::dup(this_contact->tmpl);

  /* apply all templates */
  template_name_ptr = template_names;
  for (temp_ptr = my_strsep(&template_name_ptr, ","); temp_ptr != NULL;
       temp_ptr = my_strsep(&template_name_ptr, ",")) {
    template_contact = xodtemplate_find_contact(temp_ptr);
    if (template_contact == NULL) {
      logger(log_config_error, basic)
          << "Error: Template '" << temp_ptr
          << "' specified in contact "
             "definition could not be not found (config file '"
          << xodtemplate_config_file_name(this_contact->_config_file)
          << "', starting on line " << this_contact->_start_line << ")";
      delete[] template_names;
      return ERROR;
    }

    /* resolve the template contact... */
    xodtemplate_resolve_contact(template_contact);

    /* apply missing properties from template contact... */
    if (this_contact->contact_name == NULL &&
        template_contact->contact_name != NULL)
      this_contact->contact_name = string::dup(template_contact->contact_name);
    if (this_contact->alias == NULL && template_contact->alias != NULL)
      this_contact->alias = string::dup(template_contact->alias);

    if (this_contact->have_email == false &&
        template_contact->have_email == true) {
      if (this_contact->email == NULL && template_contact->email != NULL)
        this_contact->email = string::dup(template_contact->email);
      this_contact->have_email = true;
    }
    if (this_contact->have_pager == false &&
        template_contact->have_pager == true) {
      if (this_contact->pager == NULL && template_contact->pager != NULL)
        this_contact->pager = string::dup(template_contact->pager);
      this_contact->have_pager = true;
    }
    for (x = 0; x < MAX_XODTEMPLATE_CONTACT_ADDRESSES; x++) {
      if (this_contact->have_address[x] == false &&
          template_contact->have_address[x] == true) {
        if (this_contact->address[x] == NULL &&
            template_contact->address[x] != NULL)
          this_contact->address[x] = string::dup(template_contact->address[x]);
        this_contact->have_address[x] = true;
      }
    }

    xodtemplate_get_inherited_string(&template_contact->have_contact_groups,
                                     &template_contact->contact_groups,
                                     &this_contact->have_contact_groups,
                                     &this_contact->contact_groups);
    xodtemplate_get_inherited_string(
        &template_contact->have_host_notification_commands,
        &template_contact->host_notification_commands,
        &this_contact->have_host_notification_commands,
        &this_contact->host_notification_commands);
    xodtemplate_get_inherited_string(
        &template_contact->have_service_notification_commands,
        &template_contact->service_notification_commands,
        &this_contact->have_service_notification_commands,
        &this_contact->service_notification_commands);

    if (this_contact->have_host_notification_period == false &&
        template_contact->have_host_notification_period == true) {
      if (this_contact->host_notification_period == NULL &&
          template_contact->host_notification_period != NULL)
        this_contact->host_notification_period =
            string::dup(template_contact->host_notification_period);
      this_contact->have_host_notification_period = true;
    }
    if (this_contact->have_service_notification_period == false &&
        template_contact->have_service_notification_period == true) {
      if (this_contact->service_notification_period == NULL &&
          template_contact->service_notification_period != NULL)
        this_contact->service_notification_period =
            string::dup(template_contact->service_notification_period);
      this_contact->have_service_notification_period = true;
    }
    if (this_contact->have_host_notification_options == false &&
        template_contact->have_host_notification_options == true) {
      this_contact->notify_on_host_down = template_contact->notify_on_host_down;
      this_contact->notify_on_host_unreachable =
          template_contact->notify_on_host_unreachable;
      this_contact->notify_on_host_recovery =
          template_contact->notify_on_host_recovery;
      this_contact->notify_on_host_flapping =
          template_contact->notify_on_host_flapping;
      this_contact->notify_on_host_downtime =
          template_contact->notify_on_host_downtime;
      this_contact->have_host_notification_options = true;
    }
    if (this_contact->have_service_notification_options == false &&
        template_contact->have_service_notification_options == true) {
      this_contact->notify_on_service_unknown =
          template_contact->notify_on_service_unknown;
      this_contact->notify_on_service_warning =
          template_contact->notify_on_service_warning;
      this_contact->notify_on_service_critical =
          template_contact->notify_on_service_critical;
      this_contact->notify_on_service_recovery =
          template_contact->notify_on_service_recovery;
      this_contact->notify_on_service_flapping =
          template_contact->notify_on_service_flapping;
      this_contact->notify_on_service_downtime =
          template_contact->notify_on_service_downtime;
      this_contact->have_service_notification_options = true;
    }
    if (this_contact->have_host_notifications_enabled == false &&
        template_contact->have_host_notifications_enabled == true) {
      this_contact->host_notifications_enabled =
          template_contact->host_notifications_enabled;
      this_contact->have_host_notifications_enabled = true;
    }
    if (this_contact->have_service_notifications_enabled == false &&
        template_contact->have_service_notifications_enabled == true) {
      this_contact->service_notifications_enabled =
          template_contact->service_notifications_enabled;
      this_contact->have_service_notifications_enabled = true;
    }
    if (this_contact->have_can_submit_commands == false &&
        template_contact->have_can_submit_commands == true) {
      this_contact->can_submit_commands = template_contact->can_submit_commands;
      this_contact->have_can_submit_commands = true;
    }
    if (this_contact->have_retain_status_information == false &&
        template_contact->have_retain_status_information == true) {
      this_contact->retain_status_information =
          template_contact->retain_status_information;
      this_contact->have_retain_status_information = true;
    }
    if (this_contact->have_retain_nonstatus_information == false &&
        template_contact->have_retain_nonstatus_information == true) {
      this_contact->retain_nonstatus_information =
          template_contact->retain_nonstatus_information;
      this_contact->have_retain_nonstatus_information = true;
    }

    /* apply missing custom variables from template contact... */
    for (temp_customvariablesmember = template_contact->custom_variables;
         temp_customvariablesmember != NULL;
         temp_customvariablesmember = temp_customvariablesmember->next) {
      /* see if this host has a variable by the same name */
      for (this_customvariablesmember = this_contact->custom_variables;
           this_customvariablesmember != NULL;
           this_customvariablesmember = this_customvariablesmember->next) {
        if (!strcmp(temp_customvariablesmember->variable_name,
                    this_customvariablesmember->variable_name))
          break;
      }

      /* we didn't find the same variable name, so add a new custom variable */
      if (this_customvariablesmember == NULL)
        xodtemplate_add_custom_variable_to_contact(
            this_contact, temp_customvariablesmember->variable_name,
            temp_customvariablesmember->variable_value);
    }
  }

  delete[] template_names;

  return OK;
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
    return OK;

  /* set the resolved flag */
  this_host->has_been_resolved = true;

  /* return if we have no template */
  if (this_host->tmpl == NULL)
    return OK;

  template_names = string::dup(this_host->tmpl);

  /* apply all templates */
  template_name_ptr = template_names;
  for (temp_ptr = my_strsep(&template_name_ptr, ","); temp_ptr != NULL;
       temp_ptr = my_strsep(&template_name_ptr, ",")) {
    template_host = xodtemplate_find_host(temp_ptr);
    if (template_host == NULL) {
      logger(log_config_error, basic)
          << "Error: Template '" << temp_ptr
          << "' specified in host "
             "definition could not be not found (config file '"
          << xodtemplate_config_file_name(this_host->_config_file)
          << "', starting on line " << this_host->_start_line << ")";
      delete[] template_names;
      return ERROR;
    }

    /* resolve the template host... */
    xodtemplate_resolve_host(template_host);

    /* apply missing properties from template host... */
    if (this_host->host_name == NULL && template_host->host_name != NULL)
      this_host->host_name = string::dup(template_host->host_name);
    if (this_host->have_display_name == false &&
        template_host->have_display_name == true) {
      if (this_host->display_name == NULL &&
          template_host->display_name != NULL)
        this_host->display_name = string::dup(template_host->display_name);
      this_host->have_display_name = true;
    }
    if (this_host->alias == NULL && template_host->alias != NULL)
      this_host->alias = string::dup(template_host->alias);
    if (this_host->address == NULL && template_host->address != NULL)
      this_host->address = string::dup(template_host->address);

    xodtemplate_get_inherited_string(
        &template_host->have_parents, &template_host->parents,
        &this_host->have_parents, &this_host->parents);
    xodtemplate_get_inherited_string(
        &template_host->have_host_groups, &template_host->host_groups,
        &this_host->have_host_groups, &this_host->host_groups);
    xodtemplate_get_inherited_string(
        &template_host->have_contact_groups, &template_host->contact_groups,
        &this_host->have_contact_groups, &this_host->contact_groups);
    xodtemplate_get_inherited_string(
        &template_host->have_contacts, &template_host->contacts,
        &this_host->have_contacts, &this_host->contacts);

    if (this_host->have_check_command == false &&
        template_host->have_check_command == true) {
      if (this_host->check_command == NULL &&
          template_host->check_command != NULL)
        this_host->check_command = string::dup(template_host->check_command);
      this_host->have_check_command = true;
    }
    if (this_host->have_check_period == false &&
        template_host->have_check_period == true) {
      if (this_host->check_period == NULL &&
          template_host->check_period != NULL)
        this_host->check_period = string::dup(template_host->check_period);
      this_host->have_check_period = true;
    }
    if (this_host->have_event_handler == false &&
        template_host->have_event_handler == true) {
      if (this_host->event_handler == NULL &&
          template_host->event_handler != NULL)
        this_host->event_handler = string::dup(template_host->event_handler);
      this_host->have_event_handler = true;
    }
    if (this_host->have_notification_period == false &&
        template_host->have_notification_period == true) {
      if (this_host->notification_period == NULL &&
          template_host->notification_period != NULL)
        this_host->notification_period =
            string::dup(template_host->notification_period);
      this_host->have_notification_period = true;
    }
    if (this_host->have_notes == false && template_host->have_notes == true) {
      if (this_host->notes == NULL && template_host->notes != NULL)
        this_host->notes = string::dup(template_host->notes);
      this_host->have_notes = true;
    }
    if (this_host->have_notes_url == false &&
        template_host->have_notes_url == true) {
      if (this_host->notes_url == NULL && template_host->notes_url != NULL)
        this_host->notes_url = string::dup(template_host->notes_url);
      this_host->have_notes_url = true;
    }
    if (this_host->have_action_url == false &&
        template_host->have_action_url == true) {
      if (this_host->action_url == NULL && template_host->action_url != NULL)
        this_host->action_url = string::dup(template_host->action_url);
      this_host->have_action_url = true;
    }
    if (this_host->have_icon_image == false &&
        template_host->have_icon_image == true) {
      if (this_host->icon_image == NULL && template_host->icon_image != NULL)
        this_host->icon_image = string::dup(template_host->icon_image);
      this_host->have_icon_image = true;
    }
    if (this_host->have_icon_image_alt == false &&
        template_host->have_icon_image_alt == true) {
      if (this_host->icon_image_alt == NULL &&
          template_host->icon_image_alt != NULL)
        this_host->icon_image_alt = string::dup(template_host->icon_image_alt);
      this_host->have_icon_image_alt = true;
    }
    if (this_host->have_vrml_image == false &&
        template_host->have_vrml_image == true) {
      if (this_host->vrml_image == NULL && template_host->vrml_image != NULL)
        this_host->vrml_image = string::dup(template_host->vrml_image);
      this_host->have_vrml_image = true;
    }
    if (this_host->have_statusmap_image == false &&
        template_host->have_statusmap_image == true) {
      if (this_host->statusmap_image == NULL &&
          template_host->statusmap_image != NULL)
        this_host->statusmap_image =
            string::dup(template_host->statusmap_image);
      this_host->have_statusmap_image = true;
    }
    if (this_host->have_initial_state == false &&
        template_host->have_initial_state == true) {
      this_host->initial_state = template_host->initial_state;
      this_host->have_initial_state = true;
    }
    if (this_host->have_check_interval == false &&
        template_host->have_check_interval == true) {
      this_host->check_interval = template_host->check_interval;
      this_host->have_check_interval = true;
    }
    if (this_host->have_retry_interval == false &&
        template_host->have_retry_interval == true) {
      this_host->retry_interval = template_host->retry_interval;
      this_host->have_retry_interval = true;
    }
    if (this_host->have_max_check_attempts == false &&
        template_host->have_max_check_attempts == true) {
      this_host->max_check_attempts = template_host->max_check_attempts;
      this_host->have_max_check_attempts = true;
    }
    if (this_host->have_active_checks_enabled == false &&
        template_host->have_active_checks_enabled == true) {
      this_host->active_checks_enabled = template_host->active_checks_enabled;
      this_host->have_active_checks_enabled = true;
    }
    if (this_host->have_passive_checks_enabled == false &&
        template_host->have_passive_checks_enabled == true) {
      this_host->passive_checks_enabled = template_host->passive_checks_enabled;
      this_host->have_passive_checks_enabled = true;
    }
    if (this_host->have_obsess_over_host == false &&
        template_host->have_obsess_over_host == true) {
      this_host->obsess_over_host = template_host->obsess_over_host;
      this_host->have_obsess_over_host = true;
    }
    if (this_host->have_event_handler_enabled == false &&
        template_host->have_event_handler_enabled == true) {
      this_host->event_handler_enabled = template_host->event_handler_enabled;
      this_host->have_event_handler_enabled = true;
    }
    if (this_host->have_check_freshness == false &&
        template_host->have_check_freshness == true) {
      this_host->check_freshness = template_host->check_freshness;
      this_host->have_check_freshness = true;
    }
    if (this_host->have_freshness_threshold == false &&
        template_host->have_freshness_threshold == true) {
      this_host->freshness_threshold = template_host->freshness_threshold;
      this_host->have_freshness_threshold = true;
    }
    if (this_host->have_low_flap_threshold == false &&
        template_host->have_low_flap_threshold == true) {
      this_host->low_flap_threshold = template_host->low_flap_threshold;
      this_host->have_low_flap_threshold = true;
    }
    if (this_host->have_high_flap_threshold == false &&
        template_host->have_high_flap_threshold == true) {
      this_host->high_flap_threshold = template_host->high_flap_threshold;
      this_host->have_high_flap_threshold = true;
    }
    if (this_host->have_flap_detection_enabled == false &&
        template_host->have_flap_detection_enabled == true) {
      this_host->flap_detection_enabled = template_host->flap_detection_enabled;
      this_host->have_flap_detection_enabled = true;
    }
    if (this_host->have_flap_detection_options == false &&
        template_host->have_flap_detection_options == true) {
      this_host->flap_detection_on_up = template_host->flap_detection_on_up;
      this_host->flap_detection_on_down = template_host->flap_detection_on_down;
      this_host->flap_detection_on_unreachable =
          template_host->flap_detection_on_unreachable;
      this_host->have_flap_detection_options = true;
    }
    if (this_host->have_notification_options == false &&
        template_host->have_notification_options == true) {
      this_host->notify_on_down = template_host->notify_on_down;
      this_host->notify_on_unreachable = template_host->notify_on_unreachable;
      this_host->notify_on_recovery = template_host->notify_on_recovery;
      this_host->notify_on_flapping = template_host->notify_on_flapping;
      this_host->notify_on_downtime = template_host->notify_on_downtime;
      this_host->have_notification_options = true;
    }
    if (this_host->have_notifications_enabled == false &&
        template_host->have_notifications_enabled == true) {
      this_host->notifications_enabled = template_host->notifications_enabled;
      this_host->have_notifications_enabled = true;
    }
    if (this_host->have_notification_interval == false &&
        template_host->have_notification_interval == true) {
      this_host->notification_interval = template_host->notification_interval;
      this_host->have_notification_interval = true;
    }
    if (this_host->have_first_notification_delay == false &&
        template_host->have_first_notification_delay == true) {
      this_host->first_notification_delay =
          template_host->first_notification_delay;
      this_host->have_first_notification_delay = true;
    }
    if (this_host->have_stalking_options == false &&
        template_host->have_stalking_options == true) {
      this_host->stalk_on_up = template_host->stalk_on_up;
      this_host->stalk_on_down = template_host->stalk_on_down;
      this_host->stalk_on_unreachable = template_host->stalk_on_unreachable;
      this_host->have_stalking_options = true;
    }
    if (this_host->have_process_perf_data == false &&
        template_host->have_process_perf_data == true) {
      this_host->process_perf_data = template_host->process_perf_data;
      this_host->have_process_perf_data = true;
    }
    if (this_host->have_2d_coords == false &&
        template_host->have_2d_coords == true) {
      this_host->x_2d = template_host->x_2d;
      this_host->y_2d = template_host->y_2d;
      this_host->have_2d_coords = true;
    }
    if (this_host->have_3d_coords == false &&
        template_host->have_3d_coords == true) {
      this_host->x_3d = template_host->x_3d;
      this_host->y_3d = template_host->y_3d;
      this_host->z_3d = template_host->z_3d;
      this_host->have_3d_coords = true;
    }
    if (this_host->have_retain_status_information == false &&
        template_host->have_retain_status_information == true) {
      this_host->retain_status_information =
          template_host->retain_status_information;
      this_host->have_retain_status_information = true;
    }
    if (this_host->have_retain_nonstatus_information == false &&
        template_host->have_retain_nonstatus_information == true) {
      this_host->retain_nonstatus_information =
          template_host->retain_nonstatus_information;
      this_host->have_retain_nonstatus_information = true;
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
            this_host, temp_customvariablesmember->variable_name,
            temp_customvariablesmember->variable_value);
    }
  }

  delete[] template_names;

  return OK;
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
    return OK;

  /* set the resolved flag */
  this_service->has_been_resolved = true;

  /* return if we have no template */
  if (this_service->tmpl == NULL)
    return OK;

  template_names = string::dup(this_service->tmpl);

  /* apply all templates */
  template_name_ptr = template_names;
  for (temp_ptr = my_strsep(&template_name_ptr, ","); temp_ptr != NULL;
       temp_ptr = my_strsep(&template_name_ptr, ",")) {
    template_service = xodtemplate_find_service(temp_ptr);
    if (template_service == NULL) {
      logger(log_config_error, basic)
          << "Error: Template '" << temp_ptr
          << "' specified in service "
             "definition could not be not found (config file '"
          << xodtemplate_config_file_name(this_service->_config_file)
          << "', starting on line " << this_service->_start_line << ")";
      delete[] template_names;
      return ERROR;
    }

    /* resolve the template service... */
    xodtemplate_resolve_service(template_service);

    /* apply missing properties from template service... */
    if (this_service->have_service_description == false &&
        template_service->have_service_description == true) {
      if (this_service->service_description == NULL &&
          template_service->service_description != NULL)
        this_service->service_description =
            string::dup(template_service->service_description);
      this_service->have_service_description = true;
    }
    if (this_service->have_display_name == false &&
        template_service->have_display_name == true) {
      if (this_service->display_name == NULL &&
          template_service->display_name != NULL)
        this_service->display_name =
            string::dup(template_service->display_name);
      this_service->have_display_name = true;
    }

    xodtemplate_get_inherited_string(
        &template_service->have_host_name, &template_service->host_name,
        &this_service->have_host_name, &this_service->host_name);
    xodtemplate_get_inherited_string(&template_service->have_hostgroup_name,
                                     &template_service->hostgroup_name,
                                     &this_service->have_hostgroup_name,
                                     &this_service->hostgroup_name);
    xodtemplate_get_inherited_string(&template_service->have_service_groups,
                                     &template_service->service_groups,
                                     &this_service->have_service_groups,
                                     &this_service->service_groups);
    xodtemplate_get_inherited_string(&template_service->have_contact_groups,
                                     &template_service->contact_groups,
                                     &this_service->have_contact_groups,
                                     &this_service->contact_groups);
    xodtemplate_get_inherited_string(
        &template_service->have_contacts, &template_service->contacts,
        &this_service->have_contacts, &this_service->contacts);

    if (template_service->have_check_command == true) {
      if (template_service->have_important_check_command == true) {
        delete[] this_service->check_command;
        this_service->check_command = NULL;
        this_service->have_check_command = false;
      }
      if (this_service->have_check_command == false) {
        if (this_service->check_command == NULL &&
            template_service->check_command != NULL)
          this_service->check_command =
              string::dup(template_service->check_command);
        this_service->have_check_command = true;
      }
    }
    if (this_service->have_check_period == false &&
        template_service->have_check_period == true) {
      if (this_service->check_period == NULL &&
          template_service->check_period != NULL)
        this_service->check_period =
            string::dup(template_service->check_period);
      this_service->have_check_period = true;
    }
    if (this_service->have_event_handler == false &&
        template_service->have_event_handler == true) {
      if (this_service->event_handler == NULL &&
          template_service->event_handler != NULL)
        this_service->event_handler =
            string::dup(template_service->event_handler);
      this_service->have_event_handler = true;
    }
    if (this_service->have_notification_period == false &&
        template_service->have_notification_period == true) {
      if (this_service->notification_period == NULL &&
          template_service->notification_period != NULL)
        this_service->notification_period =
            string::dup(template_service->notification_period);
      this_service->have_notification_period = true;
    }
    if (this_service->have_notes == false &&
        template_service->have_notes == true) {
      if (this_service->notes == NULL && template_service->notes != NULL)
        this_service->notes = string::dup(template_service->notes);
      this_service->have_notes = true;
    }
    if (this_service->have_notes_url == false &&
        template_service->have_notes_url == true) {
      if (this_service->notes_url == NULL &&
          template_service->notes_url != NULL)
        this_service->notes_url = string::dup(template_service->notes_url);
      this_service->have_notes_url = true;
    }
    if (this_service->have_action_url == false &&
        template_service->have_action_url == true) {
      if (this_service->action_url == NULL &&
          template_service->action_url != NULL)
        this_service->action_url = string::dup(template_service->action_url);
      this_service->have_action_url = true;
    }
    if (this_service->have_icon_image == false &&
        template_service->have_icon_image == true) {
      if (this_service->icon_image == NULL &&
          template_service->icon_image != NULL)
        this_service->icon_image = string::dup(template_service->icon_image);
      this_service->have_icon_image = true;
    }
    if (this_service->have_icon_image_alt == false &&
        template_service->have_icon_image_alt == true) {
      if (this_service->icon_image_alt == NULL &&
          template_service->icon_image_alt != NULL)
        this_service->icon_image_alt =
            string::dup(template_service->icon_image_alt);
      this_service->have_icon_image_alt = true;
    }
    if (this_service->have_initial_state == false &&
        template_service->have_initial_state == true) {
      this_service->initial_state = template_service->initial_state;
      this_service->have_initial_state = true;
    }
    if (this_service->have_max_check_attempts == false &&
        template_service->have_max_check_attempts == true) {
      this_service->max_check_attempts = template_service->max_check_attempts;
      this_service->have_max_check_attempts = true;
    }
    if (this_service->have_check_interval == false &&
        template_service->have_check_interval == true) {
      this_service->check_interval = template_service->check_interval;
      this_service->have_check_interval = true;
    }
    if (this_service->have_retry_interval == false &&
        template_service->have_retry_interval == true) {
      this_service->retry_interval = template_service->retry_interval;
      this_service->have_retry_interval = true;
    }
    if (this_service->have_active_checks_enabled == false &&
        template_service->have_active_checks_enabled == true) {
      this_service->active_checks_enabled =
          template_service->active_checks_enabled;
      this_service->have_active_checks_enabled = true;
    }
    if (this_service->have_passive_checks_enabled == false &&
        template_service->have_passive_checks_enabled == true) {
      this_service->passive_checks_enabled =
          template_service->passive_checks_enabled;
      this_service->have_passive_checks_enabled = true;
    }
    if (this_service->have_parallelize_check == false &&
        template_service->have_parallelize_check == true) {
      this_service->parallelize_check = template_service->parallelize_check;
      this_service->have_parallelize_check = true;
    }
    if (this_service->have_is_volatile == false &&
        template_service->have_is_volatile == true) {
      this_service->is_volatile = template_service->is_volatile;
      this_service->have_is_volatile = true;
    }
    if (this_service->have_obsess_over_service == false &&
        template_service->have_obsess_over_service == true) {
      this_service->obsess_over_service = template_service->obsess_over_service;
      this_service->have_obsess_over_service = true;
    }
    if (this_service->have_event_handler_enabled == false &&
        template_service->have_event_handler_enabled == true) {
      this_service->event_handler_enabled =
          template_service->event_handler_enabled;
      this_service->have_event_handler_enabled = true;
    }
    if (this_service->have_check_freshness == false &&
        template_service->have_check_freshness == true) {
      this_service->check_freshness = template_service->check_freshness;
      this_service->have_check_freshness = true;
    }
    if (this_service->have_freshness_threshold == false &&
        template_service->have_freshness_threshold == true) {
      this_service->freshness_threshold = template_service->freshness_threshold;
      this_service->have_freshness_threshold = true;
    }
    if (this_service->have_low_flap_threshold == false &&
        template_service->have_low_flap_threshold == true) {
      this_service->low_flap_threshold = template_service->low_flap_threshold;
      this_service->have_low_flap_threshold = true;
    }
    if (this_service->have_high_flap_threshold == false &&
        template_service->have_high_flap_threshold == true) {
      this_service->high_flap_threshold = template_service->high_flap_threshold;
      this_service->have_high_flap_threshold = true;
    }
    if (this_service->have_flap_detection_enabled == false &&
        template_service->have_flap_detection_enabled == true) {
      this_service->flap_detection_enabled =
          template_service->flap_detection_enabled;
      this_service->have_flap_detection_enabled = true;
    }
    if (this_service->have_flap_detection_options == false &&
        template_service->have_flap_detection_options == true) {
      this_service->flap_detection_on_ok =
          template_service->flap_detection_on_ok;
      this_service->flap_detection_on_unknown =
          template_service->flap_detection_on_unknown;
      this_service->flap_detection_on_warning =
          template_service->flap_detection_on_warning;
      this_service->flap_detection_on_critical =
          template_service->flap_detection_on_critical;
      this_service->have_flap_detection_options = true;
    }
    if (this_service->have_notification_options == false &&
        template_service->have_notification_options == true) {
      this_service->notify_on_unknown = template_service->notify_on_unknown;
      this_service->notify_on_warning = template_service->notify_on_warning;
      this_service->notify_on_critical = template_service->notify_on_critical;
      this_service->notify_on_recovery = template_service->notify_on_recovery;
      this_service->notify_on_flapping = template_service->notify_on_flapping;
      this_service->notify_on_downtime = template_service->notify_on_downtime;
      this_service->have_notification_options = true;
    }
    if (this_service->have_notifications_enabled == false &&
        template_service->have_notifications_enabled == true) {
      this_service->notifications_enabled =
          template_service->notifications_enabled;
      this_service->have_notifications_enabled = true;
    }
    if (this_service->have_notification_interval == false &&
        template_service->have_notification_interval == true) {
      this_service->notification_interval =
          template_service->notification_interval;
      this_service->have_notification_interval = true;
    }
    if (this_service->have_first_notification_delay == false &&
        template_service->have_first_notification_delay == true) {
      this_service->first_notification_delay =
          template_service->first_notification_delay;
      this_service->have_first_notification_delay = true;
    }
    if (this_service->have_stalking_options == false &&
        template_service->have_stalking_options == true) {
      this_service->stalk_on_ok = template_service->stalk_on_ok;
      this_service->stalk_on_unknown = template_service->stalk_on_unknown;
      this_service->stalk_on_warning = template_service->stalk_on_warning;
      this_service->stalk_on_critical = template_service->stalk_on_critical;
      this_service->have_stalking_options = true;
    }
    if (this_service->have_process_perf_data == false &&
        template_service->have_process_perf_data == true) {
      this_service->process_perf_data = template_service->process_perf_data;
      this_service->have_process_perf_data = true;
    }
    if (this_service->have_retain_status_information == false &&
        template_service->have_retain_status_information == true) {
      this_service->retain_status_information =
          template_service->retain_status_information;
      this_service->have_retain_status_information = true;
    }
    if (this_service->have_retain_nonstatus_information == false &&
        template_service->have_retain_nonstatus_information == true) {
      this_service->retain_nonstatus_information =
          template_service->retain_nonstatus_information;
      this_service->have_retain_nonstatus_information = true;
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
            this_service, temp_customvariablesmember->variable_name,
            temp_customvariablesmember->variable_value);
    }
  }

  delete[] template_names;

  return OK;
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
    return OK;

  /* set the resolved flag */
  this_hostdependency->has_been_resolved = true;

  /* return if we have no template */
  if (this_hostdependency->tmpl == NULL)
    return OK;

  template_names = string::dup(this_hostdependency->tmpl);

  /* apply all templates */
  template_name_ptr = template_names;
  for (temp_ptr = my_strsep(&template_name_ptr, ","); temp_ptr != NULL;
       temp_ptr = my_strsep(&template_name_ptr, ",")) {
    template_hostdependency = xodtemplate_find_hostdependency(temp_ptr);
    if (template_hostdependency == NULL) {
      logger(log_config_error, basic)
          << "Error: Template '" << temp_ptr
          << "' specified in host "
             "dependency definition could not be not found (config file '"
          << xodtemplate_config_file_name(this_hostdependency->_config_file)
          << "', starting on line " << this_hostdependency->_start_line << ")";
      delete[] template_names;
      return ERROR;
    }

    /* resolve the template hostdependency... */
    xodtemplate_resolve_hostdependency(template_hostdependency);

    /* apply missing properties from template hostdependency... */

    xodtemplate_get_inherited_string(&template_hostdependency->have_host_name,
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

    if (this_hostdependency->have_dependency_period == false &&
        template_hostdependency->have_dependency_period == true) {
      if (this_hostdependency->dependency_period == NULL &&
          template_hostdependency->dependency_period != NULL)
        this_hostdependency->dependency_period =
            string::dup(template_hostdependency->dependency_period);
      this_hostdependency->have_dependency_period = true;
    }
    if (this_hostdependency->have_inherits_parent == false &&
        template_hostdependency->have_inherits_parent == true) {
      this_hostdependency->inherits_parent =
          template_hostdependency->inherits_parent;
      this_hostdependency->have_inherits_parent = true;
    }
    if (this_hostdependency->have_execution_dependency_options == false &&
        template_hostdependency->have_execution_dependency_options == true) {
      this_hostdependency->fail_execute_on_up =
          template_hostdependency->fail_execute_on_up;
      this_hostdependency->fail_execute_on_down =
          template_hostdependency->fail_execute_on_down;
      this_hostdependency->fail_execute_on_unreachable =
          template_hostdependency->fail_execute_on_unreachable;
      this_hostdependency->fail_execute_on_pending =
          template_hostdependency->fail_execute_on_pending;
      this_hostdependency->have_execution_dependency_options = true;
    }
    if (this_hostdependency->have_notification_dependency_options == false &&
        template_hostdependency->have_notification_dependency_options == true) {
      this_hostdependency->fail_notify_on_up =
          template_hostdependency->fail_notify_on_up;
      this_hostdependency->fail_notify_on_down =
          template_hostdependency->fail_notify_on_down;
      this_hostdependency->fail_notify_on_unreachable =
          template_hostdependency->fail_notify_on_unreachable;
      this_hostdependency->fail_notify_on_pending =
          template_hostdependency->fail_notify_on_pending;
      this_hostdependency->have_notification_dependency_options = true;
    }
  }

  delete[] template_names;

  return OK;
}

/* resolves a hostescalation object */
int xodtemplate_resolve_hostescalation(
    xodtemplate_hostescalation* this_hostescalation) {
  char* temp_ptr = NULL;
  char* template_names = NULL;
  char* template_name_ptr = NULL;
  xodtemplate_hostescalation* template_hostescalation = NULL;

  /* return if this hostescalation has already been resolved */
  if (this_hostescalation->has_been_resolved == true)
    return OK;

  /* set the resolved flag */
  this_hostescalation->has_been_resolved = true;

  /* return if we have no template */
  if (this_hostescalation->tmpl == NULL)
    return OK;

  template_names = string::dup(this_hostescalation->tmpl);

  /* apply all templates */
  template_name_ptr = template_names;
  for (temp_ptr = my_strsep(&template_name_ptr, ","); temp_ptr != NULL;
       temp_ptr = my_strsep(&template_name_ptr, ",")) {
    template_hostescalation = xodtemplate_find_hostescalation(temp_ptr);
    if (template_hostescalation == NULL) {
      logger(log_config_error, basic)
          << "Error: Template '" << temp_ptr
          << "' specified in host "
             "escalation definition could not be not found (config file '"
          << xodtemplate_config_file_name(this_hostescalation->_config_file)
          << "', starting on line " << this_hostescalation->_start_line << ")";
      delete[] template_names;
      return ERROR;
    }

    /* resolve the template hostescalation... */
    xodtemplate_resolve_hostescalation(template_hostescalation);

    /* apply missing properties from template hostescalation... */
    xodtemplate_get_inherited_string(&template_hostescalation->have_host_name,
                                     &template_hostescalation->host_name,
                                     &this_hostescalation->have_host_name,
                                     &this_hostescalation->host_name);
    xodtemplate_get_inherited_string(
        &template_hostescalation->have_hostgroup_name,
        &template_hostescalation->hostgroup_name,
        &this_hostescalation->have_hostgroup_name,
        &this_hostescalation->hostgroup_name);
    xodtemplate_get_inherited_string(
        &template_hostescalation->have_contact_groups,
        &template_hostescalation->contact_groups,
        &this_hostescalation->have_contact_groups,
        &this_hostescalation->contact_groups);
    xodtemplate_get_inherited_string(&template_hostescalation->have_contacts,
                                     &template_hostescalation->contacts,
                                     &this_hostescalation->have_contacts,
                                     &this_hostescalation->contacts);

    if (this_hostescalation->have_escalation_period == false &&
        template_hostescalation->have_escalation_period == true) {
      if (this_hostescalation->escalation_period == NULL &&
          template_hostescalation->escalation_period != NULL)
        this_hostescalation->escalation_period =
            string::dup(template_hostescalation->escalation_period);
      this_hostescalation->have_escalation_period = true;
    }
    if (this_hostescalation->have_first_notification == false &&
        template_hostescalation->have_first_notification == true) {
      this_hostescalation->first_notification =
          template_hostescalation->first_notification;
      this_hostescalation->have_first_notification = true;
    }
    if (this_hostescalation->have_last_notification == false &&
        template_hostescalation->have_last_notification == true) {
      this_hostescalation->last_notification =
          template_hostescalation->last_notification;
      this_hostescalation->have_last_notification = true;
    }
    if (this_hostescalation->have_notification_interval == false &&
        template_hostescalation->have_notification_interval == true) {
      this_hostescalation->notification_interval =
          template_hostescalation->notification_interval;
      this_hostescalation->have_notification_interval = true;
    }
    if (this_hostescalation->have_escalation_options == false &&
        template_hostescalation->have_escalation_options == true) {
      this_hostescalation->escalate_on_down =
          template_hostescalation->escalate_on_down;
      this_hostescalation->escalate_on_unreachable =
          template_hostescalation->escalate_on_unreachable;
      this_hostescalation->escalate_on_recovery =
          template_hostescalation->escalate_on_recovery;
      this_hostescalation->have_escalation_options = true;
    }
  }

  delete[] template_names;

  return OK;
}

/* resolves a hostextinfo object */
int xodtemplate_resolve_hostextinfo(xodtemplate_hostextinfo* this_hostextinfo) {
  char* temp_ptr = NULL;
  char* template_names = NULL;
  char* template_name_ptr = NULL;
  xodtemplate_hostextinfo* template_hostextinfo = NULL;

  /* return if this object has already been resolved */
  if (this_hostextinfo->has_been_resolved == true)
    return OK;

  /* set the resolved flag */
  this_hostextinfo->has_been_resolved = true;

  /* return if we have no template */
  if (this_hostextinfo->tmpl == NULL)
    return OK;

  template_names = string::dup(this_hostextinfo->tmpl);

  /* apply all templates */
  template_name_ptr = template_names;
  for (temp_ptr = my_strsep(&template_name_ptr, ","); temp_ptr != NULL;
       temp_ptr = my_strsep(&template_name_ptr, ",")) {
    template_hostextinfo = xodtemplate_find_hostextinfo(temp_ptr);
    if (template_hostextinfo == NULL) {
      logger(log_config_error, basic)
          << "Error: Template '" << temp_ptr
          << "' specified in extended "
             "host info definition could not be not found (config file '"
          << xodtemplate_config_file_name(this_hostextinfo->_config_file)
          << "', starting on line " << this_hostextinfo->_start_line << ")";
      delete[] template_names;
      return ERROR;
    }

    /* resolve the template hostextinfo... */
    xodtemplate_resolve_hostextinfo(template_hostextinfo);

    /* apply missing properties from template hostextinfo... */
    if (this_hostextinfo->have_host_name == false &&
        template_hostextinfo->have_host_name == true) {
      if (this_hostextinfo->host_name == NULL &&
          template_hostextinfo->host_name != NULL)
        this_hostextinfo->host_name =
            string::dup(template_hostextinfo->host_name);
      this_hostextinfo->have_host_name = true;
    }
    if (this_hostextinfo->have_hostgroup_name == false &&
        template_hostextinfo->have_hostgroup_name == true) {
      if (this_hostextinfo->hostgroup_name == NULL &&
          template_hostextinfo->hostgroup_name != NULL)
        this_hostextinfo->hostgroup_name =
            string::dup(template_hostextinfo->hostgroup_name);
      this_hostextinfo->have_hostgroup_name = true;
    }
    if (this_hostextinfo->have_notes == false &&
        template_hostextinfo->have_notes == true) {
      if (this_hostextinfo->notes == NULL &&
          template_hostextinfo->notes != NULL)
        this_hostextinfo->notes = string::dup(template_hostextinfo->notes);
      this_hostextinfo->have_notes = true;
    }
    if (this_hostextinfo->have_notes_url == false &&
        template_hostextinfo->have_notes_url == true) {
      if (this_hostextinfo->notes_url == NULL &&
          template_hostextinfo->notes_url != NULL)
        this_hostextinfo->notes_url =
            string::dup(template_hostextinfo->notes_url);
      this_hostextinfo->have_notes_url = true;
    }
    if (this_hostextinfo->have_action_url == false &&
        template_hostextinfo->have_action_url == true) {
      if (this_hostextinfo->action_url == NULL &&
          template_hostextinfo->action_url != NULL)
        this_hostextinfo->action_url =
            string::dup(template_hostextinfo->action_url);
      this_hostextinfo->have_action_url = true;
    }
    if (this_hostextinfo->have_icon_image == false &&
        template_hostextinfo->have_icon_image == true) {
      if (this_hostextinfo->icon_image == NULL &&
          template_hostextinfo->icon_image != NULL)
        this_hostextinfo->icon_image =
            string::dup(template_hostextinfo->icon_image);
      this_hostextinfo->have_icon_image = true;
    }
    if (this_hostextinfo->have_icon_image_alt == false &&
        template_hostextinfo->have_icon_image_alt == true) {
      if (this_hostextinfo->icon_image_alt == NULL &&
          template_hostextinfo->icon_image_alt != NULL)
        this_hostextinfo->icon_image_alt =
            string::dup(template_hostextinfo->icon_image_alt);
      this_hostextinfo->have_icon_image_alt = true;
    }
    if (this_hostextinfo->have_vrml_image == false &&
        template_hostextinfo->have_vrml_image == true) {
      if (this_hostextinfo->vrml_image == NULL &&
          template_hostextinfo->vrml_image != NULL)
        this_hostextinfo->vrml_image =
            string::dup(template_hostextinfo->vrml_image);
      this_hostextinfo->have_vrml_image = true;
    }
    if (this_hostextinfo->have_statusmap_image == false &&
        template_hostextinfo->have_statusmap_image == true) {
      if (this_hostextinfo->statusmap_image == NULL &&
          template_hostextinfo->statusmap_image != NULL)
        this_hostextinfo->statusmap_image =
            string::dup(template_hostextinfo->statusmap_image);
      this_hostextinfo->have_statusmap_image = true;
    }
    if (this_hostextinfo->have_2d_coords == false &&
        template_hostextinfo->have_2d_coords == true) {
      this_hostextinfo->x_2d = template_hostextinfo->x_2d;
      this_hostextinfo->y_2d = template_hostextinfo->y_2d;
      this_hostextinfo->have_2d_coords = true;
    }
    if (this_hostextinfo->have_3d_coords == false &&
        template_hostextinfo->have_3d_coords == true) {
      this_hostextinfo->x_3d = template_hostextinfo->x_3d;
      this_hostextinfo->y_3d = template_hostextinfo->y_3d;
      this_hostextinfo->z_3d = template_hostextinfo->z_3d;
      this_hostextinfo->have_3d_coords = true;
    }
  }

  delete[] template_names;

  return OK;
}

/* resolves a serviceextinfo object */
int xodtemplate_resolve_serviceextinfo(
    xodtemplate_serviceextinfo* this_serviceextinfo) {
  char* temp_ptr = NULL;
  char* template_names = NULL;
  char* template_name_ptr = NULL;
  xodtemplate_serviceextinfo* template_serviceextinfo = NULL;

  /* return if this object has already been resolved */
  if (this_serviceextinfo->has_been_resolved == true)
    return OK;

  /* set the resolved flag */
  this_serviceextinfo->has_been_resolved = true;

  /* return if we have no template */
  if (this_serviceextinfo->tmpl == NULL)
    return OK;

  template_names = string::dup(this_serviceextinfo->tmpl);

  /* apply all templates */
  template_name_ptr = template_names;
  for (temp_ptr = my_strsep(&template_name_ptr, ","); temp_ptr != NULL;
       temp_ptr = my_strsep(&template_name_ptr, ",")) {
    template_serviceextinfo = xodtemplate_find_serviceextinfo(temp_ptr);
    if (template_serviceextinfo == NULL) {
      logger(log_config_error, basic)
          << "Error: Template '" << temp_ptr
          << "' specified in extended "
             "service info definition could not be not found (config file '"
          << xodtemplate_config_file_name(this_serviceextinfo->_config_file)
          << "', starting on line " << this_serviceextinfo->_start_line << ")";
      delete[] template_names;
      return ERROR;
    }

    /* resolve the template serviceextinfo... */
    xodtemplate_resolve_serviceextinfo(template_serviceextinfo);

    /* apply missing properties from template serviceextinfo... */
    if (this_serviceextinfo->have_host_name == false &&
        template_serviceextinfo->have_host_name == true) {
      if (this_serviceextinfo->host_name == NULL &&
          template_serviceextinfo->host_name != NULL)
        this_serviceextinfo->host_name =
            string::dup(template_serviceextinfo->host_name);
      this_serviceextinfo->have_host_name = true;
    }
    if (this_serviceextinfo->have_hostgroup_name == false &&
        template_serviceextinfo->have_hostgroup_name == true) {
      if (this_serviceextinfo->hostgroup_name == NULL &&
          template_serviceextinfo->hostgroup_name != NULL)
        this_serviceextinfo->hostgroup_name =
            string::dup(template_serviceextinfo->hostgroup_name);
      this_serviceextinfo->have_hostgroup_name = true;
    }
    if (this_serviceextinfo->have_service_description == false &&
        template_serviceextinfo->have_service_description == true) {
      if (this_serviceextinfo->service_description == NULL &&
          template_serviceextinfo->service_description != NULL)
        this_serviceextinfo->service_description =
            string::dup(template_serviceextinfo->service_description);
      this_serviceextinfo->have_service_description = true;
    }
    if (this_serviceextinfo->have_notes == false &&
        template_serviceextinfo->have_notes == true) {
      if (this_serviceextinfo->notes == NULL &&
          template_serviceextinfo->notes != NULL)
        this_serviceextinfo->notes =
            string::dup(template_serviceextinfo->notes);
      this_serviceextinfo->have_notes = true;
    }
    if (this_serviceextinfo->have_notes_url == false &&
        template_serviceextinfo->have_notes_url == true) {
      if (this_serviceextinfo->notes_url == NULL &&
          template_serviceextinfo->notes_url != NULL)
        this_serviceextinfo->notes_url =
            string::dup(template_serviceextinfo->notes_url);
      this_serviceextinfo->have_notes_url = true;
    }
    if (this_serviceextinfo->have_action_url == false &&
        template_serviceextinfo->have_action_url == true) {
      if (this_serviceextinfo->action_url == NULL &&
          template_serviceextinfo->action_url != NULL)
        this_serviceextinfo->action_url =
            string::dup(template_serviceextinfo->action_url);
      this_serviceextinfo->have_action_url = true;
    }
    if (this_serviceextinfo->have_icon_image == false &&
        template_serviceextinfo->have_icon_image == true) {
      if (this_serviceextinfo->icon_image == NULL &&
          template_serviceextinfo->icon_image != NULL)
        this_serviceextinfo->icon_image =
            string::dup(template_serviceextinfo->icon_image);
      this_serviceextinfo->have_icon_image = true;
    }
    if (this_serviceextinfo->have_icon_image_alt == false &&
        template_serviceextinfo->have_icon_image_alt == true) {
      if (this_serviceextinfo->icon_image_alt == NULL &&
          template_serviceextinfo->icon_image_alt != NULL)
        this_serviceextinfo->icon_image_alt =
            string::dup(template_serviceextinfo->icon_image_alt);
      this_serviceextinfo->have_icon_image_alt = true;
    }
  }

  delete[] template_names;

  return OK;
}

/******************************************************************/
/*************** OBJECT RECOMBOBULATION FUNCTIONS *****************/
/******************************************************************/

/* recombobulates contactgroup definitions */
int xodtemplate_recombobulate_contactgroups() {
  xodtemplate_contact* temp_contact = NULL;
  xodtemplate_contactgroup* temp_contactgroup = NULL;
  xodtemplate_memberlist* temp_memberlist = NULL;
  xodtemplate_memberlist* this_memberlist = NULL;
  char* contactgroup_names = NULL;
  char* temp_ptr = NULL;
  char* new_members = NULL;

  /* This should happen before we expand contactgroup members, to avoid
   * duplicate contact memberships 01/07/2006 EG */
  /* process all contacts that have contactgroup directives */
  for (temp_contact = xodtemplate_contact_list; temp_contact != NULL;
       temp_contact = temp_contact->next) {
    /* skip contacts without contactgroup directives or contact names */
    if (temp_contact->contact_groups == NULL ||
        temp_contact->contact_name == NULL)
      continue;

    /* preprocess the contactgroup list, to change "grp1,grp2,grp3,!grp2" into
     * "grp1,grp3" */
    if ((contactgroup_names = xodtemplate_process_contactgroup_names(
             temp_contact->contact_groups, temp_contact->_config_file,
             temp_contact->_start_line)) == NULL)
      return ERROR;

    /* process the list of contactgroups */
    for (temp_ptr = strtok(contactgroup_names, ","); temp_ptr != NULL;
         temp_ptr = strtok(NULL, ",")) {
      /* strip trailing spaces */
      strip(temp_ptr);

      /* find the contactgroup */
      temp_contactgroup = xodtemplate_find_real_contactgroup(temp_ptr);
      if (temp_contactgroup == NULL) {
        logger(log_config_error, basic)
            << "Error: Could not find contactgroup '" << temp_ptr
            << "' specified in contact '" << temp_contact->contact_name
            << "' definition (config file '"
            << xodtemplate_config_file_name(temp_contact->_config_file)
            << "', starting on line " << temp_contact->_start_line << ")";
        delete[] contactgroup_names;
        return ERROR;
      }

      /* add this contact to the contactgroup members directive */
      if (temp_contactgroup->members == NULL)
        temp_contactgroup->members = string::dup(temp_contact->contact_name);
      else {
        new_members = resize_string(temp_contactgroup->members,
                                    strlen(temp_contactgroup->members) +
                                        strlen(temp_contact->contact_name) + 2);
        temp_contactgroup->members = new_members;
        strcat(temp_contactgroup->members, ",");
        strcat(temp_contactgroup->members, temp_contact->contact_name);
      }
    }

    /* free memory */
    delete[] contactgroup_names;
    contactgroup_names = NULL;
  }

  /* expand subgroup membership recursively */
  for (temp_contactgroup = xodtemplate_contactgroup_list;
       temp_contactgroup != NULL; temp_contactgroup = temp_contactgroup->next)
    xodtemplate_recombobulate_contactgroup_subgroups(temp_contactgroup, NULL);

  /* expand members of all contactgroups - this could be done in
   * xodtemplate_register_contactgroup(), but we can save the CGIs some work if
   * we do it here */
  for (temp_contactgroup = xodtemplate_contactgroup_list;
       temp_contactgroup != NULL; temp_contactgroup = temp_contactgroup->next) {
    if (temp_contactgroup->members == NULL)
      continue;

    /* get list of contacts in the contactgroup */
    temp_memberlist = xodtemplate_expand_contactgroups_and_contacts(
        temp_contactgroup->contactgroup_members, temp_contactgroup->members,
        temp_contactgroup->_config_file, temp_contactgroup->_start_line);

    /* add all members to the contact group */
    if (temp_memberlist == NULL) {
      logger(log_config_error, basic)
          << "Error: Could not expand member contacts specified in "
             "contactgroup (config file '"
          << xodtemplate_config_file_name(temp_contactgroup->_config_file)
          << "', starting on line " << temp_contactgroup->_start_line << ")";
      return ERROR;
    }
    delete[] temp_contactgroup->members;
    temp_contactgroup->members = NULL;
    for (this_memberlist = temp_memberlist; this_memberlist != NULL;
         this_memberlist = this_memberlist->next) {
      /* add this contact to the contactgroup members directive */
      if (temp_contactgroup->members == NULL)
        temp_contactgroup->members = string::dup(this_memberlist->name1);
      else {
        new_members = resize_string(temp_contactgroup->members,
                                    strlen(temp_contactgroup->members) +
                                        strlen(this_memberlist->name1) + 2);
        temp_contactgroup->members = new_members;
        strcat(temp_contactgroup->members, ",");
        strcat(temp_contactgroup->members, this_memberlist->name1);
      }
    }
    xodtemplate_free_memberlist(&temp_memberlist);
  }

  return OK;
}

int xodtemplate_recombobulate_contactgroup_subgroups(
    xodtemplate_contactgroup* temp_contactgroup,
    char** members) {
  if (temp_contactgroup == NULL)
    return ERROR;

  /* resolve subgroup memberships first */
  if (temp_contactgroup->contactgroup_members != NULL) {
    /* save members, null pointer so we don't recurse into infinite hell */
    char* orig_cgmembers(temp_contactgroup->contactgroup_members);
    temp_contactgroup->contactgroup_members = NULL;

    /* make new working copy of members */
    char* cgmembers(string::dup(orig_cgmembers));

    char* buf(NULL);
    char* ptr(cgmembers);
    while ((buf = ptr) != NULL) {
      /* get next member for next run */
      ptr = strchr(ptr, ',');
      if (ptr) {
        ptr[0] = '\x0';
        ptr++;
      }

      strip(buf);

      /* find subgroup and recurse */
      xodtemplate_contactgroup* sub_group(NULL);
      if ((sub_group = xodtemplate_find_real_contactgroup(buf)) == NULL) {
        logger(log_config_error, basic)
            << "Error: Could not find member group '" << buf
            << "' specified in contactgroup (config file '"
            << xodtemplate_config_file_name(temp_contactgroup->_config_file)
            << "', starting on line " << temp_contactgroup->_start_line << ")";
        return ERROR;
      }

      char* newmembers(NULL);
      xodtemplate_recombobulate_contactgroup_subgroups(sub_group, &newmembers);

      /* add new (sub) members */
      if (newmembers != NULL) {
        if (temp_contactgroup->members == NULL)
          temp_contactgroup->members = string::dup(newmembers);
        else {
          temp_contactgroup->members = resize_string(
              temp_contactgroup->members,
              strlen(temp_contactgroup->members) + strlen(newmembers) + 2);
          strcat(temp_contactgroup->members, ",");
          strcat(temp_contactgroup->members, newmembers);
        }
      }
    }

    /* free memory */
    delete[] cgmembers;
    cgmembers = NULL;

    /* restore group members */
    temp_contactgroup->contactgroup_members = orig_cgmembers;
  }

  /* return contact members */
  if (members != NULL)
    *members = temp_contactgroup->members;

  return OK;
}

/* NOTE: this was originally implemented in the late alpha cycle of
 * 3.0 development, but was removed in 3.0b2, as flattening
 * contactgroups into a list of contacts makes it impossible for
 * NDOUtils to create a reverse mapping */
/* recombobulates contacts in various object definitions */
int xodtemplate_recombobulate_object_contacts() {
  return OK;
}

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
  for (temp_hostgroup = xodtemplate_hostgroup_list; temp_hostgroup != NULL;
       temp_hostgroup = temp_hostgroup->next) {
    printf("HOSTGROUP [%s]\n", temp_hostgroup->hostgroup_name);
    printf("H MEMBERS: %s\n", temp_hostgroup->members);
    printf("G MEMBERS: %s\n", temp_hostgroup->hostgroup_members);
    printf("\n");
  }
#endif

  /* This should happen before we expand hostgroup members, to avoid duplicate
   * host memberships 01/07/2006 EG */
  /* process all hosts that have hostgroup directives */
  for (temp_host = xodtemplate_host_list; temp_host != NULL;
       temp_host = temp_host->next) {
    /* skip hosts without hostgroup directives or host names */
    if (temp_host->host_groups == NULL || temp_host->host_name == NULL)
      continue;

    /* skip hosts that shouldn't be registered */
    if (temp_host->register_object == false)
      continue;

    /* preprocess the hostgroup list, to change "grp1,grp2,grp3,!grp2" into
     * "grp1,grp3" */
    /* 10/18/07 EG an empty return value means an error occured */
    if ((hostgroup_names = xodtemplate_process_hostgroup_names(
             temp_host->host_groups, temp_host->_config_file,
             temp_host->_start_line)) == NULL)
      return ERROR;

    /* process the list of hostgroups */
    for (temp_ptr = strtok(hostgroup_names, ","); temp_ptr != NULL;
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
        return ERROR;
      }

      /* add this list to the hostgroup members directive */
      if (temp_hostgroup->members == NULL)
        temp_hostgroup->members = string::dup(temp_host->host_name);
      else {
        new_members = resize_string(
            temp_hostgroup->members,
            strlen(temp_hostgroup->members) + strlen(temp_host->host_name) + 2);
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
  for (temp_hostgroup = xodtemplate_hostgroup_list; temp_hostgroup != NULL;
       temp_hostgroup = temp_hostgroup->next) {
    printf("HOSTGROUP [%s]\n", temp_hostgroup->hostgroup_name);
    printf("H MEMBERS: %s\n", temp_hostgroup->members);
    printf("G MEMBERS: %s\n", temp_hostgroup->hostgroup_members);
    printf("\n");
  }
#endif

  /* expand subgroup membership recursively */
  for (temp_hostgroup = xodtemplate_hostgroup_list; temp_hostgroup != NULL;
       temp_hostgroup = temp_hostgroup->next)
    xodtemplate_recombobulate_hostgroup_subgroups(temp_hostgroup, NULL);

  /* expand members of all hostgroups - this could be done in
   * xodtemplate_register_hostgroup(), but we can save the CGIs some work if we
   * do it here */
  for (temp_hostgroup = xodtemplate_hostgroup_list; temp_hostgroup != NULL;
       temp_hostgroup = temp_hostgroup->next) {
    if (temp_hostgroup->members == NULL &&
        temp_hostgroup->hostgroup_members == NULL)
      continue;

    /* skip hostgroups that shouldn't be registered */
    if (temp_hostgroup->register_object == false)
      continue;

    /* get list of hosts in the hostgroup */
    temp_memberlist = xodtemplate_expand_hostgroups_and_hosts(
        NULL, temp_hostgroup->members, temp_hostgroup->_config_file,
        temp_hostgroup->_start_line);

    /* add all members to the host group */
    if (temp_memberlist == NULL) {
      logger(log_config_error, basic)
          << "Error: Could not expand members specified in hostgroup "
             "(config file '"
          << xodtemplate_config_file_name(temp_hostgroup->_config_file)
          << "', starting on line " << temp_hostgroup->_start_line << ")";
      return ERROR;
    }
    delete[] temp_hostgroup->members;
    temp_hostgroup->members = NULL;
    for (this_memberlist = temp_memberlist; this_memberlist != NULL;
         this_memberlist = this_memberlist->next) {
      /* add this host to the hostgroup members directive */
      if (temp_hostgroup->members == NULL)
        temp_hostgroup->members = string::dup(this_memberlist->name1);
      else {
        new_members = resize_string(temp_hostgroup->members,
                                    strlen(temp_hostgroup->members) +
                                        strlen(this_memberlist->name1) + 2);
        temp_hostgroup->members = new_members;
        strcat(temp_hostgroup->members, ",");
        strcat(temp_hostgroup->members, this_memberlist->name1);
      }
    }
    xodtemplate_free_memberlist(&temp_memberlist);
  }

#ifdef DEBUG
  printf("** POST-EXPANSION 2\n");
  for (temp_hostgroup = xodtemplate_hostgroup_list; temp_hostgroup != NULL;
       temp_hostgroup = temp_hostgroup->next) {
    printf("HOSTGROUP [%s]\n", temp_hostgroup->hostgroup_name);
    printf("H MEMBERS: %s\n", temp_hostgroup->members);
    printf("G MEMBERS: %s\n", temp_hostgroup->hostgroup_members);
    printf("\n");
  }
#endif

  return OK;
}

int xodtemplate_recombobulate_hostgroup_subgroups(
    xodtemplate_hostgroup* temp_hostgroup,
    char** members) {
  if (temp_hostgroup == NULL)
    return ERROR;

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
            << "', starting on line " << temp_hostgroup->_start_line << ")";
        return ERROR;
      }

      char* newmembers(NULL);
      xodtemplate_recombobulate_hostgroup_subgroups(sub_group, &newmembers);

      /* add new (sub) members */
      if (newmembers != NULL) {
        if (temp_hostgroup->members == NULL)
          temp_hostgroup->members = string::dup(newmembers);
        else {
          temp_hostgroup->members = resize_string(
              temp_hostgroup->members,
              strlen(temp_hostgroup->members) + strlen(newmembers) + 2);
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

  return OK;
}

/* recombobulates servicegroup definitions */
/***** THIS NEEDS TO BE CALLED AFTER OBJECTS (SERVICES) ARE RESOLVED AND
 * DUPLICATED *****/
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

  /* This should happen before we expand servicegroup members, to avoid
   * duplicate service memberships 01/07/2006 EG */
  /* process all services that have servicegroup directives */
  for (temp_service = xodtemplate_service_list; temp_service != NULL;
       temp_service = temp_service->next) {
    /* skip services without servicegroup directives or service names */
    if (temp_service->service_groups == NULL ||
        temp_service->host_name == NULL ||
        temp_service->service_description == NULL)
      continue;

    /* skip services that shouldn't be registered */
    if (temp_service->register_object == false)
      continue;

    /* preprocess the servicegroup list, to change "grp1,grp2,grp3,!grp2" into
     * "grp1,grp3" */
    /* 10/19/07 EG an empry return value means an error occured */
    if ((servicegroup_names = xodtemplate_process_servicegroup_names(
             temp_service->service_groups, temp_service->_config_file,
             temp_service->_start_line)) == NULL)
      return ERROR;

    /* process the list of servicegroups */
    for (temp_ptr = strtok(servicegroup_names, ","); temp_ptr != NULL;
         temp_ptr = strtok(NULL, ",")) {
      /* strip trailing spaces */
      strip(temp_ptr);

      /* find the servicegroup */
      temp_servicegroup = xodtemplate_find_real_servicegroup(temp_ptr);
      if (temp_servicegroup == NULL) {
        logger(log_config_error, basic)
            << "Error: Could not find servicegroup '" << temp_ptr
            << "' specified in service '" << temp_service->service_description
            << "' on host '" << temp_service->host_name
            << "' definition (config file '"
            << xodtemplate_config_file_name(temp_service->_config_file)
            << "', starting on line " << temp_service->_start_line << ")";
        delete[] servicegroup_names;
        return ERROR;
      }

      /* add this list to the servicegroup members directive */
      if (temp_servicegroup->members == NULL) {
        temp_servicegroup->members =
            new char[strlen(temp_service->host_name) +
                     strlen(temp_service->service_description) + 2];
        strcpy(temp_servicegroup->members, temp_service->host_name);
        strcat(temp_servicegroup->members, ",");
        strcat(temp_servicegroup->members, temp_service->service_description);
      } else {
        new_members =
            resize_string(temp_servicegroup->members,
                          strlen(temp_servicegroup->members) +
                              strlen(temp_service->host_name) +
                              strlen(temp_service->service_description) + 3);
        temp_servicegroup->members = new_members;
        strcat(temp_servicegroup->members, ",");
        strcat(temp_servicegroup->members, temp_service->host_name);
        strcat(temp_servicegroup->members, ",");
        strcat(temp_servicegroup->members, temp_service->service_description);
      }
    }

    /* free servicegroup names */
    delete[] servicegroup_names;
    servicegroup_names = NULL;
  }

  /* expand subgroup membership recursively */
  for (temp_servicegroup = xodtemplate_servicegroup_list;
       temp_servicegroup != NULL; temp_servicegroup = temp_servicegroup->next)
    xodtemplate_recombobulate_servicegroup_subgroups(temp_servicegroup, NULL);

  /* expand members of all servicegroups - this could be done in
   * xodtemplate_register_servicegroup(), but we can save the CGIs some work if
   * we do it here */
  for (temp_servicegroup = xodtemplate_servicegroup_list;
       temp_servicegroup != NULL; temp_servicegroup = temp_servicegroup->next) {
    if (temp_servicegroup->members == NULL)
      continue;

    /* skip servicegroups that shouldn't be registered */
    if (temp_servicegroup->register_object == false)
      continue;

    member_names = temp_servicegroup->members;
    temp_servicegroup->members = NULL;

    for (temp_ptr = member_names; temp_ptr != NULL;
         temp_ptr = strchr(temp_ptr + 1, ',')) {
      /* this is the host name */
      if (host_name == NULL)
        host_name = string::dup((temp_ptr[0] == ',') ? temp_ptr + 1 : temp_ptr);

      /* this is the service description */
      else {
        service_description = string::dup(temp_ptr + 1);

        /* strsep and strtok cannot be used, as they're used in
         * expand_servicegroups...() */
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
            NULL, host_name, service_description,
            temp_servicegroup->_config_file, temp_servicegroup->_start_line);

        /* add all members to the service group */
        if (temp_memberlist == NULL) {
          logger(log_config_error, basic)
              << "Error: Could not expand member services specified in "
                 "servicegroup (config file '"
              << xodtemplate_config_file_name(temp_servicegroup->_config_file)
              << "', starting on line " << temp_servicegroup->_start_line
              << ")";
          delete[] member_names;
          delete[] host_name;
          delete[] service_description;
          return ERROR;
        }

        for (this_memberlist = temp_memberlist; this_memberlist != NULL;
             this_memberlist = this_memberlist->next) {
          /* add this service to the servicegroup members directive */
          if (temp_servicegroup->members == NULL) {
            temp_servicegroup->members =
                new char[strlen(this_memberlist->name1) +
                         strlen(this_memberlist->name2) + 2];
            strcpy(temp_servicegroup->members, this_memberlist->name1);
            strcat(temp_servicegroup->members, ",");
            strcat(temp_servicegroup->members, this_memberlist->name2);
          } else {
            new_members = resize_string(temp_servicegroup->members,
                                        strlen(temp_servicegroup->members) +
                                            strlen(this_memberlist->name1) +
                                            strlen(this_memberlist->name2) + 3);
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

    /* error if there were an odd number of items specified (unmatched
     * host/service pair) */
    if (host_name != NULL) {
      logger(log_config_error, basic)
          << "Error: Servicegroup members must be specified in "
             "<host_name>,<service_description> pairs (config file '"
          << xodtemplate_config_file_name(temp_servicegroup->_config_file)
          << "', starting on line " << temp_servicegroup->_start_line << ")";
      delete[] host_name;
      return ERROR;
    }
  }

  return OK;
}

int xodtemplate_recombobulate_servicegroup_subgroups(
    xodtemplate_servicegroup* temp_servicegroup,
    char** members) {
  if (temp_servicegroup == NULL)
    return ERROR;

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
            << "', starting on line " << temp_servicegroup->_start_line << ")";
        return ERROR;
      }
      char* newmembers(NULL);
      xodtemplate_recombobulate_servicegroup_subgroups(sub_group, &newmembers);

      /* add new (sub) members */
      if (newmembers != NULL) {
        if (temp_servicegroup->members == NULL)
          temp_servicegroup->members = string::dup(newmembers);
        else {
          temp_servicegroup->members = resize_string(
              temp_servicegroup->members,
              strlen(temp_servicegroup->members) + strlen(newmembers) + 2);
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

  return OK;
}

/******************************************************************/
/******************* OBJECT SEARCH FUNCTIONS **********************/
/******************************************************************/

/* finds a specific timeperiod object */
xodtemplate_timeperiod* xodtemplate_find_timeperiod(char* name) {
  if (name == NULL)
    return NULL;

  xodtemplate_timeperiod temp_timeperiod;
  temp_timeperiod.name = name;
  return ((xodtemplate_timeperiod*)skiplist_find_first(
      xobject_template_skiplists[X_TIMEPERIOD_SKIPLIST], &temp_timeperiod,
      NULL));
}

/* finds a specific command object */
xodtemplate_command* xodtemplate_find_command(char* name) {
  if (name == NULL)
    return NULL;

  xodtemplate_command temp_command;
  temp_command.name = name;
  return ((xodtemplate_command*)skiplist_find_first(
      xobject_template_skiplists[X_COMMAND_SKIPLIST], &temp_command, NULL));
}

/* finds a specific contactgroup object */
xodtemplate_contactgroup* xodtemplate_find_contactgroup(char* name) {
  if (name == NULL)
    return NULL;

  xodtemplate_contactgroup temp_contactgroup;
  temp_contactgroup.name = name;
  return ((xodtemplate_contactgroup*)skiplist_find_first(
      xobject_template_skiplists[X_CONTACTGROUP_SKIPLIST], &temp_contactgroup,
      NULL));
}

/* finds a specific contactgroup object by its REAL name, not its TEMPLATE name
 */
xodtemplate_contactgroup* xodtemplate_find_real_contactgroup(char* name) {
  if (name == NULL)
    return NULL;

  xodtemplate_contactgroup temp_contactgroup;
  temp_contactgroup.contactgroup_name = name;
  return ((xodtemplate_contactgroup*)skiplist_find_first(
      xobject_skiplists[X_CONTACTGROUP_SKIPLIST], &temp_contactgroup, NULL));
}

/* finds a specific hostgroup object */
xodtemplate_hostgroup* xodtemplate_find_hostgroup(char* name) {
  if (name == NULL)
    return NULL;

  xodtemplate_hostgroup temp_hostgroup;
  temp_hostgroup.name = name;
  return ((xodtemplate_hostgroup*)skiplist_find_first(
      xobject_template_skiplists[X_HOSTGROUP_SKIPLIST], &temp_hostgroup, NULL));
}

/* finds a specific hostgroup object by its REAL name, not its TEMPLATE name */
xodtemplate_hostgroup* xodtemplate_find_real_hostgroup(char* name) {
  if (name == NULL)
    return NULL;

  xodtemplate_hostgroup temp_hostgroup;
  temp_hostgroup.hostgroup_name = name;
  return ((xodtemplate_hostgroup*)skiplist_find_first(
      xobject_skiplists[X_HOSTGROUP_SKIPLIST], &temp_hostgroup, NULL));
}

/* finds a specific servicegroup object */
xodtemplate_servicegroup* xodtemplate_find_servicegroup(char* name) {
  if (name == NULL)
    return NULL;

  xodtemplate_servicegroup temp_servicegroup;
  temp_servicegroup.name = name;
  return ((xodtemplate_servicegroup*)skiplist_find_first(
      xobject_template_skiplists[X_SERVICEGROUP_SKIPLIST], &temp_servicegroup,
      NULL));
}

/* finds a specific servicegroup object by its REAL name, not its TEMPLATE name
 */
xodtemplate_servicegroup* xodtemplate_find_real_servicegroup(char* name) {
  if (name == NULL)
    return NULL;

  xodtemplate_servicegroup temp_servicegroup;
  temp_servicegroup.servicegroup_name = name;
  return ((xodtemplate_servicegroup*)skiplist_find_first(
      xobject_skiplists[X_SERVICEGROUP_SKIPLIST], &temp_servicegroup, NULL));
}

/* finds a specific servicedependency object */
xodtemplate_servicedependency* xodtemplate_find_servicedependency(char* name) {
  if (name == NULL)
    return NULL;

  xodtemplate_servicedependency temp_servicedependency;
  temp_servicedependency.name = name;
  return ((xodtemplate_servicedependency*)skiplist_find_first(
      xobject_template_skiplists[X_SERVICEDEPENDENCY_SKIPLIST],
      &temp_servicedependency, NULL));
}

/* finds a specific serviceescalation object */
xodtemplate_serviceescalation* xodtemplate_find_serviceescalation(char* name) {
  if (name == NULL)
    return NULL;

  xodtemplate_serviceescalation temp_serviceescalation;
  temp_serviceescalation.name = name;
  return ((xodtemplate_serviceescalation*)skiplist_find_first(
      xobject_template_skiplists[X_SERVICEESCALATION_SKIPLIST],
      &temp_serviceescalation, NULL));
}

/* finds a specific contact object */
xodtemplate_contact* xodtemplate_find_contact(char* name) {
  if (name == NULL)
    return NULL;

  xodtemplate_contact temp_contact;
  temp_contact.name = name;
  return ((xodtemplate_contact*)skiplist_find_first(
      xobject_template_skiplists[X_CONTACT_SKIPLIST], &temp_contact, NULL));
}

/* finds a specific contact object by its REAL name, not its TEMPLATE name */
xodtemplate_contact* xodtemplate_find_real_contact(char* name) {
  if (name == NULL)
    return NULL;

  xodtemplate_contact temp_contact;
  temp_contact.contact_name = name;
  return ((xodtemplate_contact*)skiplist_find_first(
      xobject_skiplists[X_CONTACT_SKIPLIST], &temp_contact, NULL));
}

/* finds a specific host object */
xodtemplate_host* xodtemplate_find_host(char* name) {
  if (name == NULL)
    return NULL;

  xodtemplate_host temp_host;
  temp_host.name = name;
  return ((xodtemplate_host*)skiplist_find_first(
      xobject_template_skiplists[X_HOST_SKIPLIST], &temp_host, NULL));
}

/* finds a specific host object by its REAL name, not its TEMPLATE name */
xodtemplate_host* xodtemplate_find_real_host(char* name) {
  if (name == NULL)
    return NULL;

  xodtemplate_host temp_host;
  temp_host.host_name = name;
  return ((xodtemplate_host*)skiplist_find_first(
      xobject_skiplists[X_HOST_SKIPLIST], &temp_host, NULL));
}

/* finds a specific hostdependency object */
xodtemplate_hostdependency* xodtemplate_find_hostdependency(char* name) {
  if (name == NULL)
    return NULL;

  xodtemplate_hostdependency temp_hostdependency;
  temp_hostdependency.name = name;
  return ((xodtemplate_hostdependency*)skiplist_find_first(
      xobject_template_skiplists[X_HOSTDEPENDENCY_SKIPLIST],
      &temp_hostdependency, NULL));
}

/* finds a specific hostescalation object */
xodtemplate_hostescalation* xodtemplate_find_hostescalation(char* name) {
  if (name == NULL)
    return NULL;

  xodtemplate_hostescalation temp_hostescalation;
  temp_hostescalation.name = name;
  return ((xodtemplate_hostescalation*)skiplist_find_first(
      xobject_template_skiplists[X_HOSTESCALATION_SKIPLIST],
      &temp_hostescalation, NULL));
}

/* finds a specific hostextinfo object */
xodtemplate_hostextinfo* xodtemplate_find_hostextinfo(char* name) {
  if (name == NULL)
    return NULL;

  xodtemplate_hostextinfo temp_hostextinfo;
  temp_hostextinfo.name = name;
  return ((xodtemplate_hostextinfo*)skiplist_find_first(
      xobject_template_skiplists[X_HOSTEXTINFO_SKIPLIST], &temp_hostextinfo,
      NULL));
}

/* finds a specific serviceextinfo object */
xodtemplate_serviceextinfo* xodtemplate_find_serviceextinfo(char* name) {
  if (name == NULL)
    return NULL;

  xodtemplate_serviceextinfo temp_serviceextinfo;
  temp_serviceextinfo.name = name;
  return ((xodtemplate_serviceextinfo*)skiplist_find_first(
      xobject_template_skiplists[X_SERVICEEXTINFO_SKIPLIST],
      &temp_serviceextinfo, NULL));
}

/* finds a specific service object */
xodtemplate_service* xodtemplate_find_service(char* name) {
  if (name == NULL)
    return NULL;

  xodtemplate_service temp_service;
  temp_service.name = name;
  return ((xodtemplate_service*)skiplist_find_first(
      xobject_template_skiplists[X_SERVICE_SKIPLIST], &temp_service, NULL));
}

/* finds a specific service object by its REAL name, not its TEMPLATE name */
xodtemplate_service* xodtemplate_find_real_service(char* host_name,
                                                   char* service_description) {
  if (host_name == NULL || service_description == NULL)
    return NULL;

  xodtemplate_service temp_service;
  temp_service.host_name = host_name;
  temp_service.service_description = service_description;
  return ((xodtemplate_service*)skiplist_find_first(
      xobject_skiplists[X_SERVICE_SKIPLIST], &temp_service, NULL));
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
  for (temp_timeperiod = (xodtemplate_timeperiod*)skiplist_get_first(
           xobject_skiplists[X_TIMEPERIOD_SKIPLIST], &ptr);
       temp_timeperiod;
       temp_timeperiod = (xodtemplate_timeperiod*)skiplist_get_next(&ptr)) {
    // Insert timeperiod object in lists.
    if (xodtemplate_register_timeperiod(temp_timeperiod) == ERROR)
      return ERROR;

    // Retrieve timeperiod object.
    timeperiod* t(find_timeperiod(temp_timeperiod->timeperiod_name));

    // Fill timeperiod with its content.
    if (xodtemplate_fill_timeperiod(temp_timeperiod, t) == ERROR)
      return ERROR;
  }

  /* register connectors */
  ptr = NULL;
  xodtemplate_connector* temp_connector(NULL);
  for (temp_connector = (xodtemplate_connector*)skiplist_get_first(
           xobject_skiplists[X_CONNECTOR_SKIPLIST], &ptr);
       temp_connector;
       temp_connector = (xodtemplate_connector*)skiplist_get_next(&ptr)) {
    if (xodtemplate_register_connector(temp_connector) == ERROR)
      return ERROR;
  }

  /* register commands */
  ptr = NULL;
  xodtemplate_command* temp_command(NULL);
  for (temp_command = (xodtemplate_command*)skiplist_get_first(
           xobject_skiplists[X_COMMAND_SKIPLIST], &ptr);
       temp_command;
       temp_command = (xodtemplate_command*)skiplist_get_next(&ptr)) {
    if (xodtemplate_register_command(temp_command) == ERROR)
      return ERROR;
  }

  /* register contactgroups */
  ptr = NULL;
  xodtemplate_contactgroup* temp_contactgroup(NULL);
  for (temp_contactgroup = (xodtemplate_contactgroup*)skiplist_get_first(
           xobject_skiplists[X_CONTACTGROUP_SKIPLIST], &ptr);
       temp_contactgroup;
       temp_contactgroup = (xodtemplate_contactgroup*)skiplist_get_next(&ptr)) {
    if (xodtemplate_register_contactgroup(temp_contactgroup) == ERROR)
      return ERROR;
  }

  /* register hostgroups */
  ptr = NULL;
  xodtemplate_hostgroup* temp_hostgroup(NULL);
  for (temp_hostgroup = (xodtemplate_hostgroup*)skiplist_get_first(
           xobject_skiplists[X_HOSTGROUP_SKIPLIST], &ptr);
       temp_hostgroup;
       temp_hostgroup = (xodtemplate_hostgroup*)skiplist_get_next(&ptr)) {
    if (xodtemplate_register_hostgroup(temp_hostgroup) == ERROR)
      return ERROR;
  }

  /* register servicegroups */
  ptr = NULL;
  xodtemplate_servicegroup* temp_servicegroup(NULL);
  for (temp_servicegroup = (xodtemplate_servicegroup*)skiplist_get_first(
           xobject_skiplists[X_SERVICEGROUP_SKIPLIST], &ptr);
       temp_servicegroup;
       temp_servicegroup = (xodtemplate_servicegroup*)skiplist_get_next(&ptr)) {
    if (xodtemplate_register_servicegroup(temp_servicegroup) == ERROR)
      return ERROR;
  }

  /* register contacts */
  ptr = NULL;
  xodtemplate_contact* temp_contact(NULL);
  for (temp_contact = (xodtemplate_contact*)skiplist_get_first(
           xobject_skiplists[X_CONTACT_SKIPLIST], &ptr);
       temp_contact;
       temp_contact = (xodtemplate_contact*)skiplist_get_next(&ptr)) {
    if (xodtemplate_register_contact(temp_contact) == ERROR)
      return ERROR;
  }

  /* register hosts */
  ptr = NULL;
  xodtemplate_host* temp_host(NULL);
  for (temp_host = (xodtemplate_host*)skiplist_get_first(
           xobject_skiplists[X_HOST_SKIPLIST], &ptr);
       temp_host; temp_host = (xodtemplate_host*)skiplist_get_next(&ptr)) {
    if (xodtemplate_register_host(temp_host) == ERROR)
      return ERROR;
  }

  /* register services */
  ptr = NULL;
  xodtemplate_service* temp_service(NULL);
  for (temp_service = (xodtemplate_service*)skiplist_get_first(
           xobject_skiplists[X_SERVICE_SKIPLIST], &ptr);
       temp_service;
       temp_service = (xodtemplate_service*)skiplist_get_next(&ptr)) {
    if (xodtemplate_register_service(temp_service) == ERROR)
      return ERROR;
  }

  /* register service dependencies */
  ptr = NULL;
  xodtemplate_servicedependency* temp_servicedependency(NULL);
  for (temp_servicedependency =
           (xodtemplate_servicedependency*)skiplist_get_first(
               xobject_skiplists[X_SERVICEDEPENDENCY_SKIPLIST], &ptr);
       temp_servicedependency;
       temp_servicedependency =
           (xodtemplate_servicedependency*)skiplist_get_next(&ptr)) {
    if (xodtemplate_register_servicedependency(temp_servicedependency) == ERROR)
      return ERROR;
  }

  /* register service escalations */
  ptr = NULL;
  xodtemplate_serviceescalation* temp_serviceescalation(NULL);
  for (temp_serviceescalation =
           (xodtemplate_serviceescalation*)skiplist_get_first(
               xobject_skiplists[X_SERVICEESCALATION_SKIPLIST], &ptr);
       temp_serviceescalation;
       temp_serviceescalation =
           (xodtemplate_serviceescalation*)skiplist_get_next(&ptr)) {
    if (xodtemplate_register_serviceescalation(temp_serviceescalation) == ERROR)
      return ERROR;
  }

  /* register host dependencies */
  ptr = NULL;
  xodtemplate_hostdependency* temp_hostdependency(NULL);
  for (temp_hostdependency = (xodtemplate_hostdependency*)skiplist_get_first(
           xobject_skiplists[X_HOSTDEPENDENCY_SKIPLIST], &ptr);
       temp_hostdependency;
       temp_hostdependency =
           (xodtemplate_hostdependency*)skiplist_get_next(&ptr)) {
    if (xodtemplate_register_hostdependency(temp_hostdependency) == ERROR)
      return ERROR;
  }

  /* register host escalations */
  ptr = NULL;
  xodtemplate_hostescalation* temp_hostescalation(NULL);
  for (temp_hostescalation = (xodtemplate_hostescalation*)skiplist_get_first(
           xobject_skiplists[X_HOSTESCALATION_SKIPLIST], &ptr);
       temp_hostescalation;
       temp_hostescalation =
           (xodtemplate_hostescalation*)skiplist_get_next(&ptr)) {
    if (xodtemplate_register_hostescalation(temp_hostescalation) == ERROR)
      return ERROR;
  }

  return OK;
}

/**
 *  Fill timeperiod with its content.
 *
 *  @param[in]  this_timeperiod Template timeperiod.
 *  @param[out] new_timeperiod  Real timeperiod.
 *
 *  @return OK on success.
 */
int xodtemplate_fill_timeperiod(xodtemplate_timeperiod* this_timeperiod,
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
         temp_daterange != NULL; temp_daterange = temp_daterange->next) {
      /* skip null entries */
      if (temp_daterange->timeranges == NULL ||
          !strcmp(temp_daterange->timeranges, XODTEMPLATE_NULL))
        continue;

      /* add new exception to timeperiod */
      new_daterange = add_exception_to_timeperiod(
          new_timeperiod, temp_daterange->type, temp_daterange->syear,
          temp_daterange->smon, temp_daterange->smday, temp_daterange->swday,
          temp_daterange->swday_offset, temp_daterange->eyear,
          temp_daterange->emon, temp_daterange->emday, temp_daterange->ewday,
          temp_daterange->ewday_offset, temp_daterange->skip_interval);
      if (new_daterange == NULL) {
        logger(log_config_error, basic)
            << "Error: Could not add date exception to timeperiod "
               "(config file '"
            << xodtemplate_config_file_name(this_timeperiod->_config_file)
            << "', starting on line " << this_timeperiod->_start_line << ")";
        return ERROR;
      }

      /* add timeranges to exception */
      day_range_ptr = temp_daterange->timeranges;
      range = 0;
      for (day_range_start_buffer = my_strsep(&day_range_ptr, ", ");
           day_range_start_buffer != NULL;
           day_range_start_buffer = my_strsep(&day_range_ptr, ", ")) {
        range++;

        /* get time ranges */
        if (xodtemplate_get_time_ranges(day_range_start_buffer,
                                        &range_start_time,
                                        &range_end_time) == ERROR) {
          logger(log_config_error, basic)
              << "Error: Could not parse timerange #" << range
              << " of timeperiod (config file '"
              << xodtemplate_config_file_name(this_timeperiod->_config_file)
              << "', starting on line " << this_timeperiod->_start_line << ")";
          return ERROR;
        }

        /* add the new time range to the date range */
        new_timerange = add_timerange_to_daterange(
            new_daterange, range_start_time, range_end_time);
        if (new_timerange == NULL) {
          logger(log_config_error, basic)
              << "Error: Could not add timerange #" << range
              << " to timeperiod (config file '"
              << xodtemplate_config_file_name(this_timeperiod->_config_file)
              << "', starting on line " << this_timeperiod->_start_line << ")";
          return ERROR;
        }
      }
    }
  }

  /* add all necessary timeranges to timeperiod */
  for (day = 0; day < 7; day++) {
    /* skip null entries */
    if (this_timeperiod->timeranges[day] == NULL ||
        !strcmp(this_timeperiod->timeranges[day], XODTEMPLATE_NULL))
      continue;

    day_range_ptr = this_timeperiod->timeranges[day];
    range = 0;
    for (day_range_start_buffer = my_strsep(&day_range_ptr, ", ");
         day_range_start_buffer != NULL;
         day_range_start_buffer = my_strsep(&day_range_ptr, ", ")) {
      range++;

      /* get time ranges */
      if (xodtemplate_get_time_ranges(day_range_start_buffer, &range_start_time,
                                      &range_end_time) == ERROR) {
        logger(log_config_error, basic)
            << "Error: Could not parse timerange #" << range << " for day "
            << day << " of timeperiod (config file '"
            << xodtemplate_config_file_name(this_timeperiod->_config_file)
            << "', starting on line " << this_timeperiod->_start_line << ")";
        return ERROR;
      }

      /* add the new time range to the time period */
      new_timerange = add_timerange_to_timeperiod(
          new_timeperiod, day, range_start_time, range_end_time);
      if (new_timerange == NULL) {
        logger(log_config_error, basic)
            << "Error: Could not add timerange #" << range << " for day " << day
            << " to timeperiod (config file '"
            << xodtemplate_config_file_name(this_timeperiod->_config_file)
            << "', starting on line " << this_timeperiod->_start_line << ")";
        return ERROR;
      }
    }
  }

  /* add timeperiod exclusions */
  if (this_timeperiod->exclusions) {
    for (temp_ptr = strtok(this_timeperiod->exclusions, ","); temp_ptr != NULL;
         temp_ptr = strtok(NULL, ",")) {
      strip(temp_ptr);
      new_timeperiodexclusion =
          add_exclusion_to_timeperiod(new_timeperiod, temp_ptr);
      if (new_timeperiodexclusion == NULL) {
        logger(log_config_error, basic)
            << "Error: Could not add excluded timeperiod '" << temp_ptr
            << "' to timeperiod (config file '"
            << xodtemplate_config_file_name(this_timeperiod->_config_file)
            << "', starting on line " << this_timeperiod->_start_line << ")";
        return ERROR;
      }
    }
  }

  return OK;
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
int xodtemplate_register_timeperiod(xodtemplate_timeperiod* this_timeperiod) {
  // Bail out if we shouldn't register this object.
  if (this_timeperiod->register_object == false)
    return OK;

  // Add the timeperiod.
  timeperiod* new_timeperiod(
      add_timeperiod(this_timeperiod->timeperiod_name, this_timeperiod->alias));

  // Return with an error if we couldn't add the timeperiod.
  if (!new_timeperiod) {
    logger(log_config_error, basic)
        << "Error: Could not register timeperiod (config file '"
        << xodtemplate_config_file_name(this_timeperiod->_config_file)
        << "', starting on line " << this_timeperiod->_start_line << ")";
    return ERROR;
  }

  return OK;
}

/* parses timerange string into start and end minutes */
int xodtemplate_get_time_ranges(char* buf,
                                unsigned long* range_start,
                                unsigned long* range_end) {
  char* range_ptr = NULL;
  char* range_buffer = NULL;
  char* time_ptr = NULL;
  char* time_buffer = NULL;
  int hours = 0;
  int minutes = 0;

  if (buf == NULL || range_start == NULL || range_end == NULL)
    return ERROR;

  range_ptr = buf;
  range_buffer = my_strsep(&range_ptr, "-");
  if (range_buffer == NULL)
    return ERROR;

  time_ptr = range_buffer;
  time_buffer = my_strsep(&time_ptr, ":");
  if (time_buffer == NULL)
    return ERROR;
  hours = atoi(time_buffer);

  time_buffer = my_strsep(&time_ptr, ":");
  if (time_buffer == NULL)
    return ERROR;
  minutes = atoi(time_buffer);

  /* calculate the range start time in seconds */
  *range_start = (unsigned long)((minutes * 60) + (hours * 60 * 60));

  range_buffer = my_strsep(&range_ptr, "-");
  if (range_buffer == NULL)
    return ERROR;

  time_ptr = range_buffer;
  time_buffer = my_strsep(&time_ptr, ":");
  if (time_buffer == NULL)
    return ERROR;
  hours = atoi(time_buffer);

  time_buffer = my_strsep(&time_ptr, ":");
  if (time_buffer == NULL)
    return ERROR;
  minutes = atoi(time_buffer);

  /* calculate the range end time in seconds */
  *range_end = (unsigned long)((minutes * 60) + (hours * 3600));

  return OK;
}

/* registers a command definition */
int xodtemplate_register_command(xodtemplate_command* this_command) {
  /* bail out if we shouldn't register this object */
  if (this_command->register_object == false)
    return OK;

  // Initialize command executon system.
  try {
    using namespace com::centreon::engine;
    commands::set& cmd_set(commands::set::instance());
    if (this_command->connector_name == NULL) {
      std::shared_ptr<commands::command> cmd(new commands::raw(
          this_command->command_name, this_command->command_line,
          &checks::checker::instance()));
      cmd_set.add_command(cmd);
    } else {
      std::shared_ptr<commands::command> cmd_forward(
          cmd_set.get_command(this_command->connector_name));

      std::shared_ptr<commands::command> cmd(
          new commands::forward(this_command->command_name,
                                this_command->command_line, *cmd_forward));
      cmd_set.add_command(cmd);
    }
  } catch (std::exception const& e) {
    logger(log_config_error, basic)
        << "Error: Could not register command (config file '"
        << xodtemplate_config_file_name(this_command->_config_file)
        << "', starting on line " << this_command->_start_line
        << "): " << e.what();
    return ERROR;
  }

  /* add the command */
  command* new_command(
      add_command(this_command->command_name, this_command->command_line));

  /* return with an error if we couldn't add the command */
  if (new_command == NULL) {
    logger(log_config_error, basic)
        << "Error: Could not register command (config file '"
        << xodtemplate_config_file_name(this_command->_config_file)
        << "', starting on line " << this_command->_start_line << ")";
    return ERROR;
  }

  return OK;
}

/* registers a connector definition */
int xodtemplate_register_connector(xodtemplate_connector* this_connector) {
  /* bail out if we shouldn't register this object */
  if (this_connector->register_object == false)
    return OK;

  // Initialize command executon system.
  try {
    using namespace com::centreon::engine;

    nagios_macros* macros(get_global_macros());
    char* command_line(NULL);
    process_macros_r(macros, this_connector->connector_line, &command_line, 0);
    std::string processed_cmd(command_line);
    delete[] command_line;

    std::shared_ptr<commands::command> cmd(
        new commands::connector(this_connector->connector_name, processed_cmd,
                                &checks::checker::instance()));
    commands::set::instance().add_command(cmd);
  } catch (std::exception const& e) {
    logger(log_config_error, basic)
        << "Error: Could not register connector (config file '"
        << xodtemplate_config_file_name(this_connector->_config_file)
        << "', starting on line " << this_connector->_start_line
        << "): " << e.what();
    return ERROR;
  }

  return OK;
}

/* registers a contactgroup definition */
int xodtemplate_register_contactgroup(
    xodtemplate_contactgroup* this_contactgroup) {
  /* bail out if we shouldn't register this object */
  if (this_contactgroup->register_object == false)
    return OK;

  /* add the contact group */
  contactgroup* new_contactgroup = add_contactgroup(
      this_contactgroup->contactgroup_name, this_contactgroup->alias);

  /* return with an error if we couldn't add the contactgroup */
  if (new_contactgroup == NULL) {
    logger(log_config_error, basic)
        << "Error: Could not register contactgroup (config file '"
        << xodtemplate_config_file_name(this_contactgroup->_config_file)
        << "', starting on line " << this_contactgroup->_start_line << ")";
    return ERROR;
  }

  /* Need to check for NULL because strtok could use a NULL value to check the
   * previous string's token value */
  if (this_contactgroup->members != NULL) {
    for (char* contact_name(strtok(this_contactgroup->members, ","));
         contact_name != NULL; contact_name = strtok(NULL, ",")) {
      strip(contact_name);
      contactsmember* new_contactsmember =
          add_contact_to_contactgroup(new_contactgroup, contact_name);
      if (new_contactsmember == NULL) {
        logger(log_config_error, basic)
            << "Error: Could not add contact '" << contact_name
            << "' to contactgroup (config file '"
            << xodtemplate_config_file_name(this_contactgroup->_config_file)
            << "', starting on line " << this_contactgroup->_start_line << ")";
        return ERROR;
      }
    }
  }

  return OK;
}

/* registers a hostgroup definition */
int xodtemplate_register_hostgroup(xodtemplate_hostgroup* this_hostgroup) {
  /* bail out if we shouldn't register this object */
  if (this_hostgroup->register_object == false)
    return OK;

  /* add the  host group */
  hostgroup* new_hostgroup =
      add_hostgroup(this_hostgroup->hostgroup_name, this_hostgroup->alias,
                    this_hostgroup->notes, this_hostgroup->notes_url,
                    this_hostgroup->action_url);

  /* return with an error if we couldn't add the hostgroup */
  if (new_hostgroup == NULL) {
    logger(log_config_error, basic)
        << "Error: Could not register hostgroup (config file '"
        << xodtemplate_config_file_name(this_hostgroup->_config_file)
        << "', starting on line " << this_hostgroup->_start_line << ")";
    return ERROR;
  }

  if (this_hostgroup->members != NULL) {
    for (char* host_name(strtok(this_hostgroup->members, ","));
         host_name != NULL; host_name = strtok(NULL, ",")) {
      strip(host_name);
      hostsmember* new_hostsmember =
          add_host_to_hostgroup(new_hostgroup, host_name);
      if (new_hostsmember == NULL) {
        logger(log_config_error, basic)
            << "Error: Could not add host '" << host_name
            << "' to hostgroup (config file '"
            << xodtemplate_config_file_name(this_hostgroup->_config_file)
            << "', starting on line " << this_hostgroup->_start_line << ")";
        return ERROR;
      }
    }
  }

  return OK;
}

/* registers a servicegroup definition */
int xodtemplate_register_servicegroup(
    xodtemplate_servicegroup* this_servicegroup) {
  /* bail out if we shouldn't register this object */
  if (this_servicegroup->register_object == false)
    return OK;

  /* add the  service group */
  servicegroup* new_servicegroup = add_servicegroup(
      this_servicegroup->servicegroup_name, this_servicegroup->alias,
      this_servicegroup->notes, this_servicegroup->notes_url,
      this_servicegroup->action_url);

  /* return with an error if we couldn't add the servicegroup */
  if (new_servicegroup == NULL) {
    logger(log_config_error, basic)
        << "Error: Could not register servicegroup (config file '"
        << xodtemplate_config_file_name(this_servicegroup->_config_file)
        << "', starting on line " << this_servicegroup->_start_line << ")";
    return ERROR;
  }

  if (this_servicegroup->members != NULL) {
    for (char* host_name(strtok(this_servicegroup->members, ","));
         host_name != NULL; host_name = strtok(NULL, ",")) {
      strip(host_name);
      char* svc_description(strtok(NULL, ","));
      if (svc_description == NULL) {
        logger(log_config_error, basic)
            << "Error: Missing service name in servicegroup definition "
               "(config file '"
            << xodtemplate_config_file_name(this_servicegroup->_config_file)
            << "', starting on line " << this_servicegroup->_start_line << ")";
        return ERROR;
      }
      strip(svc_description);

      servicesmember* new_servicesmember = add_service_to_servicegroup(
          new_servicegroup, host_name, svc_description);
      if (new_servicesmember == NULL) {
        logger(log_config_error, basic)
            << "Error: Could not add service '" << svc_description
            << "' on host '" << host_name
            << "' to servicegroup "
               "(config file '"
            << xodtemplate_config_file_name(this_servicegroup->_config_file)
            << "', starting on line " << this_servicegroup->_start_line << ")";
        return ERROR;
      }
    }
  }

  return OK;
}

/* registers a servicedependency definition */
int xodtemplate_register_servicedependency(
    xodtemplate_servicedependency* this_servicedependency) {
  servicedependency* new_servicedependency = NULL;

  /* bail out if we shouldn't register this object */
  if (this_servicedependency->register_object == false)
    return OK;

  /* throw a warning on servicedeps that have no options */
  if (this_servicedependency->have_notification_dependency_options == false &&
      this_servicedependency->have_execution_dependency_options == false) {
    logger(log_config_warning, basic)
        << "Warning: Ignoring lame service dependency (config file '"
        << xodtemplate_config_file_name(this_servicedependency->_config_file)
        << "', line " << this_servicedependency->_start_line << ")";
    return OK;
  }

  /* add the servicedependency */
  if (this_servicedependency->have_execution_dependency_options == true) {
    new_servicedependency = add_service_dependency(
        this_servicedependency->dependent_host_name,
        this_servicedependency->dependent_service_description,
        this_servicedependency->host_name,
        this_servicedependency->service_description, EXECUTION_DEPENDENCY,
        this_servicedependency->inherits_parent,
        this_servicedependency->fail_execute_on_ok,
        this_servicedependency->fail_execute_on_warning,
        this_servicedependency->fail_execute_on_unknown,
        this_servicedependency->fail_execute_on_critical,
        this_servicedependency->fail_execute_on_pending,
        this_servicedependency->dependency_period);

    /* return with an error if we couldn't add the servicedependency */
    if (new_servicedependency == NULL) {
      logger(log_config_error, basic)
          << "Error: Could not register service execution dependency "
             "(config file '"
          << xodtemplate_config_file_name(this_servicedependency->_config_file)
          << "', starting on line " << this_servicedependency->_start_line
          << ")";
      return ERROR;
    }
  }
  if (this_servicedependency->have_notification_dependency_options == true) {
    new_servicedependency = add_service_dependency(
        this_servicedependency->dependent_host_name,
        this_servicedependency->dependent_service_description,
        this_servicedependency->host_name,
        this_servicedependency->service_description, NOTIFICATION_DEPENDENCY,
        this_servicedependency->inherits_parent,
        this_servicedependency->fail_notify_on_ok,
        this_servicedependency->fail_notify_on_warning,
        this_servicedependency->fail_notify_on_unknown,
        this_servicedependency->fail_notify_on_critical,
        this_servicedependency->fail_notify_on_pending,
        this_servicedependency->dependency_period);

    /* return with an error if we couldn't add the servicedependency */
    if (new_servicedependency == NULL) {
      logger(log_config_error, basic)
          << "Error: Could not register service notification dependency "
             "(config file '"
          << xodtemplate_config_file_name(this_servicedependency->_config_file)
          << "', starting on line " << this_servicedependency->_start_line
          << ")";
      return ERROR;
    }
  }

  return OK;
}

/* registers a serviceescalation definition */
int xodtemplate_register_serviceescalation(
    xodtemplate_serviceescalation* this_serviceescalation) {
  /* bail out if we shouldn't register this object */
  if (this_serviceescalation->register_object == false)
    return OK;

  /* default options if none specified */
  if (this_serviceescalation->have_escalation_options == false) {
    this_serviceescalation->escalate_on_warning = true;
    this_serviceescalation->escalate_on_unknown = true;
    this_serviceescalation->escalate_on_critical = true;
    this_serviceescalation->escalate_on_recovery = true;
  }

  /* add the serviceescalation */
  serviceescalation* new_serviceescalation =
      add_service_escalation(this_serviceescalation->host_name,
                             this_serviceescalation->service_description,
                             this_serviceescalation->first_notification,
                             this_serviceescalation->last_notification,
                             this_serviceescalation->notification_interval,
                             this_serviceescalation->escalation_period,
                             this_serviceescalation->escalate_on_warning,
                             this_serviceescalation->escalate_on_unknown,
                             this_serviceescalation->escalate_on_critical,
                             this_serviceescalation->escalate_on_recovery);

  /* return with an error if we couldn't add the serviceescalation */
  if (new_serviceescalation == NULL) {
    logger(log_config_error, basic)
        << "Error: Could not register service escalation (config file '"
        << xodtemplate_config_file_name(this_serviceescalation->_config_file)
        << "', starting on line " << this_serviceescalation->_start_line << ")";
    return ERROR;
  }

  /* add the contact groups */
  if (this_serviceescalation->contact_groups != NULL) {
    for (char* contact_group(
             strtok(this_serviceescalation->contact_groups, ","));
         contact_group != NULL; contact_group = strtok(NULL, ", ")) {
      strip(contact_group);
      contactgroupsmember* new_contactgroupsmember =
          add_contactgroup_to_serviceescalation(new_serviceescalation,
                                                contact_group);
      if (new_contactgroupsmember == NULL) {
        logger(log_config_error, basic)
            << "Error: Could not add contactgroup '" << contact_group
            << "' to service escalation (config file '"
            << xodtemplate_config_file_name(
                   this_serviceescalation->_config_file)
            << "', starting on line " << this_serviceescalation->_start_line
            << ")";
        return ERROR;
      }
    }
  }

  /* add the contacts */
  if (this_serviceescalation->contacts != NULL) {
    for (char* contact_name(strtok(this_serviceescalation->contacts, ","));
         contact_name != NULL; contact_name = strtok(NULL, ", ")) {
      strip(contact_name);
      contactsmember* new_contactsmember =
          add_contact_to_serviceescalation(new_serviceescalation, contact_name);
      if (new_contactsmember == NULL) {
        logger(log_config_error, basic)
            << "Error: Could not add contact '" << contact_name
            << "' to service escalation (config file '"
            << xodtemplate_config_file_name(
                   this_serviceescalation->_config_file)
            << "', starting on line " << this_serviceescalation->_start_line
            << ")";
        return ERROR;
      }
    }
  }

  return OK;
}

/* registers a contact definition */
int xodtemplate_register_contact(xodtemplate_contact* this_contact) {
  contact* new_contact = NULL;
  char* command_name = NULL;
  commandsmember* new_commandsmember = NULL;
  xodtemplate_customvariablesmember* temp_customvariablesmember = NULL;

  /* bail out if we shouldn't register this object */
  if (this_contact->register_object == false)
    return OK;

  /* add the contact */
  new_contact = add_contact(
      this_contact->contact_name, this_contact->alias, this_contact->email,
      this_contact->pager, this_contact->address,
      this_contact->service_notification_period,
      this_contact->host_notification_period,
      this_contact->notify_on_service_recovery,
      this_contact->notify_on_service_critical,
      this_contact->notify_on_service_warning,
      this_contact->notify_on_service_unknown,
      this_contact->notify_on_service_flapping,
      this_contact->notify_on_service_downtime,
      this_contact->notify_on_host_recovery, this_contact->notify_on_host_down,
      this_contact->notify_on_host_unreachable,
      this_contact->notify_on_host_flapping,
      this_contact->notify_on_host_downtime,
      this_contact->host_notifications_enabled,
      this_contact->service_notifications_enabled,
      this_contact->can_submit_commands,
      this_contact->retain_status_information,
      this_contact->retain_nonstatus_information);

  /* return with an error if we couldn't add the contact */
  if (new_contact == NULL) {
    logger(log_config_error, basic)
        << "Error: Could not register contact (config file '"
        << xodtemplate_config_file_name(this_contact->_config_file)
        << "', starting on line " << this_contact->_start_line << ")";
    return ERROR;
  }

  /* add all the host notification commands */
  if (this_contact->host_notification_commands != NULL) {
    for (command_name = strtok(this_contact->host_notification_commands, ", ");
         command_name != NULL; command_name = strtok(NULL, ", ")) {
      new_commandsmember =
          add_host_notification_command_to_contact(new_contact, command_name);
      if (new_commandsmember == NULL) {
        logger(log_config_error, basic)
            << "Error: Could not add host notification command '"
            << command_name << "' to contact (config file '"
            << xodtemplate_config_file_name(this_contact->_config_file)
            << "', starting on line " << this_contact->_start_line << ")";
        return ERROR;
      }
    }
  }

  /* add all the service notification commands */
  if (this_contact->service_notification_commands != NULL) {
    for (command_name =
             strtok(this_contact->service_notification_commands, ", ");
         command_name != NULL; command_name = strtok(NULL, ", ")) {
      new_commandsmember = add_service_notification_command_to_contact(
          new_contact, command_name);
      if (new_commandsmember == NULL) {
        logger(log_config_error, basic)
            << "Error: Could not add service notification command '"
            << command_name << "' to contact (config file '"
            << xodtemplate_config_file_name(this_contact->_config_file)
            << "', starting on line " << this_contact->_start_line << ")";
        return ERROR;
      }
    }
  }

  /* add all custom variables */
  for (temp_customvariablesmember = this_contact->custom_variables;
       temp_customvariablesmember != NULL;
       temp_customvariablesmember = temp_customvariablesmember->next) {
    if ((add_custom_variable_to_contact(
            new_contact, temp_customvariablesmember->variable_name,
            temp_customvariablesmember->variable_value)) == NULL) {
      logger(log_config_error, basic)
          << "Error: Could not custom variable to contact (config file '"
          << xodtemplate_config_file_name(this_contact->_config_file)
          << "', starting on line " << this_contact->_start_line << ")";
      return ERROR;
    }
  }

  return OK;
}

/* registers a host definition */
int xodtemplate_register_host(xodtemplate_host* this_host) {
  host* new_host = NULL;
  char* parent_host = NULL;
  hostsmember* new_hostsmember = NULL;
  contactsmember* new_contactsmember = NULL;
  contactgroupsmember* new_contactgroupsmember = NULL;
  char* contact_name = NULL;
  char* contact_group = NULL;
  xodtemplate_customvariablesmember* temp_customvariablesmember = NULL;

  /* bail out if we shouldn't register this object */
  if (this_host->register_object == false)
    return OK;

  /* if host has no alias or address, use host name - added 3/11/05 */
  if (this_host->alias == NULL && this_host->host_name != NULL)
    this_host->alias = string::dup(this_host->host_name);
  if (this_host->address == NULL && this_host->host_name != NULL)
    this_host->address = string::dup(this_host->host_name);

  /* add the host definition */
  new_host = add_host(
      this_host->host_name, this_host->display_name, this_host->alias,
      (this_host->address == NULL) ? this_host->host_name : this_host->address,
      this_host->check_period, this_host->initial_state,
      this_host->check_interval, this_host->retry_interval,
      this_host->max_check_attempts, this_host->notify_on_recovery,
      this_host->notify_on_down, this_host->notify_on_unreachable,
      this_host->notify_on_flapping, this_host->notify_on_downtime,
      this_host->notification_interval, this_host->first_notification_delay,
      this_host->notification_period, this_host->notifications_enabled,
      this_host->check_command, this_host->active_checks_enabled,
      this_host->passive_checks_enabled, this_host->event_handler,
      this_host->event_handler_enabled, this_host->flap_detection_enabled,
      this_host->low_flap_threshold, this_host->high_flap_threshold,
      this_host->flap_detection_on_up, this_host->flap_detection_on_down,
      this_host->flap_detection_on_unreachable, this_host->stalk_on_up,
      this_host->stalk_on_down, this_host->stalk_on_unreachable,
      this_host->process_perf_data, this_host->check_freshness,
      this_host->freshness_threshold, this_host->notes, this_host->notes_url,
      this_host->action_url, this_host->icon_image, this_host->icon_image_alt,
      this_host->vrml_image, this_host->statusmap_image, this_host->x_2d,
      this_host->y_2d, this_host->have_2d_coords, this_host->x_3d,
      this_host->y_3d, this_host->z_3d, this_host->have_3d_coords, true,
      this_host->retain_status_information,
      this_host->retain_nonstatus_information, this_host->obsess_over_host);

  /* return with an error if we couldn't add the host */
  if (new_host == NULL) {
    logger(log_config_error, basic)
        << "Error: Could not register host (config file '"
        << xodtemplate_config_file_name(this_host->_config_file)
        << "', starting on line " << this_host->_start_line << ")";
    return ERROR;
  }

  /* add the parent hosts */
  if (this_host->parents != NULL) {
    for (parent_host = strtok(this_host->parents, ","); parent_host != NULL;
         parent_host = strtok(NULL, ",")) {
      strip(parent_host);
      new_hostsmember = add_parent_host_to_host(new_host, parent_host);
      if (new_hostsmember == NULL) {
        logger(log_config_error, basic)
            << "Error: Could not add parent host '" << parent_host
            << "' to host (config file '"
            << xodtemplate_config_file_name(this_host->_config_file)
            << "', starting on line " << this_host->_start_line << ")";
        return ERROR;
      }
    }
  }

  /* add all contact groups to the host */
  if (this_host->contact_groups != NULL) {
    for (contact_group = strtok(this_host->contact_groups, ",");
         contact_group != NULL; contact_group = strtok(NULL, ",")) {
      strip(contact_group);
      new_contactgroupsmember =
          add_contactgroup_to_host(new_host, contact_group);
      if (new_contactgroupsmember == NULL) {
        logger(log_config_error, basic)
            << "Error: Could not add contactgroup '" << contact_group
            << "' to host (config file '"
            << xodtemplate_config_file_name(this_host->_config_file)
            << "', starting on line " << this_host->_start_line << ")";
        return ERROR;
      }
    }
  }

  /* add all contacts to the host */
  if (this_host->contacts != NULL) {
    for (contact_name = strtok(this_host->contacts, ","); contact_name != NULL;
         contact_name = strtok(NULL, ",")) {
      strip(contact_name);
      new_contactsmember = add_contact_to_host(new_host, contact_name);
      if (new_contactsmember == NULL) {
        logger(log_config_error, basic)
            << "Error: Could not add contact '" << contact_name
            << "' to host (config file '"
            << xodtemplate_config_file_name(this_host->_config_file)
            << "', starting on line " << this_host->_start_line << ")";
        return ERROR;
      }
    }
  }

  /* add all custom variables */
  for (temp_customvariablesmember = this_host->custom_variables;
       temp_customvariablesmember != NULL;
       temp_customvariablesmember = temp_customvariablesmember->next) {
    if ((add_custom_variable_to_host(
            new_host, temp_customvariablesmember->variable_name,
            temp_customvariablesmember->variable_value)) == NULL) {
      logger(log_config_error, basic)
          << "Error: Could not custom variable to host (config file '"
          << xodtemplate_config_file_name(this_host->_config_file)
          << "', starting on line " << this_host->_start_line << ")";
      return ERROR;
    }
  }

  return OK;
}

/* registers a service definition */
int xodtemplate_register_service(xodtemplate_service* this_service) {
  service* new_service = NULL;
  contactsmember* new_contactsmember = NULL;
  contactgroupsmember* new_contactgroupsmember = NULL;
  char* contact_name = NULL;
  char* contact_group = NULL;
  xodtemplate_customvariablesmember* temp_customvariablesmember = NULL;

  /* bail out if we shouldn't register this object */
  if (this_service->register_object == false)
    return OK;

  /* add the service */
  new_service = add_service(
      this_service->host_name, this_service->service_description,
      this_service->display_name, this_service->check_period,
      this_service->initial_state, this_service->max_check_attempts,
      this_service->parallelize_check, this_service->passive_checks_enabled,
      this_service->check_interval, this_service->retry_interval,
      this_service->notification_interval,
      this_service->first_notification_delay, this_service->notification_period,
      this_service->notify_on_recovery, this_service->notify_on_unknown,
      this_service->notify_on_warning, this_service->notify_on_critical,
      this_service->notify_on_flapping, this_service->notify_on_downtime,
      this_service->notifications_enabled, this_service->is_volatile,
      this_service->event_handler, this_service->event_handler_enabled,
      this_service->check_command, this_service->active_checks_enabled,
      this_service->flap_detection_enabled, this_service->low_flap_threshold,
      this_service->high_flap_threshold, this_service->flap_detection_on_ok,
      this_service->flap_detection_on_warning,
      this_service->flap_detection_on_unknown,
      this_service->flap_detection_on_critical, this_service->stalk_on_ok,
      this_service->stalk_on_warning, this_service->stalk_on_unknown,
      this_service->stalk_on_critical, this_service->process_perf_data,
      this_service->check_freshness, this_service->freshness_threshold,
      this_service->notes, this_service->notes_url, this_service->action_url,
      this_service->icon_image, this_service->icon_image_alt,
      this_service->retain_status_information,
      this_service->retain_nonstatus_information,
      this_service->obsess_over_service);

  /* return with an error if we couldn't add the service */
  if (new_service == NULL) {
    logger(log_config_error, basic)
        << "Error: Could not register service (config file '"
        << xodtemplate_config_file_name(this_service->_config_file)
        << "', starting on line " << this_service->_start_line << ")";
    return ERROR;
  }

  /* add all contact groups to the service */
  if (this_service->contact_groups != NULL) {
    for (contact_group = strtok(this_service->contact_groups, ",");
         contact_group != NULL; contact_group = strtok(NULL, ",")) {
      strip(contact_group);
      new_contactgroupsmember =
          add_contactgroup_to_service(new_service, contact_group);
      if (new_contactgroupsmember == NULL) {
        logger(log_config_error, basic)
            << "Error: Could not add contactgroup '" << contact_group
            << "' to service (config file '"
            << xodtemplate_config_file_name(this_service->_config_file)
            << "', starting on line " << this_service->_start_line << ")";
        return ERROR;
      }
    }
  }

  /* add all the contacts to the service */
  if (this_service->contacts != NULL) {
    for (contact_name = strtok(this_service->contacts, ",");
         contact_name != NULL; contact_name = strtok(NULL, ",")) {
      /* add this contact to the service definition */
      strip(contact_name);
      new_contactsmember = add_contact_to_service(new_service, contact_name);

      /* stop adding contacts if we ran into an error */
      if (new_contactsmember == NULL) {
        logger(log_config_error, basic)
            << "Error: Could not add contact '" << contact_name
            << "' to service (config file '"
            << xodtemplate_config_file_name(this_service->_config_file)
            << "', starting on line " << this_service->_start_line << ")";
        return ERROR;
      }
    }
  }

  /* add all custom variables */
  for (temp_customvariablesmember = this_service->custom_variables;
       temp_customvariablesmember != NULL;
       temp_customvariablesmember = temp_customvariablesmember->next) {
    if ((add_custom_variable_to_service(
            new_service, temp_customvariablesmember->variable_name,
            temp_customvariablesmember->variable_value)) == NULL) {
      logger(log_config_error, basic)
          << "Error: Could not custom variable to service (config file '"
          << xodtemplate_config_file_name(this_service->_config_file)
          << "', starting on line " << this_service->_start_line << ")";
      return ERROR;
    }
  }

  return OK;
}

/* registers a hostdependency definition */
int xodtemplate_register_hostdependency(
    xodtemplate_hostdependency* this_hostdependency) {
  hostdependency* new_hostdependency = NULL;

  /* bail out if we shouldn't register this object */
  if (this_hostdependency->register_object == false)
    return OK;

  /* add the host execution dependency */
  if (this_hostdependency->have_execution_dependency_options == true) {
    new_hostdependency = add_host_dependency(
        this_hostdependency->dependent_host_name,
        this_hostdependency->host_name, EXECUTION_DEPENDENCY,
        this_hostdependency->inherits_parent,
        this_hostdependency->fail_execute_on_up,
        this_hostdependency->fail_execute_on_down,
        this_hostdependency->fail_execute_on_unreachable,
        this_hostdependency->fail_execute_on_pending,
        this_hostdependency->dependency_period);

    /* return with an error if we couldn't add the hostdependency */
    if (new_hostdependency == NULL) {
      logger(log_config_error, basic)
          << "Error: Could not register host execution dependency "
             "(config file '"
          << xodtemplate_config_file_name(this_hostdependency->_config_file)
          << "', starting on line " << this_hostdependency->_start_line << ")";
      return ERROR;
    }
  }

  /* add the host notification dependency */
  if (this_hostdependency->have_notification_dependency_options == true) {
    new_hostdependency = add_host_dependency(
        this_hostdependency->dependent_host_name,
        this_hostdependency->host_name, NOTIFICATION_DEPENDENCY,
        this_hostdependency->inherits_parent,
        this_hostdependency->fail_notify_on_up,
        this_hostdependency->fail_notify_on_down,
        this_hostdependency->fail_notify_on_unreachable,
        this_hostdependency->fail_notify_on_pending,
        this_hostdependency->dependency_period);

    /* return with an error if we couldn't add the hostdependency */
    if (new_hostdependency == NULL) {
      logger(log_config_error, basic)
          << "Error: Could not register host notification dependency "
             "(config file '"
          << xodtemplate_config_file_name(this_hostdependency->_config_file)
          << "', starting on line " << this_hostdependency->_start_line << ")";
      return ERROR;
    }
  }

  return OK;
}

/* registers a hostescalation definition */
int xodtemplate_register_hostescalation(
    xodtemplate_hostescalation* this_hostescalation) {
  /* bail out if we shouldn't register this object */
  if (this_hostescalation->register_object == false)
    return OK;

  /* default options if none specified */
  if (this_hostescalation->have_escalation_options == false) {
    this_hostescalation->escalate_on_down = true;
    this_hostescalation->escalate_on_unreachable = true;
    this_hostescalation->escalate_on_recovery = true;
  }

  /* add the hostescalation */
  hostescalation* new_hostescalation = add_host_escalation(
      this_hostescalation->host_name, this_hostescalation->first_notification,
      this_hostescalation->last_notification,
      this_hostescalation->notification_interval,
      this_hostescalation->escalation_period,
      this_hostescalation->escalate_on_down,
      this_hostescalation->escalate_on_unreachable,
      this_hostescalation->escalate_on_recovery);

  /* return with an error if we couldn't add the hostescalation */
  if (new_hostescalation == NULL) {
    logger(log_config_error, basic)
        << "Error: Could not register host escalation (config file '"
        << xodtemplate_config_file_name(this_hostescalation->_config_file)
        << "', starting on line " << this_hostescalation->_start_line << ")";
    return ERROR;
  }

  /* add all contact groups */
  if (this_hostescalation->contact_groups != NULL) {
    for (char* contact_group(strtok(this_hostescalation->contact_groups, ","));
         contact_group != NULL; contact_group = strtok(NULL, ",")) {
      strip(contact_group);
      contactgroupsmember* new_contactgroupsmember =
          add_contactgroup_to_host_escalation(new_hostescalation,
                                              contact_group);
      if (new_contactgroupsmember == NULL) {
        logger(log_config_error, basic)
            << "Error: Could not add contactgroup '" << contact_group
            << "' to host escalation (config file '"
            << xodtemplate_config_file_name(this_hostescalation->_config_file)
            << "', starting on line " << this_hostescalation->_start_line
            << ")";
        return ERROR;
      }
    }
  }

  /* add the contacts */
  if (this_hostescalation->contacts != NULL) {
    for (char* contact_name(strtok(this_hostescalation->contacts, ","));
         contact_name != NULL; contact_name = strtok(NULL, ", ")) {
      strip(contact_name);
      contactsmember* new_contactsmember =
          add_contact_to_host_escalation(new_hostescalation, contact_name);
      if (new_contactsmember == NULL) {
        logger(log_config_error, basic)
            << "Error: Could not add contact '" << contact_name
            << "' to host escalation (config file '"
            << xodtemplate_config_file_name(this_hostescalation->_config_file)
            << "', starting on line " << this_hostescalation->_start_line
            << ")";
        return ERROR;
      }
    }
  }

  return OK;
}

/******************************************************************/
/********************** SORTING FUNCTIONS *************************/
/******************************************************************/

/* sorts all objects by name */
int xodtemplate_sort_objects() {
  /* NOTE: with skiplists, we no longer need to sort things manually... */
  return OK;

  /* sort timeperiods */
  if (xodtemplate_sort_timeperiods() == ERROR)
    return ERROR;

  /* sort commands */
  if (xodtemplate_sort_commands() == ERROR)
    return ERROR;

  /* sort connectors */
  if (xodtemplate_sort_connectors() == ERROR)
    return ERROR;

  /* sort contactgroups */
  if (xodtemplate_sort_contactgroups() == ERROR)
    return ERROR;

  /* sort hostgroups */
  if (xodtemplate_sort_hostgroups() == ERROR)
    return ERROR;

  /* sort servicegroups */
  if (xodtemplate_sort_servicegroups() == ERROR)
    return ERROR;

  /* sort contacts */
  if (xodtemplate_sort_contacts() == ERROR)
    return ERROR;

  /* sort hosts */
  if (xodtemplate_sort_hosts() == ERROR)
    return ERROR;

  /* sort services */
  if (xodtemplate_sort_services() == ERROR)
    return ERROR;

  /* sort service dependencies */
  if (xodtemplate_sort_servicedependencies() == ERROR)
    return ERROR;

  /* sort service escalations */
  if (xodtemplate_sort_serviceescalations() == ERROR)
    return ERROR;

  /* sort host dependencies */
  if (xodtemplate_sort_hostdependencies() == ERROR)
    return ERROR;

  /* sort hostescalations */
  if (xodtemplate_sort_hostescalations() == ERROR)
    return ERROR;

  /* sort host extended info */
  /* NOT NEEDED */

  /* sort service extended info */
  /* NOT NEEDED */

  return OK;
}

/* used to compare two strings (object names) */
int xodtemplate_compare_strings1(char* string1, char* string2) {
  if (string1 == NULL && string2 == NULL)
    return 0;
  else if (string1 == NULL)
    return -1;
  else if (string2 == NULL)
    return 1;
  else
    return strcmp(string1, string2);
}

/* used to compare two sets of strings (dually-named objects, i.e. services) */
int xodtemplate_compare_strings2(char* string1a,
                                 char* string1b,
                                 char* string2a,
                                 char* string2b) {
  int result;
  if ((result = xodtemplate_compare_strings1(string1a, string2a)) == 0)
    result = xodtemplate_compare_strings1(string1b, string2b);
  return result;
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
    for (temp_timeperiod = new_timeperiod_list; temp_timeperiod != NULL;
         temp_timeperiod = temp_timeperiod->next) {
      if (xodtemplate_compare_strings1(temp_timeperiod_orig->timeperiod_name,
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

  return OK;
}

/* sort commands by name */
int xodtemplate_sort_commands() {
  xodtemplate_command* new_command_list = NULL;
  xodtemplate_command* temp_command = NULL;
  xodtemplate_command* last_command = NULL;
  xodtemplate_command* temp_command_orig = NULL;
  xodtemplate_command* next_command_orig = NULL;

  /* sort all existing commands */
  for (temp_command_orig = xodtemplate_command_list; temp_command_orig != NULL;
       temp_command_orig = next_command_orig) {
    next_command_orig = temp_command_orig->next;

    /* add command to new list, sorted by command name */
    last_command = new_command_list;
    for (temp_command = new_command_list; temp_command != NULL;
         temp_command = temp_command->next) {
      if (xodtemplate_compare_strings1(temp_command_orig->command_name,
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

  return OK;
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
       temp_connector_orig != NULL; temp_connector_orig = next_connector_orig) {
    next_connector_orig = temp_connector_orig->next;

    /* add connector to new list, sorted by connector name */
    last_connector = new_connector_list;
    for (temp_connector = new_connector_list; temp_connector != NULL;
         temp_connector = temp_connector->next) {
      if (xodtemplate_compare_strings1(temp_connector_orig->connector_name,
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

  return OK;
}

/* sort contactgroups by name */
int xodtemplate_sort_contactgroups() {
  xodtemplate_contactgroup* new_contactgroup_list = NULL;
  xodtemplate_contactgroup* temp_contactgroup = NULL;
  xodtemplate_contactgroup* last_contactgroup = NULL;
  xodtemplate_contactgroup* temp_contactgroup_orig = NULL;
  xodtemplate_contactgroup* next_contactgroup_orig = NULL;

  /* sort all existing contactgroups */
  for (temp_contactgroup_orig = xodtemplate_contactgroup_list;
       temp_contactgroup_orig != NULL;
       temp_contactgroup_orig = next_contactgroup_orig) {
    next_contactgroup_orig = temp_contactgroup_orig->next;

    /* add contactgroup to new list, sorted by contactgroup name */
    last_contactgroup = new_contactgroup_list;
    for (temp_contactgroup = new_contactgroup_list; temp_contactgroup != NULL;
         temp_contactgroup = temp_contactgroup->next) {
      if (xodtemplate_compare_strings1(
              temp_contactgroup_orig->contactgroup_name,
              temp_contactgroup->contactgroup_name) <= 0)
        break;
      else
        last_contactgroup = temp_contactgroup;
    }

    /* first item added to new sorted list */
    if (new_contactgroup_list == NULL) {
      temp_contactgroup_orig->next = NULL;
      new_contactgroup_list = temp_contactgroup_orig;
    }

    /* item goes at head of new sorted list */
    else if (temp_contactgroup == new_contactgroup_list) {
      temp_contactgroup_orig->next = new_contactgroup_list;
      new_contactgroup_list = temp_contactgroup_orig;
    }

    /* item goes in middle or at end of new sorted list */
    else {
      temp_contactgroup_orig->next = temp_contactgroup;
      last_contactgroup->next = temp_contactgroup_orig;
    }
  }

  /* list is now sorted */
  xodtemplate_contactgroup_list = new_contactgroup_list;

  return OK;
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
       temp_hostgroup_orig != NULL; temp_hostgroup_orig = next_hostgroup_orig) {
    next_hostgroup_orig = temp_hostgroup_orig->next;

    /* add hostgroup to new list, sorted by hostgroup name */
    last_hostgroup = new_hostgroup_list;
    for (temp_hostgroup = new_hostgroup_list; temp_hostgroup != NULL;
         temp_hostgroup = temp_hostgroup->next) {
      if (xodtemplate_compare_strings1(temp_hostgroup_orig->hostgroup_name,
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

  return OK;
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
    for (temp_servicegroup = new_servicegroup_list; temp_servicegroup != NULL;
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

  return OK;
}

/* sort contacts by name */
int xodtemplate_sort_contacts() {
  xodtemplate_contact* new_contact_list = NULL;
  xodtemplate_contact* temp_contact = NULL;
  xodtemplate_contact* last_contact = NULL;
  xodtemplate_contact* temp_contact_orig = NULL;
  xodtemplate_contact* next_contact_orig = NULL;

  /* sort all existing contacts */
  for (temp_contact_orig = xodtemplate_contact_list; temp_contact_orig != NULL;
       temp_contact_orig = next_contact_orig) {
    next_contact_orig = temp_contact_orig->next;

    /* add contact to new list, sorted by contact name */
    last_contact = new_contact_list;
    for (temp_contact = new_contact_list; temp_contact != NULL;
         temp_contact = temp_contact->next) {
      if (xodtemplate_compare_strings1(temp_contact_orig->contact_name,
                                       temp_contact->contact_name) <= 0)
        break;
      else
        last_contact = temp_contact;
    }

    /* first item added to new sorted list */
    if (new_contact_list == NULL) {
      temp_contact_orig->next = NULL;
      new_contact_list = temp_contact_orig;
    }

    /* item goes at head of new sorted list */
    else if (temp_contact == new_contact_list) {
      temp_contact_orig->next = new_contact_list;
      new_contact_list = temp_contact_orig;
    }

    /* item goes in middle or at end of new sorted list */
    else {
      temp_contact_orig->next = temp_contact;
      last_contact->next = temp_contact_orig;
    }
  }

  /* list is now sorted */
  xodtemplate_contact_list = new_contact_list;

  return OK;
}

int xodtemplate_compare_host(void* arg1, void* arg2) {
  xodtemplate_host* h1 = NULL;
  xodtemplate_host* h2 = NULL;
  int x = 0;

  h1 = (xodtemplate_host*)arg1;
  h2 = (xodtemplate_host*)arg2;

  if (h1 == NULL && h2 == NULL)
    return 0;
  if (h1 == NULL)
    return 1;
  if (h2 == NULL)
    return -1;

  x = strcmp((h1->host_name == NULL) ? "" : h1->host_name,
             (h2->host_name == NULL) ? "" : h2->host_name);

  return x;
}

/* sort hosts by name */
int xodtemplate_sort_hosts() {
#ifdef NEWSTUFF
  xodtemplate_host* temp_host = NULL;

  /* initialize a new skip list */
  if ((xodtemplate_host_skiplist =
           skiplist_new(15, 0.5, false, xodtemplate_compare_host)) == NULL)
    return ERROR;

  /* add all hosts to skip list */
  for (temp_host = xodtemplate_host_list; temp_host != NULL;
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
  for (temp_host_orig = xodtemplate_host_list; temp_host_orig != NULL;
       temp_host_orig = next_host_orig) {
    next_host_orig = temp_host_orig->next;

    /* add host to new list, sorted by host name */
    last_host = new_host_list;
    for (temp_host = new_host_list; temp_host != NULL;
         temp_host = temp_host->next) {
      if (xodtemplate_compare_strings1(temp_host_orig->host_name,
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

  return OK;
}

int xodtemplate_compare_service(void* arg1, void* arg2) {
  xodtemplate_service* s1 = NULL;
  xodtemplate_service* s2 = NULL;
  int x = 0;

  s1 = (xodtemplate_service*)arg1;
  s2 = (xodtemplate_service*)arg2;

  if (s1 == NULL && s2 == NULL)
    return 0;
  if (s1 == NULL)
    return 1;
  if (s2 == NULL)
    return -1;

  x = strcmp((s1->host_name == NULL) ? "" : s1->host_name,
             (s2->host_name == NULL) ? "" : s2->host_name);
  if (x == 0)
    x = strcmp(
        (s1->service_description == NULL) ? "" : s1->service_description,
        (s2->service_description == NULL) ? "" : s2->service_description);

  return x;
}

/* sort services by name */
int xodtemplate_sort_services() {
#ifdef NEWSTUFF
  xodtemplate_service* temp_service = NULL;

  /* initialize a new skip list */
  if ((xodtemplate_service_skiplist =
           skiplist_new(15, 0.5, false, xodtemplate_compare_service)) == NULL)
    return ERROR;

  /* add all services to skip list */
  for (temp_service = xodtemplate_service_list; temp_service != NULL;
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
  for (temp_service_orig = xodtemplate_service_list; temp_service_orig != NULL;
       temp_service_orig = next_service_orig) {
    next_service_orig = temp_service_orig->next;

    /* add service to new list, sorted by host name then service description */
    last_service = new_service_list;
    for (temp_service = new_service_list; temp_service != NULL;
         temp_service = temp_service->next) {
      if (xodtemplate_compare_strings2(temp_service_orig->host_name,
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

  return OK;
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

    /* add servicedependency to new list, sorted by host name then service
     * description */
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

  return OK;
}

/* sort serviceescalations by name */
int xodtemplate_sort_serviceescalations() {
  xodtemplate_serviceescalation* new_serviceescalation_list = NULL;
  xodtemplate_serviceescalation* temp_serviceescalation = NULL;
  xodtemplate_serviceescalation* last_serviceescalation = NULL;
  xodtemplate_serviceescalation* temp_serviceescalation_orig = NULL;
  xodtemplate_serviceescalation* next_serviceescalation_orig = NULL;

  /* sort all existing serviceescalations */
  for (temp_serviceescalation_orig = xodtemplate_serviceescalation_list;
       temp_serviceescalation_orig != NULL;
       temp_serviceescalation_orig = next_serviceescalation_orig) {
    next_serviceescalation_orig = temp_serviceescalation_orig->next;

    /* add serviceescalation to new list, sorted by host name then service
     * description */
    last_serviceescalation = new_serviceescalation_list;
    for (temp_serviceescalation = new_serviceescalation_list;
         temp_serviceescalation != NULL;
         temp_serviceescalation = temp_serviceescalation->next) {
      if (xodtemplate_compare_strings2(
              temp_serviceescalation_orig->host_name,
              temp_serviceescalation_orig->service_description,
              temp_serviceescalation->host_name,
              temp_serviceescalation->service_description) <= 0)
        break;
      else
        last_serviceescalation = temp_serviceescalation;
    }

    /* first item added to new sorted list */
    if (new_serviceescalation_list == NULL) {
      temp_serviceescalation_orig->next = NULL;
      new_serviceescalation_list = temp_serviceescalation_orig;
    }

    /* item goes at head of new sorted list */
    else if (temp_serviceescalation == new_serviceescalation_list) {
      temp_serviceescalation_orig->next = new_serviceescalation_list;
      new_serviceescalation_list = temp_serviceescalation_orig;
    }

    /* item goes in middle or at end of new sorted list */
    else {
      temp_serviceescalation_orig->next = temp_serviceescalation;
      last_serviceescalation->next = temp_serviceescalation_orig;
    }
  }

  /* list is now sorted */
  xodtemplate_serviceescalation_list = new_serviceescalation_list;

  return OK;
}

/* sort hostescalations by name */
int xodtemplate_sort_hostescalations() {
  xodtemplate_hostescalation* new_hostescalation_list = NULL;
  xodtemplate_hostescalation* temp_hostescalation = NULL;
  xodtemplate_hostescalation* last_hostescalation = NULL;
  xodtemplate_hostescalation* temp_hostescalation_orig = NULL;
  xodtemplate_hostescalation* next_hostescalation_orig = NULL;

  /* sort all existing hostescalations */
  for (temp_hostescalation_orig = xodtemplate_hostescalation_list;
       temp_hostescalation_orig != NULL;
       temp_hostescalation_orig = next_hostescalation_orig) {
    next_hostescalation_orig = temp_hostescalation_orig->next;

    /* add hostescalation to new list, sorted by host name then hostescalation
     * description */
    last_hostescalation = new_hostescalation_list;
    for (temp_hostescalation = new_hostescalation_list;
         temp_hostescalation != NULL;
         temp_hostescalation = temp_hostescalation->next) {
      if (xodtemplate_compare_strings1(temp_hostescalation_orig->host_name,
                                       temp_hostescalation->host_name) <= 0)
        break;
      else
        last_hostescalation = temp_hostescalation;
    }

    /* first item added to new sorted list */
    if (new_hostescalation_list == NULL) {
      temp_hostescalation_orig->next = NULL;
      new_hostescalation_list = temp_hostescalation_orig;
    }

    /* item goes at head of new sorted list */
    else if (temp_hostescalation == new_hostescalation_list) {
      temp_hostescalation_orig->next = new_hostescalation_list;
      new_hostescalation_list = temp_hostescalation_orig;
    }

    /* item goes in middle or at end of new sorted list */
    else {
      temp_hostescalation_orig->next = temp_hostescalation;
      last_hostescalation->next = temp_hostescalation_orig;
    }
  }

  /* list is now sorted */
  xodtemplate_hostescalation_list = new_hostescalation_list;

  return OK;
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

    /* add hostdependency to new list, sorted by host name then hostdependency
     * description */
    last_hostdependency = new_hostdependency_list;
    for (temp_hostdependency = new_hostdependency_list;
         temp_hostdependency != NULL;
         temp_hostdependency = temp_hostdependency->next) {
      if (xodtemplate_compare_strings1(temp_hostdependency_orig->host_name,
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

  return OK;
}

/******************************************************************/
/*********************** MERGE FUNCTIONS **************************/
/******************************************************************/

/* merge extinfo definitions */
int xodtemplate_merge_extinfo_ojects() {
  xodtemplate_hostextinfo* temp_hostextinfo = NULL;
  xodtemplate_serviceextinfo* temp_serviceextinfo = NULL;
  xodtemplate_host* temp_host = NULL;
  xodtemplate_service* temp_service = NULL;

  /* merge service extinfo definitions */
  for (temp_serviceextinfo = xodtemplate_serviceextinfo_list;
       temp_serviceextinfo != NULL;
       temp_serviceextinfo = temp_serviceextinfo->next) {
    /* make sure we have everything */
    if (temp_serviceextinfo->host_name == NULL ||
        temp_serviceextinfo->service_description == NULL)
      continue;

    /* find the service */
    if ((temp_service = xodtemplate_find_real_service(
             temp_serviceextinfo->host_name,
             temp_serviceextinfo->service_description)) == NULL)
      continue;

    /* merge the definitions */
    xodtemplate_merge_service_extinfo_object(temp_service, temp_serviceextinfo);
  }

  /* merge host extinfo definitions */
  for (temp_hostextinfo = xodtemplate_hostextinfo_list;
       temp_hostextinfo != NULL; temp_hostextinfo = temp_hostextinfo->next) {
    /* make sure we have everything */
    if (temp_hostextinfo->host_name == NULL)
      continue;

    /* find the host */
    if ((temp_host = xodtemplate_find_real_host(temp_hostextinfo->host_name)) ==
        NULL)
      continue;

    /* merge the definitions */
    xodtemplate_merge_host_extinfo_object(temp_host, temp_hostextinfo);
  }

  return OK;
}

/* merges a service extinfo definition */
int xodtemplate_merge_service_extinfo_object(
    xodtemplate_service* this_service,
    xodtemplate_serviceextinfo* this_serviceextinfo) {
  if (this_service == NULL || this_serviceextinfo == NULL)
    return ERROR;

  if (this_service->notes == NULL && this_serviceextinfo->notes != NULL)
    this_service->notes = string::dup(this_serviceextinfo->notes);
  if (this_service->notes_url == NULL && this_serviceextinfo->notes_url != NULL)
    this_service->notes_url = string::dup(this_serviceextinfo->notes_url);
  if (this_service->action_url == NULL &&
      this_serviceextinfo->action_url != NULL)
    this_service->action_url = string::dup(this_serviceextinfo->action_url);
  if (this_service->icon_image == NULL &&
      this_serviceextinfo->icon_image != NULL)
    this_service->icon_image = string::dup(this_serviceextinfo->icon_image);
  if (this_service->icon_image_alt == NULL &&
      this_serviceextinfo->icon_image_alt != NULL)
    this_service->icon_image_alt =
        string::dup(this_serviceextinfo->icon_image_alt);

  return OK;
}

/* merges a host extinfo definition */
int xodtemplate_merge_host_extinfo_object(
    xodtemplate_host* this_host,
    xodtemplate_hostextinfo* this_hostextinfo) {
  if (this_host == NULL || this_hostextinfo == NULL)
    return ERROR;

  if (this_host->notes == NULL && this_hostextinfo->notes != NULL)
    this_host->notes = string::dup(this_hostextinfo->notes);
  if (this_host->notes_url == NULL && this_hostextinfo->notes_url != NULL)
    this_host->notes_url = string::dup(this_hostextinfo->notes_url);
  if (this_host->action_url == NULL && this_hostextinfo->action_url != NULL)
    this_host->action_url = string::dup(this_hostextinfo->action_url);
  if (this_host->icon_image == NULL && this_hostextinfo->icon_image != NULL)
    this_host->icon_image = string::dup(this_hostextinfo->icon_image);
  if (this_host->icon_image_alt == NULL &&
      this_hostextinfo->icon_image_alt != NULL)
    this_host->icon_image_alt = string::dup(this_hostextinfo->icon_image_alt);
  if (this_host->vrml_image == NULL && this_hostextinfo->vrml_image != NULL)
    this_host->vrml_image = string::dup(this_hostextinfo->vrml_image);
  if (this_host->statusmap_image == NULL &&
      this_hostextinfo->statusmap_image != NULL)
    this_host->statusmap_image = string::dup(this_hostextinfo->statusmap_image);

  if (this_host->have_2d_coords == false &&
      this_hostextinfo->have_2d_coords == true) {
    this_host->x_2d = this_hostextinfo->x_2d;
    this_host->y_2d = this_hostextinfo->y_2d;
    this_host->have_2d_coords = true;
  }
  if (this_host->have_3d_coords == false &&
      this_hostextinfo->have_3d_coords == true) {
    this_host->x_3d = this_hostextinfo->x_3d;
    this_host->y_3d = this_hostextinfo->y_3d;
    this_host->z_3d = this_hostextinfo->z_3d;
    this_host->have_3d_coords = true;
  }

  return OK;
}

/******************************************************************/
/*********************** CACHE FUNCTIONS **************************/
/******************************************************************/

/* writes cached object definitions for use by web interface */
int xodtemplate_cache_objects(char* cache_file) {
  FILE* fp = NULL;
  int x = 0;
  xodtemplate_timeperiod* temp_timeperiod = NULL;
  xodtemplate_daterange* temp_daterange = NULL;
  xodtemplate_command* temp_command = NULL;
  xodtemplate_connector* temp_connector = NULL;
  xodtemplate_contactgroup* temp_contactgroup = NULL;
  xodtemplate_hostgroup* temp_hostgroup = NULL;
  xodtemplate_servicegroup* temp_servicegroup = NULL;
  xodtemplate_contact* temp_contact = NULL;
  xodtemplate_host* temp_host = NULL;
  xodtemplate_service* temp_service = NULL;
  xodtemplate_servicedependency* temp_servicedependency = NULL;
  xodtemplate_serviceescalation* temp_serviceescalation = NULL;
  xodtemplate_hostdependency* temp_hostdependency = NULL;
  xodtemplate_hostescalation* temp_hostescalation = NULL;
  xodtemplate_customvariablesmember* temp_customvariablesmember = NULL;
  time_t current_time = 0L;
  void* ptr = NULL;
  static char const* days[7] = {"sunday",   "monday", "tuesday", "wednesday",
                                "thursday", "friday", "saturday"};
  static char const* months[12] = {
      "january", "february", "march",     "april",   "may",      "june",
      "july",    "august",   "september", "october", "november", "december"};

  time(&current_time);

  /* open the cache file for writing */
  fp = fopen(cache_file, "w");
  if (fp == NULL) {
    logger(log_config_warning, basic)
        << "Warning: Could not open object cache file '" << cache_file
        << "' for writing!";
    return ERROR;
  }

  /* write header to cache file */
  fprintf(fp, "#############################################\n");
  fprintf(fp, "#     CENTREON ENGINE OBJECT CACHE FILE     #\n");
  fprintf(fp, "#                                           #\n");
  fprintf(fp, "# THIS FILE IS AUTOMATICALLY GENERATED BY   #\n");
  fprintf(fp, "# CENTREON ENGINE. DO NOT MODIFY THIS FILE! #\n");
  fprintf(fp, "#                                           #\n");
  fprintf(fp, "#############################################\n\n");
  fprintf(fp, "# Created: %s", ctime(&current_time));

  /* cache timeperiods */
  /*for(temp_timeperiod=xodtemplate_timeperiod_list;temp_timeperiod!=NULL;temp_timeperiod=temp_timeperiod->next){
   */
  ptr = NULL;
  for (temp_timeperiod = (xodtemplate_timeperiod*)skiplist_get_first(
           xobject_skiplists[X_TIMEPERIOD_SKIPLIST], &ptr);
       temp_timeperiod != NULL;
       temp_timeperiod = (xodtemplate_timeperiod*)skiplist_get_next(&ptr)) {
    if (temp_timeperiod->register_object == false)
      continue;
    fprintf(fp, "define timeperiod {\n");
    if (temp_timeperiod->timeperiod_name)
      fprintf(fp, "\ttimeperiod_name\t%s\n", temp_timeperiod->timeperiod_name);
    if (temp_timeperiod->alias)
      fprintf(fp, "\talias\t%s\n", temp_timeperiod->alias);
    for (x = 0; x < DATERANGE_TYPES; x++) {
      for (temp_daterange = temp_timeperiod->exceptions[x];
           temp_daterange != NULL; temp_daterange = temp_daterange->next) {
        /* skip null entries */
        if (temp_daterange->timeranges == NULL ||
            !strcmp(temp_daterange->timeranges, XODTEMPLATE_NULL))
          continue;

        switch (temp_daterange->type) {
          case DATERANGE_CALENDAR_DATE:
            fprintf(fp, "\t%d-%02d-%02d", temp_daterange->syear,
                    temp_daterange->smon + 1, temp_daterange->smday);
            if ((temp_daterange->smday != temp_daterange->emday) ||
                (temp_daterange->smon != temp_daterange->emon) ||
                (temp_daterange->syear != temp_daterange->eyear))
              fprintf(fp, " - %d-%02d-%02d", temp_daterange->eyear,
                      temp_daterange->emon + 1, temp_daterange->emday);
            if (temp_daterange->skip_interval > 1)
              fprintf(fp, " / %d", temp_daterange->skip_interval);
            break;

          case DATERANGE_MONTH_DATE:
            fprintf(fp, "\t%s %d", months[temp_daterange->smon],
                    temp_daterange->smday);
            if ((temp_daterange->smon != temp_daterange->emon) ||
                (temp_daterange->smday != temp_daterange->emday)) {
              fprintf(fp, " - %s %d", months[temp_daterange->emon],
                      temp_daterange->emday);
              if (temp_daterange->skip_interval > 1)
                fprintf(fp, " / %d", temp_daterange->skip_interval);
            }
            break;

          case DATERANGE_MONTH_DAY:
            fprintf(fp, "\tday %d", temp_daterange->smday);
            if (temp_daterange->smday != temp_daterange->emday) {
              fprintf(fp, " - %d", temp_daterange->emday);
              if (temp_daterange->skip_interval > 1)
                fprintf(fp, " / %d", temp_daterange->skip_interval);
            }
            break;

          case DATERANGE_MONTH_WEEK_DAY:
            fprintf(fp, "\t%s %d %s", days[temp_daterange->swday],
                    temp_daterange->swday_offset, months[temp_daterange->smon]);
            if ((temp_daterange->smon != temp_daterange->emon) ||
                (temp_daterange->swday != temp_daterange->ewday) ||
                (temp_daterange->swday_offset !=
                 temp_daterange->ewday_offset)) {
              fprintf(fp, " - %s %d %s", days[temp_daterange->ewday],
                      temp_daterange->ewday_offset,
                      months[temp_daterange->emon]);
              if (temp_daterange->skip_interval > 1)
                fprintf(fp, " / %d", temp_daterange->skip_interval);
            }
            break;

          case DATERANGE_WEEK_DAY:
            fprintf(fp, "\t%s %d", days[temp_daterange->swday],
                    temp_daterange->swday_offset);
            if ((temp_daterange->swday != temp_daterange->ewday) ||
                (temp_daterange->swday_offset !=
                 temp_daterange->ewday_offset)) {
              fprintf(fp, " - %s %d", days[temp_daterange->ewday],
                      temp_daterange->ewday_offset);
              if (temp_daterange->skip_interval > 1)
                fprintf(fp, " / %d", temp_daterange->skip_interval);
            }
            break;

          default:
            break;
        }

        fprintf(fp, "\t%s\n", temp_daterange->timeranges);
      }
    }
    for (x = 0; x < 7; x++) {
      /* skip null entries */
      if (temp_timeperiod->timeranges[x] == NULL ||
          !strcmp(temp_timeperiod->timeranges[x], XODTEMPLATE_NULL))
        continue;

      fprintf(fp, "\t%s\t%s\n", days[x], temp_timeperiod->timeranges[x]);
    }
    if (temp_timeperiod->exclusions)
      fprintf(fp, "\texclude\t%s\n", temp_timeperiod->exclusions);
    fprintf(fp, "\t}\n\n");
  }

  /* cache commands */
  /*for(temp_command=xodtemplate_command_list;temp_command!=NULL;temp_command=temp_command->next){
   */
  ptr = NULL;
  for (temp_command = (xodtemplate_command*)skiplist_get_first(
           xobject_skiplists[X_COMMAND_SKIPLIST], &ptr);
       temp_command != NULL;
       temp_command = (xodtemplate_command*)skiplist_get_next(&ptr)) {
    if (temp_command->register_object == false)
      continue;
    fprintf(fp, "define command {\n");
    if (temp_command->command_name)
      fprintf(fp, "\tcommand_name\t%s\n", temp_command->command_name);
    if (temp_command->command_line)
      fprintf(fp, "\tcommand_line\t%s\n", temp_command->command_line);
    fprintf(fp, "\t}\n\n");
  }

  /* cache connectors */
  /*for(temp_connector=xodtemplate_connector_list;temp_connector!=NULL;temp_connector=temp_connector->next){
   */
  ptr = NULL;
  for (temp_connector = (xodtemplate_connector*)skiplist_get_first(
           xobject_skiplists[X_CONNECTOR_SKIPLIST], &ptr);
       temp_connector != NULL;
       temp_connector = (xodtemplate_connector*)skiplist_get_next(&ptr)) {
    if (temp_connector->register_object == false)
      continue;
    fprintf(fp, "define connector {\n");
    if (temp_connector->connector_name)
      fprintf(fp, "\tcommand_name\t%s\n", temp_connector->connector_name);
    if (temp_connector->connector_line)
      fprintf(fp, "\tconnector_line\t%s\n", temp_connector->connector_line);
    fprintf(fp, "\t}\n\n");
  }

  /* cache contactgroups */
  /*for(temp_contactgroup=xodtemplate_contactgroup_list;temp_contactgroup!=NULL;temp_contactgroup=temp_contactgroup->next){
   */
  ptr = NULL;
  for (temp_contactgroup = (xodtemplate_contactgroup*)skiplist_get_first(
           xobject_skiplists[X_CONTACTGROUP_SKIPLIST], &ptr);
       temp_contactgroup != NULL;
       temp_contactgroup = (xodtemplate_contactgroup*)skiplist_get_next(&ptr)) {
    if (temp_contactgroup->register_object == false)
      continue;
    fprintf(fp, "define contactgroup {\n");
    if (temp_contactgroup->contactgroup_name)
      fprintf(fp, "\tcontactgroup_name\t%s\n",
              temp_contactgroup->contactgroup_name);
    if (temp_contactgroup->alias)
      fprintf(fp, "\talias\t%s\n", temp_contactgroup->alias);
    if (temp_contactgroup->members)
      fprintf(fp, "\tmembers\t%s\n", temp_contactgroup->members);
    fprintf(fp, "\t}\n\n");
  }

  /* cache hostgroups */
  /*for(temp_hostgroup=xodtemplate_hostgroup_list;temp_hostgroup!=NULL;temp_hostgroup=temp_hostgroup->next){
   */
  ptr = NULL;
  for (temp_hostgroup = (xodtemplate_hostgroup*)skiplist_get_first(
           xobject_skiplists[X_HOSTGROUP_SKIPLIST], &ptr);
       temp_hostgroup != NULL;
       temp_hostgroup = (xodtemplate_hostgroup*)skiplist_get_next(&ptr)) {
    if (temp_hostgroup->register_object == false)
      continue;
    fprintf(fp, "define hostgroup {\n");
    if (temp_hostgroup->hostgroup_name)
      fprintf(fp, "\thostgroup_name\t%s\n", temp_hostgroup->hostgroup_name);
    if (temp_hostgroup->alias)
      fprintf(fp, "\talias\t%s\n", temp_hostgroup->alias);
    if (temp_hostgroup->members)
      fprintf(fp, "\tmembers\t%s\n", temp_hostgroup->members);
    if (temp_hostgroup->notes)
      fprintf(fp, "\tnotes\t%s\n", temp_hostgroup->notes);
    if (temp_hostgroup->notes_url)
      fprintf(fp, "\tnotes_url\t%s\n", temp_hostgroup->notes_url);
    if (temp_hostgroup->action_url)
      fprintf(fp, "\taction_url\t%s\n", temp_hostgroup->action_url);
    fprintf(fp, "\t}\n\n");
  }

  /* cache servicegroups */
  /*for(temp_servicegroup=xodtemplate_servicegroup_list;temp_servicegroup!=NULL;temp_servicegroup=temp_servicegroup->next){
   */
  ptr = NULL;
  for (temp_servicegroup = (xodtemplate_servicegroup*)skiplist_get_first(
           xobject_skiplists[X_SERVICEGROUP_SKIPLIST], &ptr);
       temp_servicegroup != NULL;
       temp_servicegroup = (xodtemplate_servicegroup*)skiplist_get_next(&ptr)) {
    if (temp_servicegroup->register_object == false)
      continue;
    fprintf(fp, "define servicegroup {\n");
    if (temp_servicegroup->servicegroup_name)
      fprintf(fp, "\tservicegroup_name\t%s\n",
              temp_servicegroup->servicegroup_name);
    if (temp_servicegroup->alias)
      fprintf(fp, "\talias\t%s\n", temp_servicegroup->alias);
    if (temp_servicegroup->members)
      fprintf(fp, "\tmembers\t%s\n", temp_servicegroup->members);
    if (temp_servicegroup->notes)
      fprintf(fp, "\tnotes\t%s\n", temp_servicegroup->notes);
    if (temp_servicegroup->notes_url)
      fprintf(fp, "\tnotes_url\t%s\n", temp_servicegroup->notes_url);
    if (temp_servicegroup->action_url)
      fprintf(fp, "\taction_url\t%s\n", temp_servicegroup->action_url);
    fprintf(fp, "\t}\n\n");
  }

  /* cache contacts */
  /*for(temp_contact=xodtemplate_contact_list;temp_contact!=NULL;temp_contact=temp_contact->next){
   */
  ptr = NULL;
  for (temp_contact = (xodtemplate_contact*)skiplist_get_first(
           xobject_skiplists[X_CONTACT_SKIPLIST], &ptr);
       temp_contact != NULL;
       temp_contact = (xodtemplate_contact*)skiplist_get_next(&ptr)) {
    if (temp_contact->register_object == false)
      continue;
    fprintf(fp, "define contact {\n");
    if (temp_contact->contact_name)
      fprintf(fp, "\tcontact_name\t%s\n", temp_contact->contact_name);
    if (temp_contact->alias)
      fprintf(fp, "\talias\t%s\n", temp_contact->alias);
    if (temp_contact->service_notification_period)
      fprintf(fp, "\tservice_notification_period\t%s\n",
              temp_contact->service_notification_period);
    if (temp_contact->host_notification_period)
      fprintf(fp, "\thost_notification_period\t%s\n",
              temp_contact->host_notification_period);
    fprintf(fp, "\tservice_notification_options\t");
    x = 0;
    if (temp_contact->notify_on_service_warning == true)
      fprintf(fp, "%sw", (x++ > 0) ? "," : "");
    if (temp_contact->notify_on_service_unknown == true)
      fprintf(fp, "%su", (x++ > 0) ? "," : "");
    if (temp_contact->notify_on_service_critical == true)
      fprintf(fp, "%sc", (x++ > 0) ? "," : "");
    if (temp_contact->notify_on_service_recovery == true)
      fprintf(fp, "%sr", (x++ > 0) ? "," : "");
    if (temp_contact->notify_on_service_flapping == true)
      fprintf(fp, "%sf", (x++ > 0) ? "," : "");
    if (temp_contact->notify_on_service_downtime == true)
      fprintf(fp, "%ss", (x++ > 0) ? "," : "");
    if (x == 0)
      fprintf(fp, "n");
    fprintf(fp, "\n");
    fprintf(fp, "\thost_notification_options\t");
    x = 0;
    if (temp_contact->notify_on_host_down == true)
      fprintf(fp, "%sd", (x++ > 0) ? "," : "");
    if (temp_contact->notify_on_host_unreachable == true)
      fprintf(fp, "%su", (x++ > 0) ? "," : "");
    if (temp_contact->notify_on_host_recovery == true)
      fprintf(fp, "%sr", (x++ > 0) ? "," : "");
    if (temp_contact->notify_on_host_flapping == true)
      fprintf(fp, "%sf", (x++ > 0) ? "," : "");
    if (temp_contact->notify_on_host_downtime == true)
      fprintf(fp, "%ss", (x++ > 0) ? "," : "");
    if (x == 0)
      fprintf(fp, "n");
    fprintf(fp, "\n");
    if (temp_contact->service_notification_commands)
      fprintf(fp, "\tservice_notification_commands\t%s\n",
              temp_contact->service_notification_commands);
    if (temp_contact->host_notification_commands)
      fprintf(fp, "\thost_notification_commands\t%s\n",
              temp_contact->host_notification_commands);
    if (temp_contact->email)
      fprintf(fp, "\temail\t%s\n", temp_contact->email);
    if (temp_contact->pager)
      fprintf(fp, "\tpager\t%s\n", temp_contact->pager);
    for (x = 0; x < MAX_XODTEMPLATE_CONTACT_ADDRESSES; x++) {
      if (temp_contact->address[x])
        fprintf(fp, "\taddress%d\t%s\n", x + 1, temp_contact->address[x]);
    }
    fprintf(fp, "\thost_notifications_enabled\t%d\n",
            temp_contact->host_notifications_enabled);
    fprintf(fp, "\tservice_notifications_enabled\t%d\n",
            temp_contact->service_notifications_enabled);
    fprintf(fp, "\tcan_submit_commands\t%d\n",
            temp_contact->can_submit_commands);
    fprintf(fp, "\tretain_status_information\t%d\n",
            temp_contact->retain_status_information);
    fprintf(fp, "\tretain_nonstatus_information\t%d\n",
            temp_contact->retain_nonstatus_information);

    /* custom variables */
    for (temp_customvariablesmember = temp_contact->custom_variables;
         temp_customvariablesmember != NULL;
         temp_customvariablesmember = temp_customvariablesmember->next) {
      if (temp_customvariablesmember->variable_name)
        fprintf(fp, "\t_%s\t%s\n", temp_customvariablesmember->variable_name,
                (temp_customvariablesmember->variable_value == NULL
                     ? XODTEMPLATE_NULL
                     : temp_customvariablesmember->variable_value));
    }

    fprintf(fp, "\t}\n\n");
  }

  /* cache hosts */
  /*for(temp_host=xodtemplate_host_list;temp_host!=NULL;temp_host=temp_host->next){
   */
  ptr = NULL;
  for (temp_host = (xodtemplate_host*)skiplist_get_first(
           xobject_skiplists[X_HOST_SKIPLIST], &ptr);
       temp_host != NULL;
       temp_host = (xodtemplate_host*)skiplist_get_next(&ptr)) {
    if (temp_host->register_object == false)
      continue;
    fprintf(fp, "define host {\n");
    if (temp_host->host_name)
      fprintf(fp, "\thost_name\t%s\n", temp_host->host_name);
    if (temp_host->display_name)
      fprintf(fp, "\tdisplay_name\t%s\n", temp_host->display_name);
    if (temp_host->alias)
      fprintf(fp, "\talias\t%s\n", temp_host->alias);
    if (temp_host->address)
      fprintf(fp, "\taddress\t%s\n", temp_host->address);
    if (temp_host->parents)
      fprintf(fp, "\tparents\t%s\n", temp_host->parents);
    if (temp_host->check_period)
      fprintf(fp, "\tcheck_period\t%s\n", temp_host->check_period);
    if (temp_host->check_command)
      fprintf(fp, "\tcheck_command\t%s\n", temp_host->check_command);
    if (temp_host->event_handler)
      fprintf(fp, "\tevent_handler\t%s\n", temp_host->event_handler);
    if (temp_host->contacts)
      fprintf(fp, "\tcontacts\t%s\n", temp_host->contacts);
    if (temp_host->contact_groups)
      fprintf(fp, "\tcontact_groups\t%s\n", temp_host->contact_groups);
    if (temp_host->notification_period)
      fprintf(fp, "\tnotification_period\t%s\n",
              temp_host->notification_period);
    fprintf(fp, "\tinitial_state\t");
    if (temp_host->initial_state == HOST_DOWN)
      fprintf(fp, "d\n");
    else if (temp_host->initial_state == HOST_UNREACHABLE)
      fprintf(fp, "u\n");
    else
      fprintf(fp, "o\n");
    fprintf(fp, "\tcheck_interval\t%f\n", temp_host->check_interval);
    fprintf(fp, "\tretry_interval\t%f\n", temp_host->retry_interval);
    fprintf(fp, "\tmax_check_attempts\t%d\n", temp_host->max_check_attempts);
    fprintf(fp, "\tactive_checks_enabled\t%d\n",
            temp_host->active_checks_enabled);
    fprintf(fp, "\tpassive_checks_enabled\t%d\n",
            temp_host->passive_checks_enabled);
    fprintf(fp, "\tobsess_over_host\t%d\n", temp_host->obsess_over_host);
    fprintf(fp, "\tevent_handler_enabled\t%d\n",
            temp_host->event_handler_enabled);
    fprintf(fp, "\tlow_flap_threshold\t%f\n", temp_host->low_flap_threshold);
    fprintf(fp, "\thigh_flap_threshold\t%f\n", temp_host->high_flap_threshold);
    fprintf(fp, "\tflap_detection_enabled\t%d\n",
            temp_host->flap_detection_enabled);
    fprintf(fp, "\tflap_detection_options\t");
    x = 0;
    if (temp_host->flap_detection_on_up == true)
      fprintf(fp, "%so", (x++ > 0) ? "," : "");
    if (temp_host->flap_detection_on_down == true)
      fprintf(fp, "%sd", (x++ > 0) ? "," : "");
    if (temp_host->flap_detection_on_unreachable == true)
      fprintf(fp, "%su", (x++ > 0) ? "," : "");
    if (x == 0)
      fprintf(fp, "n");
    fprintf(fp, "\n");
    fprintf(fp, "\tfreshness_threshold\t%d\n", temp_host->freshness_threshold);
    fprintf(fp, "\tcheck_freshness\t%d\n", temp_host->check_freshness);
    fprintf(fp, "\tnotification_options\t");
    x = 0;
    if (temp_host->notify_on_down == true)
      fprintf(fp, "%sd", (x++ > 0) ? "," : "");
    if (temp_host->notify_on_unreachable == true)
      fprintf(fp, "%su", (x++ > 0) ? "," : "");
    if (temp_host->notify_on_recovery == true)
      fprintf(fp, "%sr", (x++ > 0) ? "," : "");
    if (temp_host->notify_on_flapping == true)
      fprintf(fp, "%sf", (x++ > 0) ? "," : "");
    if (temp_host->notify_on_downtime == true)
      fprintf(fp, "%ss", (x++ > 0) ? "," : "");
    if (x == 0)
      fprintf(fp, "n");
    fprintf(fp, "\n");
    fprintf(fp, "\tnotifications_enabled\t%d\n",
            temp_host->notifications_enabled);
    fprintf(fp, "\tnotification_interval\t%f\n",
            temp_host->notification_interval);
    fprintf(fp, "\tfirst_notification_delay\t%f\n",
            temp_host->first_notification_delay);
    fprintf(fp, "\tstalking_options\t");
    x = 0;
    if (temp_host->stalk_on_up == true)
      fprintf(fp, "%so", (x++ > 0) ? "," : "");
    if (temp_host->stalk_on_down == true)
      fprintf(fp, "%sd", (x++ > 0) ? "," : "");
    if (temp_host->stalk_on_unreachable == true)
      fprintf(fp, "%su", (x++ > 0) ? "," : "");
    if (x == 0)
      fprintf(fp, "n");
    fprintf(fp, "\n");
    fprintf(fp, "\tprocess_perf_data\t%d\n", temp_host->process_perf_data);
    if (temp_host->icon_image)
      fprintf(fp, "\ticon_image\t%s\n", temp_host->icon_image);
    if (temp_host->icon_image_alt)
      fprintf(fp, "\ticon_image_alt\t%s\n", temp_host->icon_image_alt);
    if (temp_host->vrml_image)
      fprintf(fp, "\tvrml_image\t%s\n", temp_host->vrml_image);
    if (temp_host->statusmap_image)
      fprintf(fp, "\tstatusmap_image\t%s\n", temp_host->statusmap_image);
    if (temp_host->have_2d_coords == true)
      fprintf(fp, "\t2d_coords\t%d,%d\n", temp_host->x_2d, temp_host->y_2d);
    if (temp_host->have_3d_coords == true)
      fprintf(fp, "\t3d_coords\t%f,%f,%f\n", temp_host->x_3d, temp_host->y_3d,
              temp_host->z_3d);
    if (temp_host->notes)
      fprintf(fp, "\tnotes\t%s\n", temp_host->notes);
    if (temp_host->notes_url)
      fprintf(fp, "\tnotes_url\t%s\n", temp_host->notes_url);
    if (temp_host->action_url)
      fprintf(fp, "\taction_url\t%s\n", temp_host->action_url);
    fprintf(fp, "\tretain_status_information\t%d\n",
            temp_host->retain_status_information);
    fprintf(fp, "\tretain_nonstatus_information\t%d\n",
            temp_host->retain_nonstatus_information);

    /* custom variables */
    for (temp_customvariablesmember = temp_host->custom_variables;
         temp_customvariablesmember != NULL;
         temp_customvariablesmember = temp_customvariablesmember->next) {
      if (temp_customvariablesmember->variable_name)
        fprintf(fp, "\t_%s\t%s\n", temp_customvariablesmember->variable_name,
                (temp_customvariablesmember->variable_value == NULL
                     ? XODTEMPLATE_NULL
                     : temp_customvariablesmember->variable_value));
    }

    fprintf(fp, "\t}\n\n");
  }

  /* cache services */
  /*for(temp_service=xodtemplate_service_list;temp_service!=NULL;temp_service=temp_service->next){
   */
  ptr = NULL;
  for (temp_service = (xodtemplate_service*)skiplist_get_first(
           xobject_skiplists[X_SERVICE_SKIPLIST], &ptr);
       temp_service != NULL;
       temp_service = (xodtemplate_service*)skiplist_get_next(&ptr)) {
    if (temp_service->register_object == false)
      continue;
    fprintf(fp, "define service {\n");
    if (temp_service->host_name)
      fprintf(fp, "\thost_name\t%s\n", temp_service->host_name);
    if (temp_service->service_description)
      fprintf(fp, "\tservice_description\t%s\n",
              temp_service->service_description);
    if (temp_service->display_name)
      fprintf(fp, "\tdisplay_name\t%s\n", temp_service->display_name);
    if (temp_service->check_period)
      fprintf(fp, "\tcheck_period\t%s\n", temp_service->check_period);
    if (temp_service->check_command)
      fprintf(fp, "\tcheck_command\t%s\n", temp_service->check_command);
    if (temp_service->event_handler)
      fprintf(fp, "\tevent_handler\t%s\n", temp_service->event_handler);
    if (temp_service->contacts)
      fprintf(fp, "\tcontacts\t%s\n", temp_service->contacts);
    if (temp_service->contact_groups)
      fprintf(fp, "\tcontact_groups\t%s\n", temp_service->contact_groups);
    if (temp_service->notification_period)
      fprintf(fp, "\tnotification_period\t%s\n",
              temp_service->notification_period);
    fprintf(fp, "\tinitial_state\t");
    if (temp_service->initial_state == STATE_WARNING)
      fprintf(fp, "w\n");
    else if (temp_service->initial_state == STATE_UNKNOWN)
      fprintf(fp, "u\n");
    else if (temp_service->initial_state == STATE_CRITICAL)
      fprintf(fp, "c\n");
    else
      fprintf(fp, "o\n");
    fprintf(fp, "\tcheck_interval\t%f\n", temp_service->check_interval);
    fprintf(fp, "\tretry_interval\t%f\n", temp_service->retry_interval);
    fprintf(fp, "\tmax_check_attempts\t%d\n", temp_service->max_check_attempts);
    fprintf(fp, "\tis_volatile\t%d\n", temp_service->is_volatile);
    fprintf(fp, "\tparallelize_check\t%d\n", temp_service->parallelize_check);
    fprintf(fp, "\tactive_checks_enabled\t%d\n",
            temp_service->active_checks_enabled);
    fprintf(fp, "\tpassive_checks_enabled\t%d\n",
            temp_service->passive_checks_enabled);
    fprintf(fp, "\tobsess_over_service\t%d\n",
            temp_service->obsess_over_service);
    fprintf(fp, "\tevent_handler_enabled\t%d\n",
            temp_service->event_handler_enabled);
    fprintf(fp, "\tlow_flap_threshold\t%f\n", temp_service->low_flap_threshold);
    fprintf(fp, "\thigh_flap_threshold\t%f\n",
            temp_service->high_flap_threshold);
    fprintf(fp, "\tflap_detection_enabled\t%d\n",
            temp_service->flap_detection_enabled);
    fprintf(fp, "\tflap_detection_options\t");
    x = 0;
    if (temp_service->flap_detection_on_ok == true)
      fprintf(fp, "%so", (x++ > 0) ? "," : "");
    if (temp_service->flap_detection_on_warning == true)
      fprintf(fp, "%sw", (x++ > 0) ? "," : "");
    if (temp_service->flap_detection_on_unknown == true)
      fprintf(fp, "%su", (x++ > 0) ? "," : "");
    if (temp_service->flap_detection_on_critical == true)
      fprintf(fp, "%sc", (x++ > 0) ? "," : "");
    if (x == 0)
      fprintf(fp, "n");
    fprintf(fp, "\n");
    fprintf(fp, "\tfreshness_threshold\t%d\n",
            temp_service->freshness_threshold);
    fprintf(fp, "\tcheck_freshness\t%d\n", temp_service->check_freshness);
    fprintf(fp, "\tnotification_options\t");
    x = 0;
    if (temp_service->notify_on_unknown == true)
      fprintf(fp, "%su", (x++ > 0) ? "," : "");
    if (temp_service->notify_on_warning == true)
      fprintf(fp, "%sw", (x++ > 0) ? "," : "");
    if (temp_service->notify_on_critical == true)
      fprintf(fp, "%sc", (x++ > 0) ? "," : "");
    if (temp_service->notify_on_recovery == true)
      fprintf(fp, "%sr", (x++ > 0) ? "," : "");
    if (temp_service->notify_on_flapping == true)
      fprintf(fp, "%sf", (x++ > 0) ? "," : "");
    if (temp_service->notify_on_downtime == true)
      fprintf(fp, "%ss", (x++ > 0) ? "," : "");
    if (x == 0)
      fprintf(fp, "n");
    fprintf(fp, "\n");
    fprintf(fp, "\tnotifications_enabled\t%d\n",
            temp_service->notifications_enabled);
    fprintf(fp, "\tnotification_interval\t%f\n",
            temp_service->notification_interval);
    fprintf(fp, "\tfirst_notification_delay\t%f\n",
            temp_service->first_notification_delay);
    fprintf(fp, "\tstalking_options\t");
    x = 0;
    if (temp_service->stalk_on_ok == true)
      fprintf(fp, "%so", (x++ > 0) ? "," : "");
    if (temp_service->stalk_on_unknown == true)
      fprintf(fp, "%su", (x++ > 0) ? "," : "");
    if (temp_service->stalk_on_warning == true)
      fprintf(fp, "%sw", (x++ > 0) ? "," : "");
    if (temp_service->stalk_on_critical == true)
      fprintf(fp, "%sc", (x++ > 0) ? "," : "");
    if (x == 0)
      fprintf(fp, "n");
    fprintf(fp, "\n");
    fprintf(fp, "\tprocess_perf_data\t%d\n", temp_service->process_perf_data);
    if (temp_service->icon_image)
      fprintf(fp, "\ticon_image\t%s\n", temp_service->icon_image);
    if (temp_service->icon_image_alt)
      fprintf(fp, "\ticon_image_alt\t%s\n", temp_service->icon_image_alt);
    if (temp_service->notes)
      fprintf(fp, "\tnotes\t%s\n", temp_service->notes);
    if (temp_service->notes_url)
      fprintf(fp, "\tnotes_url\t%s\n", temp_service->notes_url);
    if (temp_service->action_url)
      fprintf(fp, "\taction_url\t%s\n", temp_service->action_url);
    fprintf(fp, "\tretain_status_information\t%d\n",
            temp_service->retain_status_information);
    fprintf(fp, "\tretain_nonstatus_information\t%d\n",
            temp_service->retain_nonstatus_information);

    /* custom variables */
    for (temp_customvariablesmember = temp_service->custom_variables;
         temp_customvariablesmember != NULL;
         temp_customvariablesmember = temp_customvariablesmember->next) {
      if (temp_customvariablesmember->variable_name)
        fprintf(fp, "\t_%s\t%s\n", temp_customvariablesmember->variable_name,
                (temp_customvariablesmember->variable_value == NULL
                     ? XODTEMPLATE_NULL
                     : temp_customvariablesmember->variable_value));
    }

    fprintf(fp, "\t}\n\n");
  }

  /* cache service dependencies */
  /*for(temp_servicedependency=xodtemplate_servicedependency_list;temp_servicedependency!=NULL;temp_servicedependency=temp_servicedependency->next){
   */
  ptr = NULL;
  for (temp_servicedependency =
           (xodtemplate_servicedependency*)skiplist_get_first(
               xobject_skiplists[X_SERVICEDEPENDENCY_SKIPLIST], &ptr);
       temp_servicedependency != NULL;
       temp_servicedependency =
           (xodtemplate_servicedependency*)skiplist_get_next(&ptr)) {
    if (temp_servicedependency->register_object == false)
      continue;
    fprintf(fp, "define servicedependency {\n");
    if (temp_servicedependency->host_name)
      fprintf(fp, "\thost_name\t%s\n", temp_servicedependency->host_name);
    if (temp_servicedependency->service_description)
      fprintf(fp, "\tservice_description\t%s\n",
              temp_servicedependency->service_description);
    if (temp_servicedependency->dependent_host_name)
      fprintf(fp, "\tdependent_host_name\t%s\n",
              temp_servicedependency->dependent_host_name);
    if (temp_servicedependency->dependent_service_description)
      fprintf(fp, "\tdependent_service_description\t%s\n",
              temp_servicedependency->dependent_service_description);
    if (temp_servicedependency->dependency_period)
      fprintf(fp, "\tdependency_period\t%s\n",
              temp_servicedependency->dependency_period);
    fprintf(fp, "\tinherits_parent\t%d\n",
            temp_servicedependency->inherits_parent);
    if (temp_servicedependency->have_notification_dependency_options == true) {
      fprintf(fp, "\tnotification_failure_options\t");
      x = 0;
      if (temp_servicedependency->fail_notify_on_ok == true)
        fprintf(fp, "%so", (x++ > 0) ? "," : "");
      if (temp_servicedependency->fail_notify_on_unknown == true)
        fprintf(fp, "%su", (x++ > 0) ? "," : "");
      if (temp_servicedependency->fail_notify_on_warning == true)
        fprintf(fp, "%sw", (x++ > 0) ? "," : "");
      if (temp_servicedependency->fail_notify_on_critical == true)
        fprintf(fp, "%sc", (x++ > 0) ? "," : "");
      if (temp_servicedependency->fail_notify_on_pending == true)
        fprintf(fp, "%sp", (x++ > 0) ? "," : "");
      if (x == 0)
        fprintf(fp, "n");
      fprintf(fp, "\n");
    }
    if (temp_servicedependency->have_execution_dependency_options == true) {
      fprintf(fp, "\texecution_failure_options\t");
      x = 0;
      if (temp_servicedependency->fail_execute_on_ok == true)
        fprintf(fp, "%so", (x++ > 0) ? "," : "");
      if (temp_servicedependency->fail_execute_on_unknown == true)
        fprintf(fp, "%su", (x++ > 0) ? "," : "");
      if (temp_servicedependency->fail_execute_on_warning == true)
        fprintf(fp, "%sw", (x++ > 0) ? "," : "");
      if (temp_servicedependency->fail_execute_on_critical == true)
        fprintf(fp, "%sc", (x++ > 0) ? "," : "");
      if (temp_servicedependency->fail_execute_on_pending == true)
        fprintf(fp, "%sp", (x++ > 0) ? "," : "");
      if (x == 0)
        fprintf(fp, "n");
      fprintf(fp, "\n");
    }
    fprintf(fp, "\t}\n\n");
  }

  /* cache service escalations */
  /*for(temp_serviceescalation=xodtemplate_serviceescalation_list;temp_serviceescalation!=NULL;temp_serviceescalation=temp_serviceescalation->next){
   */
  ptr = NULL;
  for (temp_serviceescalation =
           (xodtemplate_serviceescalation*)skiplist_get_first(
               xobject_skiplists[X_SERVICEESCALATION_SKIPLIST], &ptr);
       temp_serviceescalation != NULL;
       temp_serviceescalation =
           (xodtemplate_serviceescalation*)skiplist_get_next(&ptr)) {
    if (temp_serviceescalation->register_object == false)
      continue;
    fprintf(fp, "define serviceescalation {\n");
    if (temp_serviceescalation->host_name)
      fprintf(fp, "\thost_name\t%s\n", temp_serviceescalation->host_name);
    if (temp_serviceescalation->service_description)
      fprintf(fp, "\tservice_description\t%s\n",
              temp_serviceescalation->service_description);
    fprintf(fp, "\tfirst_notification\t%d\n",
            temp_serviceescalation->first_notification);
    fprintf(fp, "\tlast_notification\t%d\n",
            temp_serviceescalation->last_notification);
    fprintf(fp, "\tnotification_interval\t%f\n",
            temp_serviceescalation->notification_interval);
    if (temp_serviceescalation->escalation_period)
      fprintf(fp, "\tescalation_period\t%s\n",
              temp_serviceescalation->escalation_period);
    if (temp_serviceescalation->have_escalation_options == true) {
      fprintf(fp, "\tescalation_options\t");
      x = 0;
      if (temp_serviceescalation->escalate_on_warning == true)
        fprintf(fp, "%sw", (x++ > 0) ? "," : "");
      if (temp_serviceescalation->escalate_on_unknown == true)
        fprintf(fp, "%su", (x++ > 0) ? "," : "");
      if (temp_serviceescalation->escalate_on_critical == true)
        fprintf(fp, "%sc", (x++ > 0) ? "," : "");
      if (temp_serviceescalation->escalate_on_recovery == true)
        fprintf(fp, "%sr", (x++ > 0) ? "," : "");
      if (x == 0)
        fprintf(fp, "n");
      fprintf(fp, "\n");
    }
    if (temp_serviceescalation->contacts)
      fprintf(fp, "\tcontacts\t%s\n", temp_serviceescalation->contacts);
    if (temp_serviceescalation->contact_groups)
      fprintf(fp, "\tcontact_groups\t%s\n",
              temp_serviceescalation->contact_groups);
    fprintf(fp, "\t}\n\n");
  }

  /* cache host dependencies */
  /*for(temp_hostdependency=xodtemplate_hostdependency_list;temp_hostdependency!=NULL;temp_hostdependency=temp_hostdependency->next){
   */
  ptr = NULL;
  for (temp_hostdependency = (xodtemplate_hostdependency*)skiplist_get_first(
           xobject_skiplists[X_HOSTDEPENDENCY_SKIPLIST], &ptr);
       temp_hostdependency != NULL;
       temp_hostdependency =
           (xodtemplate_hostdependency*)skiplist_get_next(&ptr)) {
    if (temp_hostdependency->register_object == false)
      continue;
    fprintf(fp, "define hostdependency {\n");
    if (temp_hostdependency->host_name)
      fprintf(fp, "\thost_name\t%s\n", temp_hostdependency->host_name);
    if (temp_hostdependency->dependent_host_name)
      fprintf(fp, "\tdependent_host_name\t%s\n",
              temp_hostdependency->dependent_host_name);
    if (temp_hostdependency->dependency_period)
      fprintf(fp, "\tdependency_period\t%s\n",
              temp_hostdependency->dependency_period);
    fprintf(fp, "\tinherits_parent\t%d\n",
            temp_hostdependency->inherits_parent);
    if (temp_hostdependency->have_notification_dependency_options == true) {
      fprintf(fp, "\tnotification_failure_options\t");
      x = 0;
      if (temp_hostdependency->fail_notify_on_up == true)
        fprintf(fp, "%so", (x++ > 0) ? "," : "");
      if (temp_hostdependency->fail_notify_on_down == true)
        fprintf(fp, "%sd", (x++ > 0) ? "," : "");
      if (temp_hostdependency->fail_notify_on_unreachable == true)
        fprintf(fp, "%su", (x++ > 0) ? "," : "");
      if (temp_hostdependency->fail_notify_on_pending == true)
        fprintf(fp, "%sp", (x++ > 0) ? "," : "");
      if (x == 0)
        fprintf(fp, "n");
      fprintf(fp, "\n");
    }
    if (temp_hostdependency->have_execution_dependency_options == true) {
      fprintf(fp, "\texecution_failure_options\t");
      x = 0;
      if (temp_hostdependency->fail_execute_on_up == true)
        fprintf(fp, "%so", (x++ > 0) ? "," : "");
      if (temp_hostdependency->fail_execute_on_down == true)
        fprintf(fp, "%sd", (x++ > 0) ? "," : "");
      if (temp_hostdependency->fail_execute_on_unreachable == true)
        fprintf(fp, "%su", (x++ > 0) ? "," : "");
      if (temp_hostdependency->fail_execute_on_pending == true)
        fprintf(fp, "%sp", (x++ > 0) ? "," : "");
      if (x == 0)
        fprintf(fp, "n");
      fprintf(fp, "\n");
    }
    fprintf(fp, "\t}\n\n");
  }

  /* cache host escalations */
  /*for(temp_hostescalation=xodtemplate_hostescalation_list;temp_hostescalation!=NULL;temp_hostescalation=temp_hostescalation->next){
   */
  ptr = NULL;
  for (temp_hostescalation = (xodtemplate_hostescalation*)skiplist_get_first(
           xobject_skiplists[X_HOSTESCALATION_SKIPLIST], &ptr);
       temp_hostescalation != NULL;
       temp_hostescalation =
           (xodtemplate_hostescalation*)skiplist_get_next(&ptr)) {
    if (temp_hostescalation->register_object == false)
      continue;
    fprintf(fp, "define hostescalation {\n");
    if (temp_hostescalation->host_name)
      fprintf(fp, "\thost_name\t%s\n", temp_hostescalation->host_name);
    fprintf(fp, "\tfirst_notification\t%d\n",
            temp_hostescalation->first_notification);
    fprintf(fp, "\tlast_notification\t%d\n",
            temp_hostescalation->last_notification);
    fprintf(fp, "\tnotification_interval\t%f\n",
            temp_hostescalation->notification_interval);
    if (temp_hostescalation->escalation_period)
      fprintf(fp, "\tescalation_period\t%s\n",
              temp_hostescalation->escalation_period);
    if (temp_hostescalation->have_escalation_options == true) {
      fprintf(fp, "\tescalation_options\t");
      x = 0;
      if (temp_hostescalation->escalate_on_down == true)
        fprintf(fp, "%sd", (x++ > 0) ? "," : "");
      if (temp_hostescalation->escalate_on_unreachable == true)
        fprintf(fp, "%su", (x++ > 0) ? "," : "");
      if (temp_hostescalation->escalate_on_recovery == true)
        fprintf(fp, "%sr", (x++ > 0) ? "," : "");
      if (x == 0)
        fprintf(fp, "n");
      fprintf(fp, "\n");
    }
    if (temp_hostescalation->contacts)
      fprintf(fp, "\tcontacts\t%s\n", temp_hostescalation->contacts);
    if (temp_hostescalation->contact_groups)
      fprintf(fp, "\tcontact_groups\t%s\n",
              temp_hostescalation->contact_groups);
    fprintf(fp, "\t}\n\n");
  }

  fclose(fp);

  return OK;
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

  xobject_template_skiplists[X_HOST_SKIPLIST] = skiplist_new(
      16, 0.5, false, false, xodtemplate_skiplist_compare_host_template);
  xobject_template_skiplists[X_SERVICE_SKIPLIST] = skiplist_new(
      16, 0.5, false, false, xodtemplate_skiplist_compare_service_template);
  xobject_template_skiplists[X_COMMAND_SKIPLIST] = skiplist_new(
      10, 0.5, false, false, xodtemplate_skiplist_compare_command_template);
  xobject_template_skiplists[X_TIMEPERIOD_SKIPLIST] = skiplist_new(
      10, 0.5, false, false, xodtemplate_skiplist_compare_timeperiod_template);
  xobject_template_skiplists[X_CONTACT_SKIPLIST] = skiplist_new(
      10, 0.5, false, false, xodtemplate_skiplist_compare_contact_template);
  xobject_template_skiplists[X_CONTACTGROUP_SKIPLIST] =
      skiplist_new(10, 0.5, false, false,
                   xodtemplate_skiplist_compare_contactgroup_template);
  xobject_template_skiplists[X_HOSTGROUP_SKIPLIST] = skiplist_new(
      10, 0.5, false, false, xodtemplate_skiplist_compare_hostgroup_template);
  xobject_template_skiplists[X_SERVICEGROUP_SKIPLIST] =
      skiplist_new(10, 0.5, false, false,
                   xodtemplate_skiplist_compare_servicegroup_template);
  xobject_template_skiplists[X_HOSTDEPENDENCY_SKIPLIST] =
      skiplist_new(16, 0.5, false, false,
                   xodtemplate_skiplist_compare_hostdependency_template);
  xobject_template_skiplists[X_SERVICEDEPENDENCY_SKIPLIST] =
      skiplist_new(16, 0.5, false, false,
                   xodtemplate_skiplist_compare_servicedependency_template);
  xobject_template_skiplists[X_HOSTESCALATION_SKIPLIST] =
      skiplist_new(16, 0.5, false, false,
                   xodtemplate_skiplist_compare_hostescalation_template);
  xobject_template_skiplists[X_SERVICEESCALATION_SKIPLIST] =
      skiplist_new(16, 0.5, false, false,
                   xodtemplate_skiplist_compare_serviceescalation_template);
  xobject_template_skiplists[X_HOSTEXTINFO_SKIPLIST] = skiplist_new(
      16, 0.5, false, false, xodtemplate_skiplist_compare_hostextinfo_template);
  xobject_template_skiplists[X_SERVICEEXTINFO_SKIPLIST] =
      skiplist_new(16, 0.5, false, false,
                   xodtemplate_skiplist_compare_serviceextinfo_template);
  xobject_skiplists[X_HOST_SKIPLIST] =
      skiplist_new(16, 0.5, false, false, xodtemplate_skiplist_compare_host);
  xobject_skiplists[X_SERVICE_SKIPLIST] =
      skiplist_new(16, 0.5, false, false, xodtemplate_skiplist_compare_service);
  xobject_skiplists[X_COMMAND_SKIPLIST] =
      skiplist_new(16, 0.5, false, false, xodtemplate_skiplist_compare_command);
  xobject_skiplists[X_CONNECTOR_SKIPLIST] = skiplist_new(
      16, 0.5, false, false, xodtemplate_skiplist_compare_connector);
  xobject_skiplists[X_TIMEPERIOD_SKIPLIST] = skiplist_new(
      16, 0.5, false, false, xodtemplate_skiplist_compare_timeperiod);
  xobject_skiplists[X_CONTACT_SKIPLIST] =
      skiplist_new(10, 0.5, false, false, xodtemplate_skiplist_compare_contact);
  xobject_skiplists[X_CONTACTGROUP_SKIPLIST] = skiplist_new(
      10, 0.5, false, false, xodtemplate_skiplist_compare_contactgroup);
  xobject_skiplists[X_HOSTGROUP_SKIPLIST] = skiplist_new(
      10, 0.5, false, false, xodtemplate_skiplist_compare_hostgroup);
  xobject_skiplists[X_SERVICEGROUP_SKIPLIST] = skiplist_new(
      10, 0.5, false, false, xodtemplate_skiplist_compare_servicegroup);
  /* allow dups in the following lists... */
  xobject_skiplists[X_HOSTDEPENDENCY_SKIPLIST] = skiplist_new(
      16, 0.5, true, false, xodtemplate_skiplist_compare_hostdependency);
  xobject_skiplists[X_SERVICEDEPENDENCY_SKIPLIST] = skiplist_new(
      16, 0.5, true, false, xodtemplate_skiplist_compare_servicedependency);
  xobject_skiplists[X_HOSTESCALATION_SKIPLIST] = skiplist_new(
      16, 0.5, true, false, xodtemplate_skiplist_compare_hostescalation);
  xobject_skiplists[X_SERVICEESCALATION_SKIPLIST] = skiplist_new(
      16, 0.5, true, false, xodtemplate_skiplist_compare_serviceescalation);
  /* host and service extinfo entries don't need to be added to a list... */

  return OK;
}

int xodtemplate_free_xobject_skiplists() {
  int x = 0;

  for (x = 0; x < NUM_XOBJECT_SKIPLISTS; x++) {
    skiplist_free(&xobject_template_skiplists[x]);
    skiplist_free(&xobject_skiplists[x]);
  }

  return OK;
}

int xodtemplate_skiplist_compare_text(const char* val1a,
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

  return result;
}

int xodtemplate_skiplist_compare_host_template(void const* a, void const* b) {
  xodtemplate_host const* oa = static_cast<xodtemplate_host const*>(a);
  xodtemplate_host const* ob = static_cast<xodtemplate_host const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return skiplist_compare_text(oa->name, NULL, ob->name, NULL);
}

int xodtemplate_skiplist_compare_host(void const* a, void const* b) {
  xodtemplate_host const* oa = static_cast<xodtemplate_host const*>(a);
  xodtemplate_host const* ob = static_cast<xodtemplate_host const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return skiplist_compare_text(oa->host_name, NULL, ob->host_name, NULL);
}

int xodtemplate_skiplist_compare_service_template(void const* a,
                                                  void const* b) {
  xodtemplate_service const* oa = static_cast<xodtemplate_service const*>(a);
  xodtemplate_service const* ob = static_cast<xodtemplate_service const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return skiplist_compare_text(oa->name, NULL, ob->name, NULL);
}

int xodtemplate_skiplist_compare_service(void const* a, void const* b) {
  xodtemplate_service const* oa = static_cast<xodtemplate_service const*>(a);
  xodtemplate_service const* ob = static_cast<xodtemplate_service const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return (skiplist_compare_text(oa->host_name, oa->service_description,
                                ob->host_name, ob->service_description));
}

int xodtemplate_skiplist_compare_timeperiod_template(void const* a,
                                                     void const* b) {
  xodtemplate_timeperiod const* oa =
      static_cast<xodtemplate_timeperiod const*>(a);
  xodtemplate_timeperiod const* ob =
      static_cast<xodtemplate_timeperiod const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return skiplist_compare_text(oa->name, NULL, ob->name, NULL);
}

int xodtemplate_skiplist_compare_timeperiod(void const* a, void const* b) {
  xodtemplate_timeperiod const* oa =
      static_cast<xodtemplate_timeperiod const*>(a);
  xodtemplate_timeperiod const* ob =
      static_cast<xodtemplate_timeperiod const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return (skiplist_compare_text(oa->timeperiod_name, NULL, ob->timeperiod_name,
                                NULL));
}

int xodtemplate_skiplist_compare_command_template(void const* a,
                                                  void const* b) {
  xodtemplate_command const* oa = static_cast<xodtemplate_command const*>(a);
  xodtemplate_command const* ob = static_cast<xodtemplate_command const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return skiplist_compare_text(oa->name, NULL, ob->name, NULL);
}

int xodtemplate_skiplist_compare_connector_template(void const* a,
                                                    void const* b) {
  xodtemplate_connector const* oa =
      static_cast<xodtemplate_connector const*>(a);
  xodtemplate_connector const* ob =
      static_cast<xodtemplate_connector const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return skiplist_compare_text(oa->name, NULL, ob->name, NULL);
}

int xodtemplate_skiplist_compare_command(void const* a, void const* b) {
  xodtemplate_command const* oa = static_cast<xodtemplate_command const*>(a);
  xodtemplate_command const* ob = static_cast<xodtemplate_command const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return (
      skiplist_compare_text(oa->command_name, NULL, ob->command_name, NULL));
}

int xodtemplate_skiplist_compare_connector(void const* a, void const* b) {
  xodtemplate_connector const* oa =
      static_cast<xodtemplate_connector const*>(a);
  xodtemplate_connector const* ob =
      static_cast<xodtemplate_connector const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return (skiplist_compare_text(oa->connector_name, NULL, ob->connector_name,
                                NULL));
}

int xodtemplate_skiplist_compare_contact_template(void const* a,
                                                  void const* b) {
  xodtemplate_contact const* oa = static_cast<xodtemplate_contact const*>(a);
  xodtemplate_contact const* ob = static_cast<xodtemplate_contact const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return skiplist_compare_text(oa->name, NULL, ob->name, NULL);
}

int xodtemplate_skiplist_compare_contact(void const* a, void const* b) {
  xodtemplate_contact const* oa = static_cast<xodtemplate_contact const*>(a);
  xodtemplate_contact const* ob = static_cast<xodtemplate_contact const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return (
      skiplist_compare_text(oa->contact_name, NULL, ob->contact_name, NULL));
}

int xodtemplate_skiplist_compare_contactgroup_template(void const* a,
                                                       void const* b) {
  xodtemplate_contactgroup const* oa =
      static_cast<xodtemplate_contactgroup const*>(a);
  xodtemplate_contactgroup const* ob =
      static_cast<xodtemplate_contactgroup const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return skiplist_compare_text(oa->name, NULL, ob->name, NULL);
}

int xodtemplate_skiplist_compare_contactgroup(void const* a, void const* b) {
  xodtemplate_contactgroup const* oa =
      static_cast<xodtemplate_contactgroup const*>(a);
  xodtemplate_contactgroup const* ob =
      static_cast<xodtemplate_contactgroup const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return (skiplist_compare_text(oa->contactgroup_name, NULL,
                                ob->contactgroup_name, NULL));
}

int xodtemplate_skiplist_compare_hostgroup_template(void const* a,
                                                    void const* b) {
  xodtemplate_hostgroup const* oa =
      static_cast<xodtemplate_hostgroup const*>(a);
  xodtemplate_hostgroup const* ob =
      static_cast<xodtemplate_hostgroup const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return skiplist_compare_text(oa->name, NULL, ob->name, NULL);
}

int xodtemplate_skiplist_compare_hostgroup(void const* a, void const* b) {
  xodtemplate_hostgroup const* oa =
      static_cast<xodtemplate_hostgroup const*>(a);
  xodtemplate_hostgroup const* ob =
      static_cast<xodtemplate_hostgroup const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return (skiplist_compare_text(oa->hostgroup_name, NULL, ob->hostgroup_name,
                                NULL));
}

int xodtemplate_skiplist_compare_servicegroup_template(void const* a,
                                                       void const* b) {
  xodtemplate_servicegroup const* oa =
      static_cast<xodtemplate_servicegroup const*>(a);
  xodtemplate_servicegroup const* ob =
      static_cast<xodtemplate_servicegroup const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return skiplist_compare_text(oa->name, NULL, ob->name, NULL);
}

int xodtemplate_skiplist_compare_servicegroup(void const* a, void const* b) {
  xodtemplate_servicegroup const* oa =
      static_cast<xodtemplate_servicegroup const*>(a);
  xodtemplate_servicegroup const* ob =
      static_cast<xodtemplate_servicegroup const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return (skiplist_compare_text(oa->servicegroup_name, NULL,
                                ob->servicegroup_name, NULL));
}

int xodtemplate_skiplist_compare_hostdependency_template(void const* a,
                                                         void const* b) {
  xodtemplate_hostdependency const* oa =
      static_cast<xodtemplate_hostdependency const*>(a);
  xodtemplate_hostdependency const* ob =
      static_cast<xodtemplate_hostdependency const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return skiplist_compare_text(oa->name, NULL, ob->name, NULL);
}

int xodtemplate_skiplist_compare_hostdependency(void const* a, void const* b) {
  xodtemplate_hostdependency const* oa =
      static_cast<xodtemplate_hostdependency const*>(a);
  xodtemplate_hostdependency const* ob =
      static_cast<xodtemplate_hostdependency const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return (skiplist_compare_text(oa->dependent_host_name, NULL,
                                ob->dependent_host_name, NULL));
}

int xodtemplate_skiplist_compare_servicedependency_template(void const* a,
                                                            void const* b) {
  xodtemplate_servicedependency const* oa =
      static_cast<xodtemplate_servicedependency const*>(a);
  xodtemplate_servicedependency const* ob =
      static_cast<xodtemplate_servicedependency const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return skiplist_compare_text(oa->name, NULL, ob->name, NULL);
}

int xodtemplate_skiplist_compare_servicedependency(void const* a,
                                                   void const* b) {
  xodtemplate_servicedependency const* oa =
      static_cast<xodtemplate_servicedependency const*>(a);
  xodtemplate_servicedependency const* ob =
      static_cast<xodtemplate_servicedependency const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return (skiplist_compare_text(
      oa->dependent_host_name, oa->dependent_service_description,
      ob->dependent_host_name, ob->dependent_service_description));
}

int xodtemplate_skiplist_compare_hostescalation_template(void const* a,
                                                         void const* b) {
  xodtemplate_hostescalation const* oa =
      static_cast<xodtemplate_hostescalation const*>(a);
  xodtemplate_hostescalation const* ob =
      static_cast<xodtemplate_hostescalation const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return skiplist_compare_text(oa->name, NULL, ob->name, NULL);
}

int xodtemplate_skiplist_compare_hostescalation(void const* a, void const* b) {
  xodtemplate_hostescalation const* oa =
      static_cast<xodtemplate_hostescalation const*>(a);
  xodtemplate_hostescalation const* ob =
      static_cast<xodtemplate_hostescalation const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return (skiplist_compare_text(oa->host_name, NULL, ob->host_name, NULL));
}

int xodtemplate_skiplist_compare_serviceescalation_template(void const* a,
                                                            void const* b) {
  xodtemplate_serviceescalation const* oa =
      static_cast<xodtemplate_serviceescalation const*>(a);
  xodtemplate_serviceescalation const* ob =
      static_cast<xodtemplate_serviceescalation const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return skiplist_compare_text(oa->name, NULL, ob->name, NULL);
}

int xodtemplate_skiplist_compare_serviceescalation(void const* a,
                                                   void const* b) {
  xodtemplate_serviceescalation const* oa =
      static_cast<xodtemplate_serviceescalation const*>(a);
  xodtemplate_serviceescalation const* ob =
      static_cast<xodtemplate_serviceescalation const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return (skiplist_compare_text(oa->host_name, oa->service_description,
                                ob->host_name, ob->service_description));
}

int xodtemplate_skiplist_compare_hostextinfo_template(void const* a,
                                                      void const* b) {
  xodtemplate_hostextinfo const* oa =
      static_cast<xodtemplate_hostextinfo const*>(a);
  xodtemplate_hostextinfo const* ob =
      static_cast<xodtemplate_hostextinfo const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return skiplist_compare_text(oa->name, NULL, ob->name, NULL);
}

int xodtemplate_skiplist_compare_serviceextinfo_template(void const* a,
                                                         void const* b) {
  xodtemplate_serviceextinfo const* oa =
      static_cast<xodtemplate_serviceextinfo const*>(a);
  xodtemplate_serviceextinfo const* ob =
      static_cast<xodtemplate_serviceextinfo const*>(b);

  if (oa == NULL && ob == NULL)
    return 0;
  if (oa == NULL)
    return 1;
  if (ob == NULL)
    return -1;

  return skiplist_compare_text(oa->name, NULL, ob->name, NULL);
}

/******************************************************************/
/********************** CLEANUP FUNCTIONS *************************/
/******************************************************************/

void xodtemplate_free_timeperiod(xodtemplate_timeperiod const* tperiod) {
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
  xodtemplate_contactgroup* this_contactgroup = NULL;
  xodtemplate_contactgroup* next_contactgroup = NULL;
  xodtemplate_hostgroup* this_hostgroup = NULL;
  xodtemplate_hostgroup* next_hostgroup = NULL;
  xodtemplate_servicegroup* this_servicegroup = NULL;
  xodtemplate_servicegroup* next_servicegroup = NULL;
  xodtemplate_servicedependency* this_servicedependency = NULL;
  xodtemplate_servicedependency* next_servicedependency = NULL;
  xodtemplate_serviceescalation* this_serviceescalation = NULL;
  xodtemplate_serviceescalation* next_serviceescalation = NULL;
  xodtemplate_contact* this_contact = NULL;
  xodtemplate_contact* next_contact = NULL;
  xodtemplate_host* this_host = NULL;
  xodtemplate_host* next_host = NULL;
  xodtemplate_service* this_service = NULL;
  xodtemplate_service* next_service = NULL;
  xodtemplate_hostdependency* this_hostdependency = NULL;
  xodtemplate_hostdependency* next_hostdependency = NULL;
  xodtemplate_hostescalation* this_hostescalation = NULL;
  xodtemplate_hostescalation* next_hostescalation = NULL;
  xodtemplate_hostextinfo* this_hostextinfo = NULL;
  xodtemplate_hostextinfo* next_hostextinfo = NULL;
  xodtemplate_serviceextinfo* this_serviceextinfo = NULL;
  xodtemplate_serviceextinfo* next_serviceextinfo = NULL;
  xodtemplate_customvariablesmember* this_customvariablesmember = NULL;
  xodtemplate_customvariablesmember* next_customvariablesmember = NULL;
  int x = 0;

  /* free memory allocated to timeperiod list */
  for (this_timeperiod = xodtemplate_timeperiod_list; this_timeperiod != NULL;
       this_timeperiod = next_timeperiod) {
    next_timeperiod = this_timeperiod->next;
    xodtemplate_free_timeperiod(this_timeperiod);
  }
  xodtemplate_timeperiod_list = NULL;
  xodtemplate_timeperiod_list_tail = NULL;

  /* free memory allocated to command list */
  for (this_command = xodtemplate_command_list; this_command != NULL;
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
  for (this_connector = xodtemplate_connector_list; this_connector != NULL;
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

  /* free memory allocated to contactgroup list */
  for (this_contactgroup = xodtemplate_contactgroup_list;
       this_contactgroup != NULL; this_contactgroup = next_contactgroup) {
    next_contactgroup = this_contactgroup->next;
    delete[] this_contactgroup->tmpl;
    delete[] this_contactgroup->name;
    delete[] this_contactgroup->contactgroup_name;
    delete[] this_contactgroup->alias;
    delete[] this_contactgroup->members;
    delete[] this_contactgroup->contactgroup_members;
    delete this_contactgroup;
  }
  xodtemplate_contactgroup_list = NULL;
  xodtemplate_contactgroup_list_tail = NULL;

  /* free memory allocated to hostgroup list */
  for (this_hostgroup = xodtemplate_hostgroup_list; this_hostgroup != NULL;
       this_hostgroup = next_hostgroup) {
    next_hostgroup = this_hostgroup->next;
    delete[] this_hostgroup->tmpl;
    delete[] this_hostgroup->name;
    delete[] this_hostgroup->hostgroup_name;
    delete[] this_hostgroup->alias;
    delete[] this_hostgroup->members;
    delete[] this_hostgroup->hostgroup_members;
    delete[] this_hostgroup->notes;
    delete[] this_hostgroup->notes_url;
    delete[] this_hostgroup->action_url;
    delete this_hostgroup;
  }
  xodtemplate_hostgroup_list = NULL;
  xodtemplate_hostgroup_list_tail = NULL;

  /* free memory allocated to servicegroup list */
  for (this_servicegroup = xodtemplate_servicegroup_list;
       this_servicegroup != NULL; this_servicegroup = next_servicegroup) {
    next_servicegroup = this_servicegroup->next;
    delete[] this_servicegroup->tmpl;
    delete[] this_servicegroup->name;
    delete[] this_servicegroup->servicegroup_name;
    delete[] this_servicegroup->alias;
    delete[] this_servicegroup->members;
    delete[] this_servicegroup->servicegroup_members;
    delete[] this_servicegroup->notes;
    delete[] this_servicegroup->notes_url;
    delete[] this_servicegroup->action_url;
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

  /* free memory allocated to serviceescalation list */
  for (this_serviceescalation = xodtemplate_serviceescalation_list;
       this_serviceescalation != NULL;
       this_serviceescalation = next_serviceescalation) {
    next_serviceescalation = this_serviceescalation->next;
    delete[] this_serviceescalation->tmpl;
    delete[] this_serviceescalation->name;
    delete[] this_serviceescalation->servicegroup_name;
    delete[] this_serviceescalation->hostgroup_name;
    delete[] this_serviceescalation->host_name;
    delete[] this_serviceescalation->service_description;
    delete[] this_serviceescalation->escalation_period;
    delete[] this_serviceescalation->contact_groups;
    delete[] this_serviceescalation->contacts;
    delete this_serviceescalation;
  }
  xodtemplate_serviceescalation_list = NULL;
  xodtemplate_serviceescalation_list_tail = NULL;

  /* free memory allocated to contact list */
  for (this_contact = xodtemplate_contact_list; this_contact != NULL;
       this_contact = next_contact) {
    /* free custom variables */
    this_customvariablesmember = this_contact->custom_variables;
    while (this_customvariablesmember != NULL) {
      next_customvariablesmember = this_customvariablesmember->next;
      delete[] this_customvariablesmember->variable_name;
      delete[] this_customvariablesmember->variable_value;
      delete this_customvariablesmember;
      this_customvariablesmember = next_customvariablesmember;
    }

    next_contact = this_contact->next;
    delete[] this_contact->tmpl;
    delete[] this_contact->name;
    delete[] this_contact->contact_name;
    delete[] this_contact->alias;
    delete[] this_contact->contact_groups;
    delete[] this_contact->email;
    delete[] this_contact->pager;
    for (x = 0; x < MAX_XODTEMPLATE_CONTACT_ADDRESSES; x++)
      delete[] this_contact->address[x];
    delete[] this_contact->service_notification_period;
    delete[] this_contact->service_notification_commands;
    delete[] this_contact->host_notification_period;
    delete[] this_contact->host_notification_commands;
    delete this_contact;
  }
  xodtemplate_contact_list = NULL;
  xodtemplate_contact_list_tail = NULL;

  /* free memory allocated to host list */
  for (this_host = xodtemplate_host_list; this_host != NULL;
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
    delete[] this_host->display_name;
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
    delete[] this_host->contact_groups;
    delete[] this_host->contacts;
    delete[] this_host->notification_period;
    delete[] this_host->notes;
    delete[] this_host->notes_url;
    delete[] this_host->action_url;
    delete[] this_host->icon_image;
    delete[] this_host->icon_image_alt;
    delete[] this_host->vrml_image;
    delete[] this_host->statusmap_image;
    delete this_host;
  }
  xodtemplate_host_list = NULL;
  xodtemplate_host_list_tail = NULL;

  /* free memory allocated to service list */
  for (this_service = xodtemplate_service_list; this_service != NULL;
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
    delete[] this_service->display_name;
    delete[] this_service->tmpl;
    delete[] this_service->name;
    delete[] this_service->hostgroup_name;
    delete[] this_service->host_name;
    delete[] this_service->service_description;
    delete[] this_service->service_groups;
    delete[] this_service->check_command;
    delete[] this_service->check_period;
    delete[] this_service->event_handler;
    delete[] this_service->notification_period;
    delete[] this_service->contact_groups;
    delete[] this_service->contacts;
    delete[] this_service->notes;
    delete[] this_service->notes_url;
    delete[] this_service->action_url;
    delete[] this_service->icon_image;
    delete[] this_service->icon_image_alt;
    delete this_service;
  }
  xodtemplate_service_list = NULL;
  xodtemplate_service_list_tail = NULL;

  /* free memory allocated to hostdependency list */
  for (this_hostdependency = xodtemplate_hostdependency_list;
       this_hostdependency != NULL; this_hostdependency = next_hostdependency) {
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

  /* free memory allocated to hostescalation list */
  for (this_hostescalation = xodtemplate_hostescalation_list;
       this_hostescalation != NULL; this_hostescalation = next_hostescalation) {
    next_hostescalation = this_hostescalation->next;
    delete[] this_hostescalation->tmpl;
    delete[] this_hostescalation->name;
    delete[] this_hostescalation->hostgroup_name;
    delete[] this_hostescalation->host_name;
    delete[] this_hostescalation->escalation_period;
    delete[] this_hostescalation->contact_groups;
    delete[] this_hostescalation->contacts;
    delete this_hostescalation;
  }
  xodtemplate_hostescalation_list = NULL;
  xodtemplate_hostescalation_list_tail = NULL;

  /* free memory allocated to hostextinfo list */
  for (this_hostextinfo = xodtemplate_hostextinfo_list;
       this_hostextinfo != NULL; this_hostextinfo = next_hostextinfo) {
    next_hostextinfo = this_hostextinfo->next;
    delete[] this_hostextinfo->tmpl;
    delete[] this_hostextinfo->name;
    delete[] this_hostextinfo->host_name;
    delete[] this_hostextinfo->hostgroup_name;
    delete[] this_hostextinfo->notes;
    delete[] this_hostextinfo->notes_url;
    delete[] this_hostextinfo->action_url;
    delete[] this_hostextinfo->icon_image;
    delete[] this_hostextinfo->icon_image_alt;
    delete[] this_hostextinfo->vrml_image;
    delete[] this_hostextinfo->statusmap_image;
    delete this_hostextinfo;
  }
  xodtemplate_hostextinfo_list = NULL;
  xodtemplate_hostextinfo_list_tail = NULL;

  /* free memory allocated to serviceextinfo list */
  for (this_serviceextinfo = xodtemplate_serviceextinfo_list;
       this_serviceextinfo != NULL; this_serviceextinfo = next_serviceextinfo) {
    next_serviceextinfo = this_serviceextinfo->next;
    delete[] this_serviceextinfo->tmpl;
    delete[] this_serviceextinfo->name;
    delete[] this_serviceextinfo->host_name;
    delete[] this_serviceextinfo->hostgroup_name;
    delete[] this_serviceextinfo->service_description;
    delete[] this_serviceextinfo->notes;
    delete[] this_serviceextinfo->notes_url;
    delete[] this_serviceextinfo->action_url;
    delete[] this_serviceextinfo->icon_image;
    delete[] this_serviceextinfo->icon_image_alt;
    delete this_serviceextinfo;
  }
  xodtemplate_serviceextinfo_list = NULL;
  xodtemplate_serviceextinfo_list_tail = NULL;

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
  return OK;
}

/* adds a member to a list */
int xodtemplate_add_member_to_memberlist(xodtemplate_memberlist** list,
                                         char* name1,
                                         char* name2) {
  xodtemplate_memberlist* temp_item = NULL;
  xodtemplate_memberlist* new_item = NULL;

  if (list == NULL)
    return ERROR;
  if (name1 == NULL)
    return ERROR;

  /* skip this member if its already in the list */
  for (temp_item = *list; temp_item; temp_item = temp_item->next) {
    if (!strcmp(temp_item->name1, name1)) {
      if (temp_item->name2 == NULL) {
        if (name2 == NULL)
          break;
      } else if (name2 != NULL && !strcmp(temp_item->name2, name2))
        break;
    }
  }
  if (temp_item)
    return OK;

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

  return OK;
}

/* frees memory allocated to a temporary member list */
int xodtemplate_free_memberlist(xodtemplate_memberlist** temp_list) {
  xodtemplate_memberlist* this_memberlist = NULL;
  xodtemplate_memberlist* next_memberlist = NULL;

  /* free memory allocated to member name list */
  for (this_memberlist = *temp_list; this_memberlist != NULL;
       this_memberlist = next_memberlist) {
    next_memberlist = this_memberlist->next;
    delete[] this_memberlist->name1;
    delete[] this_memberlist->name2;
    delete this_memberlist;
  }

  *temp_list = NULL;
  return OK;
}

/* remove an entry from the member list */
void xodtemplate_remove_memberlist_item(xodtemplate_memberlist* item,
                                        xodtemplate_memberlist** list) {
  xodtemplate_memberlist* temp_item = NULL;

  if (item == NULL || list == NULL)
    return;

  if (*list == NULL)
    return;

  if (*list == item)
    *list = item->next;
  else {
    for (temp_item = *list; temp_item != NULL; temp_item = temp_item->next) {
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

/* expands a comma-delimited list of contactgroups and/or contacts to member
 * contact names */
xodtemplate_memberlist* xodtemplate_expand_contactgroups_and_contacts(
    char* contactgroups,
    char* contacts,
    int _config_file,
    int _start_line) {
  xodtemplate_memberlist* temp_list = NULL;
  xodtemplate_memberlist* reject_list = NULL;
  xodtemplate_memberlist* list_ptr = NULL;
  xodtemplate_memberlist* reject_ptr = NULL;
  int result = OK;

  /* process list of contactgroups... */
  if (contactgroups != NULL) {
    /* expand contactgroups */
    result = xodtemplate_expand_contactgroups(
        &temp_list, &reject_list, contactgroups, _config_file, _start_line);
    if (result != OK) {
      xodtemplate_free_memberlist(&temp_list);
      xodtemplate_free_memberlist(&reject_list);
      return NULL;
    }
  }

  /* process contact names */
  if (contacts != NULL) {
    /* expand contacts */
    result = xodtemplate_expand_contacts(&temp_list, &reject_list, contacts,
                                         _config_file, _start_line);
    if (result != OK) {
      xodtemplate_free_memberlist(&temp_list);
      xodtemplate_free_memberlist(&reject_list);
      return NULL;
    }
  }

  /* remove rejects (if any) from the list (no duplicate entries exist in either
   * list) */
  /* NOTE: rejects from this list also affect contacts generated from processing
   * contactgroup names (see above) */
  for (reject_ptr = reject_list; reject_ptr != NULL;
       reject_ptr = reject_ptr->next) {
    for (list_ptr = temp_list; list_ptr != NULL; list_ptr = list_ptr->next) {
      if (!strcmp(reject_ptr->name1, list_ptr->name1)) {
        xodtemplate_remove_memberlist_item(list_ptr, &temp_list);
        break;
      }
    }
  }
  xodtemplate_free_memberlist(&reject_list);
  reject_list = NULL;

  return temp_list;
}

/* expands contactgroups */
int xodtemplate_expand_contactgroups(xodtemplate_memberlist** list,
                                     xodtemplate_memberlist** reject_list,
                                     char* contactgroups,
                                     int _config_file,
                                     int _start_line) {
  char* contactgroup_names = NULL;
  char* temp_ptr = NULL;
  xodtemplate_contactgroup* temp_contactgroup = NULL;
  regex_t preg;
  int found_match = true;
  int reject_item = false;
  int use_regexp = false;

  if (list == NULL || contactgroups == NULL)
    return ERROR;

  /* allocate memory for contactgroup name list */
  contactgroup_names = string::dup(contactgroups);

  for (temp_ptr = strtok(contactgroup_names, ","); temp_ptr != NULL;
       temp_ptr = strtok(NULL, ",")) {
    found_match = false;
    reject_item = false;

    /* strip trailing spaces */
    strip(temp_ptr);

    /* should we use regular expression matching? */
    if (config->use_regexp_matches() == true &&
        (config->use_true_regexp_matching() == true || strstr(temp_ptr, "*") ||
         strstr(temp_ptr, "?") || strstr(temp_ptr, "+") ||
         strstr(temp_ptr, "\\.")))
      use_regexp = true;
    else
      use_regexp = false;

    /* use regular expression matching */
    if (use_regexp == true) {
      /* compile regular expression */
      if (regcomp(&preg, temp_ptr, REG_EXTENDED)) {
        delete[] contactgroup_names;
        return ERROR;
      }

      /* test match against all contactgroup names */
      for (temp_contactgroup = xodtemplate_contactgroup_list;
           temp_contactgroup != NULL;
           temp_contactgroup = temp_contactgroup->next) {
        if (temp_contactgroup->contactgroup_name == NULL)
          continue;

        /* skip this contactgroup if it did not match the expression */
        if (regexec(&preg, temp_contactgroup->contactgroup_name, 0, NULL, 0))
          continue;

        found_match = true;

        /* dont' add contactgroups that shouldn't be registered */
        if (temp_contactgroup->register_object == false)
          continue;

        /* add contactgroup members to list */
        xodtemplate_add_contactgroup_members_to_memberlist(
            list, temp_contactgroup, _config_file, _start_line);
      }

      /* free memory allocated to compiled regexp */
      regfree(&preg);
    }
    /* use standard matching... */
    else {
      /* return a list of all contactgroups */
      if (!strcmp(temp_ptr, "*")) {
        found_match = true;

        for (temp_contactgroup = xodtemplate_contactgroup_list;
             temp_contactgroup != NULL;
             temp_contactgroup = temp_contactgroup->next) {
          /* dont' add contactgroups that shouldn't be registered */
          if (temp_contactgroup->register_object == false)
            continue;

          /* add contactgroup to list */
          xodtemplate_add_contactgroup_members_to_memberlist(
              list, temp_contactgroup, _config_file, _start_line);
        }
      }
      /* else this is just a single contactgroup... */
      else {
        /* this contactgroup should be excluded (rejected) */
        if (temp_ptr[0] == '!') {
          reject_item = true;
          temp_ptr++;
        }

        /* find the contactgroup */
        temp_contactgroup = xodtemplate_find_real_contactgroup(temp_ptr);
        if (temp_contactgroup != NULL) {
          found_match = true;

          /* add contactgroup members to proper list */
          xodtemplate_add_contactgroup_members_to_memberlist(
              (reject_item == true ? reject_list : list), temp_contactgroup,
              _config_file, _start_line);
        }
      }
    }

    if (found_match == false) {
      logger(log_config_error, basic)
          << "Error: Could not find any contactgroup matching '" << temp_ptr
          << "' (config file '" << xodtemplate_config_file_name(_config_file)
          << "', starting on line " << _start_line << ")";
      break;
    }
  }

  /* free memory */
  delete[] contactgroup_names;

  if (found_match == false)
    return ERROR;

  return OK;
}

/* expands contacts */
int xodtemplate_expand_contacts(xodtemplate_memberlist** list,
                                xodtemplate_memberlist** reject_list,
                                char* contacts,
                                int _config_file,
                                int _start_line) {
  char* contact_names = NULL;
  char* temp_ptr = NULL;
  xodtemplate_contact* temp_contact = NULL;
  regex_t preg;
  int found_match = true;
  int reject_item = false;
  int use_regexp = false;

  if (list == NULL || contacts == NULL)
    return ERROR;

  contact_names = string::dup(contacts);

  /* expand each contact name */
  for (temp_ptr = strtok(contact_names, ","); temp_ptr != NULL;
       temp_ptr = strtok(NULL, ",")) {
    found_match = false;
    reject_item = false;

    /* strip trailing spaces */
    strip(temp_ptr);

    /* should we use regular expression matching? */
    if (config->use_regexp_matches() == true &&
        (config->use_true_regexp_matching() == true || strstr(temp_ptr, "*") ||
         strstr(temp_ptr, "?") || strstr(temp_ptr, "+") ||
         strstr(temp_ptr, "\\.")))
      use_regexp = true;

    /* use regular expression matching */
    if (use_regexp == true) {
      /* compile regular expression */
      if (regcomp(&preg, temp_ptr, REG_EXTENDED)) {
        delete[] contact_names;
        return ERROR;
      }

      /* test match against all contacts */
      for (temp_contact = xodtemplate_contact_list; temp_contact != NULL;
           temp_contact = temp_contact->next) {
        if (temp_contact->contact_name == NULL)
          continue;

        /* skip this contact if it did not match the expression */
        if (regexec(&preg, temp_contact->contact_name, 0, NULL, 0))
          continue;

        found_match = true;

        /* dont' add contacts that shouldn't be registered */
        if (temp_contact->register_object == false)
          continue;

        /* add contact to list */
        xodtemplate_add_member_to_memberlist(list, temp_contact->contact_name,
                                             NULL);
      }

      /* free memory allocated to compiled regexp */
      regfree(&preg);
    }

    /* use standard matching... */
    else {
      /* return a list of all contacts */
      if (!strcmp(temp_ptr, "*")) {
        found_match = true;

        for (temp_contact = xodtemplate_contact_list; temp_contact != NULL;
             temp_contact = temp_contact->next) {
          if (temp_contact->contact_name == NULL)
            continue;

          /* dont' add contacts that shouldn't be registered */
          if (temp_contact->register_object == false)
            continue;

          /* add contact to list */
          xodtemplate_add_member_to_memberlist(list, temp_contact->contact_name,
                                               NULL);
        }
      }

      /* else this is just a single contact... */
      else {
        /* this contact should be excluded (rejected) */
        if (temp_ptr[0] == '!') {
          reject_item = true;
          temp_ptr++;
        }

        /* find the contact */
        temp_contact = xodtemplate_find_real_contact(temp_ptr);
        if (temp_contact != NULL) {
          found_match = true;

          /* add contact to list */
          xodtemplate_add_member_to_memberlist(
              (reject_item == true ? reject_list : list), temp_ptr, NULL);
        }
      }
    }

    if (found_match == false) {
      logger(log_config_error, basic)
          << "Error: Could not find any contact matching '" << temp_ptr
          << "' (config file '" << xodtemplate_config_file_name(_config_file)
          << "', starting on line " << _start_line << ")";
      break;
    }
  }

  /* free memory */
  delete[] contact_names;

  if (found_match == false)
    return ERROR;

  return OK;
}

/* adds members of a contactgroups to the list of expanded (accepted) or
 * rejected contacts */
int xodtemplate_add_contactgroup_members_to_memberlist(
    xodtemplate_memberlist** list,
    xodtemplate_contactgroup* temp_contactgroup,
    int _config_file,
    int _start_line) {
  char* group_members = NULL;
  char* member_name = NULL;
  char* member_ptr = NULL;

  (void)_config_file;
  (void)_start_line;

  if (list == NULL || temp_contactgroup == NULL)
    return ERROR;

  /* if we have no members, just return. Empty contactgroups are ok */
  if (temp_contactgroup->members == NULL)
    return OK;

  /* save a copy of the members */
  group_members = string::dup(temp_contactgroup->members);

  /* process all contacts that belong to the contactgroup */
  /* NOTE: members of the group have already have been expanded by
   * xodtemplate_recombobulate_contactgroups(), so we don't need to do it here
   */
  member_ptr = group_members;
  for (member_name = my_strsep(&member_ptr, ","); member_name != NULL;
       member_name = my_strsep(&member_ptr, ",")) {
    /* strip trailing spaces from member name */
    strip(member_name);

    /* add contact to the list */
    xodtemplate_add_member_to_memberlist(list, member_name, NULL);
  }

  delete[] group_members;

  return OK;
}

/* expands a comma-delimited list of hostgroups and/or hosts to member host
 * names */
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
    result = xodtemplate_expand_hostgroups(&temp_list, &reject_list, hostgroups,
                                           _config_file, _start_line);
    if (result != OK) {
      xodtemplate_free_memberlist(&temp_list);
      xodtemplate_free_memberlist(&reject_list);
      return NULL;
    }
  }

  /* process host names */
  if (hosts != NULL) {
    /* expand hosts */
    result = xodtemplate_expand_hosts(&temp_list, &reject_list, hosts,
                                      _config_file, _start_line);
    if (result != OK) {
      xodtemplate_free_memberlist(&temp_list);
      xodtemplate_free_memberlist(&reject_list);
      return NULL;
    }
  }

#ifdef TESTING
  printf("->PRIOR TO CLEANUP\n");
  printf("   REJECT LIST:\n");
  for (list_ptr = reject_list; list_ptr != NULL; list_ptr = list_ptr->next)
    printf("      '%s'\n", list_ptr->name1);
  printf("   ACCEPT LIST:\n");
  for (list_ptr = temp_list; list_ptr != NULL; list_ptr = list_ptr->next)
    printf("      '%s'\n", list_ptr->name1);
#endif

  /* remove rejects (if any) from the list (no duplicate entries exist in either
   * list) */
  /* NOTE: rejects from this list also affect hosts generated from processing
   * hostgroup names (see above) */
  for (reject_ptr = reject_list; reject_ptr != NULL;
       reject_ptr = reject_ptr->next) {
    for (list_ptr = temp_list; list_ptr != NULL; list_ptr = list_ptr->next) {
      if (!strcmp(reject_ptr->name1, list_ptr->name1)) {
        xodtemplate_remove_memberlist_item(list_ptr, &temp_list);
        break;
      }
    }
  }
  xodtemplate_free_memberlist(&reject_list);
  reject_list = NULL;
  return temp_list;
}

/* expands hostgroups */
int xodtemplate_expand_hostgroups(xodtemplate_memberlist** list,
                                  xodtemplate_memberlist** reject_list,
                                  char* hostgroups,
                                  int _config_file,
                                  int _start_line) {
  char* hostgroup_names = NULL;
  char* temp_ptr = NULL;
  xodtemplate_hostgroup* temp_hostgroup = NULL;
  regex_t preg;
  int found_match = true;
  int reject_item = false;
  int use_regexp = false;

  if (list == NULL || hostgroups == NULL)
    return ERROR;

  /* allocate memory for hostgroup name list */
  hostgroup_names = string::dup(hostgroups);

  for (temp_ptr = strtok(hostgroup_names, ","); temp_ptr != NULL;
       temp_ptr = strtok(NULL, ",")) {
    found_match = false;
    reject_item = false;

    /* strip trailing spaces */
    strip(temp_ptr);

    /* should we use regular expression matching? */
    if (config->use_regexp_matches() == true &&
        (config->use_true_regexp_matching() == true || strstr(temp_ptr, "*") ||
         strstr(temp_ptr, "?") || strstr(temp_ptr, "+") ||
         strstr(temp_ptr, "\\.")))
      use_regexp = true;
    else
      use_regexp = false;

    /* use regular expression matching */
    if (use_regexp == true) {
      /* compile regular expression */
      if (regcomp(&preg, temp_ptr, REG_EXTENDED)) {
        delete[] hostgroup_names;
        return ERROR;
      }

      /* test match against all hostgroup names */
      for (temp_hostgroup = xodtemplate_hostgroup_list; temp_hostgroup != NULL;
           temp_hostgroup = temp_hostgroup->next) {
        if (temp_hostgroup->hostgroup_name == NULL)
          continue;

        /* skip this hostgroup if it did not match the expression */
        if (regexec(&preg, temp_hostgroup->hostgroup_name, 0, NULL, 0))
          continue;

        found_match = true;

        /* dont' add hostgroups that shouldn't be registered */
        if (temp_hostgroup->register_object == false)
          continue;

        /* add hostgroup members to list */
        xodtemplate_add_hostgroup_members_to_memberlist(
            list, temp_hostgroup, _config_file, _start_line);
      }

      /* free memory allocated to compiled regexp */
      regfree(&preg);
    }
    /* use standard matching... */
    else {
      /* return a list of all hostgroups */
      if (!strcmp(temp_ptr, "*")) {
        found_match = true;

        for (temp_hostgroup = xodtemplate_hostgroup_list;
             temp_hostgroup != NULL; temp_hostgroup = temp_hostgroup->next) {
          /* dont' add hostgroups that shouldn't be registered */
          if (temp_hostgroup->register_object == false)
            continue;

          /* add hostgroup to list */
          xodtemplate_add_hostgroup_members_to_memberlist(
              list, temp_hostgroup, _config_file, _start_line);
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
              (reject_item == true ? reject_list : list), temp_hostgroup,
              _config_file, _start_line);
        }
      }
    }

    if (found_match == false) {
      logger(log_config_error, basic)
          << "Error: Could not find any hostgroup matching '" << temp_ptr
          << "' (config file '" << xodtemplate_config_file_name(_config_file)
          << "', starting on line " << _start_line << ")";
      break;
    }
  }

  /* free memory */
  delete[] hostgroup_names;

  if (found_match == false)
    return ERROR;

  return OK;
}

/* expands hosts */
int xodtemplate_expand_hosts(xodtemplate_memberlist** list,
                             xodtemplate_memberlist** reject_list,
                             char* hosts,
                             int _config_file,
                             int _start_line) {
  char* host_names = NULL;
  char* temp_ptr = NULL;
  xodtemplate_host* temp_host = NULL;
  regex_t preg;
  int found_match = true;
  int reject_item = false;
  int use_regexp = false;

  if (list == NULL || hosts == NULL)
    return ERROR;

  host_names = string::dup(hosts);

  /* expand each host name */
  for (temp_ptr = strtok(host_names, ","); temp_ptr;
       temp_ptr = strtok(NULL, ",")) {
    found_match = false;
    reject_item = false;

    /* strip trailing spaces */
    strip(temp_ptr);

    /* should we use regular expression matching? */
    if (config->use_regexp_matches() == true &&
        (config->use_true_regexp_matching() == true || strstr(temp_ptr, "*") ||
         strstr(temp_ptr, "?") || strstr(temp_ptr, "+") ||
         strstr(temp_ptr, "\\.")))
      use_regexp = true;

    /* use regular expression matching */
    if (use_regexp == true) {
      /* compile regular expression */
      if (regcomp(&preg, temp_ptr, REG_EXTENDED)) {
        delete[] host_names;
        return ERROR;
      }

      /* test match against all hosts */
      for (temp_host = xodtemplate_host_list; temp_host != NULL;
           temp_host = temp_host->next) {
        if (temp_host->host_name == NULL)
          continue;

        /* skip this host if it did not match the expression */
        if (regexec(&preg, temp_host->host_name, 0, NULL, 0))
          continue;

        found_match = true;

        /* dont' add hosts that shouldn't be registered */
        if (temp_host->register_object == false)
          continue;

        /* add host to list */
        xodtemplate_add_member_to_memberlist(list, temp_host->host_name, NULL);
      }

      /* free memory allocated to compiled regexp */
      regfree(&preg);
    }

    /* use standard matching... */
    else {
      /* return a list of all hosts */
      if (!strcmp(temp_ptr, "*")) {
        found_match = true;

        for (temp_host = xodtemplate_host_list; temp_host != NULL;
             temp_host = temp_host->next) {
          if (temp_host->host_name == NULL)
            continue;

          /* dont' add hosts that shouldn't be registered */
          if (temp_host->register_object == false)
            continue;

          /* add host to list */
          xodtemplate_add_member_to_memberlist(list, temp_host->host_name,
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
              (reject_item == true ? reject_list : list), temp_ptr, NULL);
        }
      }
    }

    if (found_match == false) {
      logger(log_config_error, basic)
          << "Error: Could not find any host matching '" << temp_ptr
          << "' (config file '" << xodtemplate_config_file_name(_config_file)
          << "', starting on line " << _start_line << ")";
      break;
    }
  }

  /* free memory */
  delete[] host_names;

  if (found_match == false)
    return ERROR;

  return OK;
}

/* adds members of a hostgroups to the list of expanded (accepted) or rejected
 * hosts */
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
    return ERROR;

  /* if we have no members, just return. Empty hostgroups are ok */
  if (temp_hostgroup->members == NULL) {
    return OK;
  }

  /* save a copy of the members */
  group_members = string::dup(temp_hostgroup->members);

  /* process all hosts that belong to the hostgroup */
  /* NOTE: members of the group have already have been expanded by
   * xodtemplate_recombobulate_hostgroups(), so we don't need to do it here */
  member_ptr = group_members;
  for (member_name = my_strsep(&member_ptr, ","); member_name != NULL;
       member_name = my_strsep(&member_ptr, ",")) {
    /* strip trailing spaces from member name */
    strip(member_name);

    /* add host to the list */
    xodtemplate_add_member_to_memberlist(list, member_name, NULL);
  }

  delete[] group_members;

  return OK;
}

/* expands a comma-delimited list of servicegroups and/or service descriptions
 */
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
        &temp_list, &reject_list, servicegroups, _config_file, _start_line);
    if (result != OK) {
      xodtemplate_free_memberlist(&temp_list);
      xodtemplate_free_memberlist(&reject_list);
      return NULL;
    }
  }

  /* process service names */
  if (host_name != NULL && services != NULL) {
    /* expand services */
    result = xodtemplate_expand_services(&temp_list, &reject_list, host_name,
                                         services, _config_file, _start_line);
    if (result != OK) {
      xodtemplate_free_memberlist(&temp_list);
      xodtemplate_free_memberlist(&reject_list);
      return NULL;
    }
  }

  /* remove rejects (if any) from the list (no duplicate entries exist in either
   * list) */
  /* NOTE: rejects from this list also affect hosts generated from processing
   * hostgroup names (see above) */
  for (reject_ptr = reject_list; reject_ptr != NULL;
       reject_ptr = reject_ptr->next) {
    for (list_ptr = temp_list; list_ptr != NULL; list_ptr = list_ptr->next) {
      if (!strcmp(reject_ptr->name1, list_ptr->name1) &&
          !strcmp(reject_ptr->name2, list_ptr->name2)) {
        xodtemplate_remove_memberlist_item(list_ptr, &temp_list);
        break;
      }
    }
  }
  xodtemplate_free_memberlist(&reject_list);
  reject_list = NULL;

  return temp_list;
}

/* expands servicegroups */
int xodtemplate_expand_servicegroups(xodtemplate_memberlist** list,
                                     xodtemplate_memberlist** reject_list,
                                     char* servicegroups,
                                     int _config_file,
                                     int _start_line) {
  xodtemplate_servicegroup* temp_servicegroup = NULL;
  regex_t preg;
  char* servicegroup_names = NULL;
  char* temp_ptr = NULL;
  int found_match = true;
  int reject_item = false;
  int use_regexp = false;

  if (list == NULL)
    return ERROR;
  if (servicegroups == NULL)
    return OK;

  /* allocate memory for servicegroup name list */
  servicegroup_names = string::dup(servicegroups);

  /* expand each servicegroup */
  for (temp_ptr = strtok(servicegroup_names, ","); temp_ptr != NULL;
       temp_ptr = strtok(NULL, ",")) {
    found_match = false;
    reject_item = false;

    /* strip trailing spaces */
    strip(temp_ptr);

    /* should we use regular expression matching? */
    if (config->use_regexp_matches() == true &&
        (config->use_true_regexp_matching() == true || strstr(temp_ptr, "*") ||
         strstr(temp_ptr, "?") || strstr(temp_ptr, "+") ||
         strstr(temp_ptr, "\\.")))
      use_regexp = true;
    else
      use_regexp = false;

    /* use regular expression matching */
    if (use_regexp == true) {
      /* compile regular expression */
      if (regcomp(&preg, temp_ptr, REG_EXTENDED)) {
        delete[] servicegroup_names;
        return ERROR;
      }

      /* test match against all servicegroup names */
      for (temp_servicegroup = xodtemplate_servicegroup_list;
           temp_servicegroup != NULL;
           temp_servicegroup = temp_servicegroup->next) {
        if (temp_servicegroup->servicegroup_name == NULL)
          continue;

        /* skip this servicegroup if it did not match the expression */
        if (regexec(&preg, temp_servicegroup->servicegroup_name, 0, NULL, 0))
          continue;

        found_match = true;

        /* dont' add servicegroups that shouldn't be registered */
        if (temp_servicegroup->register_object == false)
          continue;

        /* add servicegroup members to list */
        xodtemplate_add_servicegroup_members_to_memberlist(
            list, temp_servicegroup, _config_file, _start_line);
      }

      /* free memory allocated to compiled regexp */
      regfree(&preg);
    }

    /* use standard matching... */
    else {
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
              list, temp_servicegroup, _config_file, _start_line);
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
        if ((temp_servicegroup =
                 xodtemplate_find_real_servicegroup(temp_ptr)) != NULL) {
          found_match = true;

          /* add servicegroup members to list */
          xodtemplate_add_servicegroup_members_to_memberlist(
              (reject_item == true ? reject_list : list), temp_servicegroup,
              _config_file, _start_line);
        }
      }
    }

    /* we didn't find a matching servicegroup */
    if (found_match == false) {
      logger(log_config_error, basic)
          << "Error: Could not find any servicegroup matching '" << temp_ptr
          << "' (config file '" << xodtemplate_config_file_name(_config_file)
          << "', starting on line " << _start_line << ")";
      break;
    }
  }

  /* free memory */
  delete[] servicegroup_names;

  if (found_match == false)
    return ERROR;

  return OK;
}

/* expands services (host name is not expanded) */
int xodtemplate_expand_services(xodtemplate_memberlist** list,
                                xodtemplate_memberlist** reject_list,
                                char* host_name,
                                char* services,
                                int _config_file,
                                int _start_line) {
  char* service_names = NULL;
  char* temp_ptr = NULL;
  xodtemplate_service* temp_service = NULL;
  regex_t preg;
  regex_t preg2;
  int found_match = true;
  int reject_item = false;
  int use_regexp_host = false;
  int use_regexp_service = false;

  if (list == NULL)
    return ERROR;
  if (host_name == NULL || services == NULL)
    return OK;

  /* should we use regular expression matching for the host name? */
  if (config->use_regexp_matches() == true &&
      (config->use_true_regexp_matching() == true || strstr(host_name, "*") ||
       strstr(host_name, "?") || strstr(host_name, "+") ||
       strstr(host_name, "\\.")))
    use_regexp_host = true;

  /* compile regular expression for host name */
  if (use_regexp_host == true) {
    if (regcomp(&preg2, host_name, REG_EXTENDED))
      return ERROR;
  }

  if ((service_names = string::dup(services)) == NULL) {
    if (use_regexp_host == true)
      regfree(&preg2);
    return ERROR;
  }

  /* expand each service description */
  for (temp_ptr = strtok(service_names, ","); temp_ptr != NULL;
       temp_ptr = strtok(NULL, ",")) {
    found_match = false;
    reject_item = false;

    /* strip trailing spaces */
    strip(temp_ptr);

    /* should we use regular expression matching for the service description? */
    if (config->use_regexp_matches() == true &&
        (config->use_true_regexp_matching() == true || strstr(temp_ptr, "*") ||
         strstr(temp_ptr, "?") || strstr(temp_ptr, "+") ||
         strstr(temp_ptr, "\\.")))
      use_regexp_service = true;
    else
      use_regexp_service = false;

    /* compile regular expression for service description */
    if (use_regexp_service == true) {
      if (regcomp(&preg, temp_ptr, REG_EXTENDED)) {
        if (use_regexp_host == true)
          regfree(&preg2);
        delete[] service_names;
        return ERROR;
      }
    }

    /* use regular expression matching */
    if (use_regexp_host == true || use_regexp_service == true) {
      /* test match against all services */
      for (temp_service = xodtemplate_service_list; temp_service != NULL;
           temp_service = temp_service->next) {
        if (temp_service->host_name == NULL ||
            temp_service->service_description == NULL)
          continue;

        /* skip this service if it doesn't match the host name expression */
        if (use_regexp_host == true) {
          if (regexec(&preg2, temp_service->host_name, 0, NULL, 0))
            continue;
        } else {
          if (strcmp(temp_service->host_name, host_name))
            continue;
        }

        /* skip this service if it doesn't match the service description
         * expression */
        if (use_regexp_service == true) {
          if (regexec(&preg, temp_service->service_description, 0, NULL, 0))
            continue;
        } else {
          if (strcmp(temp_service->service_description, temp_ptr))
            continue;
        }

        found_match = true;

        /* dont' add services that shouldn't be registered */
        if (temp_service->register_object == false)
          continue;

        /* add service to the list */
        xodtemplate_add_member_to_memberlist(list, host_name,
                                             temp_service->service_description);
      }

      /* free memory allocated to compiled regexp */
      if (use_regexp_service == true)
        regfree(&preg);
    }

    /* use standard matching... */
    else {
      /* return a list of all services on the host */
      if (!strcmp(temp_ptr, "*")) {
        found_match = true;

        for (temp_service = xodtemplate_service_list; temp_service != NULL;
             temp_service = temp_service->next) {
          if (temp_service->host_name == NULL ||
              temp_service->service_description == NULL)
            continue;

          if (strcmp(temp_service->host_name, host_name))
            continue;

          /* dont' add services that shouldn't be registered */
          if (temp_service->register_object == false)
            continue;

          /* add service to the list */
          xodtemplate_add_member_to_memberlist(
              list, host_name, temp_service->service_description);
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
        if ((temp_service =
                 xodtemplate_find_real_service(host_name, temp_ptr)) != NULL) {
          found_match = true;

          /* add service to the list */
          xodtemplate_add_member_to_memberlist(
              (reject_item == true ? reject_list : list), host_name,
              temp_service->service_description);
        }
      }
    }

    /* we didn't find a match */
    if (found_match == false && reject_item == false) {
      logger(log_config_error, basic)
          << "Error: Could not find a service matching host name '" << host_name
          << "' and description '" << temp_ptr << "' (config file '"
          << xodtemplate_config_file_name(_config_file)
          << "', starting on line " << _start_line << ")";
      break;
    }
  }

  if (use_regexp_host == true)
    regfree(&preg2);
  delete[] service_names;

  if (found_match == false && reject_item == false)
    return ERROR;

  return OK;
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
    return ERROR;

  /* if we have no members, just return. Empty servicegroups are ok */
  if (temp_servicegroup->members == NULL) {
    return OK;
  }

  /* save a copy of the members */
  group_members = string::dup(temp_servicegroup->members);

  /* process all services that belong to the servicegroup */
  /* NOTE: members of the group have already have been expanded by
   * xodtemplate_recombobulate_servicegroups(), so we don't need to do it here
   */
  member_ptr = group_members;
  for (member_name = my_strsep(&member_ptr, ","); member_name != NULL;
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
      xodtemplate_add_member_to_memberlist(list, host_name, member_name);

      delete[] host_name;
      host_name = NULL;
    }
  }

  delete[] group_members;

  return OK;
}

/* returns a comma-delimited list of hostgroup names */
char* xodtemplate_process_hostgroup_names(char* hostgroups,
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
        &temp_list, &reject_list, hostgroups, _config_file, _start_line);
    if (result != OK) {
      xodtemplate_free_memberlist(&temp_list);
      xodtemplate_free_memberlist(&reject_list);
      return NULL;
    }

    /* remove rejects (if any) from the list (no duplicate entries exist in
     * either list) */
    for (reject_ptr = reject_list; reject_ptr != NULL;
         reject_ptr = reject_ptr->next) {
      for (list_ptr = temp_list; list_ptr != NULL; list_ptr = list_ptr->next) {
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
  for (this_list = temp_list; this_list != NULL; this_list = this_list->next) {
    if (buf == NULL) {
      buf = new char[strlen(this_list->name1) + 1];
      strcpy(buf, this_list->name1);
    } else {
      buf = resize_string(buf, strlen(buf) + strlen(this_list->name1) + 2);
      strcat(buf, ",");
      strcat(buf, this_list->name1);
    }
  }

  xodtemplate_free_memberlist(&temp_list);

  return buf;
}

/* return a list of hostgroup names */
int xodtemplate_get_hostgroup_names(xodtemplate_memberlist** list,
                                    xodtemplate_memberlist** reject_list,
                                    char* hostgroups,
                                    int _config_file,
                                    int _start_line) {
  char* hostgroup_names = NULL;
  char* temp_ptr = NULL;
  xodtemplate_hostgroup* temp_hostgroup = NULL;
  regex_t preg;
  int found_match = true;
  int reject_item = false;
  int use_regexp = false;

  if (list == NULL || hostgroups == NULL)
    return ERROR;

  /* allocate memory for hostgroup name list */
  hostgroup_names = string::dup(hostgroups);

  for (temp_ptr = strtok(hostgroup_names, ","); temp_ptr != NULL;
       temp_ptr = strtok(NULL, ",")) {
    found_match = false;
    reject_item = false;

    /* strip trailing spaces */
    strip(temp_ptr);

    /* should we use regular expression matching? */
    if (config->use_regexp_matches() == true &&
        (config->use_true_regexp_matching() == true || strstr(temp_ptr, "*") ||
         strstr(temp_ptr, "?") || strstr(temp_ptr, "+") ||
         strstr(temp_ptr, "\\.")))
      use_regexp = true;
    else
      use_regexp = false;

    /* use regular expression matching */
    if (use_regexp == true) {
      /* compile regular expression */
      if (regcomp(&preg, temp_ptr, REG_EXTENDED)) {
        delete[] hostgroup_names;
        return ERROR;
      }

      /* test match against all hostgroup names */
      for (temp_hostgroup = xodtemplate_hostgroup_list; temp_hostgroup != NULL;
           temp_hostgroup = temp_hostgroup->next) {
        if (temp_hostgroup->hostgroup_name == NULL)
          continue;

        /* skip this hostgroup if it did not match the expression */
        if (regexec(&preg, temp_hostgroup->hostgroup_name, 0, NULL, 0))
          continue;

        found_match = true;

        /* dont' add hostgroups that shouldn't be registered */
        if (temp_hostgroup->register_object == false)
          continue;

        /* add hostgroup to list */
        xodtemplate_add_member_to_memberlist(
            list, temp_hostgroup->hostgroup_name, NULL);
      }

      /* free memory allocated to compiled regexp */
      regfree(&preg);
    }

    /* use standard matching... */
    else {
      /* return a list of all hostgroups */
      if (!strcmp(temp_ptr, "*")) {
        found_match = true;

        for (temp_hostgroup = xodtemplate_hostgroup_list;
             temp_hostgroup != NULL; temp_hostgroup = temp_hostgroup->next) {
          /* dont' add hostgroups that shouldn't be registered */
          if (temp_hostgroup->register_object == false)
            continue;

          /* add hostgroup to list */
          xodtemplate_add_member_to_memberlist(
              list, temp_hostgroup->hostgroup_name, NULL);
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
              temp_hostgroup->hostgroup_name, NULL);
        }
      }
    }

    if (found_match == false) {
      logger(log_config_error, basic)
          << "Error: Could not find any hostgroup matching '" << temp_ptr
          << "' (config file '" << xodtemplate_config_file_name(_config_file)
          << "', starting on line " << _start_line << ")";
      break;
    }
  }

  /* free memory */
  delete[] hostgroup_names;

  if (found_match == false)
    return ERROR;

  return OK;
}

/* returns a comma-delimited list of contactgroup names */
char* xodtemplate_process_contactgroup_names(char* contactgroups,
                                             int _config_file,
                                             int _start_line) {
  xodtemplate_memberlist* temp_list = NULL;
  xodtemplate_memberlist* reject_list = NULL;
  xodtemplate_memberlist* list_ptr = NULL;
  xodtemplate_memberlist* reject_ptr = NULL;
  xodtemplate_memberlist* this_list = NULL;
  char* buf = NULL;
  int result = OK;

  /* process list of contactgroups... */
  if (contactgroups != NULL) {
    /* split group names into two lists */
    result = xodtemplate_get_contactgroup_names(
        &temp_list, &reject_list, contactgroups, _config_file, _start_line);
    if (result != OK) {
      xodtemplate_free_memberlist(&temp_list);
      xodtemplate_free_memberlist(&reject_list);
      return NULL;
    }

    /* remove rejects (if any) from the list (no duplicate entries exist in
     * either list) */
    for (reject_ptr = reject_list; reject_ptr != NULL;
         reject_ptr = reject_ptr->next) {
      for (list_ptr = temp_list; list_ptr != NULL; list_ptr = list_ptr->next) {
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
  for (this_list = temp_list; this_list != NULL; this_list = this_list->next) {
    if (buf == NULL) {
      buf = new char[strlen(this_list->name1) + 1];
      strcpy(buf, this_list->name1);
    } else {
      buf = resize_string(buf, strlen(buf) + strlen(this_list->name1) + 2);
      strcat(buf, ",");
      strcat(buf, this_list->name1);
    }
  }

  xodtemplate_free_memberlist(&temp_list);

  return buf;
}

/* return a list of contactgroup names */
int xodtemplate_get_contactgroup_names(xodtemplate_memberlist** list,
                                       xodtemplate_memberlist** reject_list,
                                       char* contactgroups,
                                       int _config_file,
                                       int _start_line) {
  char* contactgroup_names = NULL;
  char* temp_ptr = NULL;
  xodtemplate_contactgroup* temp_contactgroup = NULL;
  regex_t preg;
  int found_match = true;
  int reject_item = false;
  int use_regexp = false;

  if (list == NULL || contactgroups == NULL)
    return ERROR;

  /* allocate memory for contactgroup name list */
  contactgroup_names = string::dup(contactgroups);

  for (temp_ptr = strtok(contactgroup_names, ","); temp_ptr != NULL;
       temp_ptr = strtok(NULL, ",")) {
    found_match = false;
    reject_item = false;

    /* strip trailing spaces */
    strip(temp_ptr);

    /* should we use regular expression matching? */
    if (config->use_regexp_matches() == true &&
        (config->use_true_regexp_matching() == true || strstr(temp_ptr, "*") ||
         strstr(temp_ptr, "?") || strstr(temp_ptr, "+") ||
         strstr(temp_ptr, "\\.")))
      use_regexp = true;
    else
      use_regexp = false;

    /* use regular expression matching */
    if (use_regexp == true) {
      /* compile regular expression */
      if (regcomp(&preg, temp_ptr, REG_EXTENDED)) {
        delete[] contactgroup_names;
        return ERROR;
      }

      /* test match against all contactgroup names */
      for (temp_contactgroup = xodtemplate_contactgroup_list;
           temp_contactgroup != NULL;
           temp_contactgroup = temp_contactgroup->next) {
        if (temp_contactgroup->contactgroup_name == NULL)
          continue;

        /* skip this contactgroup if it did not match the expression */
        if (regexec(&preg, temp_contactgroup->contactgroup_name, 0, NULL, 0))
          continue;

        found_match = true;

        /* dont' add contactgroups that shouldn't be registered */
        if (temp_contactgroup->register_object == false)
          continue;

        /* add contactgroup to list */
        xodtemplate_add_member_to_memberlist(
            list, temp_contactgroup->contactgroup_name, NULL);
      }

      /* free memory allocated to compiled regexp */
      regfree(&preg);
    }

    /* use standard matching... */
    else {
      /* return a list of all contactgroups */
      if (!strcmp(temp_ptr, "*")) {
        found_match = true;

        for (temp_contactgroup = xodtemplate_contactgroup_list;
             temp_contactgroup != NULL;
             temp_contactgroup = temp_contactgroup->next) {
          /* dont' add contactgroups that shouldn't be registered */
          if (temp_contactgroup->register_object == false)
            continue;

          /* add contactgroup to list */
          xodtemplate_add_member_to_memberlist(
              list, temp_contactgroup->contactgroup_name, NULL);
        }
      }

      /* else this is just a single contactgroup... */
      else {
        /* this contactgroup should be excluded (rejected) */
        if (temp_ptr[0] == '!') {
          reject_item = true;
          temp_ptr++;
        }

        /* find the contactgroup */
        temp_contactgroup = xodtemplate_find_real_contactgroup(temp_ptr);
        if (temp_contactgroup != NULL) {
          found_match = true;

          /* add contactgroup members to proper list */
          xodtemplate_add_member_to_memberlist(
              (reject_item == true ? reject_list : list),
              temp_contactgroup->contactgroup_name, NULL);
        }
      }
    }

    if (found_match == false) {
      logger(log_config_error, basic)
          << "Error: Could not find any contactgroup matching '" << temp_ptr
          << "' (config file '" << xodtemplate_config_file_name(_config_file)
          << "', starting on line " << _start_line << ")";
      break;
    }
  }

  /* free memory */
  delete[] contactgroup_names;

  if (found_match == false)
    return ERROR;

  return OK;
}

/* returns a comma-delimited list of servicegroup names */
char* xodtemplate_process_servicegroup_names(char* servicegroups,
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
        &temp_list, &reject_list, servicegroups, _config_file, _start_line);
    if (result != OK) {
      xodtemplate_free_memberlist(&temp_list);
      xodtemplate_free_memberlist(&reject_list);
      return NULL;
    }

    /* remove rejects (if any) from the list (no duplicate entries exist in
     * either list) */
    for (reject_ptr = reject_list; reject_ptr != NULL;
         reject_ptr = reject_ptr->next) {
      for (list_ptr = temp_list; list_ptr != NULL; list_ptr = list_ptr->next) {
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
  for (this_list = temp_list; this_list != NULL; this_list = this_list->next) {
    if (buf == NULL) {
      buf = new char[strlen(this_list->name1) + 1];
      strcpy(buf, this_list->name1);
    } else {
      buf = resize_string(buf, strlen(buf) + strlen(this_list->name1) + 2);
      strcat(buf, ",");
      strcat(buf, this_list->name1);
    }
  }

  xodtemplate_free_memberlist(&temp_list);

  return buf;
}

/* return a list of servicegroup names */
int xodtemplate_get_servicegroup_names(xodtemplate_memberlist** list,
                                       xodtemplate_memberlist** reject_list,
                                       char* servicegroups,
                                       int _config_file,
                                       int _start_line) {
  char* servicegroup_names = NULL;
  char* temp_ptr = NULL;
  xodtemplate_servicegroup* temp_servicegroup = NULL;
  regex_t preg;
  int found_match = true;
  int reject_item = false;
  int use_regexp = false;

  if (list == NULL || servicegroups == NULL)
    return ERROR;

  /* allocate memory for servicegroup name list */
  servicegroup_names = string::dup(servicegroups);

  for (temp_ptr = strtok(servicegroup_names, ","); temp_ptr != NULL;
       temp_ptr = strtok(NULL, ",")) {
    found_match = false;
    reject_item = false;

    /* strip trailing spaces */
    strip(temp_ptr);

    /* should we use regular expression matching? */
    if (config->use_regexp_matches() == true &&
        (config->use_true_regexp_matching() == true || strstr(temp_ptr, "*") ||
         strstr(temp_ptr, "?") || strstr(temp_ptr, "+") ||
         strstr(temp_ptr, "\\.")))
      use_regexp = true;
    else
      use_regexp = false;

    /* use regular expression matching */
    if (use_regexp == true) {
      /* compile regular expression */
      if (regcomp(&preg, temp_ptr, REG_EXTENDED)) {
        delete[] servicegroup_names;
        return ERROR;
      }

      /* test match against all servicegroup names */
      for (temp_servicegroup = xodtemplate_servicegroup_list;
           temp_servicegroup != NULL;
           temp_servicegroup = temp_servicegroup->next) {
        if (temp_servicegroup->servicegroup_name == NULL)
          continue;

        /* skip this servicegroup if it did not match the expression */
        if (regexec(&preg, temp_servicegroup->servicegroup_name, 0, NULL, 0))
          continue;

        found_match = true;

        /* dont' add servicegroups that shouldn't be registered */
        if (temp_servicegroup->register_object == false)
          continue;

        /* add servicegroup to list */
        xodtemplate_add_member_to_memberlist(
            list, temp_servicegroup->servicegroup_name, NULL);
      }

      /* free memory allocated to compiled regexp */
      regfree(&preg);
    }

    /* use standard matching... */
    else {
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
              list, temp_servicegroup->servicegroup_name, NULL);
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
              temp_servicegroup->servicegroup_name, NULL);
        }
      }
    }

    if (found_match == false) {
      logger(log_config_error, basic)
          << "Error: Could not find any servicegroup matching '" << temp_ptr
          << "' (config file '" << xodtemplate_config_file_name(_config_file)
          << "', starting on line " << _start_line << ")";
      break;
    }
  }

  /* free memory */
  delete[] servicegroup_names;

  if (found_match == false)
    return ERROR;

  return OK;
}

/******************************************************************/
/****************** ADDITIVE INHERITANCE STUFF ********************/
/******************************************************************/

/* determines the value of an inherited string */
int xodtemplate_get_inherited_string(int* have_template_value,
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
        /* use the template value only if we need a value - otherwise stay NULL
         */
        if (*have_this_value == false) {
          /* NOTE: leave leading + sign if present, as it needed during object
           * resolution and will get stripped later */
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
          delete[] * this_value;
          *this_value = buf;
        }

        /* otherwise our value overrides/replaces the template value */
      }
    }

    /* template has a NULL value.... */

    *have_this_value = true;
  }

  return OK;
}

/* removes leading + sign from various directives */
int xodtemplate_clean_additive_string(char** str) {
  char* buf = NULL;

  /* remove the additive symbol if present */
  if (*str != NULL && *str[0] == '+') {
    buf = string::dup(*str + 1);
    delete[] * str;
    *str = buf;
  }

  return OK;
}

/* cleans strings which may contain additive inheritance directives */
/* NOTE: this must be done after objects are resolved */
int xodtemplate_clean_additive_strings() {
  xodtemplate_contactgroup* temp_contactgroup = NULL;
  xodtemplate_hostgroup* temp_hostgroup = NULL;
  xodtemplate_servicegroup* temp_servicegroup = NULL;
  xodtemplate_servicedependency* temp_servicedependency = NULL;
  xodtemplate_serviceescalation* temp_serviceescalation = NULL;
  xodtemplate_contact* temp_contact = NULL;
  xodtemplate_host* temp_host = NULL;
  xodtemplate_service* temp_service = NULL;
  xodtemplate_hostdependency* temp_hostdependency = NULL;
  xodtemplate_hostescalation* temp_hostescalation = NULL;

  /* resolve all contactgroup objects */
  for (temp_contactgroup = xodtemplate_contactgroup_list;
       temp_contactgroup != NULL; temp_contactgroup = temp_contactgroup->next) {
    xodtemplate_clean_additive_string(&temp_contactgroup->members);
    xodtemplate_clean_additive_string(&temp_contactgroup->contactgroup_members);
  }

  /* resolve all hostgroup objects */
  for (temp_hostgroup = xodtemplate_hostgroup_list; temp_hostgroup != NULL;
       temp_hostgroup = temp_hostgroup->next) {
    xodtemplate_clean_additive_string(&temp_hostgroup->members);
    xodtemplate_clean_additive_string(&temp_hostgroup->hostgroup_members);
  }

  /* resolve all servicegroup objects */
  for (temp_servicegroup = xodtemplate_servicegroup_list;
       temp_servicegroup != NULL; temp_servicegroup = temp_servicegroup->next) {
    xodtemplate_clean_additive_string(&temp_servicegroup->members);
    xodtemplate_clean_additive_string(&temp_servicegroup->servicegroup_members);
  }

  /* resolve all servicedependency objects */
  for (temp_servicedependency = xodtemplate_servicedependency_list;
       temp_servicedependency != NULL;
       temp_servicedependency = temp_servicedependency->next) {
    xodtemplate_clean_additive_string(
        &temp_servicedependency->servicegroup_name);
    xodtemplate_clean_additive_string(&temp_servicedependency->hostgroup_name);
    xodtemplate_clean_additive_string(&temp_servicedependency->host_name);
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

  /* resolve all serviceescalation objects */
  for (temp_serviceescalation = xodtemplate_serviceescalation_list;
       temp_serviceescalation != NULL;
       temp_serviceescalation = temp_serviceescalation->next) {
    /* 03/05/08 some vars are now handled in
     * xodtemplate_inherit_object_properties() */
    /*
      xodtemplate_clean_additive_string(&temp_serviceescalation->contact_groups);
      xodtemplate_clean_additive_string(&temp_serviceescalation->contacts);
    */
    xodtemplate_clean_additive_string(
        &temp_serviceescalation->servicegroup_name);
    xodtemplate_clean_additive_string(&temp_serviceescalation->hostgroup_name);
    xodtemplate_clean_additive_string(&temp_serviceescalation->host_name);
    xodtemplate_clean_additive_string(
        &temp_serviceescalation->service_description);
  }

  /* resolve all contact objects */
  for (temp_contact = xodtemplate_contact_list; temp_contact != NULL;
       temp_contact = temp_contact->next) {
    xodtemplate_clean_additive_string(&temp_contact->contact_groups);
    xodtemplate_clean_additive_string(
        &temp_contact->host_notification_commands);
    xodtemplate_clean_additive_string(
        &temp_contact->service_notification_commands);
  }

  /* clean all host objects */
  for (temp_host = xodtemplate_host_list; temp_host != NULL;
       temp_host = temp_host->next) {
    xodtemplate_clean_additive_string(&temp_host->contact_groups);
    xodtemplate_clean_additive_string(&temp_host->contacts);
    xodtemplate_clean_additive_string(&temp_host->parents);
    xodtemplate_clean_additive_string(&temp_host->host_groups);
  }

  /* clean all service objects */
  for (temp_service = xodtemplate_service_list; temp_service != NULL;
       temp_service = temp_service->next) {
    xodtemplate_clean_additive_string(&temp_service->contact_groups);
    xodtemplate_clean_additive_string(&temp_service->contacts);
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
    xodtemplate_clean_additive_string(&temp_hostdependency->hostgroup_name);
    xodtemplate_clean_additive_string(
        &temp_hostdependency->dependent_hostgroup_name);
  }

  /* resolve all hostescalation objects */
  for (temp_hostescalation = xodtemplate_hostescalation_list;
       temp_hostescalation != NULL;
       temp_hostescalation = temp_hostescalation->next) {
    /* 03/05/08 some vars are now handled in
     * xodtemplate_inherit_object_properties() */
    /*
      xodtemplate_clean_additive_string(&temp_hostescalation->contact_groups);
      xodtemplate_clean_additive_string(&temp_hostescalation->contacts);
    */
    xodtemplate_clean_additive_string(&temp_hostescalation->host_name);
    xodtemplate_clean_additive_string(&temp_hostescalation->hostgroup_name);
  }

  return OK;
}

int read_main_config_file(char const* main_config_file) {
  char* input = NULL;
  char* variable = NULL;
  char* value = NULL;
  char* error_message = NULL;
  char* temp_ptr = NULL;
  mmapfile* thefile = NULL;
  int current_line = 0;
  int error = false;
  int command_check_interval_is_seconds = false;
  char* modptr = NULL;
  char* argptr = NULL;
  DIR* tmpdir = NULL;

  /* open the config file for reading */
  if ((thefile = mmap_fopen(main_config_file)) == NULL) {
    logit(NSLOG_CONFIG_ERROR, true,
          "Error: Cannot open main configuration file '%s' for reading!",
          main_config_file);
    return ERROR;
  }

  /* save the main config file macro */
  delete[] macro_x[MACRO_MAINCONFIGFILE];
  if ((macro_x[MACRO_MAINCONFIGFILE] = (char*)string::dup(main_config_file)))
    strip(macro_x[MACRO_MAINCONFIGFILE]);

  /* process all lines in the config file */
  while (1) {
    /* free memory */
    delete[] input;
    input = NULL;
    delete[] variable;
    variable = NULL;
    delete[] value;
    value = NULL;

    /* read the next line */
    if ((input = mmap_fgets_multiline(thefile)) == NULL)
      break;

    current_line = thefile->current_line;

    strip(input);

    /* skip blank lines and comments */
    if (input[0] == '\x0' || input[0] == '#')
      continue;

    /* get the variable name */
    if ((temp_ptr = my_strtok(input, "=")) == NULL) {
      if (asprintf(&error_message, "NULL variable")) {
      }
      error = true;
      break;
    }
    if ((variable = (char*)string::dup(temp_ptr)) == NULL) {
      if (asprintf(&error_message, "malloc() error")) {
      }
      error = true;
      break;
    }

    /* get the value */
    if ((temp_ptr = my_strtok(NULL, "\n")) == NULL) {
      if (asprintf(&error_message, "NULL value")) {
      }
      error = true;
      break;
    }
    if ((value = (char*)string::dup(temp_ptr)) == NULL) {
      if (asprintf(&error_message, "malloc() error")) {
      }
      error = true;
      break;
    }
    strip(variable);
    strip(value);

    /* process the variable/value */

    if (!strcmp(variable, "resource_file")) {
      /* save the macro */
      delete[] macro_x[MACRO_RESOURCEFILE];
      macro_x[MACRO_RESOURCEFILE] = (char*)string::dup(value);

      /* process the resource file */
      // read_resource_file(value);
    }

    else if (!strcmp(variable, "log_file")) {
      if (strlen(value) > MAX_FILENAME_LENGTH - 1) {
        if (asprintf(&error_message, "Log file is too long")) {
        }
        error = true;
        break;
      }

      delete[] log_file;
      log_file = (char*)string::dup(value);

      /* save the macro */
      delete[] macro_x[MACRO_LOGFILE];
      macro_x[MACRO_LOGFILE] = (char*)string::dup(log_file);
    }

    else if (!strcmp(variable, "debug_level"))
      debug_level = atoi(value);

    else if (!strcmp(variable, "debug_verbosity"))
      debug_verbosity = atoi(value);

    else if (!strcmp(variable, "debug_file")) {
      if (strlen(value) > MAX_FILENAME_LENGTH - 1) {
        if (asprintf(&error_message, "Debug log file is too long")) {
        }
        error = true;
        break;
      }

      delete[] debug_file;
      debug_file = (char*)string::dup(value);
    }

    else if (!strcmp(variable, "max_debug_file_size"))
      max_debug_file_size = strtoul(value, NULL, 0);

    else if (!strcmp(variable, "command_file")) {
      if (strlen(value) > MAX_FILENAME_LENGTH - 1) {
        if (asprintf(&error_message, "Command file is too long")) {
        }
        error = true;
        break;
      }

      delete[] command_file;
      command_file = (char*)string::dup(value);

      /* save the macro */
      delete[] macro_x[MACRO_COMMANDFILE];
      macro_x[MACRO_COMMANDFILE] = (char*)string::dup(value);
    }

    else if (!strcmp(variable, "temp_file")) {
      // if(strlen(value)>MAX_FILENAME_LENGTH-1){
      //   if (asprintf(&error_message,"Temp file is too long")) {}
      //   error=true;
      //   break;
      // }

      // delete[] temp_file;
      // temp_file=(char *)string::dup(value);

      // /* save the macro */
      // delete[] macro_x[MACRO_TEMPFILE];
      // macro_x[MACRO_TEMPFILE]=(char *)string::dup(temp_file);
    }

    else if (!strcmp(variable, "temp_path")) {
      // if(strlen(value)>MAX_FILENAME_LENGTH-1){
      //   if (asprintf(&error_message,"Temp path is too long")) {}
      //   error=true;
      //   break;
      // }

      // if((tmpdir=opendir((char *)value))==NULL){
      //   if (asprintf(&error_message,"Temp path is not a valid directory")) {}
      //   error=true;
      //   break;
      // }
      // closedir(tmpdir);

      // delete[] temp_path;
      // if((temp_path=(char *)string::dup(value))){
      //   strip(temp_path);
      //   /* make sure we don't have a trailing slash */
      //   if(temp_path[strlen(temp_path)-1]=='/')
      //     temp_path[strlen(temp_path)-1]='\x0';
      // }

      // /* save the macro */
      // delete[] macro_x[MACRO_TEMPPATH];
      // macro_x[MACRO_TEMPPATH]=(char *)string::dup(temp_path);
    }

    else if (!strcmp(variable, "check_result_path")) {
      if (strlen(value) > MAX_FILENAME_LENGTH - 1) {
        if (asprintf(&error_message, "Check result path is too long")) {
        }
        error = true;
        break;
      }

      if ((tmpdir = opendir((char*)value)) == NULL) {
        if (asprintf(&error_message,
                     "Check result path is not a valid directory")) {
        }
        error = true;
        break;
      }
      closedir(tmpdir);

      delete[] check_result_path;
      if ((check_result_path = (char*)string::dup(value))) {
        strip(check_result_path);
        /* make sure we don't have a trailing slash */
        if (check_result_path[strlen(check_result_path) - 1] == '/')
          check_result_path[strlen(check_result_path) - 1] = '\x0';
      }
    }

    else if (!strcmp(variable, "max_check_result_file_age"))
      max_check_result_file_age = strtoul(value, NULL, 0);

    else if (!strcmp(variable, "lock_file")) {
      // if(strlen(value)>MAX_FILENAME_LENGTH-1){
      //   if (asprintf(&error_message,"Lock file is too long")) {}
      //   error=true;
      //   break;
      // }

      // delete[] lock_file;
      // lock_file=(char *)string::dup(value);
    }

    else if (!strcmp(variable, "global_host_event_handler")) {
      delete[] global_host_event_handler;
      global_host_event_handler = (char*)string::dup(value);
    }

    else if (!strcmp(variable, "global_service_event_handler")) {
      delete[] global_service_event_handler;
      global_service_event_handler = (char*)string::dup(value);
    }

    else if (!strcmp(variable, "ocsp_command")) {
      delete[] ocsp_command;
      ocsp_command = (char*)string::dup(value);
    }

    else if (!strcmp(variable, "ochp_command")) {
      delete[] ochp_command;
      ochp_command = (char*)string::dup(value);
    }

    else if (!strcmp(variable, "nagios_user")) {
      // delete[] nagios_user;
      // nagios_user=(char *)string::dup(value);
    }

    else if (!strcmp(variable, "nagios_group")) {
      // delete[] nagios_group;
      // nagios_group=(char *)string::dup(value);
    }

    else if (!strcmp(variable, "admin_email")) {
      /* save the macro */
      delete[] macro_x[MACRO_ADMINEMAIL];
      macro_x[MACRO_ADMINEMAIL] = (char*)string::dup(value);
    }

    else if (!strcmp(variable, "admin_pager")) {
      /* save the macro */
      delete[] macro_x[MACRO_ADMINPAGER];
      macro_x[MACRO_ADMINPAGER] = (char*)string::dup(value);
    }

    else if (!strcmp(variable, "use_syslog")) {
      if (strlen(value) != 1 || value[0] < '0' || value[0] > '1') {
        if (asprintf(&error_message, "Illegal value for use_syslog")) {
        }
        error = true;
        break;
      }

      use_syslog = (atoi(value) > 0) ? true : false;
    }

    else if (!strcmp(variable, "log_notifications")) {
      if (strlen(value) != 1 || value[0] < '0' || value[0] > '1') {
        if (asprintf(&error_message, "Illegal value for log_notifications")) {
        }
        error = true;
        break;
      }

      log_notifications = (atoi(value) > 0) ? true : false;
    }

    else if (!strcmp(variable, "log_service_retries")) {
      if (strlen(value) != 1 || value[0] < '0' || value[0] > '1') {
        if (asprintf(&error_message, "Illegal value for log_service_retries")) {
        }
        error = true;
        break;
      }

      log_service_retries = (atoi(value) > 0) ? true : false;
    }

    else if (!strcmp(variable, "log_host_retries")) {
      if (strlen(value) != 1 || value[0] < '0' || value[0] > '1') {
        if (asprintf(&error_message, "Illegal value for log_host_retries")) {
        }
        error = true;
        break;
      }

      log_host_retries = (atoi(value) > 0) ? true : false;
    }

    else if (!strcmp(variable, "log_event_handlers")) {
      if (strlen(value) != 1 || value[0] < '0' || value[0] > '1') {
        if (asprintf(&error_message, "Illegal value for log_event_handlers")) {
        }
        error = true;
        break;
      }

      log_event_handlers = (atoi(value) > 0) ? true : false;
    }

    else if (!strcmp(variable, "log_external_commands")) {
      if (strlen(value) != 1 || value[0] < '0' || value[0] > '1') {
        if (asprintf(&error_message,
                     "Illegal value for log_external_commands")) {
        }
        error = true;
        break;
      }

      log_external_commands = (atoi(value) > 0) ? true : false;
    }

    else if (!strcmp(variable, "log_passive_checks")) {
      if (strlen(value) != 1 || value[0] < '0' || value[0] > '1') {
        if (asprintf(&error_message, "Illegal value for log_passive_checks")) {
        }
        error = true;
        break;
      }

      log_passive_checks = (atoi(value) > 0) ? true : false;
    }

    else if (!strcmp(variable, "log_initial_states")) {
      if (strlen(value) != 1 || value[0] < '0' || value[0] > '1') {
        if (asprintf(&error_message, "Illegal value for log_initial_states")) {
        }
        error = true;
        break;
      }

      log_initial_states = (atoi(value) > 0) ? true : false;
    }

    else if (!strcmp(variable, "retain_state_information")) {
      if (strlen(value) != 1 || value[0] < '0' || value[0] > '1') {
        if (asprintf(&error_message,
                     "Illegal value for retain_state_information")) {
        }
        error = true;
        break;
      }

      retain_state_information = (atoi(value) > 0) ? true : false;
    }

    else if (!strcmp(variable, "retention_update_interval")) {
      int tmp(atoi(value));
      if (tmp < 0) {
        if (asprintf(&error_message,
                     "Illegal value for retention_update_interval")) {
        }
        error = true;
        break;
      }
      retention_update_interval = tmp;
    }

    else if (!strcmp(variable, "use_retained_program_state")) {
      if (strlen(value) != 1 || value[0] < '0' || value[0] > '1') {
        if (asprintf(&error_message,
                     "Illegal value for use_retained_program_state")) {
        }
        error = true;
        break;
      }

      use_retained_program_state = (atoi(value) > 0) ? true : false;
    }

    else if (!strcmp(variable, "use_retained_scheduling_info")) {
      if (strlen(value) != 1 || value[0] < '0' || value[0] > '1') {
        if (asprintf(&error_message,
                     "Illegal value for use_retained_scheduling_info")) {
        }
        error = true;
        break;
      }

      use_retained_scheduling_info = (atoi(value) > 0) ? true : false;
    }

    else if (!strcmp(variable, "retention_scheduling_horizon")) {
      retention_scheduling_horizon = atoi(value);

      if (retention_scheduling_horizon <= 0) {
        if (asprintf(&error_message,
                     "Illegal value for retention_scheduling_horizon")) {
        }
        error = true;
        break;
      }
    }

    else if (!strcmp(variable, "additional_freshness_latency"))
      additional_freshness_latency = atoi(value);

    else if (!strcmp(variable, "retained_host_attribute_mask"))
      retained_host_attribute_mask = strtoul(value, NULL, 0);

    else if (!strcmp(variable, "retained_service_attribute_mask")) {
      // retained_service_attribute_mask=strtoul(value,NULL,0);
    } else if (!strcmp(variable, "retained_process_host_attribute_mask"))
      retained_process_host_attribute_mask = strtoul(value, NULL, 0);

    else if (!strcmp(variable, "retained_process_service_attribute_mask")) {
      // retained_process_service_attribute_mask=strtoul(value,NULL,0);
    } else if (!strcmp(variable, "retained_contact_host_attribute_mask"))
      retained_contact_host_attribute_mask = strtoul(value, NULL, 0);

    else if (!strcmp(variable, "retained_contact_service_attribute_mask"))
      retained_contact_service_attribute_mask = strtoul(value, NULL, 0);

    else if (!strcmp(variable, "obsess_over_services")) {
      if (strlen(value) != 1 || value[0] < '0' || value[0] > '1') {
        if (asprintf(&error_message,
                     "Illegal value for obsess_over_services")) {
        }
        error = true;
        break;
      }

      obsess_over_services = (atoi(value) > 0) ? true : false;
    }

    else if (!strcmp(variable, "obsess_over_hosts")) {
      if (strlen(value) != 1 || value[0] < '0' || value[0] > '1') {
        if (asprintf(&error_message, "Illegal value for obsess_over_hosts")) {
        }
        error = true;
        break;
      }

      obsess_over_hosts = (atoi(value) > 0) ? true : false;
    }

    else if (!strcmp(variable, "translate_passive_host_checks")) {
      if (strlen(value) != 1 || value[0] < '0' || value[0] > '1') {
        if (asprintf(&error_message,
                     "Illegal value for translate_passive_host_checks")) {
        }
        error = true;
        break;
      }

      translate_passive_host_checks = (atoi(value) > 0) ? true : false;
    }

    else if (!strcmp(variable, "passive_host_checks_are_soft"))
      passive_host_checks_are_soft = (atoi(value) > 0) ? true : false;

    else if (!strcmp(variable, "service_check_timeout")) {
      service_check_timeout = atoi(value);

      if (service_check_timeout <= 0) {
        if (asprintf(&error_message,
                     "Illegal value for service_check_timeout")) {
        }
        error = true;
        break;
      }
    }

    else if (!strcmp(variable, "host_check_timeout")) {
      host_check_timeout = atoi(value);

      if (host_check_timeout <= 0) {
        if (asprintf(&error_message, "Illegal value for host_check_timeout")) {
        }
        error = true;
        break;
      }
    }

    else if (!strcmp(variable, "event_handler_timeout")) {
      event_handler_timeout = atoi(value);

      if (event_handler_timeout <= 0) {
        if (asprintf(&error_message,
                     "Illegal value for event_handler_timeout")) {
        }
        error = true;
        break;
      }
    }

    else if (!strcmp(variable, "notification_timeout")) {
      notification_timeout = atoi(value);

      if (notification_timeout <= 0) {
        if (asprintf(&error_message,
                     "Illegal value for notification_timeout")) {
        }
        error = true;
        break;
      }
    }

    else if (!strcmp(variable, "ocsp_timeout")) {
      ocsp_timeout = atoi(value);

      if (ocsp_timeout <= 0) {
        if (asprintf(&error_message, "Illegal value for ocsp_timeout")) {
        }
        error = true;
        break;
      }
    }

    else if (!strcmp(variable, "ochp_timeout")) {
      ochp_timeout = atoi(value);

      if (ochp_timeout <= 0) {
        if (asprintf(&error_message, "Illegal value for ochp_timeout")) {
        }
        error = true;
        break;
      }
    }

    else if (!strcmp(variable, "use_agressive_host_checking") ||
             !strcmp(variable, "use_aggressive_host_checking")) {
      if (strlen(value) != 1 || value[0] < '0' || value[0] > '1') {
        if (asprintf(&error_message,
                     "Illegal value for use_aggressive_host_checking")) {
        }
        error = true;
        break;
      }

      use_aggressive_host_checking = (atoi(value) > 0) ? true : false;
    }

    else if (!strcmp(variable, "cached_host_check_horizon"))
      cached_host_check_horizon = strtoul(value, NULL, 0);

    else if (!strcmp(variable, "enable_predictive_host_dependency_checks"))
      enable_predictive_host_dependency_checks =
          (atoi(value) > 0) ? true : false;

    else if (!strcmp(variable, "cached_service_check_horizon"))
      cached_service_check_horizon = strtoul(value, NULL, 0);

    else if (!strcmp(variable, "enable_predictive_service_dependency_checks"))
      enable_predictive_service_dependency_checks =
          (atoi(value) > 0) ? true : false;

    else if (!strcmp(variable, "soft_state_dependencies")) {
      if (strlen(value) != 1 || value[0] < '0' || value[0] > '1') {
        if (asprintf(&error_message,
                     "Illegal value for soft_state_dependencies")) {
        }
        error = true;
        break;
      }

      soft_state_dependencies = (atoi(value) > 0) ? true : false;
    }

    else if (!strcmp(variable, "log_rotation_method")) {
      // if(!strcmp(value,"n"))
      //   log_rotation_method=LOG_ROTATION_NONE;
      // else if(!strcmp(value,"h"))
      //   log_rotation_method=LOG_ROTATION_HOURLY;
      // else if(!strcmp(value,"d"))
      //   log_rotation_method=LOG_ROTATION_DAILY;
      // else if(!strcmp(value,"w"))
      //   log_rotation_method=LOG_ROTATION_WEEKLY;
      // else if(!strcmp(value,"m"))
      //   log_rotation_method=LOG_ROTATION_MONTHLY;
      // else{
      //   if (asprintf(&error_message,"Illegal value for log_rotation_method"))
      //   {} error=true; break;
      // }
    }

    else if (!strcmp(variable, "log_archive_path")) {
      // if(strlen(value)>MAX_FILENAME_LENGTH-1){
      //   if (asprintf(&error_message,"Log archive path too long")) {}
      //   error=true;
      //   break;
      // }

      // delete[] log_archive_path;
      // log_archive_path=(char *)string::dup(value);
    }

    else if (!strcmp(variable, "enable_event_handlers"))
      enable_event_handlers = (atoi(value) > 0) ? true : false;

    else if (!strcmp(variable, "enable_notifications"))
      enable_notifications = (atoi(value) > 0) ? true : false;

    else if (!strcmp(variable, "execute_service_checks"))
      execute_service_checks = (atoi(value) > 0) ? true : false;

    else if (!strcmp(variable, "accept_passive_service_checks"))
      accept_passive_service_checks = (atoi(value) > 0) ? true : false;

    else if (!strcmp(variable, "execute_host_checks"))
      execute_host_checks = (atoi(value) > 0) ? true : false;

    else if (!strcmp(variable, "accept_passive_host_checks"))
      accept_passive_host_checks = (atoi(value) > 0) ? true : false;

    else if (!strcmp(variable, "service_inter_check_delay_method")) {
      if (!strcmp(value, "n"))
        service_inter_check_delay_method = ICD_NONE;
      else if (!strcmp(value, "d"))
        service_inter_check_delay_method = ICD_DUMB;
      else if (!strcmp(value, "s"))
        service_inter_check_delay_method = ICD_SMART;
      else {
        service_inter_check_delay_method = ICD_USER;
        scheduling_info.service_inter_check_delay = strtod(value, NULL);
        if (scheduling_info.service_inter_check_delay <= 0.0) {
          if (asprintf(&error_message,
                       "Illegal value for service_inter_check_delay_method")) {
          }
          error = true;
          break;
        }
      }
    }

    else if (!strcmp(variable, "max_service_check_spread")) {
      strip(value);
      max_service_check_spread = atoi(value);
      if (max_service_check_spread < 1) {
        if (asprintf(&error_message,
                     "Illegal value for max_service_check_spread")) {
        }
        error = true;
        break;
      }
    }

    else if (!strcmp(variable, "host_inter_check_delay_method")) {
      if (!strcmp(value, "n"))
        host_inter_check_delay_method = ICD_NONE;
      else if (!strcmp(value, "d"))
        host_inter_check_delay_method = ICD_DUMB;
      else if (!strcmp(value, "s"))
        host_inter_check_delay_method = ICD_SMART;
      else {
        host_inter_check_delay_method = ICD_USER;
        scheduling_info.host_inter_check_delay = strtod(value, NULL);
        if (scheduling_info.host_inter_check_delay <= 0.0) {
          if (asprintf(&error_message,
                       "Illegal value for host_inter_check_delay_method")) {
          }
          error = true;
          break;
        }
      }
    }

    else if (!strcmp(variable, "max_host_check_spread")) {
      max_host_check_spread = atoi(value);
      if (max_host_check_spread < 1) {
        if (asprintf(&error_message,
                     "Illegal value for max_host_check_spread")) {
        }
        error = true;
        break;
      }
    }

    else if (!strcmp(variable, "service_interleave_factor")) {
      if (!strcmp(value, "s"))
        service_interleave_factor_method = ILF_SMART;
      else {
        service_interleave_factor_method = ILF_USER;
        scheduling_info.service_interleave_factor = atoi(value);
        if (scheduling_info.service_interleave_factor < 1)
          scheduling_info.service_interleave_factor = 1;
      }
    }

    else if (!strcmp(variable, "max_concurrent_checks")) {
      int tmp(atoi(value));
      if (tmp < 0) {
        if (asprintf(&error_message,
                     "Illegal value for max_concurrent_checks")) {
        }
        error = true;
        break;
      }
      max_parallel_service_checks = tmp;
    }

    else if (!strcmp(variable, "check_result_reaper_frequency") ||
             !strcmp(variable, "service_reaper_frequency")) {
      check_reaper_interval = atoi(value);
      if (check_reaper_interval < 1) {
        if (asprintf(&error_message,
                     "Illegal value for check_result_reaper_frequency")) {
        }
        error = true;
        break;
      }
    }

    else if (!strcmp(variable, "max_check_result_reaper_time")) {
      max_check_reaper_time = atoi(value);
      if (max_check_reaper_time < 1) {
        if (asprintf(&error_message,
                     "Illegal value for max_check_result_reaper_time")) {
        }
        error = true;
        break;
      }
    }

    else if (!strcmp(variable, "sleep_time")) {
      sleep_time = atof(value);
      if (sleep_time <= 0.0) {
        if (asprintf(&error_message, "Illegal value for sleep_time")) {
        }
        error = true;
        break;
      }
    }

    else if (!strcmp(variable, "interval_length")) {
      interval_length = atoi(value);
      if (interval_length < 1) {
        if (asprintf(&error_message, "Illegal value for interval_length")) {
        }
        error = true;
        break;
      }
    }

    else if (!strcmp(variable, "check_external_commands")) {
      if (strlen(value) != 1 || value[0] < '0' || value[0] > '1') {
        if (asprintf(&error_message,
                     "Illegal value for check_external_commands")) {
        }
        error = true;
        break;
      }

      check_external_commands = (atoi(value) > 0) ? true : false;
    }

    else if (!strcmp(variable, "command_check_interval")) {
      command_check_interval_is_seconds = (strstr(value, "s")) ? true : false;
      command_check_interval = atoi(value);
      if (command_check_interval < -1 || command_check_interval == 0) {
        if (asprintf(&error_message,
                     "Illegal value for command_check_interval")) {
        }
        error = true;
        break;
      }
    }

    else if (!strcmp(variable, "check_for_orphaned_services")) {
      if (strlen(value) != 1 || value[0] < '0' || value[0] > '1') {
        if (asprintf(&error_message,
                     "Illegal value for check_for_orphaned_services")) {
        }
        error = true;
        break;
      }

      check_orphaned_services = (atoi(value) > 0) ? true : false;
    }

    else if (!strcmp(variable, "check_for_orphaned_hosts")) {
      if (strlen(value) != 1 || value[0] < '0' || value[0] > '1') {
        if (asprintf(&error_message,
                     "Illegal value for check_for_orphaned_hosts")) {
        }
        error = true;
        break;
      }

      check_orphaned_hosts = (atoi(value) > 0) ? true : false;
    }

    else if (!strcmp(variable, "check_service_freshness")) {
      if (strlen(value) != 1 || value[0] < '0' || value[0] > '1') {
        if (asprintf(&error_message,
                     "Illegal value for check_service_freshness")) {
        }
        error = true;
        break;
      }

      check_service_freshness = (atoi(value) > 0) ? true : false;
    }

    else if (!strcmp(variable, "check_host_freshness")) {
      if (strlen(value) != 1 || value[0] < '0' || value[0] > '1') {
        if (asprintf(&error_message,
                     "Illegal value for check_host_freshness")) {
        }
        error = true;
        break;
      }

      check_host_freshness = (atoi(value) > 0) ? true : false;
    }

    else if (!strcmp(variable, "service_freshness_check_interval") ||
             !strcmp(variable, "freshness_check_interval")) {
      service_freshness_check_interval = atoi(value);
      if (service_freshness_check_interval <= 0) {
        if (asprintf(&error_message,
                     "Illegal value for service_freshness_check_interval")) {
        }
        error = true;
        break;
      }
    }

    else if (!strcmp(variable, "host_freshness_check_interval")) {
      host_freshness_check_interval = atoi(value);
      if (host_freshness_check_interval <= 0) {
        if (asprintf(&error_message,
                     "Illegal value for host_freshness_check_interval")) {
        }
        error = true;
        break;
      }
    } else if (!strcmp(variable, "auto_reschedule_checks")) {
      if (strlen(value) != 1 || value[0] < '0' || value[0] > '1') {
        if (asprintf(&error_message,
                     "Illegal value for auto_reschedule_checks")) {
        }
        error = true;
        break;
      }

      auto_reschedule_checks = (atoi(value) > 0) ? true : false;
    }

    else if (!strcmp(variable, "auto_rescheduling_interval")) {
      auto_rescheduling_interval = atoi(value);
      if (auto_rescheduling_interval <= 0) {
        if (asprintf(&error_message,
                     "Illegal value for auto_rescheduling_interval")) {
        }
        error = true;
        break;
      }
    }

    else if (!strcmp(variable, "auto_rescheduling_window")) {
      auto_rescheduling_window = atoi(value);
      if (auto_rescheduling_window <= 0) {
        if (asprintf(&error_message,
                     "Illegal value for auto_rescheduling_window")) {
        }
        error = true;
        break;
      }
    }

    else if (!strcmp(variable, "aggregate_status_updates")) {
      /* DEPRECATED - ALL UPDATED ARE AGGREGATED AS OF NAGIOS 3.X */
      /*aggregate_status_updates=(atoi(value)>0)?true:false;*/

      logit(NSLOG_CONFIG_WARNING, true,
            "Warning: aggregate_status_updates directive ignored.  All status "
            "file updates are now aggregated.");
    }

    else if (!strcmp(variable, "status_update_interval")) {
      status_update_interval = atoi(value);
      if (status_update_interval <= 1) {
        if (asprintf(&error_message,
                     "Illegal value for status_update_interval")) {
        }
        error = true;
        break;
      }
    }

    else if (!strcmp(variable, "time_change_threshold")) {
      time_change_threshold = atoi(value);

      if (time_change_threshold <= 5) {
        if (asprintf(&error_message,
                     "Illegal value for time_change_threshold")) {
        }
        error = true;
        break;
      }
    }

    else if (!strcmp(variable, "process_performance_data"))
      process_performance_data = (atoi(value) > 0) ? true : false;

    else if (!strcmp(variable, "enable_flap_detection"))
      enable_flap_detection = (atoi(value) > 0) ? true : false;

    else if (!strcmp(variable, "low_service_flap_threshold")) {
      low_service_flap_threshold = strtod(value, NULL);
      if (low_service_flap_threshold <= 0.0 ||
          low_service_flap_threshold >= 100.0) {
        if (asprintf(&error_message,
                     "Illegal value for low_service_flap_threshold")) {
        }
        error = true;
        break;
      }
    }

    else if (!strcmp(variable, "high_service_flap_threshold")) {
      high_service_flap_threshold = strtod(value, NULL);
      if (high_service_flap_threshold <= 0.0 ||
          high_service_flap_threshold > 100.0) {
        if (asprintf(&error_message,
                     "Illegal value for high_service_flap_threshold")) {
        }
        error = true;
        break;
      }
    }

    else if (!strcmp(variable, "low_host_flap_threshold")) {
      low_host_flap_threshold = strtod(value, NULL);
      if (low_host_flap_threshold <= 0.0 || low_host_flap_threshold >= 100.0) {
        if (asprintf(&error_message,
                     "Illegal value for low_host_flap_threshold")) {
        }
        error = true;
        break;
      }
    }

    else if (!strcmp(variable, "high_host_flap_threshold")) {
      high_host_flap_threshold = strtod(value, NULL);
      if (high_host_flap_threshold <= 0.0 || high_host_flap_threshold > 100.0) {
        if (asprintf(&error_message,
                     "Illegal value for high_host_flap_threshold")) {
        }
        error = true;
        break;
      }
    }

    else if (!strcmp(variable, "date_format")) {
      if (!strcmp(value, "euro"))
        date_format = DATE_FORMAT_EURO;
      else if (!strcmp(value, "iso8601"))
        date_format = DATE_FORMAT_ISO8601;
      else if (!strcmp(value, "strict-iso8601"))
        date_format = DATE_FORMAT_STRICT_ISO8601;
      else
        date_format = DATE_FORMAT_US;
    }

    else if (!strcmp(variable, "use_timezone")) {
      delete[] use_timezone;
      use_timezone = (char*)string::dup(value);
    }

    else if (!strcmp(variable, "p1_file")) {
      // if(strlen(value)>MAX_FILENAME_LENGTH-1){
      //   if (asprintf(&error_message,"P1 file is too long")) {}
      //   error=true;
      //   break;
      // }

      // delete[] p1_file;
      // p1_file=(char *)string::dup(value);
    }

    else if (!strcmp(variable, "event_broker_options")) {
      if (!strcmp(value, "-1"))
        event_broker_options = BROKER_EVERYTHING;
      else
        event_broker_options = strtoul(value, NULL, 0);
    }

    else if (!strcmp(variable, "illegal_object_name_chars"))
      illegal_object_chars = (char*)string::dup(value);

    else if (!strcmp(variable, "illegal_macro_output_chars"))
      illegal_output_chars = (char*)string::dup(value);

    else if (!strcmp(variable, "broker_module")) {
      modptr = strtok(value, " \n");
      argptr = strtok(NULL, "\n");
#ifdef USE_EVENT_BROKER
      neb_add_module(modptr, argptr, true);
#endif
    }

    else if (!strcmp(variable, "use_regexp_matching"))
      use_regexp_matches = (atoi(value) > 0) ? true : false;

    else if (!strcmp(variable, "use_true_regexp_matching"))
      use_true_regexp_matching = (atoi(value) > 0) ? true : false;

    else if (!strcmp(variable, "daemon_dumps_core")) {
      // if(strlen(value)!=1||value[0]<'0'||value[0]>'1'){
      //   if (asprintf(&error_message,"Illegal value for daemon_dumps_core"))
      //   {} error=true; break;
      // }

      // daemon_dumps_core=(atoi(value)>0)?true:false;
    }

    else if (!strcmp(variable, "use_large_installation_tweaks")) {
      if (strlen(value) != 1 || value[0] < '0' || value[0] > '1') {
        if (asprintf(&error_message,
                     "Illegal value for use_large_installation_tweaks ")) {
        }
        error = true;
        break;
      }

      use_large_installation_tweaks = (atoi(value) > 0) ? true : false;
    }

    else if (!strcmp(variable, "enable_environment_macros"))
      enable_environment_macros = (atoi(value) > 0) ? true : false;

    else if (!strcmp(variable, "free_child_process_memory")) {
      // free_child_process_memory=(atoi(value)>0)?true:false;
    } else if (!strcmp(variable, "child_processes_fork_twice")) {
      // child_processes_fork_twice=(atoi(value)>0)?true:false;
    } else if (!strcmp(variable, "enable_embedded_perl")) {
      // if(strlen(value)!=1||value[0]<'0'||value[0]>'1'){
      //   if (asprintf(&error_message,"Illegal value for
      //   enable_embedded_perl")) {} error=true; break;
      // }

      // enable_embedded_perl=(atoi(value)>0)?true:false;
    }

    else if (!strcmp(variable, "use_embedded_perl_implicitly")) {
      // if(strlen(value)!=1||value[0]<'0'||value[0]>'1'){
      //   if (asprintf(&error_message,"Illegal value for
      //   use_embedded_perl_implicitly")) {} error=true; break;
      // }

      // use_embedded_perl_implicitly=(atoi(value)>0)?true:false;
    }

    else if (!strcmp(variable, "external_command_buffer_slots"))
      external_command_buffer_slots = atoi(value);

    else if (!strcmp(variable, "check_for_updates")) {
      // check_for_updates=(atoi(value)>0)?true:false;
    } else if (!strcmp(variable, "bare_update_check")) {
      // bare_update_check=(atoi(value)>0)?true:false;
    }
    /*** AUTH_FILE VARIABLE USED BY EMBEDDED PERL INTERPRETER ***/
    else if (!strcmp(variable, "auth_file")) {
      // if(strlen(value)>MAX_FILENAME_LENGTH-1){
      //   if (asprintf(&error_message,"Auth file is too long")) {}
      //   error=true;
      //   break;
      // }

      // delete[] auth_file;
      // auth_file=(char *)string::dup(value);
    }

    /* warn about old variables */
    else if (!strcmp(variable, "comment_file") ||
             !strcmp(variable, "xcddefault_comment_file")) {
      logit(NSLOG_CONFIG_WARNING, true,
            "Warning: comment_file variable ignored.  Comments are now stored "
            "in the status and retention files.");
    } else if (!strcmp(variable, "downtime_file") ||
               !strcmp(variable, "xdddefault_downtime_file")) {
      logit(NSLOG_CONFIG_WARNING, true,
            "Warning: downtime_file variable ignored.  Downtime entries are "
            "now stored in the status and retention files.");
    }

    /* skip external data directives */
    else if (strstr(input, "x") == input)
      continue;

    /* ignore external variables */
    else if (!strcmp(variable, "status_file"))
      continue;
    else if (!strcmp(variable, "perfdata_timeout"))
      continue;
    else if (strstr(variable, "host_perfdata") == variable)
      continue;
    else if (strstr(variable, "service_perfdata") == variable)
      continue;
    else if (strstr(input, "cfg_file=") == input ||
             strstr(input, "cfg_dir=") == input)
      continue;
    else if (strstr(input, "state_retention_file=") == input)
      continue;
    else if (strstr(input, "object_cache_file=") == input)
      continue;
    else if (strstr(input, "precached_object_file=") == input)
      continue;

    /* we don't know what this variable is... */
    else {
      if (asprintf(&error_message, "UNKNOWN VARIABLE")) {
      }
      error = true;
      break;
    }
  }

  /* adjust timezone values */
  if (use_timezone != NULL)
    set_environment_var("TZ", use_timezone, 1);
  tzset();

  /* adjust command check interval */
  if (command_check_interval_is_seconds == false &&
      command_check_interval != -1)
    command_check_interval *= interval_length;

  /* adjust tweaks */
  // if(free_child_process_memory==-1)
  //   free_child_process_memory=(use_large_installation_tweaks==true)?false:true;
  // if(child_processes_fork_twice==-1)
  //   child_processes_fork_twice=(use_large_installation_tweaks==true)?false:true;

  /* handle errors */
  if (error == true) {
    logit(NSLOG_CONFIG_ERROR, true,
          "Error in configuration file '%s' - Line %d (%s)", main_config_file,
          current_line, (error_message == NULL) ? "NULL" : error_message);
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
  //     printf("Error: Log file is not specified anywhere in main config file
  //     '%s'!\n",main_config_file);
  //   return ERROR;
  // }

  return OK;
}
