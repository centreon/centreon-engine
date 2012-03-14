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

#ifndef CCE_LOGGING_LOGGER_HH
#  define CCE_LOGGING_LOGGER_HH

#  include <iomanip>
#  include <QString>
#  include <sstream>
#  include <string>
#  include "com/centreon/engine/logging/engine.hh"
#  include "com/centreon/engine/logging/object.hh"

namespace                    com {
  namespace                  centreon {
    namespace                engine {
      namespace              logging {
        enum                 e_flags {
          fixed = 0,
          scientific = 1
        };

        struct s_setprecision { int precision; };
        inline s_setprecision setprecision(int val) {
          s_setprecision s = { val };
          return (s);
        }

        /**
         *  @class logger logger.hh
         *  @brief Simple logging class.
         *
         *  Simple logging class used by the engine to write log data.
         */
        class                logger {
         public:
                             logger(unsigned long long type, unsigned int verbosity);
                             logger(logger const& right);
                             ~logger();

          logger&            operator=(logger const& right);
          logger&            operator<<(QString const& obj);
          logger&            operator<<(std::string const& obj);
          logger&            operator<<(char const* obj);
          logger&            operator<<(char obj);
          logger&            operator<<(int obj);
          logger&            operator<<(unsigned int obj);
          logger&            operator<<(long obj);
          logger&            operator<<(unsigned long obj);
          logger&            operator<<(long long obj);
          logger&            operator<<(unsigned long long obj);
          logger&            operator<<(double obj);
          logger&            operator<<(e_flags obj);
          logger&            operator<<(s_setprecision const& obj);
          logger&            operator<<(void const* obj);

         private:
          struct             redirector {
            logger& (logger::*redirect_qstring)(QString const&);
            logger& (logger::*redirect_std_string)(std::string const&);
            logger& (logger::*redirect_string)(char const*);
            logger& (logger::*redirect_char)(char);
            logger& (logger::*redirect_int)(int);
            logger& (logger::*redirect_uint)(unsigned int);
            logger& (logger::*redirect_long)(long);
            logger& (logger::*redirect_ulong)(unsigned long);
            logger& (logger::*redirect_long_long)(long long);
            logger& (logger::*redirect_ulong_long)(unsigned long long);
            logger& (logger::*redirect_double)(double);
            logger& (logger::*redirect_flags)(e_flags);
            logger& (logger::*redirect_setprecision)(s_setprecision const&);
            logger& (logger::*redirect_void_ptr)(void const*);
            void    (logger::*init)();
            void    (logger::*flush)();
          };

          template<typename T>
          logger&       _nothing(T obj) {
            (void)obj;
            return (*this);
          }

          template<typename T>
          logger&       _builder(T obj) {
            _buffer << obj;
            return (*this);
          }

          void          _init();
          void          _flush();
          void          _nothing();

          mutable std::ostringstream _buffer;
          static redirector const    _redir_builder;
          static redirector const    _redir_nothing;
          redirector const*          _redirector;
          engine&                    _engine;
          unsigned long long         _type;
          unsigned int               _verbosity;
        };

        template<>
        inline logger& logger::_builder(QString const& obj) {
          _buffer << qPrintable(obj);
          return (*this);
        }

        template<>
        inline logger& logger::_builder(e_flags obj) {
	  if (obj == fixed)
	    _buffer << std::fixed;
	  else
	    _buffer << std::scientific;
          return (*this);
        }

        template<>
        inline logger& logger::_builder(s_setprecision const& obj) {
          _buffer << std::setprecision(obj.precision);
          return (*this);
        }
      }
    }
  }
}

#endif // !CCE_LOGGING_LOGGER_HH
