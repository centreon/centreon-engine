/*
** Copyright 2011-2013,2016 Centreon
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

#ifndef CCE_BROKER_HANDLE_HH
# define CCE_BROKER_HANDLE_HH

#  include <memory>
#  include <string>
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/library.hh"

CCE_BEGIN()

namespace                    broker {
  /**
   *  @class handle handle.hh
   *  @brief Handle contains module informations.
   *
   *  Handle is a module object, contains information
   *  about module, start and stop module.
   */
  class                      handle {
  public:
                             handle(
                               std::string const& filename = "",
                               std::string const& args = "");
                             handle(handle const& right);
    virtual                  ~handle() throw ();
    handle&                  operator=(handle const& rigth);
    bool                     operator==(
                               handle const& right) const throw ();
    bool                     operator!=(
                               handle const& right) const throw ();
    void                     close();
    library*                 get_handle() const throw ();
    std::string const&       get_author() const throw ();
    std::string const&       get_copyright() const throw ();
    std::string const&       get_description() const throw ();
    std::string const&       get_filename() const throw ();
    std::string const&       get_license() const throw ();
    std::string const&       get_name() const throw ();
    std::string const&       get_version() const throw ();
    std::string const&       get_args() const throw ();
    bool                     is_loaded();
    void                     open();
    void                     open(
                               std::string const& filename,
                               std::string const& args);
    void                     reload();
    void                     set_author(
                               std::string const& author);
    void                     set_copyright(
                               std::string const& copyright);
    void                     set_description(
                               std::string const& description);
    void                     set_license(
                               std::string const& license);
    void                     set_name(std::string const& name);
    void                     set_version(
                               std::string const& version);

  private:
    void                     _internal_copy(handle const& right);

    std::string              _args;
    std::string              _author;
    std::string              _copyright;
    std::string              _description;
    std::string              _filename;
    std::shared_ptr<library> _handle;
    std::string              _license;
    std::string              _name;
    std::string              _version;
  };
}

CCE_END()

#endif // !CCE_BROKER_HANDLE_HH
