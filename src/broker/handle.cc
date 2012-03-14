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

#include "com/centreon/engine/broker/handle.hh"
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
handle::handle(QString const& filename, QString const& args)
  : _args(args), _filename(filename), _name(filename) {
  emit event_create(this);
}

/**
 *  Copy constructor.
 *
 *  @param[in] right The object to copy.
 */
handle::handle(handle const& right) : QObject() {
  _internal_copy(right);
  emit event_create(this);
}

/**
 *  Destructor.
 */
handle::~handle() throw () {
  emit event_destroy(this);
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
bool handle::operator==(handle const& right) const throw () {
  return ((_args == right._args)
          && (_author == right._author)
	  && (_copyright == right._copyright)
          && (_description == right._description)
          && (_filename == right._filename)
	  && (_license == right._license)
          && (_name == right._name)
          && (_version == right._version)
	  && (_handle.data() == right._handle.data()));
}

/**
 *  Default no egality operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return true or false.
 */
bool handle::operator!=(handle const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Close and unload module.
 */
void handle::close() {
  if (_handle.data() != NULL) {
    if (_handle->isLoaded()) {
      typedef int (*func_deinit)(int, int);
      func_deinit deinit = (func_deinit)_handle->resolve("nebmodule_deinit");
      if (deinit == NULL) {
	logger(log_info_message, basic)
          << "Cannot resolve symbole 'nebmodule_deinit' in module '"
          << _filename << "'.";
      }
      else {
	deinit(NEBMODULE_FORCE_UNLOAD, NEBMODULE_NEB_SHUTDOWN);
      }
      _handle->unload();
    }
    _handle.clear();
  }
  emit event_unloaded(this);
}

/**
 *  Get the module's arguments.
 *
 *  @return The arguments.
 */
QString const& handle::get_args() const throw () {
  return (_args);
}

/**
 *  Get the module's author name.
 *
 *  @return The author name.
 */
QString const& handle::get_author() const throw () {
  return (_author);
}

/**
 *  Get the module's copyright.
 *
 *  @return The copyright.
 */
QString const& handle::get_copyright() const throw () {
  return (_copyright);
}

/**
 *  Get the module's description.
 *
 *  @return The description.
 */
QString const& handle::get_description() const throw () {
  return (_description);
}

/**
 *  Get the module's filename.
 *
 *  @return The filename.
 */
QString const& handle::get_filename() const throw () {
  return (_filename);
}

/**
 *  Get the handle of the module.
 *
 *  @return pointer on a QLibrary.
 */
QLibrary* handle::get_handle() const throw () {
  return (_handle.data());
}

/**
 *  Get the module's license.
 *
 *  @return The license.
 */
QString const& handle::get_license() const throw () {
  return (_license);
}

/**
 *  Get the module's name.
 *
 *  @return The name.
 */
QString const& handle::get_name() const throw () {
  return (_name);
}

/**
 *  Get the module's version.
 *
 *  @return The version.
 */
QString const& handle::get_version() const throw () {
  return (_version);
}

/**
 *  Check if the module is loaded.
 *
 *  @return true if the module is loaded, false otherwise.
 */
bool handle::is_loaded() {
  return (_handle.data() != NULL && _handle->isLoaded());
}

/**
 *  Open and load module.
 */
void handle::open() {
  if (is_loaded())
    return ;

  _handle = QSharedPointer<QLibrary>(new QLibrary(_filename));
  _handle->setLoadHints(QLibrary::ResolveAllSymbolsHint
    | QLibrary::ExportExternalSymbolsHint);
  _handle->load();
  if (_handle->isLoaded() == false) {
    throw (engine_error() << _handle->errorString());
  }

  int* api_version = static_cast<int*>(_handle->resolve("__neb_api_version"));
  if (api_version == NULL || *api_version != CURRENT_NEB_API_VERSION) {
    close();
    throw (engine_error() << "Module is using an old or unspecified version of the event broker API.");
  }

  typedef int (*func_init)(int, char const*, void*);
  func_init init = (func_init)_handle->resolve("nebmodule_init");
  if (init == NULL) {
    close();
    throw (engine_error() << "Cannot resolve symbole nebmodule_init");
  }

  if (init(NEBMODULE_NORMAL_LOAD,
	   qPrintable(_args),
	   this) != OK) {
    close();
    throw (engine_error() << "Function nebmodule_init returned an error");
  }

  emit event_loaded(this);
}

/**
 *  Open and load module.
 *
 *  @param[in] filename The module filename.
 *  @param[in] args The module arguments.
 */
void handle::open(QString const& filename, QString const& args) {
  if (is_loaded())
    return ;

  close();
  _filename = filename;
  _args = args;
  open();
}

/**
 *  Set the module's author name.
 *
 *  @param[in] The author name.
 */
void handle::set_author(QString const& author) {
  _author = author;
  emit event_author(this);
}

/**
 *  Set the module's copyright.
 *
 *  @param[in] The copyright.
 */
void handle::set_copyright(QString const& copyright) {
  _copyright = copyright;
  emit event_copyright(this);
}

/**
 *  Set the module's description.
 *
 *  @param[in] The description.
 */
void handle::set_description(QString const& description) {
  _description = description;
  emit event_description(this);
}

/**
 *  Set the module's license.
 *
 *  @param[in] The license.
 */
void handle::set_license(QString const& license) {
  _license = license;
  emit event_license(this);
}

/**
 *  Set the module's name.
 *
 *  @param[in] The name.
 */
void handle::set_name(QString const& name) {
  QString old_name = _name;
  _name = name;
  emit name_changed(old_name, _name);
  emit event_name(this);
}

/**
 *  Set the module's version.
 *
 *  @param[in] The version.
 */
void handle::set_version(QString const& version) {
  _version = version;
  emit event_version(this);
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
  return ;
}
