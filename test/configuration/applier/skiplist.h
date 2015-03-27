/*
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

#ifndef CCE_COMPATIBILITY_SKIPLIST_H
#  define CCE_COMPATIBILITY_SKIPLIST_H

#  define SKIPLIST_OK              0
#  define SKIPLIST_ERROR_ARGS      1
#  define SKIPLIST_ERROR_MEMORY    2
#  define SKIPLIST_ERROR_DUPLICATE 3

// Skip lists.
#  define HOST_SKIPLIST                    0
#  define SERVICE_SKIPLIST                 1
#  define COMMAND_SKIPLIST                 2
#  define TIMEPERIOD_SKIPLIST              3
#  define HOSTGROUP_SKIPLIST               6
#  define SERVICEGROUP_SKIPLIST            7
#  define HOSTDEPENDENCY_SKIPLIST          8
#  define SERVICEDEPENDENCY_SKIPLIST       9
#  define NUM_OBJECT_SKIPLISTS             10

typedef struct                skiplistnode_struct {
  void*                       data;
  struct skiplistnode_struct* forward[1]; // this must be the last element of the struct, as we allocate # of elements during runtime
}                             skiplistnode;

typedef struct  skiplist_struct {
  int           current_level;
  int           max_levels;
  float         level_probability;
  unsigned long items;
  int           allow_duplicates;
  int           append_duplicates;
  int           (*compare_function)(void const*, void const*);
  skiplistnode* head;
}               skiplist;

extern skiplist* object_skiplists[NUM_OBJECT_SKIPLISTS];

#  ifdef __cplusplus
extern "C" {
#  endif // C++

skiplist* skiplist_new(
            int max_levels,
            float level_probability,
            int allow_duplicates,
            int append_duplicates,
            int (*compare_function)(void const*, void const*));
int skiplist_insert(skiplist* list, void* data);
skiplistnode* skiplist_new_node(skiplist* list, int node_levels);
int skiplist_random_level(skiplist* list);
int skiplist_empty(skiplist* list);
int skiplist_free(skiplist** list);
void* skiplist_peek(skiplist* list);
void* skiplist_pop(skiplist* list);
void* skiplist_get_first(skiplist* list, void** node_ptr);
void* skiplist_get_next(void** node_ptr);
void* skiplist_find_first(skiplist* list, void* data, void** node_ptr);
void* skiplist_find_next(skiplist* list, void* data, void** node_ptr);
int skiplist_delete(skiplist* list, void const* data);
int skiplist_delete_first(skiplist* list, void const* data);
int skiplist_delete_all(skiplist* list, void const* data);
int skiplist_delete_node(skiplist* list, void const* node_ptr);

int free_object_skiplists();
int init_object_skiplists();
int skiplist_compare_command(void const* a, void const* b);
int skiplist_compare_host(void const* a, void const* b);
int skiplist_compare_hostdependency(void const* a, void const* b);
int skiplist_compare_hostgroup(void const* a, void const* b);
int skiplist_compare_service(void const* a, void const* b);
int skiplist_compare_servicedependency(void const* a, void const* b);
int skiplist_compare_servicegroup(void const* a, void const* b);
int skiplist_compare_text(
      char const* val1a,
      char const* val1b,
      char const* val2a,
      char const* val2b);
int skiplist_compare_timeperiod(void const* a, void const* b);

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_COMPATIBILITY_SKIPLIST_H
