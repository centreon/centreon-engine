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

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/comment.hh"
#include "com/centreon/engine/events/defines.hh"

using namespace com::centreon::engine;

comment_map comment::comments;
uint64_t    comment::_next_comment_id = 1LLU;

uint64_t comment::get_next_comment_id() {
  return _next_comment_id;
}

void comment::set_next_comment_id(uint64_t next_comment_id) {
  _next_comment_id = next_comment_id;
}

comment::comment(comment::type comment_type,
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
                 uint64_t comment_id)
  : _comment_type{comment_type},
    _entry_type{entry_type},
    _comment_id{comment_id},
    _source{source},
    _persistent{persistent},
    _entry_time{entry_time},
    _expires{expires},
    _expire_time{expire_time},
    _host_name{host_name},
    _service_description{service_description},
    _author{author},
    _comment_data{comment_data}
{
  bool is_added = true;

  if (!comment_id) {
    _comment_id = _next_comment_id;
    _next_comment_id = 1llu;
    while(comments.find(_next_comment_id) != comments.end() || _next_comment_id == _comment_id)
      _next_comment_id++;
  } else {
    is_added = false;
  }

  broker_comment_data(
    NEBTYPE_COMMENT_LOAD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    _comment_type,
    _entry_type,
    _host_name.c_str(),
    comment_type == comment::service ? _service_description.c_str() : nullptr,
    _entry_time,
    _author.c_str(),
    _comment_data.c_str(),
    _persistent,
    _source,
    _expires,
    _expire_time,
    _comment_id,
    nullptr);

  /* send data to event broker */
  if (is_added)
    broker_comment_data(
      NEBTYPE_COMMENT_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      _comment_type,
      _entry_type,
      _host_name.c_str(),
      comment_type == comment::service ? _service_description.c_str() : nullptr,
      _entry_time,
      _author.c_str(),
      _comment_data.c_str(),
      _persistent,
      _source,
      _expires,
      _expire_time,
      _comment_id,
      nullptr);
}

/* deletes a host or service comment */
void comment::delete_comment(uint64_t comment_id) {
  comment_map::iterator found = comment::comments.find(comment_id);

  if (found != comment::comments.end() && found->second) {
    broker_comment_data(
      NEBTYPE_COMMENT_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      found->second->get_comment_type(),
      found->second->get_entry_type(),
      found->second->get_host_name().c_str(),
      found->second->get_comment_type() == comment::service ? found->second->get_service_description().c_str() : nullptr,
      found->second->get_entry_time(),
      found->second->get_author().c_str(),
      found->second->get_comment_data().c_str(),
      found->second->get_persistent(),
      found->second->get_source(),
      found->second->get_expires(),
      found->second->get_expire_time(),
      comment_id,
      nullptr);
    comment::comments.erase(comment_id);
  }
}

void comment::delete_host_comments(std::string const& host_name) {
  comment_map::iterator it(comments.begin());

  while (it != comments.end()) {
    if (it->second->get_comment_type() == comment::host&&
        it->second->get_host_name() == host_name ) {
      broker_comment_data(
        NEBTYPE_COMMENT_DELETE,
        NEBFLAG_NONE,
        NEBATTR_NONE,
        it->second->get_comment_type(),
        it->second->get_entry_type(),
        host_name.c_str(),
        nullptr,
        it->second->get_entry_time(),
        it->second->get_author().c_str(),
        it->second->get_comment_data().c_str(),
        it->second->get_persistent(),
        it->second->get_source(),
        it->second->get_expires(),
        it->second->get_expire_time(),
        it->first,
        nullptr);
      it = comments.erase(it);
    }
    else
      it++;
  }
}

void comment::delete_service_comments(
  std::string const& host_name,
  std::string const& svc_description) {
  comment_map::iterator it(comments.begin());

  while (it != comments.end()) {
    if (it->second->get_comment_type() == comment::service
        && it->second->get_host_name() == host_name
        && it->second->get_service_description() == svc_description) {
      broker_comment_data(
        NEBTYPE_COMMENT_DELETE,
        NEBFLAG_NONE,
        NEBATTR_NONE,
        it->second->get_comment_type(),
        it->second->get_entry_type(),
        host_name.c_str(),
        it->second->get_service_description().c_str(),
        it->second->get_entry_time(),
        it->second->get_author().c_str(),
        it->second->get_comment_data().c_str(),
        it->second->get_persistent(),
        it->second->get_source(),
        it->second->get_expires(),
        it->second->get_expire_time(),
        it->first,
        nullptr);
      it = comments.erase(it);
    }
    else
      it++;
  }
}

/* deletes all non-persistent acknowledgement comments for a particular host */
void comment::delete_host_acknowledgement_comments(engine::host* hst) {
  comment_map::iterator it(comments.begin());

  while (it != comments.end()) {
    if (it->second->get_comment_type() == comment::host
        && it->second->get_host_name() == hst->get_name()
        && it->second->get_entry_type() == com::centreon::engine::comment::acknowledgment
        && !it->second->get_persistent()) {
      broker_comment_data(
        NEBTYPE_COMMENT_DELETE,
        NEBFLAG_NONE,
        NEBATTR_NONE,
        it->second->get_comment_type(),
        it->second->get_entry_type(),
        it->second->get_host_name().c_str(),
        nullptr,
        it->second->get_entry_time(),
        it->second->get_author().c_str(),
        it->second->get_comment_data().c_str(),
        it->second->get_persistent(),
        it->second->get_source(),
        it->second->get_expires(),
        it->second->get_expire_time(),
        it->first,
        nullptr);
      it = comments.erase(it);
    }
    else
      it++;
  }
}


/* deletes all non-persistent acknowledgement comments for a particular service */
void comment::delete_service_acknowledgement_comments(::service* svc) {
    comment_map::iterator it(comments.begin());

    while (it != comments.end()) {
      if (it->second->get_comment_type() == comment::service
          && it->second->get_host_name() == svc->get_hostname()
          && it->second->get_service_description() == svc->get_description()
          && it->second->get_entry_type() == com::centreon::engine::comment::acknowledgment
          && !it->second->get_persistent()) {
        broker_comment_data(
          NEBTYPE_COMMENT_DELETE,
          NEBFLAG_NONE,
          NEBATTR_NONE,
          it->second->get_comment_type(),
          it->second->get_entry_type(),
          it->second->get_host_name().c_str(),
          it->second->get_service_description().c_str(),
          it->second->get_entry_time(),
          it->second->get_author().c_str(),
          it->second->get_comment_data().c_str(),
          it->second->get_persistent(),
          it->second->get_source(),
          it->second->get_expires(),
          it->second->get_expire_time(),
          it->first,
          nullptr);
        it = comments.erase(it);
      }
      else
        it++;
    }
}

/* checks for an expired comment (and removes it) */
void comment::remove_if_expired_comment(uint64_t comment_id) {
  comment_map::iterator found = comment::comments.find(comment_id);

  if (found != comment::comments.end()
      && found->second->get_expires()
      && found->second->get_expire_time() < time(nullptr))
    delete_comment(comment_id);
}

comment::type comment::get_comment_type() const {
  return _comment_type;
}

comment::e_type comment::get_entry_type() const {
  return _entry_type;
}

uint64_t comment::get_comment_id() const {
  return _comment_id;
}

comment::src comment::get_source() const {
  return _source;
}

bool comment::get_persistent() const {
  return _persistent;
}

time_t comment::get_entry_time() const {
  return _entry_time;
}

bool comment::get_expires() const {
  return _expires;
}

time_t comment::get_expire_time() const {
  return _expire_time;
}

std::string const& comment::get_host_name() const {
  return _host_name;
}

std::string const& comment::get_service_description() const {
  return _service_description;
}

std::string const& comment::get_author() const {
  return _author;
}

std::string const& comment::get_comment_data() const {
  return _comment_data;
}

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool comment::operator==(comment const& obj) throw () {
  return (_comment_type == obj.get_comment_type()
          && _entry_type == obj.get_entry_type()
          && _comment_id == obj.get_comment_id()
          && _source == obj.get_source()
          && _persistent == obj.get_persistent()
          && _entry_time == obj.get_entry_time()
          && _expires == obj.get_expires()
          && _expire_time == obj.get_expire_time()
          && _host_name == obj.get_host_name()
          && _service_description == obj.get_service_description()
          && _author == obj.get_author()
          && _comment_data == obj.get_comment_data());
}

/**
 *  Not equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool comment::operator!=(comment const& obj) throw () {
  return !(*this == obj);
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
    "  comment_type:        " << obj.get_comment_type() << "\n"
    "  entry_type:          " << obj.get_entry_type() << "\n"
    "  comment_id:          " << obj.get_comment_id() << "\n"
    "  source:              " << obj.get_source() << "\n"
    "  persistent:          " << obj.get_persistent() << "\n"
    "  entry_time:          " << obj.get_entry_time() << "\n"
    "  expires:             " << obj.get_expires() << "\n"
    "  expire_time:         " << obj.get_expire_time() << "\n"
    "  host_name:           " << obj.get_host_name() << "\n"
    "  service_description: " << obj.get_service_description() << "\n"
    "  author:              " << obj.get_author() << "\n"
    "  comment_data:        " << obj.get_comment_data() << "\n"
    "}\n";
  return (os);
}
