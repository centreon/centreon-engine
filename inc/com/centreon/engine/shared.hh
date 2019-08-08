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

#ifndef CCE_SHARED_HH
#  define CCE_SHARED_HH

#  include <string>
#  include <sys/time.h>
#  include <sys/types.h>

struct uuid {

};

class Uuid {
 public:
  Uuid();
  Uuid(Uuid const& uuid);
  Uuid const& operator=(Uuid const& uuid);

  bool operator==(Uuid const& uuid) const;

  std::string to_string() const;

 private:
  uint32_t	_time_low;
  uint16_t	_time_mid;
  uint16_t	_time_hi_and_version;
  uint8_t		_clock_seq_hi_and_reserved;
  uint8_t		_clock_seq_low;
  uint8_t		_node[6];
};


#  ifdef __cplusplus
extern "C" {
#  endif // C++

char* my_strtok(char const* buffer, char const* tokens);
void strip(char* buffer);
int compare_hashdata(
      char const* val1a,
      char const* val1b,
      char const* val2a,
      const char* val2b);
void get_datetime_string(
       time_t const* raw_time,
       char* buffer,
       int buffer_length,
       int type);
void get_time_breakdown(
       unsigned long raw_time,
       int* days,
       int* hours,
       int* minutes,
       int* seconds);
char* resize_string(char* str, size_t size);

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_SHARED_HH
