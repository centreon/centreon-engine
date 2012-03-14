/*
** Copyright 2002-2006 Ethan Galstad
** Copyright 2011-2012 Merethis
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

#ifndef CCE_SRETENTION_HH
#  define CCE_SRETENTION_HH

#  ifdef __cplusplus
extern "C" {
#  endif // C++

int initialize_retention_data(char* config_file);
int cleanup_retention_data(char* config_file);
int save_state_information(int autosave); // saves all host and state information
int read_initial_state_information(void); // reads in initial host and state information

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_SRETENTION_HH
