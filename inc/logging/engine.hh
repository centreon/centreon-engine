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
# include <QMutex>
# include "logging/object.hh"

namespace                    com {
  namespace                  centreon {
    namespace                engine {
      namespace              logging {
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

	  void               set_verbosity_level(unsigned int level) throw();
	  unsigned int       get_verbosity_level() throw();

	  void               set_options(unsigned long long options) throw();
	  unsigned long long get_options() throw();

	private:
	                     engine();
	                     ~engine() throw();

	                     engine(engine const& right);
	  engine&            operator=(engine const& right);

	  QHash<unsigned long, obj_info> _objects;
	  QMutex             _mutex;
	  unsigned long long _options;
	  unsigned long      _id;
	  unsigned int       _verbosity_level;
	};
      }
    }
  }
}

#endif // !CCE_LOGGING_ENGINE_HH
