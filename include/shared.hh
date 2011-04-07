/*
** Copyright 1999-2011 Ethan Galstad
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

#ifndef CCS_SHARED_HH
# define CCS_SHARED_HH

# include <sys/types.h>
# include <sys/time.h>

# ifdef __cplusplus
extern "C" {
# endif

// mmapfile structure - used for reading files via mmap()
typedef struct  mmapfile_struct {
  char*         path;
  int           mode;
  int           fd;
  unsigned long file_size;
  unsigned long current_position;
  unsigned long current_line;
  void*         mmap_buf;
}               mmapfile;

// only usable on compile-time initialized arrays, for obvious reasons
# define ARRAY_SIZE(ary) (sizeof(ary) / sizeof(ary[0]))

char* my_strdup(char const* str);
char* my_strtok(char* buffer, char const* tokens);
char* my_strsep(char** stringp, const char* delim);
mmapfile* mmap_fopen(char* filename);
int mmap_fclose(mmapfile* temp_mmapfile);
char* mmap_fgets(mmapfile* temp_mmapfile);
char* mmap_fgets_multiline(mmapfile*  temp_mmapfile);
void strip(char* buffer);
int hashfunc(const char* name1, const char* name2, int hashslots);
int compare_hashdata(const char* val1a, const char* val1b, const char* val2a, const char* val2b);
void get_datetime_string(time_t* raw_time, char* buffer, int buffer_length, int type);
void get_time_breakdown(unsigned long raw_time, int* days, int* hours, int* minutes, int* seconds);
char* resize_string(char* str, size_t size);

# ifdef __cplusplus
}
# endif

# ifdef __cplusplus
#  include <sstream>
#  include <string>
#  include <string.h>
template <class T> char* obj2pchar(T obj) {
  std::ostringstream oss;
  oss << obj;
  std::string const& str = oss.str();
  char* buf = new char[str.size() + 1];
  strcpy(buf, str.c_str());
  return (buf);
}
# endif

#endif // !CCS_SHARED_HH
