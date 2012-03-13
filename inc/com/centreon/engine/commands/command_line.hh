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

#ifndef CCE_COMMANDS_COMMAND_LINE_HH
#  define CCE_COMMANDS_COMMAND_LINE_HH

#  include <QString>
#  include <QStringList>
#  include <string>

namespace               com {
  namespace             centreon {
    namespace           engine {
      namespace         commands {
        /**
         *  @class command_line command_line.hh "com/centreon/misc/command_line.hh"
         *  @brief Provide method to split command line arguments into
         *         array.
         *
         *  Command line is a simple way to split command line arguments
         *  into array.
         */
        class           command_line {
        public:
                        command_line();
                        command_line(QString const& cmdline);
                        command_line(
                          QString const& appname,
                          QStringList const& args);
                        command_line(command_line const& right);
                        ~command_line() throw ();
          command_line& operator=(command_line const& right);
          bool          operator==(
                          command_line const& right) const throw ();
          bool          operator!=(
                          command_line const& right) const throw ();
          int           get_argc() const throw ();
          char**        get_argv() const throw ();
          void          parse(std::string const& cmdline);

        private:
          void          _internal_copy(command_line const& right);
          void          _release();

          int           _argc;
          char**        _argv;
          size_t        _size;
        };
      }
    }
  }
}

#endif // !CCE_COMMANDS_COMMAND_LINE_HH
