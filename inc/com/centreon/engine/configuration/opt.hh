/*
** Copyright 2011-2013 Merethis
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

#ifndef CCE_CONFIGURATION_OPT_HH
#  define CCE_CONFIGURATION_OPT_HH

#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace    configuration {
  template<typename T>
  class      opt {
  public:
             opt() : _is_set(false) {}
             opt(T const& right) : _data(right), _is_set(true) {}
             opt(opt const& right)
      : _data(right._data), _is_set(right._is_set) {}
             ~opt() throw () {}
    T const& operator=(T const& right) {
      set(right);
      return (_data);
    }
    opt&     operator=(opt const& right) {
      _data = right._data;
      _is_set = right._is_set;
      return (*this);
    }
    bool     operator==(opt const& right) const throw () {
      return (_data == right._data && _is_set == right._is_set);
    }
    bool     operator!=(opt const& right) const throw () {
      return (!operator==(right));
    }
             operator T const& () const throw () { return (_data); }
    T const& get() const throw () { return (_data); }
    bool     is_set() const throw () { return (_is_set); }
    void     reset() throw () { _is_set = false; }
    void     set(T const& right) {
      _data = right;
      _is_set = true;
    }

  private:
    T        _data;
    bool     _is_set;
  };
}

CCE_END()

#endif // !CCE_CONFIGURATION_OPT_HH

