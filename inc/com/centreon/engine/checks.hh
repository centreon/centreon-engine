/*
** Copyright 2002-2006 Ethan Galstad
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

#ifndef CCE_CHECKS_HH
#define CCE_CHECKS_HH

#include <sys/time.h>
#include <cstdio>
#include "com/centreon/engine/checkable.hh"
#include "com/centreon/engine/namespace.hh"
#include "com/centreon/engine/notifier.hh"

enum check_source { service_check, host_check };
CCE_BEGIN()
class check_result;
CCE_END()

typedef std::list<com::centreon::engine::check_result> check_result_list;

// CHECK_RESULT structure
CCE_BEGIN()
class check_result {
 public:
  check_result();
  check_result(enum check_source object_check_type,
               uint64_t host_id,
               uint64_t service_id,
               enum checkable::check_type check_type,
               int check_options,
               bool reschedule_check,
               double latency,
               struct timeval start_time,
               struct timeval finish_time,
               bool early_timeout,
               bool exited_ok,
               int return_code,
               std::string const& output);
  check_result(check_result const& other);
  check_result(check_result&& other);
  check_result& operator=(check_result const& other);

  enum check_source get_object_check_type() const;
  void set_object_check_type(enum check_source object_check_type);
  uint64_t get_host_id() const;
  void set_host_id(uint64_t host_id);
  uint64_t get_service_id() const;
  void set_service_id(uint64_t service_id);
  struct timeval get_finish_time() const;
  void set_finish_time(struct timeval finish_time);
  struct timeval get_start_time() const;
  void set_start_time(struct timeval start_time);
  int get_return_code() const;
  void set_return_code(int return_code);
  bool get_early_timeout() const;
  void set_early_timeout(bool early_timeout);
  std::string const& get_output() const;
  void set_output(std::string const& output);
  bool get_exited_ok() const;
  void set_exited_ok(bool exited_ok);
  bool get_reschedule_check() const;
  void set_reschedule_check(bool reschedule_check);
  enum checkable::check_type get_check_type() const;
  void set_check_type(enum checkable::check_type check_type);
  double get_latency() const;
  void set_latency(double latency);
  int get_check_options() const;
  void set_check_options(int check_options);

  static bool process_check_result_queue(std::string const& dirname);
  static bool process_check_result_file(std::string const& fname);
  static check_result_list results;

 private:
  enum check_source _object_check_type;  // is this a service or a host check?
  uint64_t _host_id;
  uint64_t _service_id;
  // was this an active or passive service check?
  enum checkable::check_type _check_type;
  int _check_options;
  bool _reschedule_check;  // should we reschedule the next check
  double _latency;
  struct timeval _start_time;   // time the service check was initiated
  struct timeval _finish_time;  // time the service check was completed
  bool _early_timeout;          // did the service check timeout?
  bool _exited_ok;              // did the plugin check return okay?
  int _return_code;             // plugin return code
  std::string _output;          // plugin output
};
CCE_END()

#endif  // !CCE_CHECKS_HH
