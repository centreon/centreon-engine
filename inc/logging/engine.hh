/*
** Copyright 2011      Merethis
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
# define CCE_LOGGING_ENGINE_HH

# include <QSharedPointer>
# include <QHash>
# include <QReadWriteLock>

# include "logging/object.hh"

namespace                    com {
  namespace                  centreon {
    namespace                engine {
      namespace              logging {
	/**
         *  @class engine engnie.hh
         *  @brief Class to manage logging.
         *
         *  Class to manage all logging objects.
         */
	class                engine {
	public:
	  struct                   obj_info {
	                           obj_info();
	                           obj_info(QSharedPointer<object> obj,
					    unsigned long long type,
					    unsigned int verbosity);
	                           obj_info(obj_info const& right);
	                           ~obj_info() throw();

	    obj_info&              operator=(obj_info const& right);

	    QSharedPointer<object> obj;
	    unsigned long long     type;
	    unsigned int           verbosity;
	  };

	  static engine&     instance();

	  void               log(char const* message,
				 unsigned long long type,
				 unsigned int verbosity) throw();

	  unsigned long      add_object(obj_info const& info);
	  void               remove_object(unsigned long id) throw();
	  void               update_object(unsigned long id,
					   unsigned long long type,
					   unsigned int verbosity) throw();

	private:
	                     engine();
	                     ~engine() throw();

	                     engine(engine const& right);
	  engine&            operator=(engine const& right);

	  QHash<unsigned long, obj_info> _objects;
	  QReadWriteLock     _rwlock;
	  unsigned long      _id;
	};
      }
    }
  }
}

#endif // !CCE_LOGGING_ENGINE_HH
