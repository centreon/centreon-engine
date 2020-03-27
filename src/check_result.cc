/*
** Copyright 2011-2020 Centreon
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

#include "com/centreon/engine/check_result.hh"
#include <string>
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"

using namespace com::centreon::engine;

check_result::check_result(enum check_source object_check_type,
                           notifier* notifier,
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
      _notifier{notifier},
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

notifier* check_result::get_notifier() {
  return _notifier;
}

void check_result::set_notifier(notifier* notifier) {
  _notifier = notifier;
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
