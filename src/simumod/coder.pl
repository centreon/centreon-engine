#!/usr/bin/perl

use strict;
use warnings;

open F, "<../../inc/com/centreon/engine/nebstructs.hh";

my @cb;
my @reg;
my @dereg;

my $callback = "";
my $register = "";
my $deregister = "";

my $instruct = 0;

while (<F>) {
    if (m/typedef struct.*/) {
        $instruct = 1;
        $callback = "";
    }
    if ($instruct == 1) {
        if (m/^\s*struct timeval\s*([^\s]*);/) {
            $callback .= "\n      << \"  $1=\" << timeval_str(neb_data->$1) << std::endl";
        }
        elsif (m/^\s*char\*\s+([^\s]*);/) {
            $callback .= "\n      << \"  $1=\" << (neb_data->$1 ? neb_data->$1 : \"NULL\") << std::endl";
        }
        elsif (m/^\s*[a-z\s]*\*?\s+([^\s]*);/) {
            $callback .= "\n      << \"  $1=\" << neb_data->$1 << std::endl";
        }
        elsif (m/}\s*([a-z_]*);/) {
            $instruct = 0;
            my $v = $1;
            my $fname = $v;
            $fname =~ s/nebstruct_//;
            $callback = qq(
/**
 *  \@brief This function is called when an event of type $v is emitted.
 *
 *  \@param callback_type An integer corresponding to the type.
 *  \@param data The data of type $v but cast into void*
 *
 *  \@return 0 on success and -1 otherwise.
 */
static int callback_$fname(int callback_type __attribute__((unused)), void* data) {
  $v* neb_data(
    static_cast<$v*>(data));

) . "  *fp << \"$v: \" << std::endl" . $callback . ";\n  return 0;\n}\n\n";

            my $nebcb = "NEBCALLBACK_" . uc($fname);

            # Little fix because of a typo in nebstruct.hh
            if ($nebcb eq "NEBCALLBACK_STATECHANGE_DATA") {
                $nebcb = "NEBCALLBACK_STATE_CHANGE_DATA";
            }
            $register = qq(    if (neb_register_callback(
          $nebcb,
          gl_mod_handle,
          0,
          callback_$fname)) {
      throw engine_error()
          << "$fname register callback failed";
    }
);

            $deregister = qq(    neb_deregister_callback(
      $nebcb,
      callback_$fname);
);

            push(@cb, $callback);
            push(@reg, $register);
            push(@dereg, $deregister);
        }
    }
}

close F;

print qq(/*
** Copyright 2019 Centreon
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

#include <cstdio>
#include <fstream>
#include <iostream>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/nebcallbacks.hh"
#include "com/centreon/engine/nebmodules.hh"
#include "com/centreon/engine/nebstructs.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

std::ostream* fp(NULL);
std::ofstream foutput;

//static std::string timeval_str(timeval const& t) {
//  struct tm* nowtm;
//  time_t nowtime = t.tv_sec;
//  char tmbuf[64], buf[64];
//  nowtm = localtime(&nowtime);
//  strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);
//  snprintf(buf, sizeof buf, "%s.%06ld", tmbuf, t.tv_usec);
//  return std::string(buf);
//}

/**************************************
*                                     *
*           Global Objects            *
*                                     *
**************************************/

// Specify the event broker API version.
NEB_API_VERSION(CURRENT_NEB_API_VERSION)

// Module handle
static void* gl_mod_handle(NULL);

/**************************************
*                                     *
*         Callback Function           *
*                                     *
**************************************/

);

print join("\n", @cb);

print qq(
/**************************************
 *                                     *
 *         Exported Functions          *
 *                                     *
 **************************************/

extern "C" {
/**
 *  \@brief Module exit point.
 *
 *  This function is called when the module gets unloaded by Nagios.
 *  It will deregister all previously registered callbacks and perform
 *  some shutdown stuff.
 *
 *  \@param[in] flags  Informational flags.
 *  \@param[in] reason Unload reason.
 *
 *  \@return 0 on success, any other value on failure.
 */
int nebmodule_deinit(int flags, int reason) {
  (void)flags;
  (void)reason;

  *fp << "simumod deinit..." << std::endl;
  try {
    // Unregister callbacks.
);
print join "\n", @dereg;

print qq(  }
  catch (std::exception const& e) {
      logger(log_runtime_error, basic)
	<< "external command runtime error `" << e.what() << "'.";
  }
  catch (...) {
      logger(log_runtime_error, basic)
	<< "external command runtime error `unknown'";
  }
  return 0;
}

/**
 *  \@brief Module entry point.
 *
 *  This function is called when the module gets loaded by Nagios. It
 *  will register callbacks to catch events and perform some
 *  initialization stuff like config file parsing, thread creation,
 *  ...
 *
 *  \@param[in] flags  Informational flags.
 *  \@param[in] args   The argument string of the module (shall contain the
 *                    configuration file name).
 *  \@param[in] handle The module handle.
 *
 *  \@return 0 on success, any other value on failure.
 */
int nebmodule_init(int flags __attribute__((unused)),
                   char const* args __attribute__((unused)),
                   void* handle) {
  char* env = getenv("SIMUMOD");
  if (env && *env) {
    foutput.open(env);
    if (foutput.is_open())
      fp = \&foutput;
  }
  if (!fp)
    fp = \&std::cout;

  *fp << "################################################################################" << std::endl
      << "#                                  START SIMUMOD                               #" << std::endl
      << "################################################################################" << std::endl;

  // Save module handle for future use.
  gl_mod_handle = handle;

  neb_set_module_info(
    gl_mod_handle,
    NEBMODULE_MODINFO_TITLE,
    "simumod : Centreon Broker's cbmod simulator");
  neb_set_module_info(
    gl_mod_handle,
    NEBMODULE_MODINFO_AUTHOR,
    "Centreon");
  neb_set_module_info(
    gl_mod_handle,
    NEBMODULE_MODINFO_COPYRIGHT,
    "Copyright 2019 Centreon");
  neb_set_module_info(
    gl_mod_handle,
    NEBMODULE_MODINFO_VERSION,
    "0.0.1");
  neb_set_module_info(
    gl_mod_handle,
    NEBMODULE_MODINFO_LICENSE,
    "GPL version 2");
  neb_set_module_info(
    gl_mod_handle,
    NEBMODULE_MODINFO_DESC,
    "simumod is part of Centreon Engine and is designed to "    \
    "convert internal Centreon Engine events to a "  \
    "proper data stream that can then be seen by humans to debug");

  *fp << "simumod init..." << std::endl;

  try {
);

print join "\n", @reg;

print qq(  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic)
      << "main: cbmod loading failed: " << e.what();
    nebmodule_deinit(0, 0);
    return -1;
  }
  catch (...) {
    logger(log_runtime_error, basic)
      << "main: cbmod loading failed due to an unknown exception";
    nebmodule_deinit(0, 0);
    return -1;
  }

  return 0;
}

  /**
   *  \@brief Reload module after configuration reload.
   *
   *  This will effectively send an instance_configuration object to the
   *  multiplexer.
   *
   *  \@return OK.
   */
  int nebmodule_reload() {
    return 0;
  }
}
);
