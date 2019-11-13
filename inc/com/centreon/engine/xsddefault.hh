/*
** Copyright 1999-2006 Ethan Galstad
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

#ifndef CCE_XSDDEFAULT_HH
#define CCE_XSDDEFAULT_HH

#ifdef __cplusplus
extern "C" {
#endif  // C++

int xsddefault_initialize_status_data();
int xsddefault_cleanup_status_data(int delete_status_data);
int xsddefault_save_status_data();

#ifdef __cplusplus
}
#endif  // C++

#endif  // !CCE_XSDDEFAULT_HH
