/*
** Copyright 2008      Ethan Galstad
** Copyright 2011-2012 Merethis
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

#ifndef CCE_SKIPLIST_HH
#  define CCE_SKIPLIST_HH

#  define SKIPLIST_OK              0
#  define SKIPLIST_ERROR_ARGS      1
#  define SKIPLIST_ERROR_MEMORY    2
#  define SKIPLIST_ERROR_DUPLICATE 3

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

#  ifdef __cplusplus
extern "C" {
#  endif // C++

skiplist* skiplist_new(int max_levels, float level_probability, int allow_duplicates, int append_duplicates, int (*compare_function)(void const*, void const*));
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

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_SKIPLIST_HH
