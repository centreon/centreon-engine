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

#ifndef CCE_OBJECTS_OBJECTLIST_HH
#  define CCE_OBJECTS_OBJECTLIST_HH

typedef struct              objectlist_struct {
  void*                     object_ptr;
  struct objectlist_struct* next;
}                           objectlist;

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

int add_object_to_objectlist(objectlist** list, void* ptr);
int free_objectlist(objectlist** list);
int remove_object_to_objectlist(objectlist** list, void* ptr);

#  ifdef __cplusplus
}
#  endif /* C++ */

#endif // !CCE_OBJECTS_OBJECTLIST_HH


