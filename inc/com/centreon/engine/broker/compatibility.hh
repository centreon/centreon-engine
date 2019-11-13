/*
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

#ifndef CCE_BROKER_COMPATIBILITY_HH
#define CCE_BROKER_COMPATIBILITY_HH

#include "com/centreon/engine/broker/handle.hh"
#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace broker {
/**
 *  @class compatibility compatibility.hh
 *  @brief Simple compatibility class.
 *
 *  Use to keep compatibility with old module system API.
 */
class compatibility {
 public:
  static compatibility& instance();
  static void load();
  static void unload();

  void author_module(broker::handle* mod);
  void copyright_module(broker::handle* mod);
  void create_module(broker::handle* mod);
  void description_module(broker::handle* mod);
  void destroy_module(broker::handle* mod);
  void license_module(broker::handle* mod);
  void loaded_module(broker::handle* mod);
  void name_module(broker::handle* mod);
  void unloaded_module(broker::handle* mod);
  void version_module(broker::handle* mod);

 private:
  compatibility();
  compatibility(compatibility const& right);
  virtual ~compatibility() throw();
  compatibility& operator=(compatibility const& right);
};
}  // namespace broker

CCE_END()

#endif  // !CCE_BROKER_COMPATIBILITY_HH
