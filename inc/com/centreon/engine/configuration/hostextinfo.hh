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

#  include "com/centreon/engine/configuration/object.hh"
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
    /*
    int                    2d_coords() const throw ();
    int                    3d_coords() const throw ();
    std::string const&     action_url() const throw ();
    std::string const&     gd2_image() const throw ();
    std::list<std::string> const&
                           hostgroups() const throw ();
    std::list<std::string> const&
                           host_name() const throw ();
    std::string const&     icon_image() const throw ();
    std::string const&     icon_image_alt() const throw ();
    std::string const&     notes() const throw ();
    std::string const&     notes_url() const throw ();
    std::string const&     statusmap_image() const throw ();
    std::string const&     vrml_image() const throw ();
    */

    bool                   parse(
                             std::string const& key,
                             std::string const& value);

  private:
    void                   _set_2d_coords(int value);
    void                   _set_3d_coords(int value);
    void                   _set_action_url(std::string const& value);
    void                   _set_gd2_image(std::string const& value);
    void                   _set_hostgroups(std::string const& value);
    void                   _set_hosts(std::string const& value);
    void                   _set_icon_image(std::string const& value);
    void                   _set_icon_image_alt(std::string const& value);
    void                   _set_notes(std::string const& value);
    void                   _set_notes_url(std::string const& value);
    void                   _set_statusmap_image(std::string const& value);
    void                   _set_vrml_image(std::string const& value);

    int                    _2d_coords;
    int                    _3d_coords;
    std::string            _action_url;
    std::string            _gd2_image;
    std::list<std::string> _hostgroups;
    std::list<std::string> _hosts;
    std::string            _icon_image;
    std::string            _icon_image_alt;
    std::string            _notes;
    std::string            _notes_url;
    std::string            _statusmap_image;
    std::string            _vrml_image;
  };
}

CCE_END()

#endif // !CCE_CONFIGURATION_HOSTEXTINFO_HH

