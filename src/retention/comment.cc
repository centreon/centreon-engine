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

#include "com/centreon/engine/comments.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/misc/string.hh"
#include "com/centreon/engine/retention/comment.hh"

using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine;

/**
 *  Constructor.
 *
 *  @param[in] type This is a host or service comment.
 */
retention::comment::comment(type_id comment_type)
  : object(object::comment),
    _comment_id(0),
    _comment_type(comment_type),
    _entry_type(USER_COMMENT),
    _expire_time(0),
    _expires(false),
    _persistent(false),
    _source(COMMENTSOURCE_INTERNAL) {

}

/**
 *  Destructor.
 */
retention::comment::~comment() throw () {
  _finished();
}

/**
 *  Set new value on specific property.
 *
 *  @param[in] key   The property to set.
 *  @param[in] value The new value.
 *
 *  @return True on success, otherwise false.
 */
bool retention::comment::set(
       std::string const& key,
       std::string const& value) {
  if (key == "host_name")
    _host_name = value;
  else if (key == "service_description")
    _service_description = value;
  else if (key == "entry_type")
    misc::to(value, _entry_type);
  else if (key == "comment_id")
    misc::to(value, _comment_id);
  else if (key == "source")
    misc::to(value, _source);
  else if (key == "persistent")
    misc::to(value, _persistent);
  else if (key == "entry_time")
    misc::to(value, _entry_time);
  else if (key == "expires")
    misc::to(value, _expires);
  else if (key == "expire_time")
    misc::to(value, _expire_time);
  else if (key == "author")
    _author = value;
  else if (key == "comment_data")
    _comment_data = value;
  else
    return (false);
  return (true);
}

/**
 *  Add host comment.
 */
void retention::comment::_add_host_comment() throw () {
  umap<std::string, shared_ptr<host_struct> >::const_iterator
    it(state::instance().hosts().find(_host_name));
  if (it == state::instance().hosts().end())
    return;
  host_struct* hst(&*it->second);

  // add the comment.
  add_comment(
    HOST_COMMENT,
    _entry_type,
    _host_name.c_str(),
    NULL,
    _entry_time,
    _author.c_str(),
    _comment_data.c_str(),
    _comment_id,
    _persistent,
    _expires,
    _expire_time,
    _source);

  // acknowledgement comments get deleted if they're not persistent
  // and the original problem is no longer acknowledged.
  if (_entry_type == ACKNOWLEDGEMENT_COMMENT) {
    if (!hst->problem_has_been_acknowledged && !_persistent)
      delete_comment(HOST_COMMENT, _comment_id);
  }
  // non-persistent comments don't last past restarts UNLESS
  // they're acks (see above).
  else if (!_persistent)
    delete_comment(HOST_COMMENT, _comment_id);
}

/**
 *  Add serivce comment.
 */
void retention::comment::_add_service_comment() throw () {
  umap<std::string, shared_ptr<host_struct> >::const_iterator
    it_hst(state::instance().hosts().find(_host_name));
  if (it_hst == state::instance().hosts().end())
    return;
  host_struct* hst(&*it_hst->second);
  if (!hst)
    return;

  umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >::const_iterator
    it_svc(state::instance().services().find(std::make_pair(_host_name, _service_description)));
  if (it_svc == state::instance().services().end())
    return;
  service_struct* svc(&*it_svc->second);

  // add the comment.
  add_comment(
    SERVICE_COMMENT,
    _entry_type,
    _host_name.c_str(),
    _service_description.c_str(),
    _entry_time,
    _author.c_str(),
    _comment_data.c_str(),
    _comment_id,
    _persistent,
    _expires,
    _expire_time,
    _source);

  // acknowledgement comments get deleted if they're not persistent
  // and the original problem is no longer acknowledged.
  if (_entry_type == ACKNOWLEDGEMENT_COMMENT) {
    if (!svc->problem_has_been_acknowledged && !_persistent)
      delete_comment(SERVICE_COMMENT, _comment_id);
  }
  // non-persistent comments don't last past restarts UNLESS
  // they're acks (see above).
  else if (!_persistent)
    delete_comment(SERVICE_COMMENT, _comment_id);
}

/**
 *  Finish all comment update.
 */
void retention::comment::_finished() throw () {
  if (_comment_type == host)
    _add_host_comment();
  else
    _add_service_comment();
}

