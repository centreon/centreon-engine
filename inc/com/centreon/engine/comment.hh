/*
** Copyright 1999-2010 Ethan Galstad
** Copyright 2011-2019 Centreon
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

#ifndef CCE_OBJECTS_COMMENTS_HH
#  define CCE_OBJECTS_COMMENTS_HH
#  include <map>
#  include <ostream>
#  include <time.h>
#  include "com/centreon/engine/contact.hh"
#  include "com/centreon/engine/host.hh"

CCE_BEGIN()
class comment;
class service;
CCE_END()

typedef std::map<uint64_t,
  std::shared_ptr<com::centreon::engine::comment>> comment_map;


CCE_BEGIN()
// COMMENT structure
class             comment{
 public:

  enum            src {
    internal,
    external
  };

  enum            type {
    host = 1,
    service
  };

  enum            e_type {
    user = 1,
    downtime,
    flapping,
    acknowledgment
  };

                     comment(comment::type comment_type,
                             comment::e_type entry_type,
                             std::string const& host_name,
                             std::string const& service_description,
                             time_t entry_time,
                             std::string const& author,
                             std::string const& comment_data,
                             bool persistent,
                             comment::src source,
                             bool expires,
                             time_t expire_time,
                             uint64_t comment_id = 0);

  comment::type      get_comment_type() const;
  comment::e_type    get_entry_type() const;
  uint64_t           get_comment_id() const;
  comment::src       get_source() const;
  bool               get_persistent() const;
  time_t 	           get_entry_time() const;
  bool               get_expires() const;
  time_t             get_expire_time() const;
  std::string const& get_host_name() const;
  std::string const& get_service_description() const;
  std::string const& get_author() const;
  std::string const& get_comment_data() const;

  bool               operator==(com::centreon::engine::comment const& obj) throw ();
  bool               operator!=(com::centreon::engine::comment const& obj) throw ();

  static uint64_t    get_next_comment_id();
  static void        set_next_comment_id(uint64_t comment_id);
  static void        delete_comment(uint64_t comment_id);
  static void        delete_host_comments(std::string const& host_name);
  static void        delete_service_comments(std::string const& host_name,
                       const std::string& svc_description);
  static void        delete_host_acknowledgement_comments(engine::host* hst);
  static void        delete_service_acknowledgement_comments(engine::service* svc);
  static void        remove_if_expired_comment(uint64_t comment_id);
  static comment_map comments;

 private:
  comment::type      _comment_type;
  comment::e_type    _entry_type;
  uint64_t           _comment_id;
  comment::src       _source;
  bool               _persistent;
  time_t 	           _entry_time;
  bool               _expires;
  time_t             _expire_time;
  std::string        _host_name;
  std::string        _service_description;
  std::string        _author;
  std::string        _comment_data;

  static uint64_t    _next_comment_id;
};

CCE_END()

std::ostream&     operator<<(std::ostream& os, com::centreon::engine::comment const& obj);

#endif // !CCE_OBJECTS_COMMENTS_HH
