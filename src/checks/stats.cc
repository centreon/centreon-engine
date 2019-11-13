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

#include "com/centreon/engine/checks/stats.hh"
#include <cmath>
#include <ctime>
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/globals.hh"

extern "C" {
/**
 *  Generate 1/5/15 minute stats for a given type of check.
 *
 *  @return OK on success.
 */
int generate_check_stats() {
  // Do some sanity checks on the age of the stats data before we
  // start... and get the new current bucket number.
  time_t current_time(time(NULL));
  int minutes(0);
  int new_current_bucket(0);
  minutes = ((unsigned long)current_time - (unsigned long)program_start) / 60;
  new_current_bucket = minutes % CHECK_STATS_BUCKETS;
  for (unsigned int check_type(0); check_type < MAX_CHECK_STATS_TYPES;
       ++check_type) {
    // Its been more than 15 minutes since
    // stats were updated, so clear the stats.
    if ((((unsigned long)current_time -
          (unsigned long)check_statistics[check_type].last_update) /
         60) > CHECK_STATS_BUCKETS) {
      for (unsigned int x(0); x < CHECK_STATS_BUCKETS; ++x)
        check_statistics[check_type].bucket[x] = 0;
      check_statistics[check_type].overflow_bucket = 0;
    }
    // Different current bucket number than last time.
    else if (new_current_bucket !=
             check_statistics[check_type].current_bucket) {
      // Clear stats in buckets between last current bucket and new
      // current bucket - stats haven't been updated in a while.
      for (unsigned int x(check_statistics[check_type].current_bucket);
           x < CHECK_STATS_BUCKETS * 2; ++x) {
        int this_bucket((x + CHECK_STATS_BUCKETS + 1) % CHECK_STATS_BUCKETS);
        if (this_bucket == new_current_bucket)
          break;

        // Clear old bucket value.
        check_statistics[check_type].bucket[this_bucket] = 0;
      }

      // Update the current bucket number,
      // push old value to overflow bucket.
      check_statistics[check_type].overflow_bucket =
          check_statistics[check_type].bucket[new_current_bucket];
      check_statistics[check_type].current_bucket = new_current_bucket;
      check_statistics[check_type].bucket[new_current_bucket] = 0;
    }
    // Update last check time.
    check_statistics[check_type].last_update = current_time;
  }

  // Determine weights to use for this/last buckets.
  int seconds(0);
  seconds = ((unsigned long)current_time - (unsigned long)program_start) % 60;
  float this_bucket_weight(seconds / 60.0);
  float last_bucket_weight((60 - seconds) / 60.0);

  // Update statistics for all check types.
  for (unsigned int check_type(0); check_type < MAX_CHECK_STATS_TYPES;
       ++check_type) {
    // Clear the old statistics.
    for (unsigned int x(0); x < 3; ++x)
      check_statistics[check_type].minute_stats[x] = 0;

    // Loop through each bucket.
    for (unsigned int x(0); x < CHECK_STATS_BUCKETS; ++x) {
      // Which buckets should we use for this/last bucket?
      int this_bucket((check_statistics[check_type].current_bucket +
                       CHECK_STATS_BUCKETS - x) %
                      CHECK_STATS_BUCKETS);
      int last_bucket((this_bucket + CHECK_STATS_BUCKETS - 1) %
                      CHECK_STATS_BUCKETS);

      // Raw/unweighted value for this bucket.
      int this_bucket_value(check_statistics[check_type].bucket[this_bucket]);

      // Raw/unweighted value for last bucket - use overflow
      // bucket if last bucket is current bucket.
      int last_bucket_value;
      if (last_bucket == check_statistics[check_type].current_bucket)
        last_bucket_value = check_statistics[check_type].overflow_bucket;
      else
        last_bucket_value = check_statistics[check_type].bucket[last_bucket];

      // Determine value by weighting this/last buckets... If this is
      // the current bucket, use its full value + weighted % of last
      // bucket.
      int bucket_value;
      if (!x)
        bucket_value = (int)(this_bucket_value +
                             floor(last_bucket_value * last_bucket_weight));
      // Otherwise use weighted % of this and last bucket.
      else
        bucket_value = (int)(ceil(this_bucket_value * this_bucket_weight) +
                             floor(last_bucket_value * last_bucket_weight));

      // 1 minute stats.
      if (!x)
        check_statistics[check_type].minute_stats[0] = bucket_value;

      // 5 minute stats.
      if (x < 5)
        check_statistics[check_type].minute_stats[1] += bucket_value;

      // 15 minute stats.
      if (x < 15)
        check_statistics[check_type].minute_stats[2] += bucket_value;

      // Record last update time.
      check_statistics[check_type].last_update = current_time;
    }
  }

  return (OK);
}

/**
 *  Initialize check statistics data structures.
 *
 *  @return OK on success.
 */
int init_check_stats() {
  for (unsigned int x(0); x < MAX_CHECK_STATS_TYPES; ++x) {
    check_statistics[x].current_bucket = 0;
    for (unsigned int y(0); y < CHECK_STATS_BUCKETS; ++y)
      check_statistics[x].bucket[y] = 0;
    check_statistics[x].overflow_bucket = 0;
    for (unsigned int y(0); y < 3; ++y)
      check_statistics[x].minute_stats[y] = 0;
    check_statistics[x].last_update = (time_t)0L;
  }
  return (OK);
}

/**
 *  Records stats for a given type of check.
 *
 *  @param[in] check_type Check type.
 *  @param[in] check_time Time at which the check occurred.
 *
 *  @return OK on success.
 */
int update_check_stats(int check_type, time_t check_time) {
  // Argument checking.
  if ((check_type < 0) || (check_type >= MAX_CHECK_STATS_TYPES))
    return (ERROR);

  // Get current and check time.
  time_t current_time(time(NULL));
  if (!check_time)
    check_time = current_time;

  // Do some sanity checks on the age of the stats data before we
  // start... and get the new current bucket number.
  unsigned long minutes =
      ((unsigned long)check_time - (unsigned long)program_start) / 60;
  int new_current_bucket(minutes % CHECK_STATS_BUCKETS);

  // Its been more than 15 minutes since
  // stats were updated, so clear the stats.
  if ((((unsigned long)current_time -
        (unsigned long)check_statistics[check_type].last_update) /
       60) > CHECK_STATS_BUCKETS) {
    for (unsigned int x(0); x < CHECK_STATS_BUCKETS; ++x)
      check_statistics[check_type].bucket[x] = 0;
    check_statistics[check_type].overflow_bucket = 0;
  }
  // Different current bucket number than last time.
  else if (new_current_bucket != check_statistics[check_type].current_bucket) {
    // Clear stats in buckets between last current bucket and new
    // current bucket - stats haven't been updated in a while.
    for (unsigned int x(check_statistics[check_type].current_bucket);
         x < (CHECK_STATS_BUCKETS * 2); ++x) {
      int this_bucket = (x + CHECK_STATS_BUCKETS + 1) % CHECK_STATS_BUCKETS;
      if (this_bucket == new_current_bucket)
        break;

      // Clear old bucket value.
      check_statistics[check_type].bucket[this_bucket] = 0;
    }

    // Update the current bucket number,
    // push old value to overflow bucket.
    check_statistics[check_type].overflow_bucket =
        check_statistics[check_type].bucket[new_current_bucket];
    check_statistics[check_type].current_bucket = new_current_bucket;
    check_statistics[check_type].bucket[new_current_bucket] = 0;
  }

  // Increment the value of the current bucket.
  check_statistics[check_type].bucket[new_current_bucket]++;

  // Record last update time.
  check_statistics[check_type].last_update = current_time;

  return (OK);
}
}
