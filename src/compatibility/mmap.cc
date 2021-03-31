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

#include "mmap.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstddef>
#include <cstring>
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;

/* open a file read-only via mmap() */
mmapfile* mmap_fopen(char const* filename) {
  if (!filename)
    return (NULL);

  /* open the file */
  int fd(open(filename, O_RDONLY));
  if (fd == -1)
    return (NULL);

  /* get file info */
  struct stat statbuf;
  if ((fstat(fd, &statbuf)) == -1) {
    close(fd);
    return (NULL);
  }

  /* get file size */
  unsigned long file_size((unsigned long)statbuf.st_size);
  void* mmap_buf(NULL);

  /* only mmap() if we have a file greater than 0 bytes */
  if (file_size > 0) {
    /* mmap() the file - allocate one extra byte for processing zero-byte files
     */
    if ((mmap_buf = (void*)mmap(0, file_size, PROT_READ, MAP_PRIVATE, fd, 0)) ==
        MAP_FAILED) {
      close(fd);
      return (NULL);
    }
  }

  /* allocate memory */
  mmapfile* new_mmapfile = new mmapfile();

  /* populate struct info for later use */
  new_mmapfile->path = string::dup(filename);
  new_mmapfile->fd = fd;
  new_mmapfile->file_size = (unsigned long)file_size;
  new_mmapfile->current_position = 0L;
  new_mmapfile->current_line = 0L;
  new_mmapfile->mmap_buf = mmap_buf;
  return (new_mmapfile);
}

/* close a file originally opened via mmap() */
int mmap_fclose(mmapfile* temp_mmapfile) {
  if (!temp_mmapfile)
    return (ERROR);

  /* un-mmap() the file */
  if (temp_mmapfile->file_size > 0L)
    munmap(temp_mmapfile->mmap_buf, temp_mmapfile->file_size);

  /* close the file */
  close(temp_mmapfile->fd);

  /* free memory */
  delete[] temp_mmapfile->path;
  delete temp_mmapfile;
  return OK;
}

/* gets one line of input from an mmap()'ed file */
char* mmap_fgets(mmapfile* temp_mmapfile) {
  if (!temp_mmapfile || !temp_mmapfile->file_size ||
      temp_mmapfile->current_position >= temp_mmapfile->file_size)
    return (NULL);

  /* find the end of the string (or buffer) */
  unsigned long x(0L);
  for (x = temp_mmapfile->current_position; x < temp_mmapfile->file_size; ++x) {
    if (*((char*)(temp_mmapfile->mmap_buf) + x) == '\n') {
      ++x;
      break;
    }
  }

  /* calculate length of line we just read */
  int len((int)(x - temp_mmapfile->current_position));

  /* allocate memory for the new line */
  char* buf(new char[len + 1]);

  /* copy string to newly allocated memory and terminate the string */
  memcpy(buf,
         ((char*)(temp_mmapfile->mmap_buf) + temp_mmapfile->current_position),
         len);
  buf[len] = '\x0';

  /* update the current position */
  temp_mmapfile->current_position = x;

  /* increment the current line */
  temp_mmapfile->current_line++;
  return buf;
}
