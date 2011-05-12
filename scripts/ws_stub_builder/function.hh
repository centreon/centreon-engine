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

#ifndef CCE_SCRIPT_FUNCTION_HH
# define CCE_SCRIPT_FUNCTION_HH

# include <QString>
# include <QList>

# include "argument.hh"
# include "arg_definition.hh"

namespace                       com {
  namespace                     centreon {
    namespace                   engine {
      namespace                 script {
	/**
	 *  @class function function.hh
	 *  @brief The function builder class.
	 *
	 *  This class build functions (help and execute) with their prototype.
	 */
	class                   function {
	public:
	                        function(QString const& data = "");
	                        function(function const& right);
	                        ~function() throw();

	  function&             operator=(function const& right);

	  static bool           is_valid(QString const& data) throw();

	  void                  build();

	  QString const&        get_name() const throw();

	  QString               get_help_name() const throw();
	  QString               get_exec_name() const throw();

	  QString const&        get_help_prototype() const throw();
	  QString const&        get_exec_prototype() const throw();

	  QString const&        get_help_function() const throw();
	  QString const&        get_exec_function() const throw();

	private:
	  struct                arg_info {
	    QString             type;
	    QString             name;
	    bool                is_pointer;
	    bool                is_ref;
	  };

	  static char const*    _pattern;

	  void                  _build_help_prototype();
	  void                  _build_exec_prototype();
	  void                  _build_help_function();
	  void                  _build_exec_function();

	  void                  _build_args_info(QString const& args_list);
	  QString               _build_help_args(argument const& arg) const;
	  QString               _build_exec_struct(QString const& base,
						   argument const& arg);
	  QString               _get_qstring_methode(QString type) const;
	  static QString        _clean_function_name(QString const& name);

	  arg_definition const& _def;
	  QString               _data;
	  QString               _function;
	  QString               _new_function;
	  QList<arg_info>       _args_info;
	  QString               _help_prototype;
	  QString               _exec_prototype;
	  QString               _help_function;
	  QString               _exec_function;
	  unsigned int          _nb_args;
	  unsigned int          _list_pos;
	};
      }
    }
  }
}

#endif // !CCE_SCRIPT_FUNCTION_HH
