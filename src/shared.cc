/*
** Copyright 1999-2011 Ethan Galstad
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.hh"
#include "globals.hh"
#include "utils.hh"
#include "shared.hh"

#ifdef HAVE_TZNAME
# ifdef CYGWIN
extern char*                  _tzname[2] __declspec(dllimport);
# else
extern char*                  tzname[2];
# endif
#endif

/*
 * This file holds random utility functions shared by cgi's and
 * core.
 */

char* my_strdup(char const* str) {
  char* new_str = new char[strlen(str) + 1];
  return (strcpy(new_str, str));
}

/* fix the problem with strtok() skipping empty options between tokens */
char* my_strtok(char const* buffer, char const* tokens) {
  char* token_position = NULL;
  char* sequence_head = NULL;
  static char* my_strtok_buffer = NULL;
  static char* original_my_strtok_buffer = NULL;

  if (buffer != NULL) {
    delete[] original_my_strtok_buffer;
    my_strtok_buffer = my_strdup(buffer);
    original_my_strtok_buffer = my_strtok_buffer;
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

/* open a file read-only via mmap() */
mmapfile* mmap_fopen(char* filename) {
  int fd = 0;
  void* mmap_buf = NULL;
  struct stat statbuf;
  int mode = O_RDONLY;
  unsigned long file_size = 0L;

  if (filename == NULL)
    return (NULL);

  /* open the file */
  if ((fd = open(filename, mode)) == -1) {
    return (NULL);
  }

  /* get file info */
  if ((fstat(fd, &statbuf)) == -1) {
    close(fd);
    return (NULL);
  }

  /* get file size */
  file_size = (unsigned long)statbuf.st_size;

  /* only mmap() if we have a file greater than 0 bytes */
  if (file_size > 0) {
    /* mmap() the file - allocate one extra byte for processing zero-byte files */
    if ((mmap_buf = (void*)mmap(0, file_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
      close(fd);
      return (NULL);
    }
  }
  else
    mmap_buf = NULL;

  /* allocate memory */
  mmapfile* new_mmapfile = new mmapfile();

  /* populate struct info for later use */
  new_mmapfile->path = my_strdup(filename);
  new_mmapfile->fd = fd;
  new_mmapfile->file_size = (unsigned long)file_size;
  new_mmapfile->current_position = 0L;
  new_mmapfile->current_line = 0L;
  new_mmapfile->mmap_buf = mmap_buf;
  return (new_mmapfile);
}

/* close a file originally opened via mmap() */
int mmap_fclose(mmapfile* temp_mmapfile) {
  if (temp_mmapfile == NULL)
    return (ERROR);

  /* un-mmap() the file */
  if (temp_mmapfile->file_size > 0L)
    munmap(temp_mmapfile->mmap_buf, temp_mmapfile->file_size);

  /* close the file */
  close(temp_mmapfile->fd);

  /* free memory */
  delete[] temp_mmapfile->path;
  delete temp_mmapfile;
  return (OK);
}

/* gets one line of input from an mmap()'ed file */
char* mmap_fgets(mmapfile* temp_mmapfile) {
  char* buf = NULL;
  unsigned long x = 0L;
  int len = 0;

  if (temp_mmapfile == NULL)
    return (NULL);

  /* size of file is 0 bytes */
  if (temp_mmapfile->file_size == 0L)
    return (NULL);

  /* we've reached the end of the file */
  if (temp_mmapfile->current_position >= temp_mmapfile->file_size)
    return (NULL);

  /* find the end of the string (or buffer) */
  for (x = temp_mmapfile->current_position;
       x < temp_mmapfile->file_size;
       x++) {
    if (*((char*)(temp_mmapfile->mmap_buf) + x) == '\n') {
      x++;
      break;
    }
  }

  /* calculate length of line we just read */
  len = (int)(x - temp_mmapfile->current_position);

  /* allocate memory for the new line */
  buf = new char[len + 1];

  /* copy string to newly allocated memory and terminate the string */
  memcpy(buf, ((char*)(temp_mmapfile->mmap_buf) + temp_mmapfile->current_position), len);
  buf[len] = '\x0';

  /* update the current position */
  temp_mmapfile->current_position = x;

  /* increment the current line */
  temp_mmapfile->current_line++;
  return (buf);
}

/* gets one line of input from an mmap()'ed file (may be contained on more than one line in the source file) */
char* mmap_fgets_multiline(mmapfile* temp_mmapfile) {
  char* buf = NULL;
  char* tempbuf = NULL;
  char* stripped = NULL;
  int len = 0;
  int len2 = 0;
  int end = 0;

  if (temp_mmapfile == NULL)
    return (NULL);

  while (1) {

    delete[] tempbuf;

    if ((tempbuf = mmap_fgets(temp_mmapfile)) == NULL)
      break;

    if (buf == NULL) {
      len = strlen(tempbuf);
      buf = new char[len + 1];
      memcpy(buf, tempbuf, len);
      buf[len] = '\x0';
    }
    else {
      /* strip leading white space from continuation lines */
      stripped = tempbuf;
      while (*stripped == ' ' || *stripped == '\t')
        stripped++;
      len = strlen(stripped);
      len2 = strlen(buf);
      buf = resize_string(buf, len + len2 + 1);
      strcat(buf, stripped);
      len += len2;
      buf[len] = '\x0';
    }

    if (len == 0)
      break;

    /* handle Windows/DOS CR/LF */
    if (len >= 2 && buf[len - 2] == '\r')
      end = len - 3;
    /* normal Unix LF */
    else if (len >= 1 && buf[len - 1] == '\n')
      end = len - 2;
    else
      end = len - 1;

    /* two backslashes found. unescape first backslash first and break */
    if (end >= 1 && buf[end - 1] == '\\' && buf[end] == '\\') {
      buf[end] = '\n';
      buf[end + 1] = '\x0';
      break;
    }

    /* one backslash found. continue reading the next line */
    else if (end > 0 && buf[end] == '\\')
      buf[end] = '\x0';

    /* no continuation marker was found, so break */
    else
      break;
  }

  delete[] tempbuf;
  return (buf);
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
}

/**************************************************
 *************** HASH FUNCTIONS *******************
 **************************************************/

/* dual hash function */
int hashfunc(char const* name1, char const* name2, int hashslots) {
  unsigned int i, result;

  result = 0;

  if (name1)
    for (i = 0; i < strlen(name1); i++)
      result += name1[i];

  if (name2)
    for (i = 0; i < strlen(name2); i++)
      result += name2[i];

  return (result % hashslots);
}

/* dual hash data comparison */
int compare_hashdata(char const* val1a,
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
void get_datetime_string(time_t* raw_time,
			 char* buffer,
                         int buffer_length,
			 int type) {
  time_t t;
  struct tm* tm_ptr, tm_s;
  int hour;
  int minute;
  int second;
  int month;
  int day;
  int year;
  char const* weekdays[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
  char const* months[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sept", "Oct", "Nov", "Dec" };
  char const* tzone = "";

  if (raw_time == NULL)
    time(&t);
  else
    t = *raw_time;

  if (type == HTTP_DATE_TIME)
    tm_ptr = gmtime_r(&t, &tm_s);
  else
    tm_ptr = localtime_r(&t, &tm_s);

  hour = tm_ptr->tm_hour;
  minute = tm_ptr->tm_min;
  second = tm_ptr->tm_sec;
  month = tm_ptr->tm_mon + 1;
  day = tm_ptr->tm_mday;
  year = tm_ptr->tm_year + 1900;

#ifdef HAVE_TM_ZONE
  tzone = (char*)(tm_ptr->tm_zone);
#elif HAVE_TZNAME
  tzone = tm_ptr->tm_isdst ? tzname[1] : tzname[0];
#endif /* HAVE_TM_ZONE || HAVE_TZNAME */

  /* ctime() style date/time */
  if (type == LONG_DATE_TIME)
    snprintf(buffer, buffer_length, "%s %s %d %02d:%02d:%02d %s %d",
             weekdays[tm_ptr->tm_wday], months[tm_ptr->tm_mon], day,
             hour, minute, second, tzone, year);

  /* short date/time */
  else if (type == SHORT_DATE_TIME) {
    if (config.get_date_format() == DATE_FORMAT_EURO)
      snprintf(buffer, buffer_length,
               "%02d-%02d-%04d %02d:%02d:%02d", day, month,
               year, hour, minute, second);
    else if (config.get_date_format() == DATE_FORMAT_ISO8601
             || config.get_date_format() == DATE_FORMAT_STRICT_ISO8601)
      snprintf(buffer, buffer_length,
               "%04d-%02d-%02d%c%02d:%02d:%02d", year, month,
               day, (config.get_date_format() == DATE_FORMAT_STRICT_ISO8601) ? 'T' : ' ',
	       hour, minute, second);
    else
      snprintf(buffer, buffer_length,
               "%02d-%02d-%04d %02d:%02d:%02d", month, day,
               year, hour, minute, second);
  }

  /* short date */
  else if (type == SHORT_DATE) {
    if (config.get_date_format() == DATE_FORMAT_EURO)
      snprintf(buffer, buffer_length, "%02d-%02d-%04d", day,
               month, year);
    else if (config.get_date_format() == DATE_FORMAT_ISO8601
             || config.get_date_format() == DATE_FORMAT_STRICT_ISO8601)
      snprintf(buffer, buffer_length, "%04d-%02d-%02d", year,
               month, day);
    else
      snprintf(buffer, buffer_length, "%02d-%02d-%04d", month,
               day, year);
  }

  /* expiration date/time for HTTP headers */
  else if (type == HTTP_DATE_TIME)
    snprintf(buffer, buffer_length,
             "%s, %02d %s %d %02d:%02d:%02d GMT",
             weekdays[tm_ptr->tm_wday], day, months[tm_ptr->tm_mon],
             year, hour, minute, second);

  /* short time */
  else
    snprintf(buffer, buffer_length, "%02d:%02d:%02d", hour, minute, second);

  buffer[buffer_length - 1] = '\x0';
}

/* get days, hours, minutes, and seconds from a raw time_t format or total seconds */
void get_time_breakdown(unsigned long raw_time,
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

template <>
char* obj2pchar<char const*>(char const* str) {
  return (my_strdup(str ? str : ""));
}
