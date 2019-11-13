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

#ifndef CCE_RETENTION_STATE_HH
#define CCE_RETENTION_STATE_HH

#include "com/centreon/engine/namespace.hh"
#include "com/centreon/engine/retention/comment.hh"
#include "com/centreon/engine/retention/contact.hh"
#include "com/centreon/engine/retention/downtime.hh"
#include "com/centreon/engine/retention/host.hh"
#include "com/centreon/engine/retention/info.hh"
#include "com/centreon/engine/retention/program.hh"
#include "com/centreon/engine/retention/service.hh"

CCE_BEGIN()

namespace retention {
class state {
 public:
  state();
  ~state() throw();
  state(state const& right);
  state& operator=(state const& right);
  bool operator==(state const& right) const throw();
  bool operator!=(state const& right) const throw();
  list_comment& comments() throw();
  list_comment const& comments() const throw();
  list_contact& contacts() throw();
  list_contact const& contacts() const throw();
  list_downtime& downtimes() throw();
  list_downtime const& downtimes() const throw();
  program& globals() throw();
  program const& globals() const throw();
  list_host& hosts() throw();
  list_host const& hosts() const throw();
  info& informations() throw();
  info const& informations() const throw();
  list_service& services() throw();
  list_service const& services() const throw();

 private:
  list_comment _comments;
  list_contact _contacts;
  list_downtime _downtimes;
  list_host _hosts;
  info _info;
  program _globals;
  list_service _services;
};
}  // namespace retention

CCE_END()

#endif  // !CCE_RETENTION_STATE_HH
