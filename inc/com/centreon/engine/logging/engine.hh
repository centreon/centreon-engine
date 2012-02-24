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

#ifndef CCE_LOGGING_ENGINE_HH
#  define CCE_LOGGING_ENGINE_HH

#  include <memory>
#  include <QSharedPointer>
#  include <QReadWriteLock>
#  include <vector>
#  include "com/centreon/engine/logging/object.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                    logging {
  /**
   *  @class engine engnie.hh
   *  @brief Class to manage logging.
   *
   *  Class to manage all logging objects.
   */
  class                      engine {
  public:
    class                    obj_info {
      friend class           engine;

    public:
                             obj_info();
                             obj_info(
                               QSharedPointer<object> obj,
                               unsigned long long type,
                               unsigned int verbosity);
                             obj_info(obj_info const& right);
                             ~obj_info() throw ();
      obj_info&              operator=(obj_info const& right);
      unsigned long          id() const throw ();
      unsigned long long     type() const throw ();
      unsigned int           verbosity() const throw ();

    private:
      void                   _internal_copy(obj_info const& right);

      unsigned long          _id;
      QSharedPointer<object> _obj;
      unsigned long long     _type;
      unsigned int           _verbosity;
    };

                             ~engine() throw ();
    unsigned long            add_object(obj_info& info);
    static engine&           instance();
    bool                     is_logged(
                               unsigned long long type,
                               unsigned int verbosity) const throw ();
    static void              load();
    void                     log(
                               char const* message,
                               unsigned long long type,
                               unsigned int verbosity) throw ();
    void                     remove_object(unsigned long id) throw ();
    void                     remove_object(obj_info& obj) throw ();
    static void              unload();
    void                     update_object(
                               unsigned long id,
                               unsigned long long type,
                               unsigned int verbosity) throw ();

  private:
                             engine();
                             engine(engine const& right);
    engine&                  operator=(engine const& right);
    void                     _internal_copy(engine const& right);

    unsigned long            _id;
    static std::auto_ptr<engine>
                             _instance;
    std::vector<obj_info>    _objects;
    QReadWriteLock           _rwlock;
    unsigned long long       _type[3];
  };
}

CCE_END()

#endif // !CCE_LOGGING_ENGINE_HH
