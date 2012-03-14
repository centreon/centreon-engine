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

#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/customvariablesmember.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

/**
 *  Wrapper C
 *
 *  @see com::centreon::engine::objects::release
 */
customvariablesmember const* release_customvariablesmember(customvariablesmember const* obj) {
  try {
    return (objects::release(obj));
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic) << e.what();
  }
  catch (...) {
    logger(log_runtime_error, basic) << Q_FUNC_INFO << " unknow exception.";
  }
  return (NULL);
}

/**
 *  Cleanup memory of customvariablesmember.
 *
 *  @param[in] obj The customvariable member to cleanup memory.
 *
 *  @return The next customvariablesmember.
 */
customvariablesmember const* objects::release(customvariablesmember const* obj) {
  if (obj == NULL)
    return (NULL);

  customvariablesmember const* next = obj->next;
  delete[] obj->variable_name;
  delete[] obj->variable_value;
  delete obj;
  return (next);
}

/**
 *  Add somme custom variable to a generic object with custom variables member list.
 *
 *  @param[in]  custom_vars    The custom variables to insert.
 *  @param[out] list_customvar The object custom variables.
 *
 *  @return True if insert sucessfuly, false otherwise.
 */
bool objects::add_custom_variables_to_object(QVector<QString> const& custom_vars,
                                             customvariablesmember** list_customvar) {
  if (list_customvar == NULL)
    return (false);

  for (QVector<QString>::const_iterator it = custom_vars.begin(),
         end = custom_vars.end();
       it != end;
       ++it) {
    // split string into custom var name (key) and the custom var value (value).
    int pos = it->indexOf('=');
    if (pos == -1)
      return (false);

    QString key(it->left(pos).trimmed());
    QString value(it->mid(pos + 1).trimmed());

    // add a new custom var into object.
    if (key.isEmpty() || value.isEmpty() || key[0] != '_'
	|| add_custom_variable_to_object(list_customvar,
                                         qPrintable(key),
                                         qPrintable(value)) == NULL)
      return (false);
  }
  return (true);
}
