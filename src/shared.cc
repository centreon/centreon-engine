/*
** Copyright 1999-2011 Ethan Galstad
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

#include "com/centreon/unique_array_ptr.hh"
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/engine/utils.hh"

using namespace com::centreon::engine;

#ifdef HAVE_TZNAME
#  ifdef CYGWIN
extern char* _tzname[2] __declspec(dllimport);
#  else
extern char* tzname[2];
#  endif // Cygwin
#endif // HAVE_TZNAME

/*
 * This file holds random utility functions shared by cgi's and
 * core.
 */

/* fix the problem with strtok() skipping empty options between tokens */
char* my_strtok(char const* buffer, char const* tokens) {
  char* token_position = NULL;
  char* sequence_head = NULL;
  static char* my_strtok_buffer = NULL;
  static com::centreon::unique_array_ptr<char>
    original_my_strtok_buffer;

  if (buffer != NULL) {
    original_my_strtok_buffer.reset(string::dup(buffer));
    my_strtok_buffer = original_my_strtok_buffer.get();
  }

  sequence_head = my_strtok_buffer;

  if (!sequence_head || sequence_head[0] == '\x0')
    return nullptr;

  token_position = strchr(my_strtok_buffer, tokens[0]);

  if (token_position == NULL) {
    my_strtok_buffer = strchr(my_strtok_buffer, '\x0');
    return sequence_head;
  }

  token_position[0] = '\x0';
  my_strtok_buffer = token_position + 1;
  return sequence_head;
}

/* strip newline, carriage return, and tab characters from beginning and end of a string */
void strip(char* buffer) {
  int x, z;
  int len;

  if (buffer == NULL || buffer[0] == '\x0')
    return;

  /* strip end of string */
  len = (int)strlen(buffer);
  for (x = len - 1; x >= 0; x--) {
    switch (buffer[x]) {
    case ' ':
    case '\n':
    case '\r':
    case '\t':
      buffer[x] = '\x0';
    continue;
    }
    break;
  }

  /* if we stripped all of it, just return */
  if (!x)
    return;

  /* save last position for later... */
  z = x;

  /* strip beginning of string (by shifting) */
  /* NOTE: this is very expensive to do, so avoid it whenever possible */
  for (x = 0;; x++) {
    switch (buffer[x]) {
    case ' ':
    case '\n':
    case '\r':
    case '\t':
      continue;
    }
    break;
  }

  if (x > 0 && z > 0) {
    /* new length of the string after we stripped the end */
    len = z + 1;

    /* shift chars towards beginning of string to remove leading whitespace */
    for (z = x; z < len; z++)
      buffer[z - x] = buffer[z];
    buffer[len - x] = '\x0';
  }
  return;
}

/**************************************************
 *************** HASH FUNCTIONS *******************
 **************************************************/
/* dual hash data comparison */
int compare_hashdata(
      char const* val1a,
      char const* val1b,
      char const* val2a,
      char const* val2b) {
  int result = 0;

  /* NOTE: If hash calculation changes, update the compare_strings() function! */

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

/*
 * given a date/time in time_t format, produce a corresponding
 * date/time string, including timezone
 */
void get_datetime_string(
       time_t const* raw_time,
       char* buffer,
       int buffer_length,
       int type) {
  static char const* weekdays[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
  static char const* months[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sept", "Oct", "Nov", "Dec" };

  time_t t(raw_time ? *raw_time : time(NULL));
  tm tm_s;
  if (type == HTTP_DATE_TIME)
    gmtime_r(&t, &tm_s);
  else
    localtime_r(&t, &tm_s);

  int hour(tm_s.tm_hour);
  int minute(tm_s.tm_min);
  int second(tm_s.tm_sec);
  int month(tm_s.tm_mon + 1);
  int day(tm_s.tm_mday);
  int year(tm_s.tm_year + 1900);

#ifdef HAVE_TM_ZONE
  char const* tzone(tm_s.tm_zone);
#elif HAVE_TZNAME
  char const* tzone(tm_s.tm_isdst ? tzname[1] : tzname[0]);
#endif /* HAVE_TM_ZONE || HAVE_TZNAME */

  /* ctime() style date/time */
  if (type == LONG_DATE_TIME)
    snprintf(
      buffer,
      buffer_length,
      "%s %s %d %02d:%02d:%02d %s %d",
      weekdays[tm_s.tm_wday],
      months[tm_s.tm_mon],
      day,
      hour,
      minute,
      second,
      tzone,
      year);

  /* short date/time */
  else if (type == SHORT_DATE_TIME) {
    if (config->date_format() == DATE_FORMAT_EURO)
      snprintf(
        buffer,
        buffer_length,
        "%02d-%02d-%04d %02d:%02d:%02d",
        day,
        month,
        year,
        hour,
        minute,
        second);
    else if (config->date_format() == DATE_FORMAT_ISO8601
             || config->date_format() == DATE_FORMAT_STRICT_ISO8601)
      snprintf(
        buffer,
        buffer_length,
        "%04d-%02d-%02d%c%02d:%02d:%02d",
        year,
        month,
        day,
        (config->date_format() == DATE_FORMAT_STRICT_ISO8601) ? 'T' : ' ',
        hour,
        minute,
        second);
    else
      snprintf(
        buffer,
        buffer_length,
        "%02d-%02d-%04d %02d:%02d:%02d",
        month,
        day,
        year,
        hour,
        minute,
        second);
  }

  /* short date */
  else if (type == SHORT_DATE) {
    if (config->date_format() == DATE_FORMAT_EURO)
      snprintf(
        buffer,
        buffer_length,
        "%02d-%02d-%04d",
        day,
        month,
        year);
    else if (config->date_format() == DATE_FORMAT_ISO8601
             || config->date_format() == DATE_FORMAT_STRICT_ISO8601)
      snprintf(
        buffer,
        buffer_length,
        "%04d-%02d-%02d",
        year,
        month,
        day);
    else
      snprintf(
        buffer,
        buffer_length,
        "%02d-%02d-%04d",
        month,
        day,
        year);
  }

  /* expiration date/time for HTTP headers */
  else if (type == HTTP_DATE_TIME)
    snprintf(
      buffer,
      buffer_length,
      "%s, %02d %s %d %02d:%02d:%02d GMT",
      weekdays[tm_s.tm_wday],
      day,
      months[tm_s.tm_mon],
      year,
      hour,
      minute,
      second);

  /* short time */
  else
    snprintf(
      buffer,
      buffer_length,
      "%02d:%02d:%02d",
      hour,
      minute,
      second);

  buffer[buffer_length - 1] = '\x0';
  return;
}

/* get days, hours, minutes, and seconds from a raw time_t format or total seconds */
void get_time_breakdown(
       unsigned long raw_time,
       int* days,
       int* hours,
       int* minutes,
       int* seconds) {
  unsigned long temp_time;
  int temp_days;
  int temp_hours;
  int temp_minutes;
  int temp_seconds;

  temp_time = raw_time;

  temp_days = temp_time / 86400;
  temp_time -= (temp_days * 86400);
  temp_hours = temp_time / 3600;
  temp_time -= (temp_hours * 3600);
  temp_minutes = temp_time / 60;
  temp_time -= (temp_minutes * 60);
  temp_seconds = (int)temp_time;

  *days = temp_days;
  *hours = temp_hours;
  *minutes = temp_minutes;
  *seconds = temp_seconds;
  return;
}

char* resize_string(char* str, size_t size) {
  if (size == 0) {
    delete[] str;
    return nullptr;
  }
  if (str == nullptr)
    return new char[size];
  char* new_str = new char[size];
  strncpy(new_str, str, size - 2);
  new_str[size - 1] = 0;
  delete[] str;
  return new_str;
}
