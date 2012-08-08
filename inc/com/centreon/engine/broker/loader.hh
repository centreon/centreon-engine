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

#ifndef CCE_MODULES_LOADER_HH
#  define CCE_MODULES_LOADER_HH

#  include <list>
#  include <map>
#  include <memory>
#  include <string>
#  include "com/centreon/engine/broker/handle.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/shared_ptr.hh"

CCE_BEGIN()

namespace                  broker {
  /**
   *  @class loader loader.hh
   *  @brief Modules loader.
   *
   *  Loader manage all modules.
   */
  class                    loader {
  public:
    virtual                ~loader() throw ();
    shared_ptr<handle>     add_module(
                             std::string const& filename = "",
                             std::string const& args = "");
    void                   del_module(
                             shared_ptr<handle> const& mod);
    std::list<shared_ptr<handle> >
                           get_modules() const;
    static loader&         instance();
    static void            load();
    unsigned int           load_directory(std::string const& dir);
    static void            unload();
    void                   unload_modules();

  private:
                           loader();
                           loader(loader const& right);
    loader&                operator=(loader const& right);
    void                   _internal_copy(loader const& right);

    static std::auto_ptr<loader>
                           _instance;
    std::multimap<std::string, shared_ptr<handle> >
                           _modules;
  };
}

CCE_END()

#endif // !CCE_MODULES_LOADER_HH
