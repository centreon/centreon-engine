/*
** Copyright 2007-2008 Ethan Galstad
** Copyright 2007,2010 Andreas Ericsson
** Copyright 2010      Max Schubert
** Copyright 2011      Merethis
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

#ifndef CCE_ENGINE_HH
# define CCE_ENGINE_HH

# include <sys/time.h>
# include <pthread.h>

# ifndef __GNUC__
#  define __attribute__(x) /* nothing */
# endif

# ifdef __cplusplus
extern "C" {
# endif

  /************* MISC LENGTH/SIZE DEFINITIONS ***********/

  /**
   *  @note
   *
   *  Plugin length is artificially capped at 8k to prevent runaway
   *  plugins from returning MBs/GBs of data back to Centreon Engine.
   *  If you increase the 8k cap by modifying this value, make sure you
   *  also increase the value of MAX_EXTERNAL_COMMAND_LENGTH in
   *  common.hh to allow for passive checks results received through the
   *  external command file.
   */
# define MAX_PLUGIN_OUTPUT_LENGTH       8192    /* max length of plugin output (including perf data) */

  /******************* DEFAULT VALUES *******************/

# define DEFAULT_ORPHAN_CHECK_INTERVAL  60      /* seconds between checks for orphaned hosts and services */

  /******************** HOST STATUS *********************/

# define HOST_UP			0
# define HOST_DOWN			1
# define HOST_UNREACHABLE		2

  /****************** SERVICE STATES ********************/

# define STATE_OK			0
# define STATE_WARNING			1
# define STATE_CRITICAL			2
# define STATE_UNKNOWN			3       /* changed from -1 on 02/24/2001 */

  /***************** STATE CHANGE TYPES *****************/

# define HOST_STATECHANGE                0
# define SERVICE_STATECHANGE             1

  /* SCHED_INFO structure */
typedef struct  sched_info_struct {
  int           total_services;
  int           total_scheduled_services;
  int           total_hosts;
  int           total_scheduled_hosts;
  double        average_services_per_host;
  double        average_scheduled_services_per_host;
  unsigned long service_check_interval_total;
  unsigned long host_check_interval_total;
  double        average_service_execution_time;
  double        average_service_check_interval;
  double        average_host_check_interval;
  double        average_service_inter_check_delay;
  double        average_host_inter_check_delay;
  double        service_inter_check_delay;
  double        host_inter_check_delay;
  int           service_interleave_factor;
  int           max_service_check_spread;
  int           max_host_check_spread;
  time_t        first_service_check;
  time_t        last_service_check;
  time_t        first_host_check;
  time_t        last_host_check;
}               sched_info;

/* CIRCULAR_BUFFER structure - used by worker threads */
typedef struct    circular_buffer_struct {
  void**          buffer;
  int             tail;
  int             head;
  int             items;
  int		  high;		/* highest number of items that has ever been stored in buffer */
  unsigned long   overflow;
  pthread_mutex_t buffer_lock;
}                 circular_buffer;

# define CHECK_STATS_BUCKETS                  15

/* used for tracking host and service check statistics */
typedef struct check_stats_struct {
  int          current_bucket;
  int          bucket[CHECK_STATS_BUCKETS];
  int          overflow_bucket;
  int          minute_stats[3];
  time_t       last_update;
}              check_stats;

  /******************* THREAD STUFF ********************/

  /* worker threads */
# define TOTAL_WORKER_THREADS             1

# define COMMAND_WORKER_THREAD		  0

  /******************** FUNCTIONS **********************/

# ifdef __cplusplus
}
# endif
#endif // !CCE_ENGINE_HH
