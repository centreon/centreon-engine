/*
** Copyright 2011      Merethis
**
** This file is part of Centreon Scheduler.
**
** Centreon Scheduler is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Scheduler is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Scheduler. If not, see
** <http://www.gnu.org/licenses/>.
*/

#include "common.hh"
#include "nebmodules.hh"
#include "error.hh"
#include "logging.hh"
#include "modules/compatibility.hh"
#include "modules/loader.hh"
#include "modules/handle.hh"

using namespace com::centreon::scheduler::modules;

handle::handle() : QObject() {
  _init_connection();
  event_create(this);
}

handle::handle(QString const& filename, QString const& args)
  : QObject(), _filename(filename), _args(args) {
  _init_connection();
  event_create(this);
}

handle::handle(handle const& right) : QObject() {
  _init_connection();
  operator=(right);
  event_create(this);
}

handle::~handle() throw() {
  event_destroy(this);
  if (_handle.data() != NULL) {
    _handle.clear();
  }
}

handle& handle::operator=(handle const& right) {
  if (this != &right) {
    _author = right._author;
    _copyright = right._copyright;
    _description = right._description;
    _filename = right._filename;
    _license = right._license;
    _name = right._name;
    _version = right._version;
    _args = right._args;
    _handle = right._handle;
  }
  return (*this);
}

bool handle::operator==(handle const& right) const throw() {
  return (_author == right._author
	  && _copyright == right._copyright
	  && _description == right._description
	  && _filename == right._filename
	  && _license == right._license
	  && _name == right._name
	  && _version == right._version
	  && _args == right._args
	  && _handle.data() == right._handle.data());
}

bool handle::operator!=(handle const& right) const throw() {
  return (!operator==(right));
}

void handle::open() {
  if (is_loaded() == true) {
    return;
  }

  _handle = QSharedPointer<QLibrary>(new QLibrary(_filename));
  _handle->load();
  if (_handle->isLoaded() == false) {
    throw (error() << "Cannot load library " << _handle->errorString());
  }

  int* api_version = static_cast<int*>(_handle->resolve("__neb_api_version"));
  if (api_version == NULL || *api_version != CURRENT_NEB_API_VERSION) {
    close();
    throw (error() << "Module is using an old or unspecified version of the event broker API.");
  }

  typedef int (*func_init)(int, char const*, void*);
  func_init init = (func_init)_handle->resolve("nebmodule_init");
  if (init == NULL) {
    close();
    throw (error() << "Cannot resolve symbole nebmodule_init");
  }

  if (init(NEBMODULE_NORMAL_LOAD,
	   _args.toStdString().c_str(),
	   this) != OK) {
    close();
    throw (error() << "Function nebmodule_init returned an error");
  }

  emit event_loaded(this);
}

void handle::open(QString const& filename, QString const& args) {
  if (is_loaded() == true) {
    return;
  }

  _filename = filename;
  _args = args;
  open();
}

void handle::close() {
  if (_handle.data() != NULL) {
    if (_handle->isLoaded()) {
      typedef int (*func_deinit)(int, int);
      func_deinit deinit = (func_deinit)_handle->resolve("nebmodule_deinit");
      if (deinit == NULL) {
	logit(NSLOG_INFO_MESSAGE, false,
	      "Cannot resolve symbole `nebmodule_deinit' in module `%s'.\n",
	      _filename.toStdString().c_str());
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

bool handle::is_loaded() {
  return (_handle.data() != NULL && _handle->isLoaded());
}

QLibrary* handle::get_handle() const throw() {
  return (_handle.data());
}

QString const& handle::get_author() const throw() {
  return (_author);
}

QString const& handle::get_copyright() const throw() {
  return (_copyright);
}

QString const& handle::get_description() const throw() {
  return (_description);
}

QString const& handle::get_filename() const throw() {
  return (_filename);
}

QString const& handle::get_license() const throw() {
  return (_license);
}

QString const& handle::get_name() const throw() {
  return (_name);
}

QString const& handle::get_version() const throw() {
  return (_version);
}

QString const& handle::get_args() const throw() {
  return (_args);
}

void handle::set_author(QString const& author) {
  _author = author;
  emit event_author(this);
}

void handle::set_copyright(QString const& copyright) {
  _copyright = copyright;
  emit event_copyright(this);
}

void handle::set_description(QString const& description) {
  _description = description;
  emit event_description(this);
}

void handle::set_license(QString const& license) {
  _license = license;
  emit event_license(this);
}

void handle::set_name(QString const& name) {
  QString old_name = _name;
  _name = name;
  emit name_changed(_filename, old_name, _name);
  emit event_name(this);
}

void handle::set_version(QString const& version) {
  _version = version;
  emit event_version(this);
}

void handle::_init_connection() {
  modules::loader& loader = modules::loader::instance();
  connect(this, SIGNAL(name_changed(QString const&, QString const&, QString const&)),
	  &loader, SLOT(module_name_changed(QString const&, QString const&, QString const&)));

  modules::compatibility& compatibility = modules::compatibility::instance();
  connect(this, SIGNAL(event_create(modules::handle*)),
	  &compatibility, SLOT(create_module(modules::handle*)));
  connect(this, SIGNAL(event_destroy(modules::handle*)),
	  &compatibility, SLOT(destroy_module(modules::handle*)));
  connect(this, SIGNAL(event_name(modules::handle*)),
	  &compatibility, SLOT(name_module(modules::handle*)));
  connect(this, SIGNAL(event_author(modules::handle*)),
	  &compatibility, SLOT(author_module(modules::handle*)));
  connect(this, SIGNAL(event_copyright(modules::handle*)),
	  &compatibility, SLOT(copyright_module(modules::handle*)));
  connect(this, SIGNAL(event_version(modules::handle*)),
	  &compatibility, SLOT(version_module(modules::handle*)));
  connect(this, SIGNAL(event_license(modules::handle*)),
	  &compatibility, SLOT(license_module(modules::handle*)));
  connect(this, SIGNAL(event_description(modules::handle*)),
	  &compatibility, SLOT(description_module(modules::handle*)));
  connect(this, SIGNAL(event_loaded(modules::handle*)),
	  &compatibility, SLOT(loaded_module(modules::handle*)));
  connect(this, SIGNAL(event_unloaded(modules::handle*)),
	  &compatibility, SLOT(unloaded_module(modules::handle*)));
}
