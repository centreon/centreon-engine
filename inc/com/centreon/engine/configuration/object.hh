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

#ifndef CCE_CONFIGURATION_OBJECT_HH
#  define CCE_CONFIGURATION_OBJECT_HH

#  include <algorithm>
#  include <list>
#  include <sstream>
#  include <string>
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/shared_ptr.hh"
#  include "com/centreon/unordered_hash.hh"

CCE_BEGIN()

namespace                  configuration {
  class                    object {
  public:
                           object(std::string const& type);
                           object(object const& right);
    virtual                ~object() throw ();
    object&                operator=(object const& right);
    bool                   operator==(
                             object const& right) const throw ();
    bool                   operator!=(
                             object const& right) const throw ();
    static shared_ptr<object>
                           create(std::string const& type);
    bool                   is_template() const throw ();
    virtual void           merge(object const& obj) = 0;
    std::string const&     name() const throw ();
    virtual bool           parse(
                             std::string const& key,
                             std::string const& value);
    virtual bool           parse(std::string const& line);
    void                   resolve_template(
                             umap<std::string, shared_ptr<object> >& templates);
    std::string const&     type() const throw ();

  protected:
    template<typename T, typename U, void (T::*ptr)(U)>
    struct setter {
      static bool generic(T& obj, std::string const& value) {

        U val;
        std::istringstream iss(value);
        if (!(iss >> val))
          return (false);
        (obj.*ptr)(val);

        return (true);
      }
    };

    template<typename T, void (T::*ptr)(std::string const&)>
    struct              setter<T, std::string const&, ptr> {
      static bool       generic(T& obj, std::string const& value) {
        (obj.*ptr)(value);
        return (true);
      }
    };

    void                   _set_is_template(bool value);
    void                   _set_name(std::string const& value);
    void                   _set_templates(std::string const& value);

    bool                   _is_resolve;
    bool                   _is_template;
    std::string            _name;
    std::list<std::string> _templates;
    std::string            _type;
  };

  typedef umap<std::string, shared_ptr<object> > objects;
}

CCE_END()

#  define MRG_DEFAULT(prop) \
  if (prop == default##prop && tmpl.prop != prop) prop = tmpl.prop
#  define MRG_INHERIT(prop) \
  if (prop.empty() && tmpl.prop != prop) prop.set(tmpl.prop)
#  define MRG_MAP(prop) \
  prop.insert(tmpl.prop.begin(), tmpl.prop.end())
#  define MRG_STRING(prop) \
  if (prop.empty() && tmpl.prop != prop) prop = tmpl.prop

#endif // !CCE_CONFIGURATION_OBJECT_HH

