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

#ifndef CCE_LOGGING_LOGGER_HH
# define CCE_LOGGING_LOGGER_HH

# include <QString>
# include <QTextStream>
# include <string>
# include "logging/object.hh"

namespace                     com {
  namespace                   centreon {
    namespace                 engine {
      namespace               logging {
        /**
         *  @class logger logger.hh
         *  @brief Simple logging class.
         *
         *  Simple logging class used by the engine to write log data.
         */
        class                 logger {
         public:
                              logger(unsigned long long type, unsigned int verbosity);
                              logger(logger const& right);
                              ~logger();

          logger&             operator=(logger const& right);
          template            <typename T>
          logger&             operator<<(T const& obj) {
            _stream << obj;
            return (*this);
          }

          logger&             operator<<(std::string const& str);

         private:
          QString             _buffer;
          mutable QTextStream _stream;
          unsigned long long  _type;
          unsigned int        _verbosity;
        };
      }
    }
  }
}

#endif // !CCE_LOGGING_LOGGER_HH
