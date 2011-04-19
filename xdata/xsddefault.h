/*
** Copyright 1999-2006 Ethan Galstad
** Copyright 2011      Merethis
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

#ifndef XSDDEFAULT_H
# define XSDDEFAULT_H

int xsddefault_initialize_status_data(char *);
int xsddefault_cleanup_status_data(char *,int);
int xsddefault_save_status_data(void);

int xsddefault_grab_config_info(char *);
int xsddefault_grab_config_directives(char *);

#endif /* !XSDDEFAULT_H */
