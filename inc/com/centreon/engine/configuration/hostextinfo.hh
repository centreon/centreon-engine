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

#ifndef CCE_CONFIGURATION_HOSTEXTINFO_HH
#  define CCE_CONFIGURATION_HOSTEXTINFO_HH

#  include "com/centreon/engine/configuration/group.hh"
#  include "com/centreon/engine/configuration/object.hh"
#  include "com/centreon/engine/configuration/opt.hh"
#  include "com/centreon/engine/configuration/point_2d.hh"
#  include "com/centreon/engine/configuration/point_3d.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                  configuration {
  class                    hostextinfo
    : public object {
  public:
                           hostextinfo();
                           hostextinfo(hostextinfo const& right);
                           ~hostextinfo() throw ();
    hostextinfo&           operator=(hostextinfo const& right);
    bool                   operator==(
                             hostextinfo const& right) const throw ();
    bool                   operator!=(
                             hostextinfo const& right) const throw ();
    std::size_t            id() const throw ();
    bool                   is_valid() const throw ();
    void                   merge(object const& obj);
    bool                   parse(
                             std::string const& key,
                             std::string const& value);

  private:
    bool                   _set_2d_coords(std::string const& value);
    bool                   _set_3d_coords(std::string const& value);
    bool                   _set_action_url(std::string const& value);
    bool                   _set_gd2_image(std::string const& value);
    bool                   _set_hostgroups(std::string const& value);
    bool                   _set_hosts(std::string const& value);
    bool                   _set_icon_image(std::string const& value);
    bool                   _set_icon_image_alt(std::string const& value);
    bool                   _set_notes(std::string const& value);
    bool                   _set_notes_url(std::string const& value);
    bool                   _set_statusmap_image(std::string const& value);
    bool                   _set_vrml_image(std::string const& value);

    opt<point_2d>          _2d_coords;
    opt<point_3d>          _3d_coords;
    std::string            _action_url;
    std::string            _gd2_image;
    group                  _hostgroups;
    group                  _hosts;
    std::string            _icon_image;
    std::string            _icon_image_alt;
    std::string            _notes;
    std::string            _notes_url;
    std::string            _statusmap_image;
    std::string            _vrml_image;
  };

  typedef umap<std::size_t, shared_ptr<hostextinfo> > map_hostextinfo;
}

CCE_END()

#endif // !CCE_CONFIGURATION_HOSTEXTINFO_HH
