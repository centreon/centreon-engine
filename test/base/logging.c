#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "configuration.hh"

#include "nagios.h"

#undef DEFAULT_CONFIG_FILE
#define DEFAULT_CONFIG_FILE "./smallconfig/nagios.cfg"

extern com::centreon::scheduler::configuration config;

extern char *config_file;
extern char *log_file;
extern char *temp_file;
extern char *log_archive_path;
extern int use_syslog;
extern unsigned long logging_options;

int main(void)
{
  time_t rotation_time;
  struct stat stat_info, stat_new;
  char *log_filename_localtime = NULL;
  char *temp_command = NULL;
  struct tm *t;
  int log_rotation_method;

  config_file = DEFAULT_CONFIG_FILE;

  if (reset_variables() == ERROR ||
      read_main_config_file(config_file) == ERROR ||
      read_all_object_data(config_file) == ERROR)
    return (1);

  log_file = strdup("var/nagios.log");
  temp_file = strdup("");
  log_archive_path = strdup("var");
  use_syslog = 0;
  logging_options = NSLOG_PROCESS_INFO;

  rotation_time = (time_t)1242949698;
  t = localtime(&rotation_time);

  if (asprintf(&log_filename_localtime, "var/nagios-%02d-%02d-%d-%02d.log", t->tm_mon + 1, t->tm_mday, t->tm_year + 1900, t->tm_hour) == -1)
    return (1);

  log_rotation_method = 5;
  if (rotate_log_file(rotation_time) != ERROR)
    return (1);

  log_file = strdup("renamefailure");
  log_rotation_method = LOG_ROTATION_HOURLY;
  if (rotate_log_file(rotation_time) != ERROR)
    return (1);

  log_file = strdup("var/nagios.log");
  log_rotation_method = LOG_ROTATION_HOURLY;
  if (system("cp var/nagios.log.dummy var/nagios.log") != 0)
    return (1);

  if (rotate_log_file(rotation_time) == ERROR)
    return (1);

  if (system("diff var/nagios.log var/nagios.log.expected > /dev/null") == -1)
    return (1);

  if (asprintf(&temp_command, "diff var/nagios.log.dummy %s", log_filename_localtime) == -1)
    return (1);

  if (system(temp_command) !=0)
    return (1);

  if (unlink(log_filename_localtime) == -1)
    return (1);

  if (system("chmod 777 var/nagios.log") != 0)
    return (1);

  if (stat("var/nagios.log", &stat_info) != 0)
    return (1);

  if (rotate_log_file(rotation_time) == ERROR)
    return (1);

  if (stat(log_filename_localtime, &stat_new) != 0)
    return (1);

  if (stat_info.st_mode != stat_new.st_mode)
    return (1);

  if (stat("var/nagios.log", &stat_new) != 0)
    return (1);

  if (stat_info.st_mode != stat_new.st_mode)
    return (1);

  cleanup();
  return (0);
}
