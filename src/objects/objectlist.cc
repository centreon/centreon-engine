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

#include <cstring>
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/deleter/objectlist.hh"
#include "com/centreon/engine/objects/objectlist.hh"

using namespace com::centreon::engine;

/**
 *  Adds a object to a list of objects.
 *
 *  @param[in,out] list       Object list.
 *  @param[in]     ptr Object.
 *
 *  @return OK on success.
 */
int add_object_to_objectlist(objectlist** list, void* ptr) {
  if (!list || !ptr)
    return (ERROR);

  // Skip this object if its already in the list.
  for (objectlist* obj(*list); obj; obj = obj->next)
    if (obj->object_ptr == ptr)
      return (OK);

  // Allocate memory for a new list item.
  objectlist* obj(new objectlist);
  memset(obj, 0, sizeof(*obj));

  try {
    // Initialize vars.
    obj->object_ptr = ptr;

    // Add new item to head of list.
    obj->next = *list;
    *list = obj;
  }
  catch (...) {
    deleter::objectlist(obj);
    obj = NULL;
  }

  return (OK);
}

/**
 *  Frees memory allocated to a temporary object list.
 *
 *  @param[in,out] list List to free.
 *
 *  @return OK on success.
 */
int free_objectlist(objectlist** list) {
  if (!list)
    return (ERROR);

  // Free memory allocated to object list.
  for (objectlist *obj(*list), *next_objectlist;
       obj;
       obj = next_objectlist) {
    next_objectlist = obj->next;
    delete obj;
  }
  *list = NULL;

  return (OK);
}

/**
 *  Remove a object to a list of objects.
 *
 *  @param[in,out] list Object list.
 *  @param[in]     ptr  Object.
 *
 *  @return OK on success.
 */
int remove_object_to_objectlist(objectlist** list, void* ptr) {
  if (!list)
    return (ERROR);

  for (objectlist *obj(*list), *prev(NULL);
       obj;
       prev = obj, obj = obj->next) {
    if (obj == ptr) {
      if (!prev)
	*list = obj->next;
      else
	prev->next = obj->next;
      delete obj;
      return (OK);
    }
  }
  return (ERROR);
}

