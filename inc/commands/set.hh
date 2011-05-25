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

#ifndef CCE_COMMANDS_SET_HH
# define CCE_COMMANDS_SET_HH

# include <QObject>
# include <QString>
# include <QHash>
# include <QSharedPointer>

# include "commands/command.hh"

namespace                         com {
  namespace                       centreon {
    namespace                     engine {
      namespace                   commands {
	/**
	 *  @class set set.hh
	 *  @brief Store all command.
	 *
	 *  Set is a singleton to store all command class and have a simple
	 *  access to used it.
	 */
	class                     set : public QObject {
	  Q_OBJECT
	public:
	  static set&             instance();

	  void                    add_command(command const& cmd);
	  void                    add_command(QSharedPointer<command> cmd);

	  void                    remove_command(QString const& cmd_name) throw();

	  QSharedPointer<command> get_command(QString const& cmd_name);

	public slots:
	  void                    command_name_changed(QString const& old_name,
						       QString const& new_name);

	private:
	  set();
	  set(set const& right);
	  ~set() throw();

	  set&                    operator=(set const& right);

	  QHash<QString, QSharedPointer<command> > _list;
	};
      }
    }
  }
}

#endif // !CCE_COMMANDS_SET_HH
