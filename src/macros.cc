/*
** Copyright 1999-2010 Ethan Galstad
** Copyright 2011-2014 Merethis
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
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/engine/utils.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

/******************************************************************/
/********************** MACRO GRAB FUNCTIONS **********************/
/******************************************************************/

/* grab hostgroup macros */
int grab_hostgroup_macros_r(nagios_macros* mac, hostgroup* hg) {
  /* clear hostgroup macros */
  clear_hostgroup_macros_r(mac);

  /* save the hostgroup pointer for later */
  mac->hostgroup_ptr = hg;

  if (hg == NULL)
    return (ERROR);
  return (OK);
}

int grab_hostgroup_macros(hostgroup* hg) {
  return (grab_hostgroup_macros_r(get_global_macros(), hg));
}

/* grab macros that are specific to a particular servicegroup */
int grab_servicegroup_macros_r(nagios_macros* mac, servicegroup* sg) {
  /* clear servicegroup macros */
  clear_servicegroup_macros_r(mac);

  /* save the pointer for later */
  mac->servicegroup_ptr = sg;

  if (sg == NULL)
    return (ERROR);
  return (OK);
}

int grab_servicegroup_macros(servicegroup* sg) {
  return (grab_servicegroup_macros_r(get_global_macros(), sg));
}

/* grab macros that are specific to a particular contact */
int grab_contact_macros_r(nagios_macros* mac, contact* cntct) {
  /* clear contact-related macros */
  clear_contact_macros_r(mac);
  clear_contactgroup_macros_r(mac);

  /* save pointer to contact for later */
  mac->contact_ptr = cntct;
  mac->contactgroup_ptr = NULL;

  if (cntct == NULL)
    return (ERROR);

  /* save pointer to first/primary contactgroup for later */
  if (cntct->contactgroups_ptr)
    mac->contactgroup_ptr
      = (contactgroup*)cntct->contactgroups_ptr->object_ptr;
  return (OK);
}

int grab_contact_macros(contact* cntct) {
  return (grab_contact_macros_r(get_global_macros(), cntct));
}

/******************************************************************/
/******************* MACRO GENERATION FUNCTIONS *******************/
/******************************************************************/

/* calculates the value of a custom macro */
int grab_custom_macro_value_r(
      nagios_macros* mac,
      char* macro_name,
      char const* arg1,
      char const* arg2,
      char** output) {
  hostgroup* temp_hostgroup = NULL;
  hostsmember* temp_hostsmember = NULL;
  servicegroup* temp_servicegroup = NULL;
  servicesmember* temp_servicesmember = NULL;
  contactgroup* temp_contactgroup = NULL;
  contactsmember* temp_contactsmember = NULL;
  int delimiter_len = 0;
  char* temp_buffer = NULL;
  int result = OK;

  if (macro_name == NULL || output == NULL)
    return (ERROR);

  /***** CUSTOM HOST MACRO *****/
  if (strstr(macro_name, "_HOST") == macro_name) {
    host* temp_host(NULL);
    /* a standard host macro */
    if (arg2 == NULL) {
      /* find the host for on-demand macros */
      if (arg1) {
        if ((temp_host = find_host(arg1)) == NULL)
          return (ERROR);
      }
      /* else use saved host pointer */
      else if ((temp_host = mac->host_ptr) == NULL)
        return (ERROR);

      /* get the host macro value */
      result = grab_custom_object_macro_r(
                 mac,
                 macro_name + 5,
                 temp_host->custom_variables,
                 output);
    }
    /* a host macro with a hostgroup name and delimiter */
    else {
      if ((temp_hostgroup = find_hostgroup(arg1)) == NULL)
        return (ERROR);

      delimiter_len = strlen(arg2);

      /* concatenate macro values for all hostgroup members */
      for (temp_hostsmember = temp_hostgroup->members;
           temp_hostsmember != NULL;
           temp_hostsmember = temp_hostsmember->next) {

        if ((temp_host = temp_hostsmember->host_ptr) == NULL)
          continue;

        /* get the macro value for this host */
        grab_custom_macro_value_r(
          mac,
          macro_name,
          temp_host->name,
          NULL,
          &temp_buffer);

        if (temp_buffer == NULL)
          continue;

        /* add macro value to already running macro */
        if (*output == NULL)
          *output = string::dup(temp_buffer);
        else {
          *output = resize_string(
                      *output,
                      strlen(*output) + strlen(temp_buffer) + delimiter_len + 1);
          strcat(*output, arg2);
          strcat(*output, temp_buffer);
        }
        delete[] temp_buffer;
        temp_buffer = NULL;
      }
    }
  }
  /***** CUSTOM SERVICE MACRO *****/
  else if (strstr(macro_name, "_SERVICE") == macro_name) {
    service* temp_service(NULL);

    /* use saved service pointer */
    if (arg1 == NULL && arg2 == NULL) {
      if ((temp_service = mac->service_ptr) == NULL)
        return (ERROR);

      /* get the service macro value */
      result = grab_custom_object_macro_r(
                 mac,
                 macro_name + 8,
                 temp_service->custom_variables,
                 output);
    }
    /* else and ondemand macro... */
    else {
      /* if first arg is blank, it means use the current host name */
      if (mac->host_ptr == NULL)
        return (ERROR);
      if ((temp_service = find_service(
                            mac->host_ptr ? mac->host_ptr->name : NULL,
                            arg2))) {
        /* get the service macro value */
        result = grab_custom_object_macro_r(
                   mac,
                   macro_name + 8,
                   temp_service->custom_variables,
                   output);
      }
      /* else we have a service macro with a servicegroup name and a delimiter... */
      else {
        if ((temp_servicegroup = find_servicegroup(arg1)) == NULL)
          return (ERROR);

        delimiter_len = strlen(arg2);

        /* concatenate macro values for all servicegroup members */
        for (temp_servicesmember = temp_servicegroup->members;
             temp_servicesmember != NULL;
             temp_servicesmember = temp_servicesmember->next) {

          if ((temp_service = temp_servicesmember->service_ptr) == NULL)
            continue;

          /* get the macro value for this service */
          grab_custom_macro_value_r(
            mac,
            macro_name,
            temp_service->host_name,
            temp_service->description,
            &temp_buffer);

          if (temp_buffer == NULL)
            continue;

          /* add macro value to already running macro */
          if (*output == NULL)
            *output = string::dup(temp_buffer);
          else {
            *output = resize_string(
                        *output,
                        strlen(*output) + strlen(temp_buffer) + delimiter_len + 1);
            strcat(*output, arg2);
            strcat(*output, temp_buffer);
          }
          delete[] temp_buffer;
          temp_buffer = NULL;
        }
      }
    }
  }
  /***** CUSTOM CONTACT VARIABLE *****/
  else if (strstr(macro_name, "_CONTACT") == macro_name) {
    contact* temp_contact(NULL);

    /* a standard contact macro */
    if (arg2 == NULL) {
      /* find the contact for on-demand macros */
      if (arg1) {
        if ((temp_contact = find_contact(arg1)) == NULL)
          return (ERROR);
      }
      /* else use saved contact pointer */
      else if ((temp_contact = mac->contact_ptr) == NULL)
        return (ERROR);

      /* get the contact macro value */
      result = grab_custom_object_macro_r(
                 mac,
                 macro_name + 8,
                 temp_contact->custom_variables,
                 output);
    }
    /* a contact macro with a contactgroup name and delimiter */
    else {
      if ((temp_contactgroup = find_contactgroup(arg1)) == NULL)
        return (ERROR);

      delimiter_len = strlen(arg2);

      /* concatenate macro values for all contactgroup members */
      for (temp_contactsmember = temp_contactgroup->members;
           temp_contactsmember != NULL;
           temp_contactsmember = temp_contactsmember->next) {

        if ((temp_contact = temp_contactsmember->contact_ptr) == NULL)
          continue;

        /* get the macro value for this contact */
        grab_custom_macro_value_r(
          mac,
          macro_name,
          temp_contact->name,
          NULL,
          &temp_buffer);

        if (temp_buffer == NULL)
          continue;

        /* add macro value to already running macro */
        if (*output == NULL)
          *output = string::dup(temp_buffer);
        else {
          *output = resize_string(
                      *output,
                      strlen(*output) + strlen(temp_buffer) + delimiter_len + 1);
          strcat(*output, arg2);
          strcat(*output, temp_buffer);
        }
        delete[] temp_buffer;
        temp_buffer = NULL;
      }
    }
  }
  else
    return (ERROR);
  return (result);
}

int grab_custom_macro_value(
      char* macro_name,
      char const* arg1,
      char const* arg2,
      char** output) {
  return (grab_custom_macro_value_r(
            get_global_macros(),
            macro_name,
            arg1,
            arg2,
            output));
}

/* calculates a date/time macro */
int grab_datetime_macro_r(
      nagios_macros* mac,
      int macro_type,
      char const* arg1,
      char const* arg2,
      char** output) {
  time_t current_time = 0L;
  timeperiod* temp_timeperiod = NULL;
  time_t test_time = 0L;
  time_t next_valid_time = 0L;

  (void)mac;

  if (output == NULL)
    return (ERROR);

  /* get the current time */
  time(&current_time);

  /* parse args, do prep work */
  switch (macro_type) {
  case MACRO_ISVALIDTIME:
  case MACRO_NEXTVALIDTIME:
    /* find the timeperiod */
    if ((temp_timeperiod = find_timeperiod(arg1)) == NULL)
      return (ERROR);
    /* what timestamp should we use? */
    if (arg2)
      test_time = (time_t)strtoul(arg2, NULL, 0);
    else
      test_time = current_time;
    break;

  default:
    break;
  }

  /*
  ** Calculate the value.
  **
  ** Default timezone is likely not valid but fetching it would require
  ** a heavy API refactoring for what I believe to be very little gain.
  */
  switch (macro_type) {
  case MACRO_LONGDATETIME:
    if (*output == NULL)
      *output = new char[MAX_DATETIME_LENGTH];
    get_datetime_string(
      &current_time,
      *output,
      MAX_DATETIME_LENGTH,
      LONG_DATE_TIME);
    break;

  case MACRO_SHORTDATETIME:
    if (*output == NULL)
      *output = new char[MAX_DATETIME_LENGTH];
    get_datetime_string(
      &current_time,
      *output,
      MAX_DATETIME_LENGTH,
      SHORT_DATE_TIME);
    break;

  case MACRO_DATE:
    if (*output == NULL)
      *output = new char[MAX_DATETIME_LENGTH];
    get_datetime_string(
      &current_time,
      *output,
      MAX_DATETIME_LENGTH,
      SHORT_DATE);
    break;

  case MACRO_TIME:
    if (*output == NULL)
      *output = new char[MAX_DATETIME_LENGTH];
    get_datetime_string(
      &current_time,
      *output,
      MAX_DATETIME_LENGTH,
      SHORT_TIME);
    break;

  case MACRO_TIMET:
    string::setstr(*output, current_time);
    break;

  case MACRO_ISVALIDTIME:
    string::setstr(
      *output,
      !check_time_against_period(test_time, temp_timeperiod, NULL));
    break;

  case MACRO_NEXTVALIDTIME:
    get_next_valid_time(
      test_time,
      &next_valid_time,
      temp_timeperiod,
      NULL);
    if (next_valid_time == test_time
        && check_time_against_period(
             test_time,
             temp_timeperiod,
             NULL) == ERROR)
      next_valid_time = (time_t)0L;
    string::setstr(*output, next_valid_time);
    break;

  default:
    return (ERROR);
  }
  return (OK);
}

int grab_datetime_macro(
      int macro_type,
      char const* arg1,
      char const* arg2,
      char** output) {
  return (grab_datetime_macro_r(
            get_global_macros(),
            macro_type,
            arg1,
            arg2,
            output));
}

/* computes a hostgroup macro */
int grab_standard_hostgroup_macro_r(
      nagios_macros* mac,
      int macro_type,
      hostgroup* temp_hostgroup,
      char** output) {
  hostsmember* temp_hostsmember = NULL;
  char* temp_buffer = NULL;
  unsigned int temp_len = 0;
  unsigned int init_len = 0;

  if (temp_hostgroup == NULL || output == NULL)
    return (ERROR);

  /* get the macro value */
  switch (macro_type) {
  case MACRO_HOSTGROUPNAME:
    *output = string::dup(temp_hostgroup->group_name);
    break;

  case MACRO_HOSTGROUPALIAS:
    if (temp_hostgroup->alias)
      *output = string::dup(temp_hostgroup->alias);
    break;

  case MACRO_HOSTGROUPMEMBERS:
    /* make the calculations for total string length */
    for (temp_hostsmember = temp_hostgroup->members;
         temp_hostsmember != NULL;
         temp_hostsmember = temp_hostsmember->next) {
      if (temp_hostsmember->host_name == NULL)
        continue;
      if (temp_len == 0)
        temp_len += strlen(temp_hostsmember->host_name) + 1;
      else
        temp_len += strlen(temp_hostsmember->host_name) + 2;
    }
    /* allocate or reallocate the memory buffer */
    if (*output == NULL)
      *output = new char[temp_len];
    else {
      init_len = strlen(*output);
      temp_len += init_len;
      *output = resize_string(*output, temp_len);
    }
    /* now fill in the string with the member names */
    for (temp_hostsmember = temp_hostgroup->members;
         temp_hostsmember != NULL;
         temp_hostsmember = temp_hostsmember->next) {
      if (temp_hostsmember->host_name == NULL)
        continue;
      temp_buffer = *output + init_len;
      if (init_len == 0)      /* If our buffer didn't contain anything, we just need to write "%s,%s" */
        init_len += sprintf(temp_buffer, "%s", temp_hostsmember->host_name);
      else
        init_len += sprintf(temp_buffer, ",%s", temp_hostsmember->host_name);
    }
    break;

  case MACRO_HOSTGROUPACTIONURL:
    if (temp_hostgroup->action_url)
      *output = string::dup(temp_hostgroup->action_url);
    break;

  case MACRO_HOSTGROUPNOTESURL:
    if (temp_hostgroup->notes_url)
      *output = string::dup(temp_hostgroup->notes_url);
    break;

  case MACRO_HOSTGROUPNOTES:
    if (temp_hostgroup->notes)
      *output = string::dup(temp_hostgroup->notes);
    break;

  default:
    logger(dbg_macros, basic)
      << "UNHANDLED HOSTGROUP MACRO #" << macro_type << "! THIS IS A BUG!";
    return (ERROR);
  }

  /* post-processing */
  /* notes, notes URL and action URL macros may themselves contain macros, so process them... */
  switch (macro_type) {
  case MACRO_HOSTGROUPACTIONURL:
  case MACRO_HOSTGROUPNOTESURL:
    process_macros_r(
      mac,
      *output,
      &temp_buffer,
      URL_ENCODE_MACRO_CHARS);
    delete[] *output;
    *output = temp_buffer;
    break;

  case MACRO_HOSTGROUPNOTES:
    process_macros_r(mac, *output, &temp_buffer, 0);
    delete[] *output;
    *output = temp_buffer;
    break;

  default:
    break;
  }

  return (OK);
}

int grab_standard_hostgroup_macro(
      int macro_type,
      hostgroup* temp_hostgroup,
      char** output) {
  return (grab_standard_hostgroup_macro_r(
            get_global_macros(),
            macro_type,
            temp_hostgroup,
            output));
}

/* computes a servicegroup macro */
int grab_standard_servicegroup_macro_r(
      nagios_macros* mac,
      int macro_type,
      servicegroup* temp_servicegroup,
      char** output) {
  servicesmember* temp_servicesmember = NULL;
  char* temp_buffer = NULL;
  unsigned int temp_len = 0;
  unsigned int init_len = 0;

  if (temp_servicegroup == NULL || output == NULL)
    return (ERROR);

  /* get the macro value */
  switch (macro_type) {
  case MACRO_SERVICEGROUPNAME:
    *output = string::dup(temp_servicegroup->group_name);
    break;

  case MACRO_SERVICEGROUPALIAS:
    if (temp_servicegroup->alias)
      *output = string::dup(temp_servicegroup->alias);
    break;

  case MACRO_SERVICEGROUPMEMBERS:
    /* make the calculations for total string length */
    for (temp_servicesmember = temp_servicegroup->members;
         temp_servicesmember != NULL;
         temp_servicesmember = temp_servicesmember->next) {
      if (temp_servicesmember->host_name == NULL
          || temp_servicesmember->service_description == NULL)
        continue;
      if (temp_len == 0) {
        temp_len +=
          strlen(temp_servicesmember->host_name) +
          strlen(temp_servicesmember->service_description) + 2;
      }
      else {
        temp_len +=
          strlen(temp_servicesmember->host_name) +
          strlen(temp_servicesmember->service_description) + 3;
      }
    }
    /* allocate or reallocate the memory buffer */
    if (*output == NULL)
      *output = new char[temp_len];
    else {
      init_len = strlen(*output);
      temp_len += init_len;
      *output = resize_string(*output, temp_len);
    }
    /* now fill in the string with the group members */
    for (temp_servicesmember = temp_servicegroup->members;
         temp_servicesmember != NULL;
         temp_servicesmember = temp_servicesmember->next) {
      if (temp_servicesmember->host_name == NULL
          || temp_servicesmember->service_description == NULL)
        continue;
      temp_buffer = *output + init_len;
      if (init_len == 0)      /* If our buffer didn't contain anything, we just need to write "%s,%s" */
        init_len += sprintf(
                      temp_buffer,
                      "%s,%s",
                      temp_servicesmember->host_name,
                      temp_servicesmember->service_description);
      else                    /* Now we need to write ",%s,%s" */
        init_len += sprintf(
                      temp_buffer,
                      ",%s,%s",
                      temp_servicesmember->host_name,
                      temp_servicesmember->service_description);
    }
    break;
  case MACRO_SERVICEGROUPACTIONURL:
    if (temp_servicegroup->action_url)
      *output = string::dup(temp_servicegroup->action_url);
    break;

  case MACRO_SERVICEGROUPNOTESURL:
    if (temp_servicegroup->notes_url)
      *output = string::dup(temp_servicegroup->notes_url);
    break;

  case MACRO_SERVICEGROUPNOTES:
    if (temp_servicegroup->notes)
      *output = string::dup(temp_servicegroup->notes);
    break;

  default:
    logger(dbg_macros, basic)
      << "UNHANDLED SERVICEGROUP MACRO #" << macro_type
      << "! THIS IS A BUG!";
    return (ERROR);
  }

  /* post-processing */
  /* notes, notes URL and action URL macros may themselves contain macros, so process them... */
  switch (macro_type) {
  case MACRO_SERVICEGROUPACTIONURL:
  case MACRO_SERVICEGROUPNOTESURL:
    process_macros_r(
      mac,
      *output,
      &temp_buffer,
      URL_ENCODE_MACRO_CHARS);
    delete[] *output;
    *output = temp_buffer;
    break;

  case MACRO_SERVICEGROUPNOTES:
    process_macros_r(mac, *output, &temp_buffer, 0);
    delete[] *output;
    *output = temp_buffer;
    break;

  default:
    break;
  }
  return (OK);
}

int grab_standard_servicegroup_macro(
      int macro_type,
      servicegroup* temp_servicegroup,
      char** output) {
  return (grab_standard_servicegroup_macro_r(
            get_global_macros(),
            macro_type,
            temp_servicegroup,
            output));
}

/* computes a contact macro */
int grab_standard_contact_macro_r(
      nagios_macros* mac,
      int macro_type,
      contact* temp_contact,
      char** output) {
  contactgroup* temp_contactgroup = NULL;
  objectlist* temp_objectlist = NULL;

  (void)mac;

  if (temp_contact == NULL || output == NULL)
    return (ERROR);

  /* get the macro value */
  switch (macro_type) {
  case MACRO_CONTACTNAME:
    *output = string::dup(temp_contact->name);
    break;

  case MACRO_CONTACTALIAS:
    *output = string::dup(temp_contact->alias);
    break;

  case MACRO_CONTACTEMAIL:
    if (temp_contact->email)
      *output = string::dup(temp_contact->email);
    break;

  case MACRO_CONTACTPAGER:
    if (temp_contact->pager)
      *output = string::dup(temp_contact->pager);
    break;

  case MACRO_CONTACTGROUPNAMES: {
    std::string buf;
    /* get the contactgroup names */
    /* find all contactgroups this contact is a member of */
    for (temp_objectlist = temp_contact->contactgroups_ptr;
         temp_objectlist != NULL;
         temp_objectlist = temp_objectlist->next) {
      if ((temp_contactgroup = (contactgroup*)temp_objectlist->object_ptr) == NULL)
        continue;

      if (!buf.empty())
        buf.append(",");
      buf.append(temp_contactgroup->group_name);
    }
    if (!buf.empty())
      *output = string::dup(buf);
  }
    break;

  default:
    logger(dbg_macros, basic)
      << "UNHANDLED CONTACT MACRO #" << macro_type
      << "! THIS IS A BUG!";
    return (ERROR);
  }
  return (OK);
}

int grab_standard_contact_macro(
      int macro_type,
      contact* temp_contact,
      char** output) {
  return (grab_standard_contact_macro_r(
            get_global_macros(),
            macro_type,
            temp_contact,
            output));
}

/* computes a contact address macro */
int grab_contact_address_macro(
      unsigned int macro_num,
      contact* temp_contact,
      char** output) {
  if (macro_num >= MAX_CONTACT_ADDRESSES)
    return (ERROR);

  if (temp_contact == NULL || output == NULL)
    return (ERROR);

  /* get the macro */
  if (temp_contact->address[macro_num])
    *output = string::dup(temp_contact->address[macro_num]);
  return (OK);
}

/* computes a contactgroup macro */
int grab_standard_contactgroup_macro(
      int macro_type,
      contactgroup* temp_contactgroup,
      char** output) {
  contactsmember* temp_contactsmember = NULL;

  if (temp_contactgroup == NULL || output == NULL)
    return (ERROR);

  /* get the macro value */
  switch (macro_type) {
  case MACRO_CONTACTGROUPNAME:
    *output = string::dup(temp_contactgroup->group_name);
    break;

  case MACRO_CONTACTGROUPALIAS:
    if (temp_contactgroup->alias)
      *output = string::dup(temp_contactgroup->alias);
    break;

  case MACRO_CONTACTGROUPMEMBERS:
    /* get the member list */
    for (temp_contactsmember = temp_contactgroup->members;
         temp_contactsmember != NULL;
         temp_contactsmember = temp_contactsmember->next) {
      if (temp_contactsmember->contact_name == NULL)
        continue;
      if (*output == NULL)
        *output = string::dup(temp_contactsmember->contact_name);
      else {
        *output = resize_string(
                    *output,
                    strlen(*output) + strlen(temp_contactsmember->contact_name) + 2);
        strcat(*output, ",");
        strcat(*output, temp_contactsmember->contact_name);
      }
    }
    break;

  default:
    logger(dbg_macros, basic)
      << "UNHANDLED CONTACTGROUP MACRO #" << macro_type
      << "! THIS IS A BUG!";
      return (ERROR);
  }
  return (OK);
}

/* computes a custom object macro */
int grab_custom_object_macro_r(
      nagios_macros* mac,
      char* macro_name,
      customvariablesmember* vars,
      char** output) {
  customvariablesmember* temp_customvariablesmember = NULL;
  int result = ERROR;

  (void)mac;

  if (macro_name == NULL || vars == NULL || output == NULL)
    return (ERROR);

  /* get the custom variable */
  for (temp_customvariablesmember = vars;
       temp_customvariablesmember != NULL;
       temp_customvariablesmember = temp_customvariablesmember->next) {

    if (temp_customvariablesmember->variable_name == NULL)
      continue;

    if (!strcmp(macro_name, temp_customvariablesmember->variable_name)) {
      if (temp_customvariablesmember->variable_value)
        *output = string::dup(temp_customvariablesmember->variable_value);
      result = OK;
      break;
    }
  }
  return (result);
}

int grab_custom_object_macro(
      char* macro_name,
      customvariablesmember* vars,
      char** output) {
  return (grab_custom_object_macro_r(
            get_global_macros(),
            macro_name,
            vars,
            output));
}

/******************************************************************/
/********************* MACRO STRING FUNCTIONS *********************/
/******************************************************************/

/* cleans illegal characters in macros before output */
char const* clean_macro_chars(char* macro, int options) {
  if (macro == NULL)
    return ("");

  int len((int)strlen(macro));

  /* strip illegal characters out of macro */
  if (options & STRIP_ILLEGAL_MACRO_CHARS) {
    int y(0);
    for (int x(0); x < len; x++) {
      /*ch=(int)macro[x]; */
      /* allow non-ASCII characters (Japanese, etc) */
      int ch(macro[x] & 0xff);

      /* illegal ASCII characters */
      if (ch < 32 || ch == 127)
        continue;

      /* illegal user-specified characters */
      if (config->illegal_output_chars().find(ch) == std::string::npos)
        macro[y++] = macro[x];
    }

    macro[y++] = '\x0';
  }
  return (macro);
}

/* encodes a string in proper URL format */
char* get_url_encoded_string(char* input) {
  int x = 0;
  int y = 0;
  char* encoded_url_string = NULL;
  char temp_expansion[6] = "";

  /* bail if no input */
  if (input == NULL)
    return (NULL);

  /* allocate enough memory to escape all characters if necessary */
  encoded_url_string = new char[strlen(input) * 3 + 1];

  /* check/encode all characters */
  for (x = 0, y = 0; input[x] != (char)'\x0'; x++) {
    /* alpha-numeric characters and a few other characters don't get encoded */
    if (((char)input[x] >= '0' && (char)input[x] <= '9')
        || ((char)input[x] >= 'A' && (char)input[x] <= 'Z')
        || ((char)input[x] >= (char)'a' && (char)input[x] <= (char)'z')
        || (char)input[x] == (char)'.' || (char)input[x] == (char)'-'
        || (char)input[x] == (char)'_' || (char)input[x] == (char)':'
        || (char)input[x] == (char)'/' || (char)input[x] == (char)'?'
        || (char)input[x] == (char)'=' || (char)input[x] == (char)'&')
      encoded_url_string[y++] = input[x];
    /* spaces are pluses */
    else if (input[x] == ' ')
      encoded_url_string[y++] = '+';
    /* anything else gets represented by its hex value */
    else {
      encoded_url_string[y] = '\x0';
      sprintf(temp_expansion, "%%%02X", (unsigned int)(input[x] & 0xFF));
      strcat(encoded_url_string, temp_expansion);
      y += 3;
    }
  }

  /* terminate encoded string */
  encoded_url_string[y] = '\x0';
  return (encoded_url_string);
}

/******************************************************************/
/***************** MACRO INITIALIZATION FUNCTIONS *****************/
/******************************************************************/

/* initializes global macros */
int init_macros() {
  init_macrox_names();

  /*
   * non-volatile macros are free()'d when they're set.
   * We must do this in order to not lose the constant
   * ones when we get SIGHUP or a RESTART_PROGRAM event
   * from the command fifo. Otherwise a memset() would
   * have been better.
   */
  clear_volatile_macros_r(get_global_macros());

  /* backwards compatibility hack */
  macro_x = get_global_macros()->x;
  return (OK);
}

/*
 * initializes the names of macros, using this nifty little macro
 * which ensures we never add any typos to the list
 */
#define add_macrox_name(name) macro_x_names[MACRO_##name] = string::dup(#name)
int init_macrox_names() {
  unsigned int x = 0;

  /* initialize macro names */
  for (x = 0; x < MACRO_X_COUNT; x++)
    macro_x_names[x] = NULL;

  /* initialize each macro name */
  add_macrox_name(HOSTNAME);
  add_macrox_name(HOSTALIAS);
  add_macrox_name(HOSTADDRESS);
  add_macrox_name(SERVICEDESC);
  add_macrox_name(SERVICESTATE);
  add_macrox_name(SERVICESTATEID);
  add_macrox_name(SERVICEATTEMPT);
  add_macrox_name(SERVICEISVOLATILE);
  add_macrox_name(LONGDATETIME);
  add_macrox_name(SHORTDATETIME);
  add_macrox_name(DATE);
  add_macrox_name(TIME);
  add_macrox_name(TIMET);
  add_macrox_name(LASTHOSTCHECK);
  add_macrox_name(LASTSERVICECHECK);
  add_macrox_name(LASTHOSTSTATECHANGE);
  add_macrox_name(LASTSERVICESTATECHANGE);
  add_macrox_name(HOSTOUTPUT);
  add_macrox_name(SERVICEOUTPUT);
  add_macrox_name(HOSTPERFDATA);
  add_macrox_name(SERVICEPERFDATA);
  add_macrox_name(CONTACTNAME);
  add_macrox_name(CONTACTALIAS);
  add_macrox_name(CONTACTEMAIL);
  add_macrox_name(CONTACTPAGER);
  add_macrox_name(ADMINEMAIL);
  add_macrox_name(ADMINPAGER);
  add_macrox_name(HOSTSTATE);
  add_macrox_name(HOSTSTATEID);
  add_macrox_name(HOSTATTEMPT);
  add_macrox_name(NOTIFICATIONTYPE);
  add_macrox_name(NOTIFICATIONNUMBER);
  add_macrox_name(NOTIFICATIONISESCALATED);
  add_macrox_name(HOSTEXECUTIONTIME);
  add_macrox_name(SERVICEEXECUTIONTIME);
  add_macrox_name(HOSTLATENCY);
  add_macrox_name(SERVICELATENCY);
  add_macrox_name(HOSTDURATION);
  add_macrox_name(SERVICEDURATION);
  add_macrox_name(HOSTDURATIONSEC);
  add_macrox_name(SERVICEDURATIONSEC);
  add_macrox_name(HOSTDOWNTIME);
  add_macrox_name(SERVICEDOWNTIME);
  add_macrox_name(HOSTSTATETYPE);
  add_macrox_name(SERVICESTATETYPE);
  add_macrox_name(HOSTPERCENTCHANGE);
  add_macrox_name(SERVICEPERCENTCHANGE);
  add_macrox_name(HOSTGROUPNAME);
  add_macrox_name(HOSTGROUPALIAS);
  add_macrox_name(SERVICEGROUPNAME);
  add_macrox_name(SERVICEGROUPALIAS);
  add_macrox_name(HOSTACKAUTHOR);
  add_macrox_name(HOSTACKCOMMENT);
  add_macrox_name(SERVICEACKAUTHOR);
  add_macrox_name(SERVICEACKCOMMENT);
  add_macrox_name(LASTSERVICEOK);
  add_macrox_name(LASTSERVICEWARNING);
  add_macrox_name(LASTSERVICEUNKNOWN);
  add_macrox_name(LASTSERVICECRITICAL);
  add_macrox_name(LASTHOSTUP);
  add_macrox_name(LASTHOSTDOWN);
  add_macrox_name(LASTHOSTUNREACHABLE);
  add_macrox_name(SERVICECHECKCOMMAND);
  add_macrox_name(HOSTCHECKCOMMAND);
  add_macrox_name(MAINCONFIGFILE);
  add_macrox_name(STATUSDATAFILE);
  add_macrox_name(HOSTDISPLAYNAME);
  add_macrox_name(SERVICEDISPLAYNAME);
  add_macrox_name(RETENTIONDATAFILE);
  add_macrox_name(OBJECTCACHEFILE);
  add_macrox_name(TEMPFILE);
  add_macrox_name(LOGFILE);
  add_macrox_name(RESOURCEFILE);
  add_macrox_name(COMMANDFILE);
  add_macrox_name(HOSTPERFDATAFILE);
  add_macrox_name(SERVICEPERFDATAFILE);
  add_macrox_name(HOSTACTIONURL);
  add_macrox_name(HOSTNOTESURL);
  add_macrox_name(HOSTNOTES);
  add_macrox_name(SERVICEACTIONURL);
  add_macrox_name(SERVICENOTESURL);
  add_macrox_name(SERVICENOTES);
  add_macrox_name(TOTALHOSTSUP);
  add_macrox_name(TOTALHOSTSDOWN);
  add_macrox_name(TOTALHOSTSUNREACHABLE);
  add_macrox_name(TOTALHOSTSDOWNUNHANDLED);
  add_macrox_name(TOTALHOSTSUNREACHABLEUNHANDLED);
  add_macrox_name(TOTALHOSTPROBLEMS);
  add_macrox_name(TOTALHOSTPROBLEMSUNHANDLED);
  add_macrox_name(TOTALSERVICESOK);
  add_macrox_name(TOTALSERVICESWARNING);
  add_macrox_name(TOTALSERVICESCRITICAL);
  add_macrox_name(TOTALSERVICESUNKNOWN);
  add_macrox_name(TOTALSERVICESWARNINGUNHANDLED);
  add_macrox_name(TOTALSERVICESCRITICALUNHANDLED);
  add_macrox_name(TOTALSERVICESUNKNOWNUNHANDLED);
  add_macrox_name(TOTALSERVICEPROBLEMS);
  add_macrox_name(TOTALSERVICEPROBLEMSUNHANDLED);
  add_macrox_name(PROCESSSTARTTIME);
  add_macrox_name(HOSTCHECKTYPE);
  add_macrox_name(SERVICECHECKTYPE);
  add_macrox_name(LONGHOSTOUTPUT);
  add_macrox_name(LONGSERVICEOUTPUT);
  add_macrox_name(TEMPPATH);
  add_macrox_name(HOSTNOTIFICATIONNUMBER);
  add_macrox_name(SERVICENOTIFICATIONNUMBER);
  add_macrox_name(HOSTNOTIFICATIONID);
  add_macrox_name(SERVICENOTIFICATIONID);
  add_macrox_name(HOSTEVENTID);
  add_macrox_name(LASTHOSTEVENTID);
  add_macrox_name(SERVICEEVENTID);
  add_macrox_name(LASTSERVICEEVENTID);
  add_macrox_name(HOSTGROUPNAMES);
  add_macrox_name(SERVICEGROUPNAMES);
  add_macrox_name(HOSTACKAUTHORNAME);
  add_macrox_name(HOSTACKAUTHORALIAS);
  add_macrox_name(SERVICEACKAUTHORNAME);
  add_macrox_name(SERVICEACKAUTHORALIAS);
  add_macrox_name(MAXHOSTATTEMPTS);
  add_macrox_name(MAXSERVICEATTEMPTS);
  add_macrox_name(TOTALHOSTSERVICES);
  add_macrox_name(TOTALHOSTSERVICESOK);
  add_macrox_name(TOTALHOSTSERVICESWARNING);
  add_macrox_name(TOTALHOSTSERVICESUNKNOWN);
  add_macrox_name(TOTALHOSTSERVICESCRITICAL);
  add_macrox_name(HOSTGROUPNOTES);
  add_macrox_name(HOSTGROUPNOTESURL);
  add_macrox_name(HOSTGROUPACTIONURL);
  add_macrox_name(SERVICEGROUPNOTES);
  add_macrox_name(SERVICEGROUPNOTESURL);
  add_macrox_name(SERVICEGROUPACTIONURL);
  add_macrox_name(HOSTGROUPMEMBERS);
  add_macrox_name(SERVICEGROUPMEMBERS);
  add_macrox_name(CONTACTGROUPNAME);
  add_macrox_name(CONTACTGROUPALIAS);
  add_macrox_name(CONTACTGROUPMEMBERS);
  add_macrox_name(CONTACTGROUPNAMES);
  add_macrox_name(NOTIFICATIONRECIPIENTS);
  add_macrox_name(NOTIFICATIONAUTHOR);
  add_macrox_name(NOTIFICATIONAUTHORNAME);
  add_macrox_name(NOTIFICATIONAUTHORALIAS);
  add_macrox_name(NOTIFICATIONCOMMENT);
  add_macrox_name(EVENTSTARTTIME);
  add_macrox_name(HOSTPROBLEMID);
  add_macrox_name(LASTHOSTPROBLEMID);
  add_macrox_name(SERVICEPROBLEMID);
  add_macrox_name(LASTSERVICEPROBLEMID);
  add_macrox_name(ISVALIDTIME);
  add_macrox_name(NEXTVALIDTIME);
  add_macrox_name(LASTHOSTSTATE);
  add_macrox_name(LASTHOSTSTATEID);
  add_macrox_name(LASTSERVICESTATE);
  add_macrox_name(LASTSERVICESTATEID);

  return (OK);
}

/******************************************************************/
/********************* MACRO CLEANUP FUNCTIONS ********************/
/******************************************************************/

/* free memory associated with the macrox names */
int free_macrox_names() {
  unsigned int x = 0;

  /* free each macro name */
  for (x = 0; x < MACRO_X_COUNT; x++) {
    delete[] macro_x_names[x];
    macro_x_names[x] = NULL;
  }
  return (OK);
}

/* clear argv macros - used in commands */
int clear_argv_macros_r(nagios_macros* mac) {
  unsigned int x = 0;

  /* command argument macros */
  for (x = 0; x < MAX_COMMAND_ARGUMENTS; x++) {
    delete[] mac->argv[x];
    mac->argv[x] = NULL;
  }
  return (OK);
}

int clear_argv_macros() {
  return (clear_argv_macros_r(get_global_macros()));
}

/*
 * copies non-volatile macros from global macro_x to **dest, which
 * must be large enough to hold at least MACRO_X_COUNT entries.
 * We use a shortlived macro to save up on typing
 */
#define cp_macro(name) dest[MACRO_##name] = get_global_macros()->x[MACRO_##name]
void copy_constant_macros(char** dest) {
  cp_macro(ADMINEMAIL);
  cp_macro(ADMINPAGER);
  cp_macro(MAINCONFIGFILE);
  cp_macro(STATUSDATAFILE);
  cp_macro(RETENTIONDATAFILE);
  cp_macro(OBJECTCACHEFILE);
  cp_macro(TEMPFILE);
  cp_macro(LOGFILE);
  cp_macro(RESOURCEFILE);
  cp_macro(COMMANDFILE);
  cp_macro(HOSTPERFDATAFILE);
  cp_macro(SERVICEPERFDATAFILE);
  cp_macro(PROCESSSTARTTIME);
  cp_macro(TEMPPATH);
  cp_macro(EVENTSTARTTIME);
  return;
}
#undef cp_macro

/* clear all macros that are not "constant" (i.e. they change throughout the course of monitoring) */
int clear_volatile_macros_r(nagios_macros* mac) {
  customvariablesmember* this_customvariablesmember = NULL;
  customvariablesmember* next_customvariablesmember = NULL;
  unsigned int x = 0;

  for (x = 0; x < MACRO_X_COUNT; x++) {
    switch (x) {

    case MACRO_ADMINEMAIL:
    case MACRO_ADMINPAGER:
    case MACRO_MAINCONFIGFILE:
    case MACRO_STATUSDATAFILE:
    case MACRO_RETENTIONDATAFILE:
    case MACRO_OBJECTCACHEFILE:
    case MACRO_TEMPFILE:
    case MACRO_LOGFILE:
    case MACRO_RESOURCEFILE:
    case MACRO_COMMANDFILE:
    case MACRO_HOSTPERFDATAFILE:
    case MACRO_SERVICEPERFDATAFILE:
    case MACRO_PROCESSSTARTTIME:
    case MACRO_TEMPPATH:
    case MACRO_EVENTSTARTTIME:
      /* these don't change during the course of monitoring, so no need to free them */
      break;

    default:
      delete[] mac->x[x];
      mac->x[x] = NULL;
      break;
    }
  }

  /* contact address macros */
  for (x = 0; x < MAX_CONTACT_ADDRESSES; x++) {
    delete[] mac->contactaddress[x];
    mac->contactaddress[x] = NULL;
  }

  /* clear macro pointers */
  mac->host_ptr = NULL;
  mac->hostgroup_ptr = NULL;
  mac->service_ptr = NULL;
  mac->servicegroup_ptr = NULL;
  mac->contact_ptr = NULL;
  mac->contactgroup_ptr = NULL;

  /* clear on-demand macro */
  delete[] mac->ondemand;
  mac->ondemand = NULL;

  /* clear ARGx macros */
  clear_argv_macros_r(mac);

  /* clear custom host variables */
  for (this_customvariablesmember = mac->custom_host_vars;
       this_customvariablesmember != NULL;
       this_customvariablesmember = next_customvariablesmember) {
    next_customvariablesmember = this_customvariablesmember->next;
    delete[] this_customvariablesmember->variable_name;
    delete[] this_customvariablesmember->variable_value;
    delete this_customvariablesmember;
  }
  mac->custom_host_vars = NULL;

  /* clear custom service variables */
  for (this_customvariablesmember = mac->custom_service_vars;
       this_customvariablesmember != NULL;
       this_customvariablesmember = next_customvariablesmember) {
    next_customvariablesmember = this_customvariablesmember->next;
    delete[] this_customvariablesmember->variable_name;
    delete[] this_customvariablesmember->variable_value;
    delete this_customvariablesmember;
  }
  mac->custom_service_vars = NULL;

  /* clear custom contact variables */
  for (this_customvariablesmember = mac->custom_contact_vars;
       this_customvariablesmember != NULL;
       this_customvariablesmember = next_customvariablesmember) {
    next_customvariablesmember = this_customvariablesmember->next;
    delete[] this_customvariablesmember->variable_name;
    delete[] this_customvariablesmember->variable_value;
    delete this_customvariablesmember;
  }
  mac->custom_contact_vars = NULL;

  return (OK);
}

int clear_volatile_macros() {
  return (clear_volatile_macros_r(get_global_macros()));
}

/* clear contact macros */
int clear_contact_macros_r(nagios_macros* mac) {
  unsigned int x;
  customvariablesmember* this_customvariablesmember = NULL;
  customvariablesmember* next_customvariablesmember = NULL;

  for (x = 0; x < MACRO_X_COUNT; x++) {
    switch (x) {
    case MACRO_CONTACTNAME:
    case MACRO_CONTACTALIAS:
    case MACRO_CONTACTEMAIL:
    case MACRO_CONTACTPAGER:
    case MACRO_CONTACTGROUPNAMES:
      delete[] mac->x[x];
      mac->x[x] = NULL;
      break;

    default:
      break;
    }
  }

  /* clear contact addresses */
  for (x = 0; x < MAX_CONTACT_ADDRESSES; x++) {
    delete[] mac->contactaddress[x];
    mac->contactaddress[x] = NULL;
  }

  /* clear custom contact variables */
  for (this_customvariablesmember = mac->custom_contact_vars;
       this_customvariablesmember != NULL;
       this_customvariablesmember = next_customvariablesmember) {
    next_customvariablesmember = this_customvariablesmember->next;
    delete[] this_customvariablesmember->variable_name;
    delete[] this_customvariablesmember->variable_value;
    delete this_customvariablesmember;
  }
  mac->custom_contact_vars = NULL;

  /* clear pointers */
  mac->contact_ptr = NULL;

  return (OK);
}

int clear_contact_macros() {
  return (clear_contact_macros_r(get_global_macros()));
}

/* clear contactgroup macros */
int clear_contactgroup_macros_r(nagios_macros* mac) {
  unsigned int x;

  for (x = 0; x < MACRO_X_COUNT; x++) {
    switch (x) {
    case MACRO_CONTACTGROUPNAME:
    case MACRO_CONTACTGROUPALIAS:
    case MACRO_CONTACTGROUPMEMBERS:
      delete[] mac->x[x];
      mac->x[x] = NULL;
      break;

    default:
      break;
    }
  }

  /* clear pointers */
  mac->contactgroup_ptr = NULL;

  return (OK);
}

int clear_contactgroup_macros() {
  return (clear_contactgroup_macros_r(get_global_macros()));
}

/* clear summary macros */
int clear_summary_macros_r(nagios_macros* mac) {
  unsigned int x;

  for (x = MACRO_TOTALHOSTSUP;
       x <= MACRO_TOTALSERVICEPROBLEMSUNHANDLED;
       x++) {
    delete[] mac->x[x];
    mac->x[x] = NULL;
  }

  return (OK);
}

int clear_summary_macros() {
  return (clear_summary_macros_r(get_global_macros()));
}

/******************************************************************/
/****************** ENVIRONMENT MACRO FUNCTIONS *******************/
/******************************************************************/

/* sets or unsets all macro environment variables */
int set_all_macro_environment_vars_r(nagios_macros* mac, int set) {
  if (config->enable_environment_macros() == false)
    return (ERROR);

  set_macrox_environment_vars_r(mac, set);
  set_argv_macro_environment_vars_r(mac, set);
  set_custom_macro_environment_vars_r(mac, set);
  set_contact_address_environment_vars_r(mac, set);

  return (OK);
}

int set_all_macro_environment_vars(int set) {
  return (set_all_macro_environment_vars_r(get_global_macros(), set));
}

/* sets or unsets macrox environment variables */
int set_macrox_environment_vars_r(nagios_macros* mac, int set) {
  unsigned int x = 0;
  int free_macro = false;
  int generate_macro = true;

  /* set each of the macrox environment variables */
  for (x = 0; x < MACRO_X_COUNT; x++) {
    free_macro = false;

    /* generate the macro value if it hasn't already been done */
    /* THIS IS EXPENSIVE */
    if (set == true) {
      generate_macro = true;

      /* skip summary macro generation if lage installation tweaks are enabled */
      if ((x >= MACRO_TOTALHOSTSUP
           && x <= MACRO_TOTALSERVICEPROBLEMSUNHANDLED)
          && config->use_large_installation_tweaks() == true)
        generate_macro = false;

      if (mac->x[x] == NULL && generate_macro == true)
        grab_macrox_value_r(
          mac,
          x,
          NULL,
          NULL,
          &mac->x[x],
          &free_macro);
    }

    /* set the value */
    set_macro_environment_var(macro_x_names[x], mac->x[x], set);
  }

  return (OK);
}

int set_macrox_environment_vars(int set) {
  return (set_macrox_environment_vars_r(get_global_macros(), set));
}

/* sets or unsets argv macro environment variables */
int set_argv_macro_environment_vars_r(nagios_macros* mac, int set) {
  /* set each of the argv macro environment variables */
  for (unsigned int x(0); x < MAX_COMMAND_ARGUMENTS; x++) {
    std::ostringstream oss;
    oss << "ARG" << x + 1;
    set_macro_environment_var(oss.str().c_str(), mac->argv[x], set);
  }

  return (OK);
}

int set_argv_macro_environment_vars(int set) {
  return (set_argv_macro_environment_vars_r(get_global_macros(), set));
}

/* sets or unsets custom host/service/contact macro environment variables */
int set_custom_macro_environment_vars_r(nagios_macros* mac, int set) {
  customvariablesmember* temp_customvariablesmember = NULL;
  host* temp_host = NULL;
  service* temp_service = NULL;
  contact* temp_contact = NULL;

  /***** CUSTOM HOST VARIABLES *****/
  /* generate variables and save them for later */
  if ((temp_host = mac->host_ptr) && set == true) {
    for (temp_customvariablesmember = temp_host->custom_variables;
         temp_customvariablesmember != NULL;
         temp_customvariablesmember = temp_customvariablesmember->next) {
      std::string var("_HOST");
      var.append(temp_customvariablesmember->variable_name);
      add_custom_variable_to_object(
        &mac->custom_host_vars,
        var.c_str(),
        temp_customvariablesmember->variable_value);
    }
  }
  /* set variables */
  for (temp_customvariablesmember = mac->custom_host_vars;
       temp_customvariablesmember != NULL;
       temp_customvariablesmember = temp_customvariablesmember->next) {
    set_macro_environment_var(
      temp_customvariablesmember->variable_name,
      clean_macro_chars(
        temp_customvariablesmember->variable_value,
        STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS),
      set);
  }

  /***** CUSTOM SERVICE VARIABLES *****/
  /* generate variables and save them for later */
  if ((temp_service = mac->service_ptr) && set == true) {
    for (temp_customvariablesmember = temp_service->custom_variables;
         temp_customvariablesmember != NULL;
         temp_customvariablesmember = temp_customvariablesmember->next) {
      std::string var("_SERVICE");
      var.append(temp_customvariablesmember->variable_name);
      add_custom_variable_to_object(
        &mac->custom_service_vars,
        var.c_str(),
        temp_customvariablesmember->variable_value);
    }
  }
  /* set variables */
  for (temp_customvariablesmember = mac->custom_service_vars;
       temp_customvariablesmember != NULL;
       temp_customvariablesmember = temp_customvariablesmember->next)
    set_macro_environment_var(
      temp_customvariablesmember->variable_name,
      clean_macro_chars(
        temp_customvariablesmember->variable_value,
        STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS),
      set);

  /***** CUSTOM CONTACT VARIABLES *****/
  /* generate variables and save them for later */
  if ((temp_contact = mac->contact_ptr) && set == true) {
    for (temp_customvariablesmember = temp_contact->custom_variables;
         temp_customvariablesmember != NULL;
         temp_customvariablesmember = temp_customvariablesmember->next) {
      std::string var("_CONTACT");
      var.append(temp_customvariablesmember->variable_name);
      add_custom_variable_to_object(
        &mac->custom_contact_vars,
        var.c_str(),
        temp_customvariablesmember->variable_value);
    }
  }
  /* set variables */
  for (temp_customvariablesmember = mac->custom_contact_vars;
       temp_customvariablesmember != NULL;
       temp_customvariablesmember = temp_customvariablesmember->next)
    set_macro_environment_var(
      temp_customvariablesmember->variable_name,
      clean_macro_chars(
        temp_customvariablesmember->variable_value,
        STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS),
      set);

  return (OK);
}

int set_custom_macro_environment_vars(int set) {
  return (set_custom_macro_environment_vars_r(
            get_global_macros(),
            set));
}

/* sets or unsets contact address environment variables */
int set_contact_address_environment_vars_r(
      nagios_macros* mac,
      int set) {
  /* these only get set during notifications */
  if (mac->contact_ptr == NULL)
    return (OK);

  for (unsigned int x(0); x < MAX_CONTACT_ADDRESSES; x++) {
    std::ostringstream oss;
    oss << "CONTACTADDRESS" << x;
    set_macro_environment_var(
      oss.str().c_str(),
      mac->contact_ptr->address[x],
      set);
  }

  return (OK);
}

int set_contact_address_environment_vars(int set) {
  return (set_contact_address_environment_vars_r(
            get_global_macros(),
            set));
}

/* sets or unsets a macro environment variable */
int set_macro_environment_var(
      char const* name,
      char const* value,
      int set) {
  /* we won't mess with null variable names */
  if (name == NULL)
    return (ERROR);

  /* create environment var name */
  std::string var(MACRO_ENV_VAR_PREFIX);
  var.append(name);

  /* set or unset the environment variable */
  set_environment_var(var.c_str(), value, set);

  return (OK);
}
