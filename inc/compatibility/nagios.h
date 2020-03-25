/*
** Copyright 2007-2008 Ethan Galstad
** Copyright 2007,2010 Andreas Ericsson
** Copyright 2010      Max Schubert
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

#ifndef CCE_COMPATIBILITY_NAGIOS_H
#define CCE_COMPATIBILITY_NAGIOS_H

#include "broker.h"
#include "com/centreon/engine/check_result.hh"
#include "com/centreon/engine/circular_buffer.hh"
#include "com/centreon/engine/comment.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/flapping.hh"
#include "com/centreon/engine/sehandlers.hh"
#include "com/centreon/engine/utils.hh"
#include "common.h"
#include "config.h"
#include "embedded_perl.h"
#include "locations.h"
#include "logging.h"
#include "macros.h"
#include "nebcallbacks.h"
#include "nebstructs.h"
#include "objects.h"
#include "sighandlers.h"

// Inter-check delay calculation types.
#define ICD_NONE 0   // No inter-check delay.
#define ICD_DUMB 1   // Dumb delay of 1 second.
#define ICD_SMART 2  // Smart delay.
#define ICD_USER 3   // User-specified delay.

// Interleave factor calculation types.
#define ILF_USER 0   // User-specified interleave factor.
#define ILF_SMART 1  // Smart interleave.

// Misc.
#define COMMAND_WORKER_THREAD 0
#define MAX_PLUGIN_OUTPUT_LENGTH -1  // Plugin output length is not caped.

#endif  // !CCE_COMPATIBILITY_NAGIOS_H
