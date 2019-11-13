/*
** Copyright 1999-2009 Ethan Galstad
** Copyright 2007,2010 Andreas Ericsson
** Copyright 2010      Max Schubert
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

#ifndef CCE_CHECKS_STATS_HH
#define CCE_CHECKS_STATS_HH

#include <time.h>

#define ACTIVE_SCHEDULED_SERVICE_CHECK_STATS 0
#define ACTIVE_ONDEMAND_SERVICE_CHECK_STATS 1
#define PASSIVE_SERVICE_CHECK_STATS 2
#define ACTIVE_SCHEDULED_HOST_CHECK_STATS 3
#define ACTIVE_ONDEMAND_HOST_CHECK_STATS 4
#define PASSIVE_HOST_CHECK_STATS 5
#define ACTIVE_CACHED_HOST_CHECK_STATS 6
#define ACTIVE_CACHED_SERVICE_CHECK_STATS 7
#define EXTERNAL_COMMAND_STATS 8
#define PARALLEL_HOST_CHECK_STATS 9
#define SERIAL_HOST_CHECK_STATS 10
#define MAX_CHECK_STATS_TYPES 11
#define CHECK_STATS_BUCKETS 15

/**
 *  @struct check_stats checks_stats.hh "com/centreon/engine/checks/stats.hh"
 *  @brief Used for tracking host and service check statistics.
 *
 *  Used for tracking host and service check statistics.
 */
typedef struct check_stats_struct {
  int current_bucket;
  int bucket[CHECK_STATS_BUCKETS];
  int overflow_bucket;
  int minute_stats[3];
  time_t last_update;
} check_stats;

#ifdef __cplusplus
extern "C" {
#endif  // C++

int generate_check_stats();
int init_check_stats();
int update_check_stats(int check_type, time_t check_time);

#ifdef __cplusplus
}
#endif  // C++

#endif  // !CCE_CHECKS_STATS_HH
