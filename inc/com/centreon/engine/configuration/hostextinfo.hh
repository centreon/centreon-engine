/*
** Copyright 2011-2013,2017 Centreon
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

#ifndef CCE_CONFIGURATION_HOSTEXTINFO_HH
#  define CCE_CONFIGURATION_HOSTEXTINFO_HH

#  include <list>
#  include "com/centreon/engine/configuration/group.hh"
#  include "com/centreon/engine/configuration/object.hh"
#  include "com/centreon/engine/opt.hh"
#  include "com/centreon/engine/configuration/point_2d.hh"
#  include "com/centreon/engine/configuration/point_3d.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                  configuration {
  class                    host;
  class                    hostextinfo : public object {
    friend class host;

   public:
                           hostextinfo();
                           hostextinfo(hostextinfo const& right);
                           ~hostextinfo() throw () override;
    hostextinfo&           operator=(hostextinfo const& right);
    bool                   operator==(
                             hostextinfo const& right) const throw ();
    bool                   operator!=(
                             hostextinfo const& right) const throw ();
    void                   check_validity() const override;
    void                   merge(object const& obj) override;
    bool                   parse(char const* key, char const* value) override;

    std::string const&     action_url() const throw ();
    point_2d const&        coords_2d() const throw ();
    point_3d const&        coords_3d() const throw ();
    set_string const&      hostgroups() const throw ();
    set_string const&      hosts() const throw ();
    std::string const&     icon_image() const throw ();
    std::string const&     icon_image_alt() const throw ();
    std::string const&     notes() const throw ();
    std::string const&     notes_url() const throw ();
    std::string const&     statusmap_image() const throw ();
    std::string const&     vrml_image() const throw ();

   private:
    typedef bool (*setter_func)(hostextinfo&, char const*);

    bool                   _set_action_url(std::string const& value);
    bool                   _set_coords_2d(std::string const& value);
    bool                   _set_coords_3d(std::string const& value);
    bool                   _set_hostgroups(std::string const& value);
    bool                   _set_hosts(std::string const& value);
    bool                   _set_icon_image(std::string const& value);
    bool                   _set_icon_image_alt(std::string const& value);
    bool                   _set_notes(std::string const& value);
    bool                   _set_notes_url(std::string const& value);
    bool                   _set_statusmap_image(std::string const& value);
    bool                   _set_vrml_image(std::string const& value);

    std::string            _action_url;
    opt<point_2d>          _coords_2d;
    opt<point_3d>          _coords_3d;
    group<set_string>      _hostgroups;
    group<set_string>      _hosts;
    std::string            _icon_image;
    std::string            _icon_image_alt;
    std::string            _notes;
    std::string            _notes_url;
    static std::unordered_map<std::string, setter_func> const _setters;
    std::string            _statusmap_image;
    std::string            _vrml_image;
  };

  typedef std::shared_ptr<hostextinfo> hostextinfo_ptr;
  typedef std::list<hostextinfo_ptr>   list_hostextinfo;
}

CCE_END()

#endif // !CCE_CONFIGURATION_HOSTEXTINFO_HH
