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

#ifndef CCE_RETENTION_COMMENT_HH
#define CCE_RETENTION_COMMENT_HH

#include <list>
#include <string>
#include "com/centreon/engine/namespace.hh"
#include "com/centreon/engine/retention/object.hh"

CCE_BEGIN()

namespace retention {
class comment : public object {
 public:
  enum type_id { host = 0, service = 1 };

  comment(type_id type);
  comment(comment const& right);
  ~comment() throw() override;
  comment& operator=(comment const& right);
  bool operator==(comment const& right) const throw();
  bool operator!=(comment const& right) const throw();
  bool set(char const* key, char const* value) override;

  std::string const& author() const throw();
  std::string const& comment_data() const throw();
  unsigned long comment_id() const throw();
  type_id comment_type() const throw();
  time_t entry_time() const throw();
  unsigned int entry_type() const throw();
  time_t expire_time() const throw();
  bool expires() const throw();
  std::string const& host_name() const throw();
  bool persistent() const throw();
  std::string const& service_description() const throw();
  int source() const throw();

 private:
  struct setters {
    char const* name;
    bool (*func)(retention::comment&, char const*);
  };

  bool _set_author(std::string const& value);
  bool _set_comment_data(std::string const& value);
  bool _set_comment_id(unsigned long value);
  bool _set_entry_time(time_t value);
  bool _set_entry_type(unsigned int value);
  bool _set_expire_time(time_t value);
  bool _set_expires(bool value);
  bool _set_host_name(std::string const& value);
  bool _set_persistent(bool value);
  bool _set_service_description(std::string const& value);
  bool _set_source(int value);

  std::string _author;
  std::string _comment_data;
  unsigned long _comment_id;
  type_id _comment_type;
  time_t _entry_time;
  unsigned int _entry_type;
  time_t _expire_time;
  bool _expires;
  std::string _host_name;
  bool _persistent;
  std::string _service_description;
  static setters const _setters[];
  int _source;
};

typedef std::shared_ptr<comment> comment_ptr;
typedef std::list<comment_ptr> list_comment;
}  // namespace retention

CCE_END()

#endif  // !CCE_RETENTION_COMMENT_HH
