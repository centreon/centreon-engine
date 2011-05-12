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

#ifndef CCE_SCRIPT_ARGUMENT_HH
# define CCE_SCRIPT_ARGUMENT_HH

# include <QString>
# include <QList>

namespace                        com {
  namespace                      centreon {
    namespace                    engine {
      namespace                  script {
	/**
	 *  @class argument argument.hh
	 *  @brief Argument information class.
	 *
	 *  Argument provide basic information on variable type.
	 */
	class                    argument {
	public:
	                         argument(QString const& type = "",
					  QString const& name = "",
					  QString const& help = "");
	                         argument(argument const& right);
	                         ~argument() throw();

	  argument&              operator=(argument const& right);
	  bool                   operator==(argument const& right) const throw();
	  bool                   operator!=(argument const& right) const throw();

	  QString const&         get_type() const throw();
	  QString const&         get_name() const throw();
	  QString const&         get_help() const throw();

	  argument&              set_name(QString const& name);
	  argument&              set_help(QString const& help);

	  argument&              add(argument const& arg);
	  QList<argument> const& get_args() const throw();

	  bool                   is_primitive() const throw();

	private:
	  QString                _type;
	  QString                _name;
	  QString                _help;
	  QList<argument>        _list;
	};
      }
    }
  }
}

#endif // !CCE_SCRIPT_ARGUMENT_HH
