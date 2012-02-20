/*
** Copyright 2007-2008 Ethan Galstad
** Copyright 2007,2010 Andreas Ericsson
** Copyright 2010      Max Schubert
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

#ifndef CCE_COMPATIBILITY_NAGIOS_H
# define CCE_COMPATIBILITY_NAGIOS_H

# include "check_result.h"
# include "config.h"
# include "embedded_perl.h"
# include "logging.h"
# include "sighandlers.h"
# include "common.h"
# include "locations.h"
# include "objects.h"
# include "macros.h"
# include "broker.h"
# include "nebstructs.h"
# include "nebcallbacks.h"
# include "checks.hh"
# include "flapping.hh"
# include "notifications.hh"
# include "sehandlers.hh"
# include "events.hh"
# include "utils.hh"
# include "comments.hh"
# include "config.hh"
# include "engine.hh"

/******* INTER-CHECK DELAY CALCULATION TYPES **********/

# define ICD_NONE  0 /* no inter-check delay */
# define ICD_DUMB  1 /* dumb delay of 1 second */
# define ICD_SMART 2 /* smart delay */
# define ICD_USER  3 /* user-specified delay */

/******* INTERLEAVE FACTOR CALCULATION TYPES **********/

# define ILF_USER  0 /* user-specified interleave factor */
# define ILF_SMART 1 /* smart interleave */

#endif /* !CCE_COMPATIBILITY_NAGIOS_H */
