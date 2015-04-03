/*
** Copyright 1999-2011      Ethan Galstad
** Copyright 2011-2013,2015 Merethis
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

  if (sequence_head[0] == '\x0')
    return (NULL);

  token_position = strchr(my_strtok_buffer, tokens[0]);

  if (token_position == NULL) {
    my_strtok_buffer = strchr(my_strtok_buffer, '\x0');
    return (sequence_head);
  }

  token_position[0] = '\x0';
  my_strtok_buffer = token_position + 1;
  return (sequence_head);
}

/* fixes compiler problems under Solaris, since strsep() isn't included */
/* this code is taken from the glibc source */
char* my_strsep(char** stringp, char const* delim) {
  char* begin;
  char* end;

  if ((begin = *stringp) == NULL)
    return (NULL);

  /* A frequent case is when the delimiter string contains only one
   * character.  Here we don't need to call the expensive `strpbrk'
   * function and instead work using `strchr'.  */
  if (delim[0] == '\0' || delim[1] == '\0') {
    char ch = delim[0];

    if (ch == '\0' || begin[0] == '\0')
      end = NULL;
    else {
      if (*begin == ch)
        end = begin;
      else
        end = strchr(begin + 1, ch);
    }
  }
  else {
    /* find the end of the token.  */
    end = strpbrk(begin, delim);
  }

  if (end) {
    /* terminate the token and set *STRINGP past NUL character.  */
    *end++ = '\0';
    *stringp = end;
  }
  else
    /* no more delimiters; this is the last token.  */
    *stringp = NULL;
  return (begin);
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
    return (NULL);
  }
  if (str == NULL)
    return (new char[size]);
  char* new_str = new char[size];
  strcpy(new_str, str);
  delete[] str;
  return (new_str);
}
