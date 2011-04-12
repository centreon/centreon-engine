/*
** Copyright 1999-2009 Ethan Galstad
** Copyright 2009-2010 Nagios Core Development Team and Community Contributors
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

#include <exception>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

/*#define DEBUG_MEMORY 1*/
#ifdef DEBUG_MEMORY
# include <mcheck.h>
#endif

#include "comments.hh"
#include "downtime.hh"
#include "statusdata.hh"
#include "sretention.hh"
#include "perfdata.hh"
#include "broker.hh"
#include "nebmods.hh"
#include "notifications.hh"
#include "config.hh"
#include "utils.hh"
#include "logging.hh"
#include "configuration.hh"
#include "modules/loader.hh"
#include "nagios.hh"

using namespace com::centreon::scheduler;

configuration             config;

char*                     config_file = NULL;

command*                  global_host_event_handler_ptr = NULL;
command*                  global_service_event_handler_ptr = NULL;

command*                  ocsp_command_ptr = NULL;
command*                  ochp_command_ptr = NULL;

unsigned long             logging_options = 0;
unsigned long             syslog_options = 0;

unsigned long             update_uid = 0L;
int                       update_available = FALSE;
char*                     last_program_version = NULL;
char*                     new_program_version = NULL;

time_t                    last_command_check = 0L;
time_t                    last_command_status_update = 0L;
time_t                    last_log_rotation = 0L;

unsigned long             modified_host_process_attributes = MODATTR_NONE;
unsigned long             modified_service_process_attributes = MODATTR_NONE;

unsigned long             next_comment_id = 0L;
unsigned long             next_downtime_id = 0L;
unsigned long             next_event_id = 0L;
unsigned long             next_problem_id = 0L;
unsigned long             next_notification_id = 0L;

int                       sigshutdown = FALSE;
int                       sigrestart = FALSE;

char const*               sigs[35] = {
  "EXIT", "HUP", "INT", "QUIT", "ILL",
  "TRAP", "ABRT", "BUS", "FPE", "KILL",
  "USR1", "SEGV", "USR2", "PIPE", "ALRM",
  "TERM", "STKFLT", "CHLD", "CONT", "STOP",
  "TSTP", "TTIN", "TTOU", "URG", "XCPU",
  "XFSZ", "VTALRM", "PROF", "WINCH", "IO",
  "PWR", "UNUSED", "ZERR", "DEBUG", NULL
};

int                       caught_signal = FALSE;
int                       sig_id = 0;

int                       restarting = FALSE;

int                       verify_config = FALSE;
int                       verify_object_relationships = TRUE;
int                       verify_circular_paths = TRUE;
int                       test_scheduling = FALSE;
int                       precache_objects = FALSE;
int                       use_precached_objects = FALSE;

unsigned int              currently_running_service_checks = 0;
unsigned int              currently_running_host_checks = 0;

time_t                    program_start = 0L;
time_t                    event_start = 0L;
int                       nagios_pid = 0;

int                       embedded_perl_initialized = FALSE;

int                       command_file_fd;
FILE*                     command_file_fp;
int                       command_file_created = FALSE;


extern contact*           contact_list;
extern contactgroup*      contactgroup_list;
extern hostgroup*         hostgroup_list;
extern command*           command_list;
extern timeperiod*        timeperiod_list;
extern serviceescalation* serviceescalation_list;

notification*             notification_list;

check_result              check_result_info;
check_result*             check_result_list = NULL;

dbuf                      check_result_dbuf;

circular_buffer           external_command_buffer;
circular_buffer           check_result_buffer;
pthread_t                 worker_threads[TOTAL_WORKER_THREADS];

check_stats               check_statistics[MAX_CHECK_STATS_TYPES];

/* Following main() declaration required by older versions of Perl ut 5.00503 */
#ifdef EMBEDDEDPERL
int main(int argc, char** argv, char** env) {
#else
int main(int argc, char** argv) {
#endif
  int result = ERROR;
  int error = FALSE;
  char* buffer = NULL;
  int display_license = FALSE;
  int display_help = FALSE;
  int c = 0;
  struct tm* tm, tm_s;
  time_t now;
  char datestring[256];
  nagios_macros* mac;

  mac = get_global_macros();

#ifdef HAVE_GETOPT_H
  int option_index = 0;
  static struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"version", no_argument, 0, 'V'},
    {"license", no_argument, 0, 'V'},
    {"verify-config", no_argument, 0, 'v'},
    {"test-scheduling", no_argument, 0, 's'},
    {"dont-verify-objects", no_argument, 0, 'o'},
    {"dont-verify-paths", no_argument, 0, 'x'},
    {"precache-objects", no_argument, 0, 'p'},
    {"use-precached-objects", no_argument, 0, 'u'},
    {0, 0, 0, 0}
  };
#endif

  /* make sure we have the correct number of command line arguments */
  if (argc < 2)
    error = TRUE;

  /* get all command line arguments */
  while (1) {

#ifdef HAVE_GETOPT_H
    c = getopt_long(argc, argv, "+hVvsoxpu", long_options, &option_index);
#else
    c = getopt(argc, argv, "+hVvsoxpu");
#endif

    if (c == -1 || c == EOF)
      break;

    switch (c) {
    case '?':                  /* usage */
    case 'h':
      display_help = TRUE;
    break;

    case 'V':                  /* version */
      display_license = TRUE;
      break;

    case 'v':                  /* verify */
      verify_config = TRUE;
      break;

    case 's':                  /* scheduling check */
      test_scheduling = TRUE;
      break;

    case 'o':                  /* don't verify objects */
      /*
	verify_object_relationships=FALSE;
      */
      break;

    case 'x':                  /* don't verify circular paths */
      verify_circular_paths = FALSE;
      break;

    case 'p':                  /* precache object config */
      precache_objects = TRUE;
      break;

    case 'u':                  /* use precached object config */
      use_precached_objects = TRUE;
      break;

    default:
      break;
    }
  }

  /* make sure we have the right combination of arguments */
  if (precache_objects == TRUE && (test_scheduling == FALSE && verify_config == FALSE)) {
    error = TRUE;
    display_help = TRUE;
  }

#ifdef DEBUG_MEMORY
  mtrace();
#endif

  /* just display the license */
  if (display_license == TRUE) {
    printf("This program is free software; you can redistribute it and/or modify\n");
    printf("it under the terms of the GNU General Public License version 2 as\n");
    printf("published by the Free Software Foundation.\n\n");
    printf("This program is distributed in the hope that it will be useful,\n");
    printf("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
    printf("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
    printf("GNU General Public License for more details.\n\n");
    printf("You should have received a copy of the GNU General Public License\n");
    printf("along with this program; if not, write to the Free Software\n");
    printf("Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n\n");

    exit(OK);
  }

  /* make sure we got the main config file on the command line... */
  if (optind >= argc)
    error = TRUE;

  /* if there are no command line options (or if we encountered an error), print usage */
  if (error == TRUE || display_help == TRUE) {
    printf("Usage: %s [options] <main_config_file>\n", argv[0]);
    printf("\n");
    printf("Options:\n");
    printf("\n");
    printf("  -v, --verify-config          Verify all configuration data\n");
    printf("  -s, --test-scheduling        Shows projected/recommended check scheduling and other\n");
    printf("                               diagnostic info based on the current configuration files.\n");
    /*printf("  -o, --dont-verify-objects    Don't verify object relationships - USE WITH CAUTION!\n"); */
    printf("  -x, --dont-verify-paths      Don't check for circular object paths - USE WITH CAUTION!\n");
    printf("  -p, --precache-objects       Precache object configuration - use with -v or -s options\n");
    printf("  -u, --use-precached-objects  Use precached object config file\n");
    printf("\n");
    printf("Visit the Nagios website at http://www.nagios.org/ for bug fixes, new\n");
    printf("releases, online documentation, FAQs, information on subscribing to\n");
    printf("the mailing lists, and commercial support options for Nagios.\n");
    printf("\n");

    exit(ERROR);
  }

  /* config file is last argument specified */
  config_file = my_strdup(argv[optind]);

  /* make sure the config file uses an absolute path */
  if (config_file[0] != '/') {
    /* save the name of the config file */
    buffer = my_strdup(config_file);

    delete[] config_file;
    config_file = new char[MAX_FILENAME_LENGTH];

    /* get absolute path of current working directory */
    if (getcwd(config_file, MAX_FILENAME_LENGTH) == NULL) {
      perror("Error ");
      exit(ERROR);
    }

    /* append a forward slash */
    strncat(config_file, "/", 1);
    config_file[MAX_FILENAME_LENGTH - 1] = '\x0';

    /* append the config file to the path */
    strncat(config_file, buffer, MAX_FILENAME_LENGTH - strlen(config_file) - 1);
    config_file[MAX_FILENAME_LENGTH - 1] = '\x0';

    delete[] buffer;
  }

  /* we're just verifying the configuration... */
  if (verify_config == TRUE) {
    /* reset program variables */
    reset_variables();

    printf("Reading configuration data...\n");

    /* read in the configuration files (main config file, resource and object config files) */
    try {
      config.parse(config_file);
      std::cout << "Read main config file okay..." << std::endl;

      /* read object config files */
      if ((result = read_all_object_data(config_file)) == OK)
	printf("   Read object config files okay...\n");
      else
	printf("   Error processing object config files!\n");
    }
    catch(std::exception const &e) {
      std::cerr << e.what() << std::endl;
      std::cerr << "Error processing main config file!" << std::endl;
    }

    printf("\n");

    /* there was a problem reading the config files */
    if (result != OK) {
      /* if the config filename looks fishy, warn the user */
      if (!strstr(config_file, "nagios.cfg")) {
	printf("\n***> The name of the main configuration file looks suspicious...\n");
	printf("\n");
	printf("     Make sure you are specifying the name of the MAIN configuration file on\n");
	printf("     the command line and not the name of another configuration file.  The\n");
	printf("     main configuration file is typically '/usr/local/nagios/etc/nagios.cfg'\n");
      }

      printf("\n***> One or more problems was encountered while processing the config files...\n");
      printf("\n");
      printf("     Check your configuration file(s) to ensure that they contain valid\n");
      printf("     directives and data defintions.  If you are upgrading from a previous\n");
      printf("     version of Nagios, you should be aware that some variables/definitions\n");
      printf("     may have been removed or modified in this version.  Make sure to read\n");
      printf("     the HTML documentation regarding the config files, as well as the\n");
      printf("     'Whats New' section to find out what has changed.\n\n");
    }

    /* the config files were okay, so run the pre-flight check */
    else {

      printf("Running pre-flight check on configuration data...\n\n");

      /* run the pre-flight check to make sure things look okay... */
      result = pre_flight_check();

      if (result == OK)
	printf("\nThings look okay - No serious problems were detected during the pre-flight check\n");
      else {
	printf("\n***> One or more problems was encountered while running the pre-flight check...\n");
	printf("\n");
	printf("     Check your configuration file(s) to ensure that they contain valid\n");
	printf("     directives and data defintions.  If you are upgrading from a previous\n");
	printf("     version of Nagios, you should be aware that some variables/definitions\n");
	printf("     may have been removed or modified in this version.  Make sure to read\n");
	printf("     the HTML documentation regarding the config files, as well as the\n");
	printf("     'Whats New' section to find out what has changed.\n\n");
      }
    }

    /* clean up after ourselves */
    cleanup();

    /* free config_file */
    delete[] config_file;

    /* exit */
    exit(result);
  }

  /* we're just testing scheduling... */
  else if (test_scheduling == TRUE) {

    /* reset program variables */
    reset_variables();

    /* read in the configuration files (main config file and all host config files) */
    try {
      config.parse(config_file);

      /* read object config files */
      result = read_all_object_data(config_file);
    }
    catch(std::exception const &e) {
      std::cerr << e.what() << std::endl;
    }

    /* read initial service and host state information */
    if (result == OK) {
      initialize_retention_data(config_file);
      read_initial_state_information();
    }

    if (result != OK)
      printf("***> One or more problems was encountered while reading configuration data...\n");

    /* run the pre-flight check to make sure everything looks okay */
    if (result == OK) {
      if ((result = pre_flight_check()) != OK)
	printf("***> One or more problems was encountered while running the pre-flight check...\n");
    }

    if (result == OK) {

      /* initialize the event timing loop */
      init_timing_loop();

      /* display scheduling information */
      display_scheduling_info();

      if (precache_objects == TRUE) {
	printf("\n");
	printf("OBJECT PRECACHING\n");
	printf("-----------------\n");
	printf("Object config files were precached.\n");
      }
    }

#undef TEST_TIMEPERIODS
#ifdef TEST_TIMEPERIODS
    /* DO SOME TIMEPERIOD TESTING - ADDED 08/11/2009 */
    time_t now, pref_time, valid_time;
    timeperiod* tp;
    tp = find_timeperiod("247_exclusion");
    time(&now);
    pref_time = now;
    get_next_valid_time(pref_time, &valid_time, tp);
    printf("=====\n");
    printf("CURRENT:   %lu = %s", (unsigned long)now, ctime(&now));
    printf("PREFERRED: %lu = %s", (unsigned long)pref_time, ctime(&pref_time));
    printf("NEXT:      %lu = %s", (unsigned long)valid_time, ctime(&valid_time));
    printf("=====\n");
#endif

    /* clean up after ourselves */
    cleanup();

    /* exit */
    exit(result);
  }

  /* else start to monitor things... */
  else {
    /* keep monitoring things until we get a shutdown command */
    do {
      /* reset program variables */
      reset_variables();

      /* get PID */
      nagios_pid = (int)getpid();

      /* read in the configuration files (main and resource config files) */
      try {
	config.parse(config_file);
	result = OK;
      }
      catch(std::exception const &e) {
	std::cerr << e.what() << std::endl;
      }

      /* NOTE 11/06/07 EG moved to after we read config files, as user may have overridden timezone offset */
      /* get program (re)start time and save as macro */
      program_start = time(NULL);
      delete[] mac->x[MACRO_PROCESSSTARTTIME];
      try {
	mac->x[MACRO_PROCESSSTARTTIME] = obj2pchar<unsigned long>(program_start);
      }
      catch(...) {
	cleanup();
	throw;
      }

      /* open debug log */
      open_debug_log();

#ifdef USE_EVENT_BROKER
      /* initialize modules */
      neb_init_modules();
      neb_init_callback_list();

      try {
	modules::loader& loader = modules::loader::instance();
	loader.set_directory(config.get_broker_module_directory());
	loader.load();
      }
      catch (std::exception const& e) {
	logit(NSLOG_INFO_MESSAGE, false, "Event broker module initialize failed.\n");
	result = ERROR;
      }
#endif

      /* this must be logged after we read config data, as user may have changed location of main log file */
      logit(NSLOG_PROCESS_INFO, TRUE,
	    "Nagios %s starting... (PID=%d)\n", PROGRAM_VERSION, (int)getpid());

      /* log the local time - may be different than clock time due to timezone offset */
      now = time(NULL);
      tm = localtime_r(&now, &tm_s);
      strftime(datestring, sizeof(datestring), "%a %b %d %H:%M:%S %Z %Y", tm);
      logit(NSLOG_PROCESS_INFO, TRUE, "Local time is %s", datestring);

      /* write log version/info */
      write_log_file_info(NULL);

#ifdef USE_EVENT_BROKER
      /* load modules */
      neb_load_all_modules();

      /* send program data to broker */
      broker_program_state(NEBTYPE_PROCESS_PRELAUNCH,
			   NEBFLAG_NONE,
			   NEBATTR_NONE,
			   NULL);
#endif

      /* read in all object config data */
      if (result == OK)
	result = read_all_object_data(config_file);

      /* there was a problem reading the config files */
      if (result != OK)
	logit(NSLOG_PROCESS_INFO | NSLOG_RUNTIME_ERROR | NSLOG_CONFIG_ERROR, TRUE,
	      "Bailing out due to one or more errors encountered in the configuration files. Run Nagios from the command line with the -v option to verify your config before restarting. (PID=%d)",
	      (int)getpid());
      else {
	/* run the pre-flight check to make sure everything looks okay */
	if ((result = pre_flight_check()) != OK)
	  logit(NSLOG_PROCESS_INFO | NSLOG_RUNTIME_ERROR | NSLOG_VERIFICATION_ERROR, TRUE,
		"Bailing out due to errors encountered while running the pre-flight check.  Run Nagios from the command line with the -v option to verify your config before restarting. (PID=%d)\n",
		(int)getpid());
      }

      /* an error occurred that prevented us from (re)starting */
      if (result != OK) {
	/* if we were restarting, we need to cleanup from the previous run */
	if (sigrestart == TRUE) {
	  /* clean up the status data */
	  cleanup_status_data(config_file, TRUE);

	  /* shutdown the external command worker thread */
	  shutdown_command_file_worker_thread();

	  /* close and delete the external command file FIFO */
	  close_command_file();

	  /* cleanup embedded perl interpreter */
	  if (embedded_perl_initialized == TRUE)
	    deinit_embedded_perl();
	}

#ifdef USE_EVENT_BROKER
	/* send program data to broker */
	broker_program_state(NEBTYPE_PROCESS_SHUTDOWN,
			     NEBFLAG_PROCESS_INITIATED,
			     NEBATTR_SHUTDOWN_ABNORMAL,
			     NULL);
#endif
	cleanup();
	exit(ERROR);
      }

      /* initialize embedded Perl interpreter */
      /* NOTE 02/15/08 embedded Perl must be initialized if compiled in, regardless of whether or not its enabled in the config file */
      /* It compiled it, but not initialized, Nagios will segfault in readdir() calls, as libperl takes this function over */
      if (embedded_perl_initialized == FALSE) {
	/*				if(enable_embedded_perl==TRUE){*/
#ifdef EMBEDDEDPERL
	init_embedded_perl(env);
#else
	init_embedded_perl(NULL);
#endif
	embedded_perl_initialized = TRUE;
	/*					}*/
      }

      /* handle signals (interrupts) */
      setup_sighandler();

#ifdef USE_EVENT_BROKER
      /* send program data to broker */
      broker_program_state(NEBTYPE_PROCESS_START,
			   NEBFLAG_NONE,
			   NEBATTR_NONE,
			   NULL);
#endif

      /* open the command file (named pipe) for reading */
      result = open_command_file();
      if (result != OK) {

	logit(NSLOG_PROCESS_INFO | NSLOG_RUNTIME_ERROR, TRUE,
	      "Bailing out due to errors encountered while trying to initialize the external command file... (PID=%d)\n",
	      (int)getpid());

#ifdef USE_EVENT_BROKER
	/* send program data to broker */
	broker_program_state(NEBTYPE_PROCESS_SHUTDOWN,
			     NEBFLAG_PROCESS_INITIATED,
			     NEBATTR_SHUTDOWN_ABNORMAL,
			     NULL);
#endif
	cleanup();
	exit(ERROR);
      }

      /* initialize status data unless we're starting */
      if (sigrestart == FALSE)
	initialize_status_data(config_file);

      /* read initial service and host state information  */
      initialize_retention_data(config_file);
      read_initial_state_information();

      /* initialize comment data */
      initialize_comment_data(config_file);

      /* initialize scheduled downtime data */
      initialize_downtime_data(config_file);

      /* initialize performance data */
      initialize_performance_data(config_file);

      /* initialize the event timing loop */
      init_timing_loop();

      /* initialize check statistics */
      init_check_stats();

      /* update all status data (with retained information) */
      update_all_status_data();

      /* log initial host and service state */
      log_host_states(INITIAL_STATES, NULL);
      log_service_states(INITIAL_STATES, NULL);

      /* reset the restart flag */
      sigrestart = FALSE;

#ifdef USE_EVENT_BROKER
      /* send program data to broker */
      broker_program_state(NEBTYPE_PROCESS_EVENTLOOPSTART,
			   NEBFLAG_NONE,
			   NEBATTR_NONE,
			   NULL);
#endif

      /* get event start time and save as macro */
      event_start = time(NULL);
      delete[] mac->x[MACRO_EVENTSTARTTIME];
      try {
	mac->x[MACRO_EVENTSTARTTIME] = obj2pchar<unsigned long>(event_start);
      }
      catch(...) {
#ifdef USE_EVENT_BROKER
	/* send program data to broker */
	broker_program_state(NEBTYPE_PROCESS_SHUTDOWN,
			     NEBFLAG_PROCESS_INITIATED,
			     NEBATTR_SHUTDOWN_ABNORMAL,
			     NULL);
#endif
	cleanup();
      }

      /***** start monitoring all services *****/
      /* (doesn't return until a restart or shutdown signal is encountered) */
      event_execution_loop();

      /* 03/01/2007 EG Moved from sighandler() to prevent FUTEX locking problems under NPTL */
      /* did we catch a signal? */
      if (caught_signal == TRUE) {
	if (sig_id == SIGHUP) {
	  try {
	    buffer = my_strdup("Caught SIGHUP, restarting...\n");
	  }
	  catch(...) {
#ifdef USE_EVENT_BROKER
	    /* send program data to broker */
	    broker_program_state(NEBTYPE_PROCESS_SHUTDOWN,
				 NEBFLAG_PROCESS_INITIATED,
				 NEBATTR_SHUTDOWN_ABNORMAL,
				 NULL);
#endif
	    cleanup();
	  }
	}
	else {
	  try {
	    std::ostringstream oss;
	    oss << "Caught SIG" << sigs[sig_id] << ", shutting down...\n";
	    buffer = my_strdup(oss.str().c_str());
	  }
	  catch(...) {
#ifdef USE_EVENT_BROKER
	    /* send program data to broker */
	    broker_program_state(NEBTYPE_PROCESS_SHUTDOWN,
				 NEBFLAG_PROCESS_INITIATED,
				 NEBATTR_SHUTDOWN_ABNORMAL,
				 NULL);
#endif
	    cleanup();
	  }
	}

	write_to_all_logs(buffer, NSLOG_PROCESS_INFO);
	delete[] buffer;
      }

#ifdef USE_EVENT_BROKER
      /* send program data to broker */
      broker_program_state(NEBTYPE_PROCESS_EVENTLOOPEND,
			   NEBFLAG_NONE,
			   NEBATTR_NONE,
			   NULL);
      if (sigshutdown == TRUE)
	broker_program_state(NEBTYPE_PROCESS_SHUTDOWN,
			     NEBFLAG_USER_INITIATED,
			     NEBATTR_SHUTDOWN_NORMAL,
			     NULL);
      else if (sigrestart == TRUE)
	broker_program_state(NEBTYPE_PROCESS_RESTART,
			     NEBFLAG_USER_INITIATED,
			     NEBATTR_RESTART_NORMAL,
			     NULL);
#endif

      /* save service and host state information */
      save_state_information(FALSE);
      cleanup_retention_data(config_file);

      /* clean up performance data */
      cleanup_performance_data(config_file);

      /* clean up the scheduled downtime data */
      cleanup_downtime_data(config_file);

      /* clean up the comment data */
      cleanup_comment_data(config_file);

      /* clean up the status data unless we're restarting */
      if (sigrestart == FALSE)
	cleanup_status_data(config_file, TRUE);

      /* close and delete the external command file FIFO unless we're restarting */
      if (sigrestart == FALSE) {
	shutdown_command_file_worker_thread();
	close_command_file();
      }

      /* cleanup embedded perl interpreter */
      if (sigrestart == FALSE)
	deinit_embedded_perl();

      /* shutdown stuff... */
      if (sigshutdown == TRUE) {
	/* log a shutdown message */
	logit(NSLOG_PROCESS_INFO, TRUE,
	      "Successfully shutdown... (PID=%d)\n", (int)getpid());
      }

      /* clean up after ourselves */
      cleanup();

      /* close debug log */
      close_debug_log();

    } while (sigrestart == TRUE && sigshutdown == FALSE);

    /* free misc memory */
    delete[] config_file;
  }

  return (OK);
}
