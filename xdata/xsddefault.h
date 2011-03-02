/*****************************************************************************
 *
 * XSDDEFAULT.H - Header file for default status data routines
 *
 * Copyright (c) 1999-2006 Ethan Galstad (egalstad@nagios.org)
 * Last Modified:   03-01-2006
 *
 * License:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *****************************************************************************/

#ifndef _XSDDEFAULT_H
#define _XSDDEFAULT_H

int xsddefault_initialize_status_data(char *);
int xsddefault_cleanup_status_data(char *,int);
int xsddefault_save_status_data(void);

int xsddefault_grab_config_info(char *);
int xsddefault_grab_config_directives(char *);

#endif
