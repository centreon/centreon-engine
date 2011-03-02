/*****************************************************************************
 *
 * STATUSDATA.H - Header for external status data routines
 *
 * Copyright (c) 2000-2007 Ethan Galstad (egalstad@nagios.org)
 * Last Modified:   10-19-2007
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

#ifndef _STATUSDATA_H
#define _STATUSDATA_H

#include "objects.h"

#ifdef __cplusplus
  extern "C" {
#endif

int initialize_status_data(char *);                     /* initializes status data at program start */
int update_all_status_data(void);                       /* updates all status data */
int cleanup_status_data(char *,int);                    /* cleans up status data at program termination */
int update_program_status(int);                         /* updates program status data */
int update_host_status(host *,int);                     /* updates host status data */
int update_service_status(service *,int);               /* updates service status data */
int update_contact_status(contact *,int);               /* updates contact status data */

#ifdef __cplusplus
  }
#endif

#endif
