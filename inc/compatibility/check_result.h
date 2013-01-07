/*
** Copyright 2012-2013 Merethis
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

#ifndef CCE_COMPATIBILITY_CHECK_RESULT_H
#  define CCE_COMPATIBILITY_CHECK_RESULT_H

#  include "com/centreon/engine/checks.hh"

#  ifdef __cplusplus
extern "C" {
#  endif // C++

int add_check_result_to_list(check_result* new_cr);
int delete_check_result_file(char const* fname);
int free_check_result_list();
int init_check_result(check_result* info);
int move_check_result_to_queue(char const* checkresult_file);
int process_check_result_queue(char const* dirname);
int process_check_result_file(char const* fname);
check_result* read_check_result();

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_COMPATIBILITY_CHECK_RESULT_H
