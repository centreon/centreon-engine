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
//#  include "com/centreon/engine/service.hh"

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
    host,
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
  void               set_comment_type(comment::type comment_type);
  comment::e_type    get_entry_type() const;
  void               set_entry_type(comment::e_type entry_type);
  uint64_t           get_comment_id() const;
  void               set_comment_id(uint64_t comment_id);
  comment::src       get_source() const;
  void               set_source(comment::src source);
  bool               get_persistent() const;
  void               set_persistent(bool persistent);
  time_t 	           get_entry_time() const;
  void               set_entry_time(time_t entry_time);
  bool               get_expires() const;
  void               set_expires(bool expires);
  time_t             get_expire_time() const;
  void               set_expire_time(time_t expire_time);
  std::string const& get_host_name() const;
  void               set_host_name(std::string const& host_name);
  std::string const& get_service_description() const;
  void               set_service_description(std::string const& service_description);
  std::string const& get_author() const;
  void               set_author(std::string const& author);
  std::string const& get_comment_data() const;
  void               set_comment_data(std::string const& comment_data);

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
  static int         number_of_host_comments(std::string const& host_name);
  static int         number_of_service_comments(
                       std::string const& host_name,
                       std::string const& svc_description);
  static comment_map comments;

  comment*           nexthash;

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
