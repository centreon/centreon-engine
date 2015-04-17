/*
** Copyright 1999-2010 Ethan Galstad
** Copyright 2011-2015 Merethis
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

#ifndef CCE_MACROS_DEFINES_HH
#  define CCE_MACROS_DEFINES_HH

#  include "com/centreon/engine/objects/customvariablesmember.hh"
#  include "com/centreon/engine/objects/host.hh"
#  include "com/centreon/engine/objects/service.hh"

// Length Limitations
#  define MAX_COMMAND_ARGUMENTS                  32   // maximum number of $ARGx$ macros

// Macro Definitions
#  define MACRO_ENV_VAR_PREFIX                   "NAGIOS_"
#  define MAX_USER_MACROS                        256  // maximum number of $USERx$ macros
#  define MACRO_X_COUNT                          153  // size of macro_x[] array

#  define MACRO_HOSTNAME                         0
#  define MACRO_HOSTALIAS                        1
#  define MACRO_HOSTADDRESS                      2
#  define MACRO_SERVICEDESC                      3
#  define MACRO_SERVICESTATE                     4
#  define MACRO_SERVICESTATEID                   5
#  define MACRO_SERVICEATTEMPT                   6
#  define MACRO_TIMET                            11
#  define MACRO_LASTHOSTCHECK                    12
#  define MACRO_LASTSERVICECHECK                 13
#  define MACRO_LASTHOSTSTATECHANGE              14
#  define MACRO_LASTSERVICESTATECHANGE           15
#  define MACRO_HOSTOUTPUT                       16
#  define MACRO_SERVICEOUTPUT                    17
#  define MACRO_HOSTPERFDATA                     18
#  define MACRO_SERVICEPERFDATA                  19
#  define MACRO_HOSTSTATE                        26
#  define MACRO_HOSTSTATEID                      27
#  define MACRO_HOSTATTEMPT                      28
#  define MACRO_HOSTEXECUTIONTIME                31
#  define MACRO_SERVICEEXECUTIONTIME             32
#  define MACRO_HOSTLATENCY                      33
#  define MACRO_SERVICELATENCY                   34
#  define MACRO_HOSTDURATION                     35
#  define MACRO_SERVICEDURATION                  36
#  define MACRO_HOSTDURATIONSEC                  37
#  define MACRO_SERVICEDURATIONSEC               38
#  define MACRO_HOSTSTATETYPE                    41
#  define MACRO_SERVICESTATETYPE                 42
#  define MACRO_HOSTPERCENTCHANGE                43
#  define MACRO_SERVICEPERCENTCHANGE             44
#  define MACRO_LASTSERVICEOK                    53
#  define MACRO_LASTSERVICEWARNING               54
#  define MACRO_LASTSERVICEUNKNOWN               55
#  define MACRO_LASTSERVICECRITICAL              56
#  define MACRO_LASTHOSTUP                       57
#  define MACRO_LASTHOSTDOWN                     58
#  define MACRO_LASTHOSTUNREACHABLE              59
#  define MACRO_SERVICECHECKCOMMAND              60
#  define MACRO_HOSTCHECKCOMMAND                 61
#  define MACRO_MAINCONFIGFILE                   62
#  define MACRO_STATUSDATAFILE                   63
#  define MACRO_RETENTIONDATAFILE                66
#  define MACRO_LOGFILE                          69
#  define MACRO_RESOURCEFILE                     70
#  define MACRO_COMMANDFILE                      71
#  define MACRO_HOSTACTIONURL                    74
#  define MACRO_HOSTNOTESURL                     75
#  define MACRO_HOSTNOTES                        76
#  define MACRO_SERVICEACTIONURL                 77
#  define MACRO_SERVICENOTESURL                  78
#  define MACRO_SERVICENOTES                     79
#  define MACRO_TOTALHOSTSUP                     80
#  define MACRO_TOTALHOSTSDOWN                   81
#  define MACRO_TOTALHOSTSUNREACHABLE            82
#  define MACRO_TOTALHOSTPROBLEMS                83
#  define MACRO_TOTALSERVICESOK                  84
#  define MACRO_TOTALSERVICESWARNING             85
#  define MACRO_TOTALSERVICESCRITICAL            86
#  define MACRO_TOTALSERVICESUNKNOWN             87
#  define MACRO_TOTALSERVICEPROBLEMS             88
#  define MACRO_PROCESSSTARTTIME                 96
#  define MACRO_HOSTCHECKTYPE                    97
#  define MACRO_SERVICECHECKTYPE                 98
#  define MACRO_LONGHOSTOUTPUT                   99
#  define MACRO_LONGSERVICEOUTPUT                100
#  define MACRO_HOSTEVENTID                      106
#  define MACRO_LASTHOSTEVENTID                  107
#  define MACRO_SERVICEEVENTID                   108
#  define MACRO_LASTSERVICEEVENTID               109
#  define MACRO_MAXHOSTATTEMPTS                  116
#  define MACRO_MAXSERVICEATTEMPTS               117
#  define MACRO_SERVICEISVOLATILE                118
#  define MACRO_TOTALHOSTSERVICES                119
#  define MACRO_TOTALHOSTSERVICESOK              120
#  define MACRO_TOTALHOSTSERVICESWARNING         121
#  define MACRO_TOTALHOSTSERVICESUNKNOWN         122
#  define MACRO_TOTALHOSTSERVICESCRITICAL        123
#  define MACRO_EVENTSTARTTIME                   142
#  define MACRO_HOSTPROBLEMID                    143
#  define MACRO_LASTHOSTPROBLEMID                144
#  define MACRO_SERVICEPROBLEMID                 145
#  define MACRO_LASTSERVICEPROBLEMID             146
#  define MACRO_ISVALIDTIME                      147
#  define MACRO_NEXTVALIDTIME                    148
#  define MACRO_LASTHOSTSTATE                    149
#  define MACRO_LASTHOSTSTATEID                  150
#  define MACRO_LASTSERVICESTATE                 151
#  define MACRO_LASTSERVICESTATEID               152

// Macro Cleaning Options
#  define STRIP_ILLEGAL_MACRO_CHARS              1
#  define ESCAPE_MACRO_CHARS                     2
#  define URL_ENCODE_MACRO_CHARS                 4

// NAGIOS_MACROS structure
struct                   nagios_macros {
  char*                  x[MACRO_X_COUNT];
  char*                  argv[MAX_COMMAND_ARGUMENTS];
  char*                  ondemand;
  host*                  host_ptr;
  service*               service_ptr;
  customvariablesmember* custom_host_vars;
  customvariablesmember* custom_service_vars;
};

typedef struct nagios_macros nagios_macros;

#endif // !CCE_MACROS_DEFINES_HH
