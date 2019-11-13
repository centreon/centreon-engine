/*
** Copyright 2003-2006 Ethan Galstad
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

#ifndef CCE_NEBERRORS_HH
#define CCE_NEBERRORS_HH

// Generic defines.
#define NEB_OK 0
#define NEB_ERROR -1

#define NEB_TRUE 1
#define NEB_FALSE 0

// Callback errors.
#define NEBERROR_NOCALLBACKFUNC 200   /* no callback function was specified */
#define NEBERROR_NOCALLBACKLIST 201   /* callback list not initialized */
#define NEBERROR_CALLBACKBOUNDS 202   /* callback type was out of bounds */
#define NEBERROR_CALLBACKNOTFOUND 203 /* the callback could not be found */
#define NEBERROR_NOMODULEHANDLE 204   /* no module handle specified */
#define NEBERROR_BADMODULEHANDLE 205  /* bad module handle */
#define NEBERROR_CALLBACKOVERRIDE \
  206 /* module wants to override default handling of event */
#define NEBERROR_CALLBACKCANCEL \
  207 /* module wants to cancel callbacks to other modules */

// Module errors.
#define NEBERROR_NOMEM 100    /* memory could not be allocated */
#define NEBERROR_NOMODULE 300 /* no module was specified */

// Module info errors.
#define NEBERROR_MODINFOBOUNDS 400 /* module info index was out of bounds */

#endif  // !CCE_NEBERRORS_HH
