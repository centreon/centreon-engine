/*
** Copyright 2011-2012 Merethis
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

#include <cctype>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/daterange.hh"
#include "com/centreon/engine/objects/timeperiod.hh"
#include "com/centreon/engine/objects/timeperiodexclusion.hh"
#include "com/centreon/engine/objects/timerange.hh"
#include "com/centreon/engine/objects/utils.hh"
#include "com/centreon/engine/skiplist.hh"
#include "com/centreon/engine/xodtemplate.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::objects::utils;

/**
 *  Trim a string.
 *
 *  @param[in] str Base string.
 *
 *  @return Trimmed string.
 */
static std::string trim(std::string const& str) {
  std::string trimmed(str);
  while (!trimmed.empty() && isspace(trimmed[0]))
    trimmed.erase(trimmed.begin());
  while (!trimmed.empty() && isspace(trimmed[trimmed.size() - 1]))
    trimmed.resize(trimmed.size() - 1);
  return (trimmed);
}

/**
 *  Wrapper C
 *
 *  @see com::centreon::engine::objects::release
 */
void release_timeperiod(timeperiod const* obj) {
  try {
    objects::release(obj);
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic) << e.what();
  }
  catch (...) {
    logger(log_runtime_error, basic)
      << "release_timeperiod: unknow exception";
  }
  return;
}

/**
 *  Create and add a timeperiod into the engine.
 *
 *  @param[in] name    The name of the timeperiod.
 *  @param[in] alias   The alias of the timeperiod.
 *  @param[in] range   The time range of the timeperiod.
 *  @param[in] exclude The time exclution of the timeperiod.
 */
void objects::add_timeperiod(
                std::string const& name,
                std::string const& alias,
                std::vector<std::string> const& range,
                std::vector<std::string> const& exclude) {
  // Check if timeperiod already exist.
  if (find_timeperiod(name.c_str()))
    throw (engine_error() << "timeperiod '"
           << name << "' timeperiod already exist");

  // Duplicate timeperiod name.
  char* name_str(my_strdup(name.c_str()));

  // Create timeperiod template.
  xodtemplate_timeperiod* tmpl_tperiod(new xodtemplate_timeperiod());
  memset(tmpl_tperiod, 0, sizeof(*tmpl_tperiod));
  tmpl_tperiod->timeperiod_name = name_str;
  tmpl_tperiod->alias = my_strdup(alias.c_str());
  tmpl_tperiod->register_object = true;

  for (std::vector<std::string>::const_iterator
         it(range.begin()), end(range.end());
       it != end;
       ++it) {
    std::string base(trim(*it));
    size_t pos(base.find(' '));
    std::string key(trim(base.substr(0, pos)));
    std::string value(trim(base.substr(pos + 1)));

    if ((pos == std::string::npos)
        || xodtemplate_parse_timeperiod_directive(
             tmpl_tperiod,
             key.c_str(),
             value.c_str()) == ERROR) {
      xodtemplate_free_timeperiod(tmpl_tperiod);
      throw (engine_error() << "timeperiod '"
             << name << "' invalid exception");
    }
  }

  std::string exclude_str;
  for (std::vector<std::string>::const_iterator
         it(exclude.begin()), end(exclude.end());
       it != end;
       ++it) {
    exclude_str += *it;
    if (it + 1 != end)
      exclude_str += ", ";
  }

  tmpl_tperiod->exclusions = my_strdup(exclude_str.c_str());
  int res(xodtemplate_register_timeperiod(tmpl_tperiod));
  if (OK == res) {
    timeperiod* tmprd(find_timeperiod(tmpl_tperiod->timeperiod_name));
    if (!tmprd)
      res = ERROR;
    else
      res = xodtemplate_fill_timeperiod(tmpl_tperiod, tmprd);
  }
  xodtemplate_free_timeperiod(tmpl_tperiod);
  if (res != OK)
    throw (engine_error() << "timeperiod '"
           << name << "' create failed");
  return;
}

/**
 *  Cleanup memory timeperiod.
 *
 *  @param[in] obj The timeperiod to cleanup memory.
 */
void objects::release(timeperiod const* obj) {
  if (obj == NULL)
    return;

  skiplist_delete(object_skiplists[TIMEPERIOD_SKIPLIST], obj);
  remove_object_list(obj, &timeperiod_list, &timeperiod_list_tail);

  for (unsigned int i = 0; i < 7; ++i)
    release(obj->days[i]);
  for (unsigned int i = 0; i < DATERANGE_TYPES; ++i)
    release(obj->exceptions[i]);
  release(obj->exclusions);

  delete[] obj->name;
  delete[] obj->alias;
  delete obj;
  return;
}
