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

#ifndef CCE_CONFIGURATION_SERVICEEXTINFO_HH
#  define CCE_CONFIGURATION_SERVICEEXTINFO_HH

#  include <list>
#  include "com/centreon/engine/configuration/object.hh"
#  include "com/centreon/engine/opt.hh"
#  include "com/centreon/engine/configuration/group.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                  configuration {
  class                    service;

  class                    serviceextinfo : public object {
    friend class service;

  public:
                           serviceextinfo();
                           serviceextinfo(serviceextinfo const& right);
                           ~serviceextinfo() throw ();
    serviceextinfo&        operator=(serviceextinfo const& right);
    bool                   operator==(
                             serviceextinfo const& right) const throw ();
    bool                   operator!=(
                             serviceextinfo const& right) const throw ();
    void                   check_validity() const;
    void                   merge(object const& obj);
    bool                   parse(char const* key, char const* value);

    std::string const&     action_url() const throw ();
    std::string const&     icon_image() const throw ();
    std::string const&     icon_image_alt() const throw ();
    list_string const&     hostgroups() const throw ();
    list_string const&     hosts() const throw ();
    std::string const&     notes() const throw ();
    std::string const&     notes_url() const throw ();
    std::string const&     service_description() const throw ();

  private:
    struct                 setters {
      char const*          name;
      bool                 (*func)(serviceextinfo&, char const*);
    };

    bool                   _set_action_url(std::string const& value);
    bool                   _set_icon_image(std::string const& value);
    bool                   _set_icon_image_alt(std::string const& value);
    bool                   _set_hosts(std::string const& value);
    bool                   _set_hostgroups(std::string const& value);
    bool                   _set_notes(std::string const& value);
    bool                   _set_notes_url(std::string const& value);
    bool                   _set_service_description(std::string const& value);

    std::string            _action_url;
    std::string            _icon_image;
    std::string            _icon_image_alt;
    group                  _hostgroups;
    group                  _hosts;
    std::string            _notes;
    std::string            _notes_url;
    std::string            _service_description;
    static setters const   _setters[];
  };

  typedef shared_ptr<serviceextinfo>    serviceextinfo_ptr;
  typedef std::list<serviceextinfo_ptr> list_serviceextinfo;
}

CCE_END()

#endif // !CCE_CONFIGURATION_SERVICEEXTINFO_HH

