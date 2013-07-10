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
#include "com/centreon/engine/retention/applier/comment.hh"

using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::retention;
using namespace com::centreon::engine;

/**
 *  Constructor.
 */
applier::comment::comment() {

}

/**
 *  Destructor.
 */
applier::comment::~comment() throw () {

}

/**
 *  Add comments on appropriate hosts and services.
 *
 *  @param[in] lst The comment list to add.
 */
void applier::comment::apply(std::list<retention::comment> const& lst) {
  for (std::list<retention::comment>::const_iterator
         it(lst.begin()), end(lst.end());
       it != end;
       ++it) {
    if (it->comment_type() == retention::comment::host)
      _add_host_comment(*it);
    else
      _add_service_comment(*it);
  }
}

/**
 *  Add host comment.
 *
 *  @param[in] obj The comment to add into the host.
 */
void applier::comment::_add_host_comment(
       retention::comment const& obj) throw () {
  umap<std::string, shared_ptr<host_struct> >::const_iterator
    it(state::instance().hosts().find(obj.host_name()));
  if (it == state::instance().hosts().end())
    return;
  host_struct* hst(it->second.get());

  // add the comment.
  add_comment(
    HOST_COMMENT,
    obj.entry_type(),
    obj.host_name().c_str(),
    NULL,
    obj.entry_time(),
    obj.author().c_str(),
    obj.comment_data().c_str(),
    obj.comment_id(),
    obj.persistent(),
    obj.expires(),
    obj.expire_time(),
    obj.source());

  // acknowledgement comments get deleted if they're not persistent
  // and the original problem is no longer acknowledged.
  if (obj.entry_type() == ACKNOWLEDGEMENT_COMMENT) {
    if (!hst->problem_has_been_acknowledged && !obj.persistent())
      delete_comment(HOST_COMMENT, obj.comment_id());
  }
  // non-persistent comments don't last past restarts UNLESS
  // they're acks (see above).
  else if (!obj.persistent())
    delete_comment(HOST_COMMENT, obj.comment_id());
}

/**
 *  Add serivce comment.
*
 *  @param[in] obj The comment to add into the service.
 */
void applier::comment::_add_service_comment(
       retention::comment const& obj) throw () {
  umap<std::string, shared_ptr<host_struct> >::const_iterator
    it_hst(state::instance().hosts().find(obj.host_name()));
  if (it_hst == state::instance().hosts().end())
    return ;
  host_struct* hst(it_hst->second.get());
  if (!hst)
    return ;

  umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >::const_iterator
    it_svc(state::instance().services().find(std::make_pair(obj.host_name(), obj.service_description())));
  if (it_svc == state::instance().services().end())
    return;
  service_struct* svc(&*it_svc->second);

  // add the comment.
  add_comment(
    SERVICE_COMMENT,
    obj.entry_type(),
    obj.host_name().c_str(),
    obj.service_description().c_str(),
    obj.entry_time(),
    obj.author().c_str(),
    obj.comment_data().c_str(),
    obj.comment_id(),
    obj.persistent(),
    obj.expires(),
    obj.expire_time(),
    obj.source());

  // acknowledgement comments get deleted if they're not persistent
  // and the original problem is no longer acknowledged.
  if (obj.entry_type() == ACKNOWLEDGEMENT_COMMENT) {
    if (!svc->problem_has_been_acknowledged && !obj.persistent())
      delete_comment(SERVICE_COMMENT, obj.comment_id());
  }
  // non-persistent comments don't last past restarts UNLESS
  // they're acks (see above).
  else if (!obj.persistent())
    delete_comment(SERVICE_COMMENT, obj.comment_id());
}
