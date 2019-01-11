/*
** Copyright 2011-2014 Merethis
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

#ifndef CCE_OBJECTS_CUSTOMVARIABLESMEMBER_HH
#  define CCE_OBJECTS_CUSTOMVARIABLESMEMBER_HH
#  include "com/centreon/engine/objects/customvariable.hh"

/* Forward declarations. */
struct contact_struct;
struct host_struct;
struct service_struct;

typedef struct                         customvariablesmember_struct {
  char*                                variable_name;
  char*                                variable_value;
  int                                  has_been_modified;
  bool                                 is_sent;
  struct customvariablesmember_struct* next;
}                                      customvariablesmember;

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

customvariablesmember* add_custom_variable_to_contact(
                         contact_struct* cntct,
                         char const* varname,
                         com::centreon::engine::customvariable const& cv);
customvariablesmember* add_custom_variable_to_host(
                         host_struct* hst,
                         char const* varname,
                         com::centreon::engine::customvariable const& cv);
customvariablesmember* add_custom_variable_to_object(
                         customvariablesmember** object_ptr,
                         char const* varname,
                         char const* varvalue,
                         bool is_sent);
customvariablesmember* add_custom_variable_to_service(
                         service_struct* svc,
                         char const* varname,
                         com::centreon::engine::customvariable const& cv);
void                   remove_all_custom_variables_from_contact(
                         contact_struct* cntct);
void                   remove_all_custom_variables_from_host(
                         host_struct* hst);
void                   remove_all_custom_variables_from_service(
                         service_struct* svc);

#  ifdef __cplusplus
}

#    include <ostream>
#    include <string>
#    include "com/centreon/engine/namespace.hh"

bool          operator==(
                customvariablesmember const& obj1,
                customvariablesmember const& obj2) throw ();
bool          operator!=(
                customvariablesmember const& obj1,
                customvariablesmember const& obj2) throw ();
bool          operator<(
                customvariablesmember const& obj1,
                customvariablesmember const& obj2) throw ();
std::ostream& operator<<(
                std::ostream& os,
                customvariablesmember const& obj);

CCE_BEGIN()

bool          update_customvariable(
                customvariablesmember* lst,
                std::string const& key,
                std::string const& value);

CCE_END()

#  endif /* C++ */

#endif // !CCE_OBJECTS_CUSTOMVARIABLESMEMBER_HH


