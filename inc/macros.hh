/*
** Copyright 1999-2010 Ethan Galstad
** Copyright 2011      Merethis
**
** This file is part of Centreon Scheduler.
**
** Centreon Scheduler is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Scheduler is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Scheduler. If not, see
** <http://www.gnu.org/licenses/>.
*/

#ifndef CCS_MACROS_HH
# define CCS_MACROS_HH

# include "objects.hh"

# ifdef __cplusplus
extern "C" {
# endif

// Length Limitations
static const unsigned int MAX_COMMAND_ARGUMENTS                = 32;  // maximum number of $ARGx$ macros

// Macro Definitions
static const char         MACRO_ENV_VAR_PREFIX[]               = "NAGIOS_";
static const unsigned int MAX_USER_MACROS                      = 256; // maximum number of $USERx$ macros
static const unsigned int MACRO_X_COUNT                        = 153; // size of macro_x[] array

static const unsigned int MACRO_HOSTNAME                       = 0;
static const unsigned int MACRO_HOSTALIAS                      = 1;
static const unsigned int MACRO_HOSTADDRESS                    = 2;
static const unsigned int MACRO_SERVICEDESC                    = 3;
static const unsigned int MACRO_SERVICESTATE                   = 4;
static const unsigned int MACRO_SERVICESTATEID                 = 5;
static const unsigned int MACRO_SERVICEATTEMPT                 = 6;
static const unsigned int MACRO_LONGDATETIME                   = 7;
static const unsigned int MACRO_SHORTDATETIME                  = 8;
static const unsigned int MACRO_DATE                           = 9;
static const unsigned int MACRO_TIME                           = 10;
static const unsigned int MACRO_TIMET                          = 11;
static const unsigned int MACRO_LASTHOSTCHECK                  = 12;
static const unsigned int MACRO_LASTSERVICECHECK               = 13;
static const unsigned int MACRO_LASTHOSTSTATECHANGE            = 14;
static const unsigned int MACRO_LASTSERVICESTATECHANGE         = 15;
static const unsigned int MACRO_HOSTOUTPUT                     = 16;
static const unsigned int MACRO_SERVICEOUTPUT                  = 17;
static const unsigned int MACRO_HOSTPERFDATA                   = 18;
static const unsigned int MACRO_SERVICEPERFDATA                = 19;
static const unsigned int MACRO_CONTACTNAME                    = 20;
static const unsigned int MACRO_CONTACTALIAS                   = 21;
static const unsigned int MACRO_CONTACTEMAIL                   = 22;
static const unsigned int MACRO_CONTACTPAGER                   = 23;
static const unsigned int MACRO_ADMINEMAIL                     = 24;
static const unsigned int MACRO_ADMINPAGER                     = 25;
static const unsigned int MACRO_HOSTSTATE                      = 26;
static const unsigned int MACRO_HOSTSTATEID                    = 27;
static const unsigned int MACRO_HOSTATTEMPT                    = 28;
static const unsigned int MACRO_NOTIFICATIONTYPE               = 29;
static const unsigned int MACRO_NOTIFICATIONNUMBER             = 30; // deprecated - see HOSTNOTIFICATIONNUMBER and SERVICENOTIFICATIONNUMBER macros
static const unsigned int MACRO_HOSTEXECUTIONTIME              = 31;
static const unsigned int MACRO_SERVICEEXECUTIONTIME           = 32;
static const unsigned int MACRO_HOSTLATENCY                    = 33;
static const unsigned int MACRO_SERVICELATENCY                 = 34;
static const unsigned int MACRO_HOSTDURATION                   = 35;
static const unsigned int MACRO_SERVICEDURATION                = 36;
static const unsigned int MACRO_HOSTDURATIONSEC                = 37;
static const unsigned int MACRO_SERVICEDURATIONSEC             = 38;
static const unsigned int MACRO_HOSTDOWNTIME                   = 39;
static const unsigned int MACRO_SERVICEDOWNTIME                = 40;
static const unsigned int MACRO_HOSTSTATETYPE                  = 41;
static const unsigned int MACRO_SERVICESTATETYPE               = 42;
static const unsigned int MACRO_HOSTPERCENTCHANGE              = 43;
static const unsigned int MACRO_SERVICEPERCENTCHANGE           = 44;
static const unsigned int MACRO_HOSTGROUPNAME                  = 45;
static const unsigned int MACRO_HOSTGROUPALIAS                 = 46;
static const unsigned int MACRO_SERVICEGROUPNAME               = 47;
static const unsigned int MACRO_SERVICEGROUPALIAS              = 48;
static const unsigned int MACRO_HOSTACKAUTHOR                  = 49;
static const unsigned int MACRO_HOSTACKCOMMENT                 = 50;
static const unsigned int MACRO_SERVICEACKAUTHOR               = 51;
static const unsigned int MACRO_SERVICEACKCOMMENT              = 52;
static const unsigned int MACRO_LASTSERVICEOK                  = 53;
static const unsigned int MACRO_LASTSERVICEWARNING             = 54;
static const unsigned int MACRO_LASTSERVICEUNKNOWN             = 55;
static const unsigned int MACRO_LASTSERVICECRITICAL            = 56;
static const unsigned int MACRO_LASTHOSTUP                     = 57;
static const unsigned int MACRO_LASTHOSTDOWN                   = 58;
static const unsigned int MACRO_LASTHOSTUNREACHABLE            = 59;
static const unsigned int MACRO_SERVICECHECKCOMMAND            = 60;
static const unsigned int MACRO_HOSTCHECKCOMMAND               = 61;
static const unsigned int MACRO_MAINCONFIGFILE                 = 62;
static const unsigned int MACRO_STATUSDATAFILE                 = 63;
static const unsigned int MACRO_HOSTDISPLAYNAME                = 64;
static const unsigned int MACRO_SERVICEDISPLAYNAME             = 65;
static const unsigned int MACRO_RETENTIONDATAFILE              = 66;
static const unsigned int MACRO_OBJECTCACHEFILE                = 67;
static const unsigned int MACRO_TEMPFILE                       = 68;
static const unsigned int MACRO_LOGFILE                        = 69;
static const unsigned int MACRO_RESOURCEFILE                   = 70;
static const unsigned int MACRO_COMMANDFILE                    = 71;
static const unsigned int MACRO_HOSTPERFDATAFILE               = 72;
static const unsigned int MACRO_SERVICEPERFDATAFILE            = 73;
static const unsigned int MACRO_HOSTACTIONURL                  = 74;
static const unsigned int MACRO_HOSTNOTESURL                   = 75;
static const unsigned int MACRO_HOSTNOTES                      = 76;
static const unsigned int MACRO_SERVICEACTIONURL               = 77;
static const unsigned int MACRO_SERVICENOTESURL                = 78;
static const unsigned int MACRO_SERVICENOTES                   = 79;
static const unsigned int MACRO_TOTALHOSTSUP                   = 80;
static const unsigned int MACRO_TOTALHOSTSDOWN                 = 81;
static const unsigned int MACRO_TOTALHOSTSUNREACHABLE          = 82;
static const unsigned int MACRO_TOTALHOSTSDOWNUNHANDLED        = 83;
static const unsigned int MACRO_TOTALHOSTSUNREACHABLEUNHANDLED = 84;
static const unsigned int MACRO_TOTALHOSTPROBLEMS              = 85;
static const unsigned int MACRO_TOTALHOSTPROBLEMSUNHANDLED     = 86;
static const unsigned int MACRO_TOTALSERVICESOK                = 87;
static const unsigned int MACRO_TOTALSERVICESWARNING           = 88;
static const unsigned int MACRO_TOTALSERVICESCRITICAL          = 89;
static const unsigned int MACRO_TOTALSERVICESUNKNOWN           = 90;
static const unsigned int MACRO_TOTALSERVICESWARNINGUNHANDLED  = 91;
static const unsigned int MACRO_TOTALSERVICESCRITICALUNHANDLED = 92;
static const unsigned int MACRO_TOTALSERVICESUNKNOWNUNHANDLED  = 93;
static const unsigned int MACRO_TOTALSERVICEPROBLEMS           = 94;
static const unsigned int MACRO_TOTALSERVICEPROBLEMSUNHANDLED  = 95;
static const unsigned int MACRO_PROCESSSTARTTIME               = 96;
static const unsigned int MACRO_HOSTCHECKTYPE                  = 97;
static const unsigned int MACRO_SERVICECHECKTYPE               = 98;
static const unsigned int MACRO_LONGHOSTOUTPUT                 = 99;
static const unsigned int MACRO_LONGSERVICEOUTPUT              = 100;
static const unsigned int MACRO_TEMPPATH                       = 101;
static const unsigned int MACRO_HOSTNOTIFICATIONNUMBER         = 102;
static const unsigned int MACRO_SERVICENOTIFICATIONNUMBER      = 103;
static const unsigned int MACRO_HOSTNOTIFICATIONID             = 104;
static const unsigned int MACRO_SERVICENOTIFICATIONID          = 105;
static const unsigned int MACRO_HOSTEVENTID                    = 106;
static const unsigned int MACRO_LASTHOSTEVENTID                = 107;
static const unsigned int MACRO_SERVICEEVENTID                 = 108;
static const unsigned int MACRO_LASTSERVICEEVENTID             = 109;
static const unsigned int MACRO_HOSTGROUPNAMES                 = 110;
static const unsigned int MACRO_SERVICEGROUPNAMES              = 111;
static const unsigned int MACRO_HOSTACKAUTHORNAME              = 112;
static const unsigned int MACRO_HOSTACKAUTHORALIAS             = 113;
static const unsigned int MACRO_SERVICEACKAUTHORNAME           = 114;
static const unsigned int MACRO_SERVICEACKAUTHORALIAS          = 115;
static const unsigned int MACRO_MAXHOSTATTEMPTS                = 116;
static const unsigned int MACRO_MAXSERVICEATTEMPTS             = 117;
static const unsigned int MACRO_SERVICEISVOLATILE              = 118;
static const unsigned int MACRO_TOTALHOSTSERVICES              = 119;
static const unsigned int MACRO_TOTALHOSTSERVICESOK            = 120;
static const unsigned int MACRO_TOTALHOSTSERVICESWARNING       = 121;
static const unsigned int MACRO_TOTALHOSTSERVICESUNKNOWN       = 122;
static const unsigned int MACRO_TOTALHOSTSERVICESCRITICAL      = 123;
static const unsigned int MACRO_HOSTGROUPNOTES                 = 124;
static const unsigned int MACRO_HOSTGROUPNOTESURL              = 125;
static const unsigned int MACRO_HOSTGROUPACTIONURL             = 126;
static const unsigned int MACRO_SERVICEGROUPNOTES              = 127;
static const unsigned int MACRO_SERVICEGROUPNOTESURL           = 128;
static const unsigned int MACRO_SERVICEGROUPACTIONURL          = 129;
static const unsigned int MACRO_HOSTGROUPMEMBERS               = 130;
static const unsigned int MACRO_SERVICEGROUPMEMBERS            = 131;
static const unsigned int MACRO_CONTACTGROUPNAME               = 132;
static const unsigned int MACRO_CONTACTGROUPALIAS              = 133;
static const unsigned int MACRO_CONTACTGROUPMEMBERS            = 134;
static const unsigned int MACRO_CONTACTGROUPNAMES              = 135;
static const unsigned int MACRO_NOTIFICATIONRECIPIENTS         = 136;
static const unsigned int MACRO_NOTIFICATIONISESCALATED        = 137;
static const unsigned int MACRO_NOTIFICATIONAUTHOR             = 138;
static const unsigned int MACRO_NOTIFICATIONAUTHORNAME         = 139;
static const unsigned int MACRO_NOTIFICATIONAUTHORALIAS        = 140;
static const unsigned int MACRO_NOTIFICATIONCOMMENT            = 141;
static const unsigned int MACRO_EVENTSTARTTIME                 = 142;
static const unsigned int MACRO_HOSTPROBLEMID                  = 143;
static const unsigned int MACRO_LASTHOSTPROBLEMID              = 144;
static const unsigned int MACRO_SERVICEPROBLEMID               = 145;
static const unsigned int MACRO_LASTSERVICEPROBLEMID           = 146;
static const unsigned int MACRO_ISVALIDTIME                    = 147;
static const unsigned int MACRO_NEXTVALIDTIME                  = 148;
static const unsigned int MACRO_LASTHOSTSTATE                  = 149;
static const unsigned int MACRO_LASTHOSTSTATEID                = 150;
static const unsigned int MACRO_LASTSERVICESTATE               = 151;
static const unsigned int MACRO_LASTSERVICESTATEID             = 152;

// Macro Cleaning Options
static const unsigned int STRIP_ILLEGAL_MACRO_CHARS            = 1;
static const unsigned int ESCAPE_MACRO_CHARS                   = 2;
static const unsigned int URL_ENCODE_MACRO_CHARS               = 4;

// NAGIOS_MACROS structure
struct                   nagios_macros {
  char*                  x[MACRO_X_COUNT];
  char*                  argv[MAX_COMMAND_ARGUMENTS];
  char*                  contactaddress[MAX_CONTACT_ADDRESSES];
  char*                  ondemand;
  host*                  host_ptr;
  hostgroup*             hostgroup_ptr;
  service*               service_ptr;
  servicegroup*          servicegroup_ptr;
  contact*               contact_ptr;
  contactgroup*          contactgroup_ptr;
  customvariablesmember* custom_host_vars;
  customvariablesmember* custom_service_vars;
  customvariablesmember* custom_contact_vars;
};

// Macro Functions
nagios_macros* get_global_macros(void);

// thread-safe version of process_macros.
int process_macros_r(nagios_macros* mac, char* input_buffer, char** output_buffer, int options);

/*
 * Replace macros with their actual values
 * This function modifies the global_macros struct and is thus
 * not thread-safe.
 */
int process_macros(char* input_buffer, char** output_buffer, int options);

/*
 * These functions updates **macros with the values from
 * their respective object type.
 */
int grab_host_macros(nagios_macros* mac, host* hst);
int grab_hostgroup_macros(nagios_macros* mac, hostgroup* hg);
int grab_service_macros(nagios_macros* mac, service* svc);
int grab_servicegroup_macros(nagios_macros* mac, servicegroup* sg);
int grab_contact_macros(nagios_macros* mac, contact* cntct);

int grab_macro_value(nagios_macros* mac, char* macro_buffer, char** output, int* clean_options, int* free_macro);
int grab_macrox_value(nagios_macros* mac, int macro_type, char* arg1, char* arg2, char** output, int* free_macro);
int grab_custom_macro_value(nagios_macros* mac, char* macro_name, char* arg1, char* arg2, char** output);
int grab_datetime_macro(nagios_macros* mac, int macro_type, char* arg1, char* arg2, char** output);
int grab_standard_host_macro(nagios_macros* mac, unsigned int macro_type, host* temp_host, char** output, int* free_macro);
int grab_standard_hostgroup_macro(nagios_macros* mac, int macro_type, hostgroup* temp_hostgroup, char** output);
int grab_standard_service_macro(nagios_macros* mac, unsigned int macro_type, service* temp_service, char** output, int* free_macro);
int grab_standard_servicegroup_macro(nagios_macros* mac, int macro_type, servicegroup* temp_servicegroup, char** output);
int grab_standard_contact_macro(nagios_macros* mac, int macro_type, contact* temp_contact, char** output);
int grab_contact_address_macro(unsigned int macro_num, contact* temp_contact, char** output);
int grab_standard_contactgroup_macro(int macro_type, contactgroup* temp_contactgroup, char** output);
int grab_custom_object_macro(nagios_macros* mac, char* macro_name, customvariablesmember* vars, char** output);

char const* clean_macro_chars(char* macro,int options); // cleans macros characters before insertion into output string

char* get_url_encoded_string(char* input);              // URL encode a string

int init_macros(void);
int init_macrox_names(void);
int free_macrox_names(void);

void copy_constant_macros(char** dest);

int clear_argv_macros(nagios_macros* mac);
int clear_volatile_macros(nagios_macros* mac);
int clear_service_macros(nagios_macros* mac);
int clear_host_macros(nagios_macros* mac);
int clear_hostgroup_macros(nagios_macros* mac);
int clear_servicegroup_macros(nagios_macros* mac);
int clear_contact_macros(nagios_macros* mac);
int clear_contactgroup_macros(nagios_macros* mac);
int clear_summary_macros(nagios_macros* mac);

int set_all_macro_environment_vars(nagios_macros* mac, int set);
int set_macrox_environment_vars(nagios_macros* mac, int set);
int set_argv_macro_environment_vars(nagios_macros* mac, int set);
int set_custom_macro_environment_vars(nagios_macros* mac, int set);
int set_contact_address_environment_vars(nagios_macros* mac, int set);
int set_macro_environment_var(char const* name, char const* value, int set);

# ifdef __cplusplus
}
# endif

#endif // !CCS_MACROS_HH
