/*
** Copyright 2008           Ethan Galstad
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

#include <cstdlib>
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/globals.hh"
#include "skiplist.h"

skiplist* object_skiplists[NUM_OBJECT_SKIPLISTS];

skiplist* skiplist_new(
            int max_levels,
            float level_probability,
            int allow_duplicates,
            int append_duplicates,
            int (*compare_function)(void const*, void const*)) {
  skiplist* newlist = NULL;

  /* alloc memory for new list structure */
  newlist = new skiplist;

  /* initialize levels, etc. */
  newlist->current_level = 0;
  newlist->max_levels = max_levels;
  newlist->level_probability = level_probability;
  newlist->allow_duplicates = allow_duplicates;
  newlist->append_duplicates = append_duplicates;
  newlist->items = 0;
  newlist->compare_function = compare_function;

  /* initialize head node */
  newlist->head = skiplist_new_node(newlist, max_levels);
  return (newlist);
}

int skiplist_insert(skiplist* list, void* data) {
  skiplistnode** update = NULL;
  skiplistnode* thisnode = NULL;
  skiplistnode* nextnode = NULL;
  skiplistnode* newnode = NULL;
  int level = 0;
  int x = 0;

  if (list == NULL || data == NULL)
    return (SKIPLIST_ERROR_ARGS);

  /* initialize update vector */
  update = new skiplistnode*[list->max_levels];

  for (x = 0; x < list->max_levels; x++)
    update[x] = NULL;

  /* check to make sure we don't have duplicates */
  /* NOTE: this could made be more efficient */
  if (list->allow_duplicates == false) {
    if (skiplist_find_first(list, data, NULL))
      return (SKIPLIST_ERROR_DUPLICATE);
  }

  /* find proper position for insert, remember pointers  with an update vector */
  thisnode = list->head;
  for (level = list->current_level; level >= 0; level--) {
    while ((nextnode = thisnode->forward[level])) {
      if (list->append_duplicates == true) {
        if (list->compare_function(nextnode->data, data) > 0)
          break;
      }
      else {
        if (list->compare_function(nextnode->data, data) >= 0)
          break;
      }
      thisnode = nextnode;
    }
    update[level] = thisnode;
  }

  /* get a random level the new node should be inserted at */
  level = skiplist_random_level(list);
  /*printf("INSERTION LEVEL: %d\n",level); */

  /* we're adding a new level... */
  if (level > list->current_level) {
    /*printf("NEW LEVEL!\n"); */
    list->current_level++;
    level = list->current_level;
    update[level] = list->head;
  }

  /* create a new node */
  if ((newnode = skiplist_new_node(list, level)) == NULL) {
    /*printf("NODE ERROR\n"); */
    delete[] update;
    return (SKIPLIST_ERROR_MEMORY);
  }
  newnode->data = data;

  /* update pointers to insert node at proper location */
  do {
    thisnode = update[level];
    newnode->forward[level] = thisnode->forward[level];
    thisnode->forward[level] = newnode;

  } while (--level >= 0);

  /* update counters */
  list->items++;

  /* free memory */
  delete[] update;

  return (SKIPLIST_OK);
}

skiplistnode* skiplist_new_node(skiplist* list, int node_levels) {
  skiplistnode* newnode = NULL;
  int x = 0;

  if (list == NULL)
    return (NULL);

  if (node_levels < 0 || node_levels > list->max_levels)
    return (NULL);

  /* allocate memory for node + variable number of level pointers */
  newnode = new skiplistnode[node_levels + 1];

  /* initialize forward pointers */
  for (x = 0; x < node_levels; x++)
    newnode->forward[x] = NULL;

  /* initialize data pointer */
  newnode->data = NULL;
  return (newnode);
}

int skiplist_random_level(skiplist* list) {
  int level = 0;
  float r = 0.0;

  if (list == NULL)
    return (-1);

  for (level = 0; level < list->max_levels; level++) {
    r = ((float)rand() / (float)RAND_MAX);
    if (r > list->level_probability)
      break;
  }

  return ((level >= list->max_levels) ? list->max_levels - 1 : level);
}

int skiplist_empty(skiplist* list) {
  skiplistnode* self = NULL;
  skiplistnode* next = NULL;
  int level = 0;

  if (list == NULL)
    return (ERROR);

  /* free all list nodes (but not header) */
  for (self = list->head->forward[0]; self != NULL; self = next) {
    next = self->forward[0];
    delete self;
  }

  /* reset level pointers */
  for (level = list->current_level; level >= 0; level--)
    list->head->forward[level] = NULL;

  /* reset list level */
  list->current_level = 0;

  /* reset items */
  list->items = 0;

  return (OK);
}

int skiplist_free(skiplist** list) {
  skiplistnode* self = NULL;
  skiplistnode* next = NULL;

  if (list == NULL)
    return (ERROR);
  if (*list == NULL)
    return (OK);

  /* free header and all list nodes */
  for (self = (*list)->head; self != NULL; self = next) {
    next = self->forward[0];
    delete[] self;
  }

  /* free list structure */
  delete *list;
  *list = NULL;

  return (OK);
}

/* get first item in list */
void* skiplist_peek(skiplist* list) {
  if (list == NULL)
    return (NULL);
  /* return first item */
  return (list->head->forward[0]->data);
}

/* get/remove first item in list */
void* skiplist_pop(skiplist* list) {
  skiplistnode* thisnode = NULL;
  void* data = NULL;
  int level = 0;

  if (list == NULL)
    return (NULL);

  /* get first item */
  thisnode = list->head->forward[0];
  if (thisnode == NULL)
    return (NULL);

  /* get data for first item */
  data = thisnode->data;

  /* remove first item from queue - update forward links from head to first node */
  for (level = 0; level <= list->current_level; level++) {
    if (list->head->forward[level] == thisnode)
      list->head->forward[level] = thisnode->forward[level];
  }

  /* free deleted node */
  delete thisnode;

  /* adjust items */
  list->items--;
  return (data);
}

/* get first item in list */
void* skiplist_get_first(skiplist* list, void** node_ptr) {
  skiplistnode* thisnode = NULL;

  if (list == NULL)
    return (NULL);

  /* get first node */
  thisnode = list->head->forward[0];

  /* return pointer to node */
  if (node_ptr)
    *node_ptr = (void*)thisnode;

  if (thisnode)
    return (thisnode->data);
  return (NULL);
}

/* get next item in list */
void* skiplist_get_next(void** node_ptr) {
  skiplistnode* thisnode = NULL;
  skiplistnode* nextnode = NULL;

  if (node_ptr == NULL || *node_ptr == NULL)
    return (NULL);

  thisnode = (skiplistnode*)(*node_ptr);
  nextnode = thisnode->forward[0];

  *node_ptr = (void*)nextnode;

  if (nextnode)
    return (nextnode->data);
  return (NULL);
}

/* first first item in list */
void* skiplist_find_first(skiplist* list, void* data, void** node_ptr) {
  skiplistnode* thisnode = NULL;
  skiplistnode* nextnode = NULL;
  int level = 0;

  if (list == NULL || data == NULL)
    return (NULL);

  thisnode = list->head;
  for (level = list->current_level; level >= 0; level--) {
    while ((nextnode = thisnode->forward[level])) {
      if (list->compare_function(nextnode->data, data) >= 0)
        break;
      thisnode = nextnode;
    }
  }

  /* we found it! */
  if (nextnode && list->compare_function(nextnode->data, data) == 0) {
    if (node_ptr)
      *node_ptr = (void*)nextnode;
    return (nextnode->data);
  }

  if (node_ptr)
    *node_ptr = NULL;
  return (NULL);
}

/* find next match */
void* skiplist_find_next(skiplist* list, void* data, void** node_ptr) {
  skiplistnode* thisnode = NULL;
  skiplistnode* nextnode = NULL;

  if (list == NULL || data == NULL || node_ptr == NULL)
    return (NULL);
  if (*node_ptr == NULL)
    return (NULL);

  thisnode = (skiplistnode*)(*node_ptr);
  nextnode = thisnode->forward[0];

  if (nextnode) {
    if (list->compare_function(nextnode->data, data) == 0) {
      *node_ptr = (void*)nextnode;
      return (nextnode->data);
    }
  }
  *node_ptr = NULL;
  return (NULL);
}

/* delete (all) matching item(s) from list */
int skiplist_delete(skiplist* list, void const* data) {
  return (skiplist_delete_all(list, data));
}

/* delete first matching item from list */
int skiplist_delete_first(skiplist* list, void const* data) {
  skiplistnode** update = NULL;
  skiplistnode* thisnode = NULL;
  skiplistnode* nextnode = NULL;
  int level = 0;
  int top_level = 0;
  int deleted = false;
  int x = 0;

  if (list == NULL || data == NULL)
    return (ERROR);

  /* initialize update vector */
  update = new skiplistnode*[list->max_levels];

  for (x = 0; x < list->max_levels; x++)
    update[x] = NULL;

  /* find location in list */
  thisnode = list->head;
  for (top_level = level = list->current_level; level >= 0; level--) {
    while ((nextnode = thisnode->forward[level])) {
      if (list->compare_function(nextnode->data, data) >= 0)
        break;
      thisnode = nextnode;
    }
    update[level] = thisnode;
  }

  /* we found a match! */
  if (nextnode != NULL
      && list->compare_function(nextnode->data, data) == 0) {

    /* adjust level pointers to bypass (soon to be) removed node */
    for (level = 0; level <= top_level; level++) {

      thisnode = update[level];
      if (thisnode->forward[level] != nextnode)
        break;

      thisnode->forward[level] = nextnode->forward[level];
    }

    /* free node memory */
    delete[] nextnode;

    /* adjust top/current level of list is necessary */
    while (list->head->forward[top_level] == NULL && top_level > 0)
      top_level--;
    list->current_level = top_level;

    /* adjust items */
    list->items--;

    deleted = true;
  }

  /* free memory */
  delete[] update;

  return (deleted);
}

/* delete all matching items from list */
int skiplist_delete_all(skiplist* list, void const* data) {
  int total_deleted(0);

  /* NOTE: there is a more efficient way to do this... */
  while (skiplist_delete_first(list, data) == 1)
    ++total_deleted;
  return (total_deleted);
}

/* delete specific node from list */
int skiplist_delete_node(skiplist* list, void const* node_ptr) {
  void* data = NULL;
  skiplistnode** update = NULL;
  skiplistnode* thenode = NULL;
  skiplistnode* thisnode = NULL;
  skiplistnode* nextnode = NULL;
  int level = 0;
  int top_level = 0;
  int deleted = false;
  int x = 0;

  if (list == NULL || node_ptr == NULL)
    return (ERROR);

  /* we'll need the data from the node to first find the node */
  thenode = (skiplistnode*) node_ptr;
  data = thenode->data;

  /* initialize update vector */
  update = new skiplistnode*[list->max_levels];

  for (x = 0; x < list->max_levels; x++)
    update[x] = NULL;

  /* find location in list */
  thisnode = list->head;
  for (top_level = level = list->current_level; level >= 0; level--) {
    while ((nextnode = thisnode->forward[level])) {
      /* next node would be too far */
      if (list->compare_function(nextnode->data, data) > 0)
        break;
      /* this is the exact node we want */
      if (list->compare_function(nextnode->data, data) == 0
          && nextnode == thenode)
        break;

      thisnode = nextnode;
    }
    update[level] = thisnode;
  }

  /* we found a match! (value + pointers match) */
  if (nextnode
      && list->compare_function(nextnode->data, data) == 0
      && nextnode == thenode) {

    /* adjust level pointers to bypass (soon to be) removed node */
    for (level = 0; level <= top_level; level++) {
      thisnode = update[level];
      if (thisnode->forward[level] != nextnode)
        break;

      thisnode->forward[level] = nextnode->forward[level];
    }

    /* free node memory */
    delete nextnode;

    /* adjust top/current level of list is necessary */
    while (list->head->forward[top_level] == NULL && top_level > 0)
      top_level--;
    list->current_level = top_level;

    /* adjust items */
    list->items--;

    deleted = true;
  }

  /* free memory */
  delete[] update;
  return (deleted);
}

/**
 *  Free all object skiplists.
 *
 *  @return OK.
 */
int free_object_skiplists() {
  for (unsigned int x(0); x < NUM_OBJECT_SKIPLISTS; ++x)
    skiplist_free(&object_skiplists[x]);
  return (OK);
}

/**
 *  Initialize all object skiplists.
 *
 *  @return OK on successful initialization.
 */
int init_object_skiplists() {
  // Reset pointers.
  for (unsigned int x(0); x < NUM_OBJECT_SKIPLISTS; ++x)
    object_skiplists[x] = NULL;

  // Allocate skiplists.
  object_skiplists[HOST_SKIPLIST]
    = skiplist_new(15, 0.5, false, false, skiplist_compare_host);
  object_skiplists[SERVICE_SKIPLIST]
    = skiplist_new(15, 0.5, false, false, skiplist_compare_service);
  object_skiplists[COMMAND_SKIPLIST]
    = skiplist_new(10, 0.5, false, false, skiplist_compare_command);
  object_skiplists[TIMEPERIOD_SKIPLIST]
    = skiplist_new(10, 0.5, false, false, skiplist_compare_timeperiod);
  object_skiplists[HOSTGROUP_SKIPLIST]
    = skiplist_new(10, 0.5, false, false, skiplist_compare_hostgroup);
  object_skiplists[SERVICEGROUP_SKIPLIST]
    = skiplist_new(10, 0.5, false, false, skiplist_compare_servicegroup);
  object_skiplists[HOSTDEPENDENCY_SKIPLIST]
    = skiplist_new(15, 0.5, true, false, skiplist_compare_hostdependency);
  object_skiplists[SERVICEDEPENDENCY_SKIPLIST]
    = skiplist_new(15, 0.5, true, false, skiplist_compare_servicedependency);

  return (OK);
}

/**
 *  Compare two commands.
 *
 *  @param[in] a Uncasted command #1.
 *  @param[in] b Uncasted command #2.
 *
 *  @return Similar to strcmp.
 */
int skiplist_compare_command(void const* a, void const* b) {
  command const* oa(static_cast<command const*>(a));
  command const* ob(static_cast<command const*>(b));
  if (!oa && !ob)
    return (0);
  if (!oa)
    return (1);
  if (!ob)
    return (-1);
  return (skiplist_compare_text(oa->name, NULL, ob->name, NULL));
}

/**
 *  Compare two hosts.
 *
 *  @param[in] a Uncasted host #1.
 *  @param[in] b Uncasted host #2.
 *
 *  @return Similar to strcmp.
 */
int skiplist_compare_host(void const* a, void const* b) {
  host const* oa(static_cast<host const*>(a));
  host const* ob(static_cast<host const*>(b));
  if (!oa && !ob)
    return (0);
  if (!oa)
    return (1);
  if (!ob)
    return (-1);
  return (skiplist_compare_text(oa->name, NULL, ob->name, NULL));
}

/**
 *  Compare two host dependencies.
 *
 *  @param[in] a Uncasted host dependency #1.
 *  @param[in] b Uncasted host dependency #2.
 *
 *  @return Similar to strcmp.
 */
int skiplist_compare_hostdependency(void const* a, void const* b) {
  hostdependency const* oa(static_cast<hostdependency const*>(a));
  hostdependency const* ob(static_cast<hostdependency const*>(b));
  if (!oa && !ob)
    return (0);
  if (!oa)
    return (1);
  if (!ob)
    return (-1);
  return (skiplist_compare_text(
            oa->dependent_host_name,
            NULL,
            ob->dependent_host_name,
            NULL));
}

/**
 *  Compare two host groups.
 *
 *  @param[in] a Uncasted host group #1.
 *  @param[in] b Uncasted host group #2.
 *
 *  @return Similar to strcmp.
 */
int skiplist_compare_hostgroup(void const* a, void const* b) {
  hostgroup const* oa(static_cast<hostgroup const*>(a));
  hostgroup const* ob(static_cast<hostgroup const*>(b));
  if (!oa && !ob)
    return (0);
  if (!oa)
    return (1);
  if (!ob)
    return (-1);
  return (skiplist_compare_text(
            oa->group_name,
            NULL,
            ob->group_name,
            NULL));
}

/**
 *  Compare two services.
 *
 *  @param[in] a Uncasted service #1.
 *  @param[in] b Uncasted service #2.
 *
 *  @return Similar to strcmp.
 */
int skiplist_compare_service(void const* a, void const* b) {
  service const* oa(static_cast<service const*>(a));
  service const* ob(static_cast<service const*>(b));
  if (!oa && !ob)
    return (0);
  if (!oa)
    return (1);
  if (!ob)
    return (-1);
  return (skiplist_compare_text(
            oa->host_name,
            oa->description,
            ob->host_name,
            ob->description));
}

/**
 *  Compare two service dependencies.
 *
 *  @param[in] a Uncasted service #1.
 *  @param[in] b Uncasted service #2.
 *
 *  @return Similar to strcmp.
 */
int skiplist_compare_servicedependency(void const* a, void const* b) {
  servicedependency const* oa(static_cast<servicedependency const*>(a));
  servicedependency const* ob(static_cast<servicedependency const*>(b));
  if (!oa && !ob)
    return (0);
  if (!oa)
    return (1);
  if (!ob)
    return (-1);
  return (skiplist_compare_text(
            oa->dependent_host_name,
            oa->dependent_service_description,
            ob->dependent_host_name,
            ob->dependent_service_description));
}

/**
 *  Compare two service groups.
 *
 *  @param[in] a Uncasted service group #1.
 *  @param[in] b Uncasted service group #2.
 *
 *  @return Similar to strcmp.
 */
int skiplist_compare_servicegroup(void const* a, void const* b) {
  servicegroup const* oa(static_cast<servicegroup const*>(a));
  servicegroup const* ob(static_cast<servicegroup const*>(b));
  if (!oa && !ob)
    return (0);
  if (!oa)
    return (1);
  if (!ob)
    return (-1);
  return (skiplist_compare_text(
            oa->group_name,
            NULL,
            ob->group_name,
            NULL));
}

/**
 *  Compare four strings, two by two.
 *
 *  @param[in] val1a Compared to val2a.
 *  @param[in] val1b Compared to val2b.
 *  @param[in] val2a Compared to val1a.
 *  @param[in] val2b Compared to val1b.
 *
 *  @return Similar to strcmp.
 */
int skiplist_compare_text(
      char const* val1a,
      char const* val1b,
      char const* val2a,
      char const* val2b) {
  int result(0);

  // Check first name.
  if (!val1a && !val2a)
    result = 0;
  else if (!val1a)
    result = 1;
  else if (!val2a)
    result = -1;
  else
    result = strcmp(val1a, val2a);

  // Check second name if necessary.
  if (result == 0) {
    if (!val1b && !val2b)
      result = 0;
    else if (!val1b)
      result = 1;
    else if (!val2b)
      result = -1;
    else
      result = strcmp(val1b, val2b);
  }

  return (result);
}

/**
 *  Compare two timeperiods.
 *
 *  @param[in] a Uncasted timeperiod #1.
 *  @param[in] b Uncasted timeperiod #2.
 *
 *  @return Similar to strcmp.
 */
int skiplist_compare_timeperiod(void const* a, void const* b) {
  timeperiod const* oa(static_cast<timeperiod const*>(a));
  timeperiod const* ob(static_cast<timeperiod const*>(b));
  if (!oa && !ob)
    return (0);
  if (!oa)
    return (1);
  if (!ob)
    return (-1);
  return (skiplist_compare_text(oa->name, NULL, ob->name, NULL));
}
