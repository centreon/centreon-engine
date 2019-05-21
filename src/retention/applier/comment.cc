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

#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/objects/comment.hh"
#include "com/centreon/engine/retention/applier/comment.hh"

using namespace com::centreon::engine::retention;
using namespace com::centreon::engine;

/**
 *  Add comments on appropriate hosts and services.
 *
 *  @param[in] lst The comment list to add.
 */
void applier::comment::apply(list_comment const& lst) {
  // Big speedup when reading retention.dat in bulk.
  defer_comment_sorting = 1;

  for (list_comment::const_iterator it(lst.begin()), end(lst.end());
       it != end;
       ++it) {
    if ((*it)->comment_type() == retention::comment::host)
      _add_host_comment(**it);
    else
      _add_service_comment(**it);
  }

  // Sort all comments.
  sort_comments();
}

/**
 *  Add host comment.
 *
 *  @param[in] obj The comment to add into the host.
 */
void applier::comment::_add_host_comment(
       retention::comment const& obj) throw () {
  umap<unsigned long,
       std::shared_ptr<com::centreon::engine::host>>::const_iterator
    it(configuration::applier::state::instance().hosts().find(get_host_id(obj.host_name().c_str())));
  if (it == configuration::applier::state::instance().hosts().end())
    return;
  com::centreon::engine::host* hst(it->second.get());

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
    if (!hst->get_problem_has_been_acknowledged() && !obj.persistent())
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
  if (!is_host_exist(get_host_id(obj.host_name().c_str())))
    return;

  std::pair<unsigned int, unsigned int>
    id(get_host_and_service_id(obj.host_name().c_str(), obj.service_description().c_str()));
  umap<std::pair<unsigned long, unsigned long>,
       std::shared_ptr<service_struct> >::const_iterator
    it_svc(configuration::applier::state::instance().services().find(id));
  if (it_svc == configuration::applier::state::instance().services().end())
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
