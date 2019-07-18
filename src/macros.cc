/*
** Copyright 1999-2010 Ethan Galstad
** Copyright 2011-201016 Centreon
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
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/engine/utils.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;

/******************************************************************/
/********************** MACRO GRAB FUNCTIONS **********************/
/******************************************************************/

/* grab macros that are specific to a particular contact */
int grab_contact_macros_r(nagios_macros* mac, contact* cntct) {
  /* clear contact-related macros */
  clear_contact_macros_r(mac);
  clear_contactgroup_macros_r(mac);

  /* save pointer to contact for later */
  mac->contact_ptr = cntct;
  mac->contactgroup_ptr = nullptr;

  if (cntct == nullptr)
    return ERROR;

  /* save pointer to first/primary contactgroup for later */
  if (!cntct->get_parent_groups().empty())
    mac->contactgroup_ptr
      = cntct->get_parent_groups().front();
  return OK;
}


/******************************************************************/
/******************* MACRO GENERATION FUNCTIONS *******************/
/******************************************************************/

/* calculates the value of a custom macro */
int grab_custom_macro_value_r(
      nagios_macros* mac,
      std::string const& macro_name,
      std::string const& arg1,
      std::string const& arg2,
      std::string& output) {
  std::string temp_buffer;
  int result = OK;

  /***** CUSTOM HOST MACRO *****/
  if (macro_name.rfind("_HOST", 0) == 0) {

    host *temp_host(nullptr);
    /* a standard host macro */
    if (arg2.empty()) {
      /* find the host for on-demand macros */
      if (!arg1.empty()) {
        host_map::const_iterator it = host::hosts.find(arg1);
        if (it != host::hosts.end())
          temp_host = it->second.get();

        if(temp_host == nullptr)
          return ERROR;
      }
      /* else use saved host pointer */
      else if ((temp_host = mac->host_ptr) == nullptr)
        return ERROR;

      /* get the host macro value */
      result = grab_custom_object_macro_r(
                 mac,
                 macro_name.substr(5),
                 temp_host->custom_variables,
                 output);
    }
    /* a host macro with a hostgroup name and delimiter */
    else {
      hostgroup_map::const_iterator
        it_hg(hostgroup::hostgroups.find(arg1));
      if (it_hg == hostgroup::hostgroups.end() || it_hg->second == nullptr)
        return ERROR;

      /* concatenate macro values for all hostgroup members */
      for (host_map_unsafe::iterator
             it{it_hg->second->members.begin()},
             end{it_hg->second->members.begin()};
           it != end;
           ++it) {

        if (!it->second)
          continue;

        /* get the macro value for this host */
        grab_custom_macro_value_r(
          mac,
          macro_name,
          it->first,
          "",
          temp_buffer);

        if (temp_buffer.empty())
          continue;

        /* add macro value to already running macro */
        if (output.empty())
          output = temp_buffer;
        else {
          output.append(arg2);
          output.append(temp_buffer);
        }
        temp_buffer = "";
      }
    }
  }
  /***** CUSTOM SERVICE MACRO *****/
  else if (macro_name.rfind("_SERVICE", 0) == 0) {
    com::centreon::engine::service* temp_service(nullptr);

    /* use saved service pointer */
    if (arg1.empty() && arg2.empty()) {
      if ((temp_service = mac->service_ptr) == nullptr)
        return ERROR;

      /* get the service macro value */
      result = grab_custom_object_macro_r(
                 mac,
                 macro_name.substr(8),
                 temp_service->custom_variables,
                 output);
    }
    /* else and ondemand macro... */
    else {
      /* if first arg is blank, it means use the current host name */
      if (mac->host_ptr == nullptr)
        return ERROR;

      service_map::const_iterator found = service::services.find(
        {mac->host_ptr ? mac->host_ptr->get_name(): "", arg2});

      if (found != service::services.end() && found->second) {

        /* get the service macro value */
        result = grab_custom_object_macro_r(
                   mac,
                   macro_name.substr(8),
                   found->second->custom_variables,
                   output);
      }
      /* else we have a service macro with a servicegroup name and a delimiter... */
      else {
        servicegroup_map::const_iterator sg_it{servicegroup::servicegroups.find(arg1)};
        if (sg_it == servicegroup::servicegroups.end() || !sg_it->second)
          return ERROR;

        std::shared_ptr<servicegroup> const& temp_servicegroup{sg_it->second};

        /* concatenate macro values for all servicegroup members */
          for (service_map_unsafe::iterator
                 it{temp_servicegroup->members.begin()},
                 end{temp_servicegroup->members.end()};
               it != end;
               ++it) {

          if ((temp_service = it->second) == nullptr)
            continue;

          /* get the macro value for this service */
          grab_custom_macro_value_r(
            mac,
            macro_name,
            temp_service->get_hostname(),
            temp_service->get_description(),
            temp_buffer);

          if (temp_buffer.empty())
            continue;

          /* add macro value to already running macro */
          if (output.empty())
            output = temp_buffer;
          else {
            output.append(arg2);
            output.append(temp_buffer);
          }
          temp_buffer = "";
        }
      }
    }
  }
  /***** CUSTOM CONTACT VARIABLE *****/
  else if (macro_name.rfind("_CONTACT", 0) == 0) {
    contact* temp_contact(nullptr);

    /* a standard contact macro */
    if (arg2.empty()) {
      /* find the contact for on-demand macros */
      if (!arg1.empty()) {
        contact_map::const_iterator it{contact::contacts.find(arg1)};
        if (it == contact::contacts.end())
          return ERROR;
        else
          temp_contact = it->second.get();
      }
      /* else use saved contact pointer */
      else if ((temp_contact = mac->contact_ptr) == nullptr)
        return ERROR;

      /* get the contact macro value */
      result = grab_custom_object_macro_r(
                 mac,
                 macro_name.substr(8),
                 temp_contact->get_custom_variables(),
                 output);
    }
    /* a contact macro with a contactgroup name and delimiter */
    else {
      contactgroup_map::iterator cg_it = contactgroup::contactgroups.find(arg1);
      if (cg_it == contactgroup::contactgroups.end() || !cg_it->second)
        return (ERROR);

      /* concatenate macro values for all contactgroup members */
      for (contact_map_unsafe::const_iterator
             it{cg_it->second->get_members().begin()},
             end{cg_it->second->get_members().end()};
           it != end;
           ++it) {

        if (!it->second)
          continue;

        /* get the macro value for this contact */
        grab_custom_macro_value_r(
          mac,
          macro_name,
          it->second->get_name().c_str(),
          "",
          temp_buffer);

        if (temp_buffer.empty())
          continue;

        /* add macro value to already running macro */
        if (output.empty())
          output = temp_buffer;
        else {
          output.append(arg2);
          output.append(temp_buffer);
        }
        temp_buffer = "";
      }
    }
  }
  else
    return ERROR;
  return result;
}

/* calculates a date/time macro */
int grab_datetime_macro_r(
      nagios_macros* mac,
      int macro_type,
      std::string const& arg1,
      std::string const& arg2,
      std::string & output) {
  time_t current_time = 0L;
  timeperiod* temp_timeperiod = nullptr;
  time_t test_time = 0L;
  time_t next_valid_time = 0L;
  timeperiod_map::const_iterator it;
  char tmp_date[MAX_DATETIME_LENGTH];

  (void)mac;

  /* get the current time */
  time(&current_time);

  /* parse args, do prep work */
  switch (macro_type) {
  case MACRO_ISVALIDTIME:
  case MACRO_NEXTVALIDTIME:
    /* find the timeperiod */
    temp_timeperiod = nullptr;
    it = timeperiod::timeperiods.find(arg1);

    if (it != timeperiod::timeperiods.end())
      temp_timeperiod = it->second.get();

    if (temp_timeperiod == nullptr)
      return ERROR;

    /* what timestamp should we use? */
    if (!arg2.empty())
      test_time = (time_t)strtoul(arg2.c_str(), nullptr, 0);
    else
      test_time = current_time;
    break;

  default:
    break;
  }

  /* calculate the value */
  switch (macro_type) {
  case MACRO_LONGDATETIME:
    get_datetime_string(
      &current_time,
      tmp_date,
      MAX_DATETIME_LENGTH,
      LONG_DATE_TIME);
    output = tmp_date;
    break;

  case MACRO_SHORTDATETIME:
    get_datetime_string(
      &current_time,
      tmp_date,
      MAX_DATETIME_LENGTH,
      SHORT_DATE_TIME);
      output = tmp_date;
    break;

  case MACRO_DATE:
    get_datetime_string(
      &current_time,
      tmp_date,
      MAX_DATETIME_LENGTH,
      SHORT_DATE);
      output = tmp_date;
    break;

  case MACRO_TIME:
    get_datetime_string(
      &current_time,
      tmp_date,
      MAX_DATETIME_LENGTH,
      SHORT_TIME);
      output = tmp_date;
    break;

  case MACRO_TIMET:
    output = current_time;
    break;

  case MACRO_ISVALIDTIME:
      output = !check_time_against_period(test_time, temp_timeperiod);
    break;

  case MACRO_NEXTVALIDTIME:
    get_next_valid_time(test_time, &next_valid_time, temp_timeperiod);
    if (next_valid_time == test_time
        && !check_time_against_period(
             test_time,
             temp_timeperiod))
      next_valid_time = (time_t)0L;
    output = next_valid_time;
    break;

  default:
    return ERROR;
  }
  return OK;
}

/* computes a hostgroup macro */
int grab_standard_hostgroup_macro_r(
      nagios_macros* mac,
      int macro_type,
      hostgroup* temp_hostgroup,
      std::string & output) {
  std::string temp_buffer;
  if (temp_hostgroup == nullptr)
    return ERROR;

  /* get the macro value */
  switch (macro_type) {
  case MACRO_HOSTGROUPNAME:
    output = temp_hostgroup->get_group_name();
    break;

  case MACRO_HOSTGROUPALIAS:
    if (!temp_hostgroup->get_alias().empty())
      output = temp_hostgroup->get_alias();
    break;

  case MACRO_HOSTGROUPMEMBERS:
    /* now fill in the string with the member names */
      for (host_map_unsafe::const_iterator
             it{temp_hostgroup->members.begin()},
             end{temp_hostgroup->members.begin()};
           it != end;
           ++it) {
        if (it->first.empty())
        continue;
      if (temp_buffer.empty())      /* If our buffer didn't contain anything, we just need to write "%s,%s" */
        temp_buffer.append(it->first);
      else
        temp_buffer += "," + it->first;
    }
    break;

  case MACRO_HOSTGROUPACTIONURL:
    if (!temp_hostgroup->get_action_url().empty())
      output = temp_hostgroup->get_action_url();
    break;

  case MACRO_HOSTGROUPNOTESURL:
    if (!temp_hostgroup->get_notes_url().empty())
      output = temp_hostgroup->get_notes_url();
    break;

  case MACRO_HOSTGROUPNOTES:
    if (!temp_hostgroup->get_notes().empty())
      output = temp_hostgroup->get_notes();
    break;

  default:
    logger(dbg_macros, basic)
      << "UNHANDLED HOSTGROUP MACRO #" << macro_type << "! THIS IS A BUG!";
    return ERROR;
  }

  /* post-processing */
  /* notes, notes URL and action URL macros may themselves contain macros, so process them... */
  switch (macro_type) {
  case MACRO_HOSTGROUPACTIONURL:
  case MACRO_HOSTGROUPNOTESURL:
    process_macros_r(
      mac,
      output,
      temp_buffer,
      URL_ENCODE_MACRO_CHARS);
    output = temp_buffer;
    break;

  case MACRO_HOSTGROUPNOTES:
    process_macros_r(mac, output, temp_buffer, 0);
    output = temp_buffer;
    break;

  default:
    break;
  }

  return OK;
}

/* computes a servicegroup macro */
int grab_standard_servicegroup_macro_r(
      nagios_macros* mac,
      int macro_type,
      servicegroup* temp_servicegroup,
      std::string & output) {
  std::string temp_buffer;

  if (temp_servicegroup == nullptr)
    return ERROR;

  /* get the macro value */
  switch (macro_type) {
  case MACRO_SERVICEGROUPNAME:
    output = temp_servicegroup->get_group_name();
    break;

  case MACRO_SERVICEGROUPALIAS:
    if (!temp_servicegroup->get_alias().empty())
      output = temp_servicegroup->get_alias();
    break;

  case MACRO_SERVICEGROUPMEMBERS:
    /* fill the string with the group members */
    for (service_map_unsafe::iterator
           it{temp_servicegroup->members.begin()},
           end{temp_servicegroup->members.end()};
         it != end;
         ++it) {
      if (it->first.first.empty() || it->first.second.empty())
        continue;
      if (temp_buffer.empty())      /* If our buffer didn't contain anything, we just need to write "%s,%s" */
        temp_buffer = it->first.first + "," + it->first.second;
      else                    /* Now we need to write ",%s,%s" */
        temp_buffer = "," + it->first.first + "," + it->first.second;
    }
    break;
  case MACRO_SERVICEGROUPACTIONURL:
    if (!temp_servicegroup->get_action_url().empty())
      output = temp_servicegroup->get_action_url();
    break;

  case MACRO_SERVICEGROUPNOTESURL:
    if (!temp_servicegroup->get_notes_url().empty())
      output = temp_servicegroup->get_notes_url();
    break;

  case MACRO_SERVICEGROUPNOTES:
    if (!temp_servicegroup->get_notes().empty())
      output = temp_servicegroup->get_notes();
    break;

  default:
    logger(dbg_macros, basic)
      << "UNHANDLED SERVICEGROUP MACRO #" << macro_type
      << "! THIS IS A BUG!";
    return ERROR;
  }

  /* post-processing */
  /* notes, notes URL and action URL macros may themselves contain macros, so process them... */
  switch (macro_type) {
  case MACRO_SERVICEGROUPACTIONURL:
  case MACRO_SERVICEGROUPNOTESURL:
    process_macros_r(
      mac,
      output,
      temp_buffer,
      URL_ENCODE_MACRO_CHARS);
    output = temp_buffer;
    break;

  case MACRO_SERVICEGROUPNOTES:
    process_macros_r(mac, output, output, 0);
    output = temp_buffer;
    break;

  default:
    break;
  }
  return OK;
}

/* computes a contact macro */
int grab_standard_contact_macro_r(
      nagios_macros* mac,
      int macro_type,
      contact* temp_contact,
      std::string& output) {
  (void)mac;

  if (temp_contact == nullptr)
    return ERROR;

  /* get the macro value */
  switch (macro_type) {
  case MACRO_CONTACTNAME:
    output = temp_contact->get_name();
    break;

  case MACRO_CONTACTALIAS:
    output = temp_contact->get_alias();
    break;

  case MACRO_CONTACTEMAIL:
    if (!temp_contact->get_email().empty())
      output = temp_contact->get_email();
    break;

  case MACRO_CONTACTPAGER:
    if (!temp_contact->get_pager().empty())
      output = temp_contact->get_pager();
    break;

  case MACRO_CONTACTGROUPNAMES: {
    std::string buf;
    /* get the contactgroup names */
    /* find all contactgroups this contact is a member of */
    for (std::list<contactgroup*>::const_iterator
           it{temp_contact->get_parent_groups().begin()},
           end{temp_contact->get_parent_groups().end()};
         it != end; ++it) {
      if (!*it)
        continue;

      if (!buf.empty())
        buf.append(",");
      buf.append((*it)->get_name());
    }
    if (!buf.empty())
      output = buf;
  }
    break;

  case MACRO_CONTACTTIMEZONE: {
    output = temp_contact->get_timezone();
  }
    break ;

  default:
    logger(dbg_macros, basic)
      << "UNHANDLED CONTACT MACRO #" << macro_type
      << "! THIS IS A BUG!";
    return ERROR;
  }
  return OK;
}

/* computes a contact address macro */
int grab_contact_address_macro(
      unsigned int macro_num,
      contact* temp_contact,
      std::string& output) {
  if (macro_num >= MAX_CONTACT_ADDRESSES)
    return ERROR;

  if (temp_contact == nullptr)
    return ERROR;

  /* get the macro */
  if (!temp_contact->get_address(macro_num).empty())
    output = temp_contact->get_address(macro_num);
  return OK;
}

/* computes a contactgroup macro */
int grab_standard_contactgroup_macro(
      int macro_type,
      contactgroup* temp_contactgroup,
      std::string& output) {

  if (temp_contactgroup == nullptr)
    return ERROR;

  /* get the macro value */
  switch (macro_type) {
  case MACRO_CONTACTGROUPNAME:
    output = temp_contactgroup->get_name();
    break;

  case MACRO_CONTACTGROUPALIAS:
    if (!temp_contactgroup->get_alias().empty())
      output = temp_contactgroup->get_alias();
    break;

  case MACRO_CONTACTGROUPMEMBERS:
    /* get the member list */
    for (contact_map_unsafe::const_iterator
           it{temp_contactgroup->get_members().begin()},
           end{temp_contactgroup->get_members().end()};
         it != end;
         ++it) {
      if (it->second->get_name().empty())
        continue;
      if (output.empty())
        output = it->first;
      else {
        output.append(",");
        output.append(it->first);
      }
    }
    break;

  default:
    logger(dbg_macros, basic)
      << "UNHANDLED CONTACTGROUP MACRO #" << macro_type
      << "! THIS IS A BUG!";
      return ERROR;
  }
  return OK;
}

/* computes a custom object macro */
int grab_custom_object_macro_r(
      nagios_macros* mac,
      std::string const& macro_name,
      map_customvar const& vars,
      std::string & output) {
  int result = ERROR;

  (void)mac;

  if (macro_name.empty() || vars.empty())
    return ERROR;

  /* get the custom variable */
  for (std::pair<std::string, customvariable> const& cv : vars) {

    if (macro_name == cv.first) {
      output = cv.second.get_value();
      result = OK;
      break;
    }
  }
  return result;
}
/******************************************************************/
/********************* MACRO STRING FUNCTIONS *********************/
/******************************************************************/

/* cleans illegal characters in macros before output */
std::string clean_macro_chars(std::string const& macro, int options) {
  std::string retval(macro);

  int len(retval.size());

  /* strip illegal characters out of retval */
  if (options & STRIP_ILLEGAL_MACRO_CHARS) {
    int y(0);
    for (int x(0); x < len; x++) {
      /*ch=(int)retval[x]; */
      /* allow non-ASCII characters (Japanese, etc) */
      int ch(retval[x] & 0xff);

      /* illegal ASCII characters */
      if (ch < 32 || ch == 127)
        continue;

      /* illegal user-specified characters */
      if (config->illegal_output_chars().find(ch) == std::string::npos)
        retval[y++] = retval[x];
    }

    retval.resize(y);
  }
  return retval;
}

std::string url_encode(std::string const& value) {
  std::ostringstream escaped;
  escaped.fill('0');
  escaped << std::hex;

  for (std::string::const_iterator i{value.begin()}, n{value.end()};
          i != n; ++i) {
    std::string::value_type c{*i};

    // Keep alphanumeric and other accepted characters intact
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      escaped << c;
      continue;
    }

    // Any other characters are percent-encoded
    escaped << std::uppercase;
    escaped << '%' << std::setw(2) << int((unsigned char) c);
    escaped << std::nouppercase;
  }

  return escaped.str();
}

///* encodes a string in proper URL format */
//char* get_url_encoded_string(std::string const& input) {
//  int x = 0;
//  int y = 0;
//  char* encoded_url_string = nullptr;
//  char temp_expansion[6] = "";
//
//  /* bail if no input */
//  if (input.empty())
//    return nullptr;
//
//  /* allocate enough memory to escape all characters if necessary */
//  encoded_url_string = new char[input.length() * 3 + 1];
//
//  /* check/encode all characters */
//  for (x = 0, y = 0; input[x] != (char)'\x0'; x++) {
//    /* alpha-numeric characters and a few other characters don't get encoded */
//    if (((char)input[x] >= '0' && (char)input[x] <= '9')
//        || ((char)input[x] >= 'A' && (char)input[x] <= 'Z')
//        || ((char)input[x] >= (char)'a' && (char)input[x] <= (char)'z')
//        || (char)input[x] == (char)'.' || (char)input[x] == (char)'-'
//        || (char)input[x] == (char)'_' || (char)input[x] == (char)':'
//        || (char)input[x] == (char)'/' || (char)input[x] == (char)'?'
//        || (char)input[x] == (char)'=' || (char)input[x] == (char)'&')
//      encoded_url_string[y++] = input[x];
//    /* spaces are pluses */
//    else if (input[x] == ' ')
//      encoded_url_string[y++] = '+';
//    /* anything else gets represented by its hex value */
//    else {
//      encoded_url_string[y] = '\x0';
//      sprintf(temp_expansion, "%%%02X", (unsigned int)(input[x] & 0xFF));
//      strcat(encoded_url_string, temp_expansion);
//      y += 3;
//    }
//  }
//
//  /* terminate encoded string */
//  encoded_url_string[y] = '\x0';
//  return encoded_url_string;
//}

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
  return OK;
}

/*
 * initializes the names of macros, using this nifty little macro
 * which ensures we never add any typos to the list
 */
#define add_macrox_name(name) macro_x_names[MACRO_##name] = #name
int init_macrox_names() {
  unsigned int x = 0;

  /* initialize macro names */
  for (x = 0; x < MACRO_X_COUNT; x++)
    macro_x_names[x].clear();

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
  add_macrox_name(HOSTPARENTS);
  add_macrox_name(HOSTCHILDREN);
  add_macrox_name(HOSTID);
  add_macrox_name(SERVICEID);
  add_macrox_name(HOSTTIMEZONE);
  add_macrox_name(SERVICETIMEZONE);
  add_macrox_name(CONTACTTIMEZONE);

  return OK;
}

/******************************************************************/
/********************* MACRO CLEANUP FUNCTIONS ********************/
/******************************************************************/

/* free memory associated with the macrox names */
int free_macrox_names() {
  unsigned int x = 0;

  /* free each macro name */
  for (x = 0; x < MACRO_X_COUNT; x++) {
    macro_x_names[x].clear();
  }
  return OK;
}

/* clear argv macros - used in commands */
int clear_argv_macros_r(nagios_macros* mac) {
  unsigned int x = 0;

  /* command argument macros */
  for (x = 0; x < MAX_COMMAND_ARGUMENTS; x++) {
    mac->argv[x] = "";
  }
  return OK;
}

/* clear all macros that are not "constant" (i.e. they change throughout the course of monitoring) */
int clear_volatile_macros_r(nagios_macros* mac) {
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
      mac->x[x] = "";
      break;
    }
  }

  /* contact address macros */
  for (x = 0; x < MAX_CONTACT_ADDRESSES; x++) {
    mac->contactaddress[x] = "";
  }

  /* clear macro pointers */
  mac->host_ptr = nullptr;
  mac->hostgroup_ptr = nullptr;
  mac->service_ptr = nullptr;
  mac->servicegroup_ptr = nullptr;
  mac->contact_ptr = nullptr;
  mac->contactgroup_ptr = nullptr;

  /* clear on-demand macro */
  mac->ondemand = "";

  /* clear ARGx macros */
  clear_argv_macros_r(mac);

  /* clear custom host variables */
  mac->custom_host_vars.clear();

  /* clear custom service variables */
  mac->custom_service_vars.clear();

  /* clear custom contact variables */
  mac->custom_contact_vars.clear();

  return OK;
}

/* clear contact macros */
int clear_contact_macros_r(nagios_macros* mac) {
  unsigned int x;

  for (x = 0; x < MACRO_X_COUNT; x++) {
    switch (x) {
    case MACRO_CONTACTNAME:
    case MACRO_CONTACTALIAS:
    case MACRO_CONTACTEMAIL:
    case MACRO_CONTACTPAGER:
    case MACRO_CONTACTGROUPNAMES:
      mac->x[x] = "";
      break;

    default:
      break;
    }
  }

  /* clear contact addresses */
  for (x = 0; x < MAX_CONTACT_ADDRESSES; x++) {
    mac->contactaddress[x] = "";
  }

  /* clear custom contact variables */
  mac->custom_contact_vars.clear();

  /* clear pointers */
  mac->contact_ptr = nullptr;

  return OK;
}

/* clear contactgroup macros */
int clear_contactgroup_macros_r(nagios_macros* mac) {
  unsigned int x;

  for (x = 0; x < MACRO_X_COUNT; x++) {
    switch (x) {
    case MACRO_CONTACTGROUPNAME:
    case MACRO_CONTACTGROUPALIAS:
    case MACRO_CONTACTGROUPMEMBERS:
      mac->x[x] = "";
      break;

    default:
      break;
    }
  }

  /* clear pointers */
  mac->contactgroup_ptr = nullptr;

  return OK;
}

/* clear summary macros */
int clear_summary_macros_r(nagios_macros* mac) {
  unsigned int x;

  for (x = MACRO_TOTALHOSTSUP;
       x <= MACRO_TOTALSERVICEPROBLEMSUNHANDLED;
       x++) {
    mac->x[x] = "";
  }

  return OK;
}
