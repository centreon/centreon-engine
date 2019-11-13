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

#include "com/centreon/engine/broker/handle.hh"
#include "com/centreon/engine/broker/compatibility.hh"
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/nebmodules.hh"

using namespace com::centreon::engine::broker;
using namespace com::centreon::engine::logging;

/**************************************
 *                                     *
 *           Public Methods            *
 *                                     *
 **************************************/

/**
 *  Constructor.
 *
 *  @param[in] filename The module filename.
 *  @param[in] args     The module args.
 */
handle::handle(std::string const& filename, std::string const& args)
    : _args(args), _filename(filename), _name(filename) {
  broker::compatibility::instance().create_module(this);
}

/**
 *  Copy constructor.
 *
 *  @param[in] right The object to copy.
 */
handle::handle(handle const& right) {
  _internal_copy(right);
  broker::compatibility::instance().create_module(this);
}

/**
 *  Destructor.
 */
handle::~handle() throw() {
  broker::compatibility::instance().destroy_module(this);
}

/**
 *  Assignment operator.
 *
 *  @param[in] right The object to copy.
 *
 *  @return This object.
 */
handle& handle::operator=(handle const& right) {
  if (this != &right)
    _internal_copy(right);
  return (*this);
}

/**
 *  Default egality operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return true or false.
 */
bool handle::operator==(handle const& right) const throw() {
  return ((_args == right._args) && (_author == right._author) &&
          (_copyright == right._copyright) &&
          (_description == right._description) &&
          (_filename == right._filename) && (_license == right._license) &&
          (_name == right._name) && (_version == right._version) &&
          (_handle.get() == right._handle.get()));
}

/**
 *  Default no egality operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return true or false.
 */
bool handle::operator!=(handle const& right) const throw() {
  return (!operator==(right));
}

/**
 *  Close and unload module.
 */
void handle::close() {
  if (_handle.get()) {
    if (_handle->is_loaded()) {
      typedef int (*func_deinit)(int, int);
      func_deinit deinit(
          (func_deinit)_handle->resolve_proc("nebmodule_deinit"));
      if (!deinit)
        logger(log_info_message, basic)
            << "Cannot resolve symbole 'nebmodule_deinit' in module '"
            << _filename << "'.";
      else
        deinit(NEBMODULE_FORCE_UNLOAD | NEBMODULE_ENGINE,
               NEBMODULE_NEB_SHUTDOWN);
      _handle->unload();
    }
    _handle.reset();
  }
  broker::compatibility::instance().unloaded_module(this);
  return;
}

/**
 *  Get the module's arguments.
 *
 *  @return The arguments.
 */
std::string const& handle::get_args() const throw() {
  return (_args);
}

/**
 *  Get the module's author name.
 *
 *  @return The author name.
 */
std::string const& handle::get_author() const throw() {
  return (_author);
}

/**
 *  Get the module's copyright.
 *
 *  @return The copyright.
 */
std::string const& handle::get_copyright() const throw() {
  return (_copyright);
}

/**
 *  Get the module's description.
 *
 *  @return The description.
 */
std::string const& handle::get_description() const throw() {
  return (_description);
}

/**
 *  Get the module's filename.
 *
 *  @return The filename.
 */
std::string const& handle::get_filename() const throw() {
  return (_filename);
}

/**
 *  Get the handle of the module.
 *
 *  @return pointer on a library.
 */
com::centreon::library* handle::get_handle() const throw() {
  return (_handle.get());
}

/**
 *  Get the module's license.
 *
 *  @return The license.
 */
std::string const& handle::get_license() const throw() {
  return (_license);
}

/**
 *  Get the module's name.
 *
 *  @return The name.
 */
std::string const& handle::get_name() const throw() {
  return (_name);
}

/**
 *  Get the module's version.
 *
 *  @return The version.
 */
std::string const& handle::get_version() const throw() {
  return (_version);
}

/**
 *  Check if the module is loaded.
 *
 *  @return true if the module is loaded, false otherwise.
 */
bool handle::is_loaded() {
  return (_handle.get() && _handle->is_loaded());
}

/**
 *  Open and load module.
 */
void handle::open() {
  if (is_loaded())
    return;

  try {
    _handle = std::shared_ptr<library>(new library(_filename));
    _handle->load();

    int api_version(*static_cast<int*>(_handle->resolve("__neb_api_version")));
    if (api_version != CURRENT_NEB_API_VERSION)
      throw(engine_error() << "Module is using an old or unspecified "
                              "version of the event broker API");

    typedef int (*func_init)(int, char const*, void*);
    func_init init((func_init)_handle->resolve_proc("nebmodule_init"));

    if (init(NEBMODULE_NORMAL_LOAD | NEBMODULE_ENGINE, _args.c_str(), this) !=
        OK)
      throw(engine_error() << "Function nebmodule_init "
                              "returned an error");

    broker::compatibility::instance().loaded_module(this);
  } catch (std::exception const& e) {
    (void)e;
    close();
    throw;
  }
  return;
}

/**
 *  Open and load module.
 *
 *  @param[in] filename The module filename.
 *  @param[in] args The module arguments.
 */
void handle::open(std::string const& filename, std::string const& args) {
  if (is_loaded())
    return;

  close();
  _filename = filename;
  _args = args;
  open();
  return;
}

/**
 *  Reload the module.
 */
void handle::reload() {
  if (!is_loaded())
    return;
  typedef int (*func_reload)();
  func_reload routine((func_reload)_handle->resolve_proc("nebmodule_reload"));
  if (routine)
    routine();
  return;
}

/**
 *  Set the module's author name.
 *
 *  @param[in] The author name.
 */
void handle::set_author(std::string const& author) {
  _author = author;
  broker::compatibility::instance().author_module(this);
  return;
}

/**
 *  Set the module's copyright.
 *
 *  @param[in] The copyright.
 */
void handle::set_copyright(std::string const& copyright) {
  _copyright = copyright;
  broker::compatibility::instance().copyright_module(this);
  return;
}

/**
 *  Set the module's description.
 *
 *  @param[in] The description.
 */
void handle::set_description(std::string const& description) {
  _description = description;
  broker::compatibility::instance().description_module(this);
  return;
}

/**
 *  Set the module's license.
 *
 *  @param[in] The license.
 */
void handle::set_license(std::string const& license) {
  _license = license;
  broker::compatibility::instance().license_module(this);
  return;
}

/**
 *  Set the module's name.
 *
 *  @param[in] The name.
 */
void handle::set_name(std::string const& name) {
  _name = name;
  broker::compatibility::instance().name_module(this);
  return;
}

/**
 *  Set the module's version.
 *
 *  @param[in] The version.
 */
void handle::set_version(std::string const& version) {
  _version = version;
  broker::compatibility::instance().version_module(this);
  return;
}

/**************************************
 *                                     *
 *           Private Methods           *
 *                                     *
 **************************************/

/**
 *  Copy internal data members.
 *
 *  @param[in] right Object to copy.
 */
void handle::_internal_copy(handle const& right) {
  _args = right._args;
  _author = right._author;
  _copyright = right._copyright;
  _description = right._description;
  _filename = right._filename;
  _handle = right._handle;
  _license = right._license;
  _name = right._name;
  _version = right._version;
  return;
}
