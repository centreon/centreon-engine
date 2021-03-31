/*
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

#ifndef CCE_COMPATIBILITY_MMAP_H
#define CCE_COMPATIBILITY_MMAP_H

/* mmapfile structure - used for reading files via mmap() */
typedef struct mmapfile_struct {
  char* path;
  int mode;
  int fd;
  unsigned long file_size;
  unsigned long current_position;
  unsigned long current_line;
  void* mmap_buf;
} mmapfile;

#ifdef __cplusplus
extern "C" {
#endif /* C++ */

mmapfile* mmap_fopen(char const* filename);
int mmap_fclose(mmapfile* temp_mmapfile);
char* mmap_fgets(mmapfile* temp_mmapfile);

#ifdef __cplusplus
}
#endif /* C++ */

#endif  // !CCE_COMPATIBILITY_MMAP_H
