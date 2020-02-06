/*
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

#include "com/centreon/engine/checks.hh"
#include <dirent.h>
#include <fcntl.h>
#include <mmap.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <utility>
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"

using namespace com::centreon::engine;

check_result_list check_result::results;

check_result::check_result()
    : _object_check_type{host_check},
      _host_id{0UL},
      _service_id{0UL},
      _check_type(checkable::check_active),
      _check_options{CHECK_OPTION_NONE},
      _reschedule_check{false},
      _latency{0.0},
      _early_timeout{false},
      _exited_ok{false},
      _return_code{0},
      _output{""} {
  timeval tv{0, 0};
  _start_time = tv;
  _finish_time = tv;
}

check_result::check_result(check_result const& other)
    : _object_check_type{other._object_check_type},
      _host_id{other._host_id},
      _service_id{other._service_id},
      _check_type{other._check_type},
      _check_options{other._check_options},
      _reschedule_check{other._reschedule_check},
      _latency{other._latency},
      _start_time(other._start_time),
      _finish_time(other._finish_time),
      _early_timeout{other._early_timeout},
      _exited_ok{other._exited_ok},
      _return_code{other._return_code},
      _output{other._output} {}

check_result& check_result::operator=(check_result const& other) {
  if (this != &other) {
    _object_check_type = other._object_check_type;
    _host_id = other._host_id;
    _service_id = other._service_id;
    _check_type = other._check_type;
    _check_options = other._check_options;
    _reschedule_check = other._reschedule_check;
    _latency = other._latency;
    _start_time = other._start_time;
    _finish_time = other._finish_time;
    _early_timeout = other._early_timeout;
    _exited_ok = other._exited_ok;
    _return_code = other._return_code;
    _output = other._output;
  }
  return *this;
}

check_result::check_result(check_result&& other)
    : _object_check_type{other._object_check_type},
      _host_id{other._host_id},
      _service_id{other._service_id},
      _check_type{other._check_type},
      _check_options{other._check_options},
      _reschedule_check{other._reschedule_check},
      _latency{other._latency},
      _start_time(other._start_time),
      _finish_time(other._finish_time),
      _early_timeout{other._early_timeout},
      _exited_ok{other._exited_ok},
      _return_code{other._return_code},
      _output{std::move(other._output)} {}

check_result::check_result(enum check_source object_check_type,
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
                           std::string const& output)
    : _object_check_type{object_check_type},
      _host_id{host_id},
      _service_id{service_id},
      _check_type(check_type),
      _check_options{check_options},
      _reschedule_check{reschedule_check},
      _latency{latency},
      _start_time(start_time),
      _finish_time(finish_time),
      _early_timeout{early_timeout},
      _exited_ok{exited_ok},
      _return_code{return_code},
      _output{output} {}

enum check_source check_result::get_object_check_type() const {
  return _object_check_type;
}

void check_result::set_object_check_type(enum check_source object_check_type) {
  _object_check_type = object_check_type;
}

uint64_t check_result::get_host_id() const {
  return _host_id;
}

void check_result::set_host_id(uint64_t host_id) {
  _host_id = host_id;
}

uint64_t check_result::get_service_id() const {
  return _service_id;
}

void check_result::set_service_id(uint64_t service_id) {
  _service_id = service_id;
}

struct timeval check_result::get_finish_time() const {
  return _finish_time;
}

void check_result::set_finish_time(struct timeval finish_time) {
  _finish_time = finish_time;
}

struct timeval check_result::get_start_time() const {
  return _start_time;
}

void check_result::set_start_time(struct timeval start_time) {
  _start_time = start_time;
}

int check_result::get_return_code() const {
  return _return_code;
}

void check_result::set_return_code(int return_code) {
  _return_code = return_code;
}

bool check_result::get_early_timeout() const {
  return _early_timeout;
}

void check_result::set_early_timeout(bool early_timeout) {
  _early_timeout = early_timeout;
}

std::string const& check_result::get_output() const {
  return _output;
}

void check_result::set_output(std::string const& output) {
  _output = output;
}

bool check_result::get_exited_ok() const {
  return _exited_ok;
}

void check_result::set_exited_ok(bool exited_ok) {
  _exited_ok = exited_ok;
}

bool check_result::get_reschedule_check() const {
  return _reschedule_check;
}

void check_result::set_reschedule_check(bool reschedule_check) {
  _reschedule_check = reschedule_check;
}

enum checkable::check_type check_result::get_check_type() const {
  return _check_type;
}

void check_result::set_check_type(enum checkable::check_type check_type) {
  _check_type = check_type;
}

double check_result::get_latency() const {
  return _latency;
}

void check_result::set_latency(double latency) {
  _latency = latency;
}

int check_result::get_check_options() const {
  return _check_options;
}

void check_result::set_check_options(int check_options) {
  _check_options = check_options;
}
