/*
** Copyright 1999-2010 Ethan Galstad
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

#include <cstdlib>
#include <cstring>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/deleter/comment.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/objects/comment.hh"
#include "com/centreon/engine/objects/tool.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/engine/utils.hh"
#include "com/centreon/engine/xcddefault.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::string;

static comment* comment_hashlist[COMMENT_HASHSLOTS];

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool operator==(comment const& obj1, comment const& obj2) throw () {
  return (obj1.comment_type == obj2.comment_type
          && obj1.entry_type == obj2.entry_type
          && obj1.comment_id == obj2.comment_id
          && obj1.source == obj2.source
          && obj1.persistent == obj2.persistent
          && obj1.entry_time == obj2.entry_time
          && obj1.expires == obj2.expires
          && obj1.expire_time == obj2.expire_time
          && is_equal(obj1.host_name, obj2.host_name)
          && is_equal(obj1.service_description, obj2.service_description)
          && is_equal(obj1.author, obj2.author)
          && is_equal(obj1.comment_data, obj2.comment_data));
}

/**
 *  Not equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool operator!=(comment const& obj1, comment const& obj2) throw () {
  return (!operator==(obj1, obj2));
}

/**
 *  Dump downtime content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The downtime to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, comment const& obj) {
  os << "comment {\n"
    "  comment_type:        " << obj.comment_type << "\n"
    "  entry_type:          " << obj.entry_type << "\n"
    "  comment_id:          " << obj.comment_id << "\n"
    "  source:              " << obj.source << "\n"
    "  persistent:          " << obj.persistent << "\n"
    "  entry_time:          " << obj.entry_time << "\n"
    "  expires:             " << obj.expires << "\n"
    "  expire_time:         " << obj.expire_time << "\n"
    "  host_name:           " << chkstr(obj.host_name) << "\n"
    "  service_description: " << chkstr(obj.service_description) << "\n"
    "  author:              " << chkstr(obj.author) << "\n"
    "  comment_data:        " << chkstr(obj.comment_data) << "\n"
    "}\n";
  return (os);
}

/******************************************************************/
/**************** INITIALIZATION/CLEANUP FUNCTIONS ****************/
/******************************************************************/

/* initializes comment data */
int initialize_comment_data() {
  return (xcddefault_initialize_comment_data());
}

/******************************************************************/
/****************** COMMENT OUTPUT FUNCTIONS **********************/
/******************************************************************/

/* adds a new host or service comment */
int add_new_comment(
      unsigned int type,
      int entry_type,
      char const* host_name,
      char const* svc_description,
      time_t entry_time,
      char const* author_name,
      char const* comment_data,
      int persistent, int source,
      int expires,
      time_t expire_time,
      unsigned long* comment_id) {
  int result = OK;
  unsigned long new_comment_id = 0L;

  if (type == HOST_COMMENT)
    result = add_new_host_comment(
               entry_type,
               host_name,
               entry_time,
               author_name,
               comment_data,
               persistent,
               source,
               expires,
               expire_time,
               &new_comment_id);
  else
    result = add_new_service_comment(
               entry_type,
               host_name,
               svc_description,
               entry_time,
               author_name,
               comment_data,
               persistent,
               source,
               expires,
               expire_time,
               &new_comment_id);

  /* add an event to expire comment data if necessary... */
  if (expires == true)
    schedule_new_event(
      EVENT_EXPIRE_COMMENT,
      false,
      expire_time,
      false,
      0,
      NULL,
      true,
      (void*)new_comment_id,
      NULL,
      0);

  /* save comment id */
  if (comment_id != NULL)
    *comment_id = new_comment_id;

  return (result);
}

/* adds a new host comment */
int add_new_host_comment(
      int entry_type,
      char const* host_name,
      time_t entry_time,
      char const* author_name,
      char const* comment_data,
      int persistent,
      int source,
      int expires,
      time_t expire_time,
      unsigned long* comment_id) {
  int result = OK;
  unsigned long new_comment_id = 0L;

  result = xcddefault_add_new_host_comment(
             entry_type,
             host_name,
             entry_time,
             author_name,
             comment_data,
             persistent,
             source,
             expires,
             expire_time,
             &new_comment_id);

  /* save comment id */
  if (comment_id != NULL)
    *comment_id = new_comment_id;

  /* send data to event broker */
  broker_comment_data(
    NEBTYPE_COMMENT_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    HOST_COMMENT,
    entry_type,
    host_name,
    NULL,
    entry_time,
    author_name,
    comment_data,
    persistent,
    source,
    expires,
    expire_time,
    new_comment_id,
    NULL);
  return (result);
}

/* adds a new service comment */
int add_new_service_comment(
      int entry_type,
      char const* host_name,
      char const* svc_description,
      time_t entry_time,
      char const* author_name,
      char const* comment_data,
      int persistent,
      int source,
      int expires,
      time_t expire_time,
      unsigned long* comment_id) {
  int result = OK;
  unsigned long new_comment_id = 0L;

  result = xcddefault_add_new_service_comment(
             entry_type,
             host_name,
             svc_description,
             entry_time,
             author_name,
             comment_data,
             persistent,
             source,
             expires,
             expire_time,
             &new_comment_id);

  /* save comment id */
  if (comment_id != NULL)
    *comment_id = new_comment_id;

  /* send data to event broker */
  broker_comment_data(
    NEBTYPE_COMMENT_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    SERVICE_COMMENT,
    entry_type,
    host_name,
    svc_description,
    entry_time,
    author_name,
    comment_data,
    persistent,
    source,
    expires,
    expire_time,
    new_comment_id,
    NULL);
  return (result);
}

/******************************************************************/
/***************** COMMENT DELETION FUNCTIONS *********************/
/******************************************************************/

/* deletes a host or service comment */
int delete_comment(unsigned int type, unsigned long comment_id) {
  int result = OK;
  comment* this_comment = NULL;
  comment* last_comment = NULL;
  comment* next_comment = NULL;
  int hashslot = 0;
  comment* this_hash = NULL;
  comment* last_hash = NULL;

  /* find the comment we should remove */
  for (this_comment = comment_list, last_comment = comment_list;
       this_comment != NULL;
       this_comment = next_comment) {
    next_comment = this_comment->next;

    /* we found the comment we should delete */
    if (this_comment->comment_id == comment_id
        && this_comment->comment_type == type)
      break;

    last_comment = this_comment;
  }

  /* remove the comment from the list in memory */
  if (this_comment != NULL) {

    /* send data to event broker */
    broker_comment_data(
      NEBTYPE_COMMENT_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      type, this_comment->entry_type,
      this_comment->host_name,
      this_comment->service_description,
      this_comment->entry_time,
      this_comment->author,
      this_comment->comment_data,
      this_comment->persistent,
      this_comment->source,
      this_comment->expires,
      this_comment->expire_time,
      comment_id,
      NULL);

    /* first remove from chained hash list */
    hashslot = hashfunc(
                 this_comment->host_name,
                 NULL,
                 COMMENT_HASHSLOTS);
    last_hash = NULL;
    for (this_hash = comment_hashlist[hashslot];
	 this_hash;
	 this_hash = this_hash->nexthash) {
      if (this_hash == this_comment) {
        if (last_hash)
          last_hash->nexthash = this_hash->nexthash;
        else
	  comment_hashlist[hashslot] = this_hash->nexthash;
        break;
      }
      last_hash = this_hash;
    }

    /* then removed from linked list */
    if (comment_list == this_comment)
      comment_list = this_comment->next;
    else
      last_comment->next = next_comment;

    /* free memory */
    deleter::comment(this_comment);

    result = OK;
  }
  else
    result = ERROR;

  if (type == HOST_COMMENT)
    result = xcddefault_delete_host_comment(comment_id);
  else
    result = xcddefault_delete_service_comment(comment_id);
  return (result);
}

/* deletes a host comment */
int delete_host_comment(unsigned long comment_id) {
  /* delete the comment from memory */
  return (delete_comment(HOST_COMMENT, comment_id));
}

/* deletes a service comment */
int delete_service_comment(unsigned long comment_id) {
  /* delete the comment from memory */
  return (delete_comment(SERVICE_COMMENT, comment_id));
}

/* deletes all comments for a particular host or service */
int delete_all_comments(
      unsigned int type,
      char const* host_name,
      char const* svc_description) {
  if (type == HOST_COMMENT)
    return (delete_all_host_comments(host_name));
  return (delete_all_service_comments(host_name, svc_description));
}

/* deletes all comments for a particular host */
int delete_all_host_comments(char const* host_name) {
  comment* temp_comment = NULL;
  comment* next_comment = NULL;

  if (host_name == NULL)
    return (ERROR);

  /* delete host comments from memory */
  for (temp_comment = get_first_comment_by_host(host_name);
       temp_comment != NULL;
       temp_comment = next_comment) {
    next_comment = get_next_comment_by_host(host_name, temp_comment);
    if (temp_comment->comment_type == HOST_COMMENT)
      delete_comment(HOST_COMMENT, temp_comment->comment_id);
  }

  return (OK);
}

/* deletes all non-persistent acknowledgement comments for a particular host */
int delete_host_acknowledgement_comments(host* hst) {
  comment* temp_comment = NULL;
  comment* next_comment = NULL;

  if (hst == NULL)
    return (ERROR);

  /* delete comments from memory */
  for (temp_comment = get_first_comment_by_host(hst->get_name().c_str());
       temp_comment != NULL;
       temp_comment = next_comment) {
    next_comment = get_next_comment_by_host(hst->get_name().c_str(),
      temp_comment);
    if (temp_comment->comment_type == HOST_COMMENT
        && temp_comment->entry_type == ACKNOWLEDGEMENT_COMMENT
        && temp_comment->persistent == false)
      delete_comment(HOST_COMMENT, temp_comment->comment_id);
  }
  return (OK);
}

/* deletes all comments for a particular service */
int delete_all_service_comments(
      char const* host_name,
      char const* svc_description) {
  comment* temp_comment = NULL;
  comment* next_comment = NULL;

  if (host_name == NULL || svc_description == NULL)
    return (ERROR);

  /* delete service comments from memory */
  for (temp_comment = comment_list;
       temp_comment != NULL;
       temp_comment = next_comment) {
    next_comment = temp_comment->next;
    if (temp_comment->comment_type == SERVICE_COMMENT
        && !strcmp(temp_comment->host_name, host_name)
        && !strcmp(temp_comment->service_description, svc_description))
      delete_comment(SERVICE_COMMENT, temp_comment->comment_id);
  }
  return (OK);
}

/* deletes all non-persistent acknowledgement comments for a particular service */
int delete_service_acknowledgement_comments(service* svc) {
  comment* temp_comment = NULL;
  comment* next_comment = NULL;

  if (svc == NULL)
    return (ERROR);

  /* delete comments from memory */
  for (temp_comment = comment_list;
       temp_comment != NULL;
       temp_comment = next_comment) {
    next_comment = temp_comment->next;
    if (temp_comment->comment_type == SERVICE_COMMENT
        && !strcmp(temp_comment->host_name, svc->host_name)
        && !strcmp(temp_comment->service_description, svc->description)
        && temp_comment->entry_type == ACKNOWLEDGEMENT_COMMENT
        && temp_comment->persistent == false)
      delete_comment(SERVICE_COMMENT, temp_comment->comment_id);
  }
  return (OK);
}

/* checks for an expired comment (and removes it) */
int check_for_expired_comment(unsigned long comment_id) {
  comment* temp_comment = NULL;

  /* check all comments */
  for (temp_comment = comment_list;
       temp_comment != NULL;
       temp_comment = temp_comment->next) {

    /* delete the now expired comment */
    if (temp_comment->comment_id == comment_id
        && temp_comment->expires == true
        && temp_comment->expire_time < time(NULL)) {
      delete_comment(temp_comment->comment_type, comment_id);
      break;
    }
  }
  return (OK);
}

/******************************************************************/
/****************** CHAINED HASH FUNCTIONS ************************/
/******************************************************************/

/* adds comment to hash list in memory */
int add_comment_to_hashlist(comment* new_comment) {
  static bool init = false;
  comment* temp_comment = NULL;
  comment* lastpointer = NULL;
  int hashslot = 0;

  /* initialize hash list */
  if (init == false) {
    for (unsigned int i = 0; i < COMMENT_HASHSLOTS; i++)
      comment_hashlist[i] = NULL;
    init = true;
  }

  if (!new_comment)
    return (0);

  hashslot = hashfunc(new_comment->host_name, NULL, COMMENT_HASHSLOTS);
  lastpointer = NULL;
  // TODO: XXX rewrite for (mutiple call of compare_hashdata).
  for (temp_comment = comment_hashlist[hashslot];
       temp_comment && compare_hashdata(temp_comment->host_name, NULL, new_comment->host_name, NULL) < 0;
       temp_comment = temp_comment->nexthash) {
    if (compare_hashdata(
          temp_comment->host_name,
          NULL,
          new_comment->host_name,
          NULL) >= 0)
      break;
    lastpointer = temp_comment;
  }

  /* multiples are allowed */
  if (lastpointer)
    lastpointer->nexthash = new_comment;
  else
    comment_hashlist[hashslot] = new_comment;
  new_comment->nexthash = temp_comment;
  return (1);
}

/******************************************************************/
/******************** ADDITION FUNCTIONS **************************/
/******************************************************************/

/* adds a host comment to the list in memory */
int add_host_comment(
      int entry_type,
      char const* host_name,
      time_t entry_time,
      char const* author,
      char const* comment_data,
      unsigned long comment_id,
      int persistent,
      int expires,
      time_t expire_time,
      int source) {
  return (add_comment(
            HOST_COMMENT,
            entry_type,
            host_name,
            NULL,
            entry_time,
            author,
            comment_data,
            comment_id,
            persistent,
            expires,
            expire_time,
            source));
}

/* adds a service comment to the list in memory */
int add_service_comment(
      int entry_type,
      char const* host_name,
      char const* svc_description,
      time_t entry_time,
      char const* author,
      char const* comment_data,
      unsigned long comment_id,
      int persistent,
      int expires,
      time_t expire_time,
      int source) {
  return (add_comment(
            SERVICE_COMMENT,
            entry_type,
            host_name,
            svc_description,
            entry_time,
            author,
            comment_data,
            comment_id,
            persistent,
            expires,
            expire_time,
            source));
}

/* adds a comment to the list in memory */
int add_comment(
      unsigned int comment_type,
      int entry_type,
      char const* host_name,
      char const* svc_description,
      time_t entry_time,
      char const* author,
      char const* comment_data,
      unsigned long comment_id,
      int persistent,
      int expires,
      time_t expire_time,
      int source) {
  /* make sure we have the data we need */
  if (host_name == NULL
      || author == NULL
      || comment_data == NULL
      || (comment_type == SERVICE_COMMENT
          && svc_description == NULL))
    return (ERROR);

  /* allocate memory for the comment */
  comment* new_comment(new comment);
  memset(new_comment, 0, sizeof(*new_comment));

  /* duplicate vars */
  new_comment->host_name = string::dup(host_name);
  if (comment_type == SERVICE_COMMENT)
    new_comment->service_description = string::dup(svc_description);
  new_comment->author = string::dup(author);
  new_comment->comment_data = string::dup(comment_data);
  new_comment->comment_type = comment_type;
  new_comment->entry_type = entry_type;
  new_comment->source = source;
  new_comment->entry_time = entry_time;
  new_comment->comment_id = comment_id;
  new_comment->persistent = (persistent == true) ? true : false;
  new_comment->expires = (expires == true) ? true : false;
  new_comment->expire_time = expire_time;

  /* add comment to hash list */
  add_comment_to_hashlist(new_comment);

  if (defer_comment_sorting) {
    new_comment->next = comment_list;
    comment_list = new_comment;
  }
  else {
    /* add new comment to comment list, sorted by comment id */
    comment* last_comment(comment_list);
    comment* temp_comment(NULL);
    for (temp_comment = comment_list;
	 temp_comment != NULL;
         temp_comment = temp_comment->next) {
      if (new_comment->comment_id < temp_comment->comment_id) {
        new_comment->next = temp_comment;
        if (temp_comment == comment_list)
          comment_list = new_comment;
        else
          last_comment->next = new_comment;
        break;
      }
      else
        last_comment = temp_comment;
    }
    if (comment_list == NULL) {
      new_comment->next = NULL;
      comment_list = new_comment;
    }
    else if (temp_comment == NULL) {
      new_comment->next = NULL;
      last_comment->next = new_comment;
    }
  }

  /* send data to event broker */
  broker_comment_data(
    NEBTYPE_COMMENT_LOAD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    comment_type,
    entry_type,
    host_name,
    svc_description,
    entry_time,
    author,
    comment_data,
    persistent,
    source,
    expires,
    expire_time,
    comment_id,
    NULL);
  return (OK);
}

static int comment_compar(void const* p1, void const* p2) {
  comment* c1 = *(comment**)p1;
  comment* c2 = *(comment**)p2;

  if (c1->comment_id < c2->comment_id)
    return (-1);
  return (c1->comment_id - c2->comment_id);
}

/*
** If you are going to be adding a lot of comments in sequence, set
** defer_comment_sorting to 1 before you start and then call
** sort_comments afterwards. Things will go MUCH faster.
**
** extern int defer_comment_sorting;
*/
int sort_comments() {
  comment** array;
  comment* temp_comment;
  unsigned long i = 0;
  unsigned long unsorted_comments = 0;

  if (!defer_comment_sorting)
    return (OK);
  defer_comment_sorting = 0;

  temp_comment = comment_list;
  while (temp_comment != NULL) {
    temp_comment = temp_comment->next;
    unsorted_comments++;
  }

  if (!unsorted_comments)
    return (OK);

  array = new comment*[unsorted_comments];

  while (comment_list) {
    array[i++] = comment_list;
    comment_list = comment_list->next;
  }

  qsort((void*)array, i, sizeof(*array), comment_compar);
  comment_list = temp_comment = array[0];
  for (i = 1; i < unsorted_comments; i++) {
    temp_comment->next = array[i];
    temp_comment = temp_comment->next;
  }
  temp_comment->next = NULL;
  delete[] array;
  return (OK);
}

/******************************************************************/
/********************* CLEANUP FUNCTIONS **************************/
/******************************************************************/

/* frees memory allocated for the comment data */
void free_comment_data() {
  deleter::listmember(comment_list, &deleter::comment);
  return;
}

/******************************************************************/
/********************* UTILITY FUNCTIONS **************************/
/******************************************************************/

/* get the number of comments associated with a particular host */
int number_of_host_comments(char const* host_name) {
  comment* temp_comment = NULL;
  int total_comments = 0;

  if (host_name == NULL)
    return (0);

  for (temp_comment = get_first_comment_by_host(host_name);
       temp_comment != NULL;
       temp_comment = get_next_comment_by_host(host_name, temp_comment)) {
    if (temp_comment->comment_type == HOST_COMMENT)
      total_comments++;
  }
  return (total_comments);
}

/* get the number of comments associated with a particular service */
int number_of_service_comments(
      char const* host_name,
      char const* svc_description) {
  comment* temp_comment = NULL;
  int total_comments = 0;

  if (host_name == NULL || svc_description == NULL)
    return (0);

  for (temp_comment = get_first_comment_by_host(host_name);
       temp_comment != NULL;
       temp_comment = get_next_comment_by_host(host_name, temp_comment)) {
    if (temp_comment->comment_type == SERVICE_COMMENT
        && !strcmp(temp_comment->service_description, svc_description))
      total_comments++;
  }
  return (total_comments);
}

/******************************************************************/
/********************* TRAVERSAL FUNCTIONS ************************/
/******************************************************************/

comment* get_first_comment_by_host(char const* host_name) {
  return (get_next_comment_by_host(host_name, NULL));
}

comment* get_next_comment_by_host(
           char const* host_name,
           comment* start) {
  comment* temp_comment = NULL;

  if (host_name == NULL || comment_hashlist == NULL)
    return (NULL);

  if (start == NULL)
    temp_comment = comment_hashlist[hashfunc(
                                      host_name,
                                      NULL,
                                      COMMENT_HASHSLOTS)];
  else
    temp_comment = start->nexthash;

  while (temp_comment
         && compare_hashdata(
              temp_comment->host_name,
              NULL,
              host_name,
              NULL) < 0)
    temp_comment = temp_comment->nexthash;

  if (temp_comment
      && compare_hashdata(
           temp_comment->host_name,
           NULL,
           host_name,
           NULL) == 0)
    return (temp_comment);
  return (NULL);
}

/******************************************************************/
/********************** SEARCH FUNCTIONS **************************/
/******************************************************************/

/* find a service comment by id */
comment* find_service_comment(unsigned long comment_id) {
  return (find_comment(comment_id, SERVICE_COMMENT));
}

/* find a host comment by id */
comment* find_host_comment(unsigned long comment_id) {
  return (find_comment(comment_id, HOST_COMMENT));
}

/* find a comment by id */
comment* find_comment(
           unsigned long comment_id,
           unsigned int comment_type) {
  comment* temp_comment = NULL;

  for (temp_comment = comment_list;
       temp_comment != NULL;
       temp_comment = temp_comment->next) {
    if (temp_comment->comment_id == comment_id
        && temp_comment->comment_type == comment_type)
      return (temp_comment);
  }
  return (NULL);
}
