/*
** Copyright 2007-2008 Ethan Galstad
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

#ifndef CCE_COMPATIBILITY_CIRCULAR_BUFFER_HH
#define CCE_COMPATIBILITY_CIRCULAR_BUFFER_HH

#include <pthread.h>

typedef struct circular_buffer_struct {
  void** buffer;
  int tail;
  int head;
  int items;
  int high;
  unsigned long overflow;
  pthread_mutex_t buffer_lock;
} circular_buffer;

#endif  // !CCE_COMPATIBILITY_CIRCULAR_BUFFER_HH
