/*
** Copyright 2011 Merethis
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

#include "error.hh"
#include "globals.hh"
#include "skiplist.hh"
#include "xodtemplate.hh"
#include "logging/logger.hh"
#include "objects/utils.hh"
#include "objects/timerange.hh"
#include "objects/daterange.hh"
#include "objects/timeperiodexclusion.hh"
#include "objects/timeperiod.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::objects::utils;

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
    logger(log_runtime_error, basic) << Q_FUNC_INFO << " unknow exception.";
  }
}

/**
 *  Create and add a timeperiod into the engine.
 *
 *  @param[in] name    The name of the timeperiod.
 *  @param[in] alias   The alias of the timeperiod.
 *  @param[in] range   The time range of the timeperiod.
 *  @param[in] exclude The time exclution of the timeperiod.
 */
void objects::add_timeperiod(QString const& name,
                             QString const& alias,
                             QVector<QString> const& range,
                             QVector<QString> const& exclude) {
  char* name_str = my_strdup(qPrintable(name));
  if (find_timeperiod(name_str) != NULL) {
    delete[] name_str;
    throw (engine_error() << "timeperiod '" << name << "' timeperiod already exist.");
  }

  xodtemplate_timeperiod* tmpl_tperiod = new xodtemplate_timeperiod();
  memset(tmpl_tperiod, 0, sizeof(*tmpl_tperiod));

  tmpl_tperiod->timeperiod_name = name_str;
  tmpl_tperiod->alias = my_strdup(qPrintable(alias));
  tmpl_tperiod->register_object = true;
  for (QVector<QString>::const_iterator it = range.begin(), end = range.end();
       it != end;
       ++it) {
    QString base(it->trimmed());

    int pos = base.indexOf(' ');
    QString key(base.left(pos).trimmed());
    QString value(base.mid(pos + 1).trimmed());

    if (pos == -1 || xodtemplate_parse_timeperiod_directive(tmpl_tperiod,
                                                            qPrintable(key),
                                                            qPrintable(value)) == ERROR) {
      xodtemplate_free_timeperiod(tmpl_tperiod);
      throw (engine_error() << "timeperiod '" << name << "' invalid exception.");
    }
  }

  QString exclude_str;
  for (QVector<QString>::const_iterator it = exclude.begin(), end = exclude.end();
       it != end;
       ++it) {
    exclude_str += *it;
    if (it + 1 != end)
      exclude_str += ", ";
  }

  tmpl_tperiod->exclusions = my_strdup(qPrintable(exclude_str));
  int res = xodtemplate_register_timeperiod(tmpl_tperiod);
  xodtemplate_free_timeperiod(tmpl_tperiod);
  if (res != OK)
    throw (engine_error() << "timeperiod '" << name << "' create failed.");
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
}
