/*
** Copyright 2011 Merethis
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

#include <QDir>
#include <QFile>
#include <exception>
#include <iostream>
#include <math.h>

#include "globals.hh"
#include "common.hh"
#include "error.hh"
#include "logging/engine.hh"
#include "logging/file.hh"
#include "logging/object.hh"

using namespace com::centreon::engine::logging;

/**************************************
 *                                     *
 *         Exported Functions          *
 *                                     *
 **************************************/

/**
 *  Define Symbole to compile, but unused.
 */
extern "C" int neb_add_module(char const* filename, char const* args, int should_be_loaded) {
  (void)filename; (void)args; (void)should_be_loaded;
  return (0);
}

/**
 *  Define Symbole to compile, but unused.
 */
extern "C" void broker_log_data(int type,
				int flags,
				int attr,
				char* data,
				unsigned long data_type,
				time_t entry_time,
				struct timeval* timestamp) {
  (void)type;
  (void)flags;
  (void)attr;
  (void)data;
  (void)data_type;
  (void)entry_time;
  (void)timestamp;
}

/**
 *  Check the file's content.
 *
 *  @param[in] filename The file name.
 *  @param[in] text     The content reference.
 */
void check_file(QString const& filename, QString const& text) {
  QFile file(filename);
  file.open(QIODevice::ReadOnly);
  if (file.error() != QFile::NoError) {
    throw (engine_error() << filename << ": " << file.errorString());
  }

  if (file.readAll() != text) {
    throw (engine_error() << filename << ": bad content.");
  }

  file.close();
}

/**
 *  Check the logging file working.
 *  - file rotate.
 *  - file truncate.
 */
int main(void) {
  try {
    // Get instance of logging engine.
    engine& engine = engine::instance();
    unsigned int id1 = 0;
    unsigned int id2 = 0;

    {
      // Add new object (file) to log into engine and test limit size.
      QSharedPointer<file> obj1(new file("./test_logging_file_size_limit.log", 10));
      engine::obj_info info1(obj1, object::log_all, object::most);
      id1 = engine.add_object(info1);

      // Add new object (file) to log into engine and test rotation.
      QSharedPointer<file> obj2(new file("./test_logging_file_rotate.log", "./"));
      engine::obj_info info2(obj2, object::log_all, object::most);
      id2 = engine.add_object(info2);
    }

    // Send message to all object.
    engine.log("012345", object::log_info_message, object::basic);
    engine.log("0123456789", object::log_info_message, object::basic);
    // Make a rotation for obj2.
    file::rotate_all();
    // Send message to all object.
    engine.log("qwerty", object::log_info_message, object::basic);

    // Cleanup.
    engine.remove_object(id2);
    engine.remove_object(id1);

    QDir dir("./");
    QStringList filters("test_logging_file_*.log*");
    QFileInfoList files = dir.entryInfoList(filters);

    // Check the content of all file log.
    for (QFileInfoList::const_iterator it = files.begin(), end = files.end();
    	 it != end;
    	 ++it) {
      if (it->fileName() == "test_logging_file_size_limit.log") {
    	check_file(it->fileName(), "qwerty");
      }
      else if (it->fileName() == "test_logging_file_size_limit.log.old") {
    	check_file(it->fileName(), "0123456789");
      }
      else if (it->fileName() == "test_logging_file_rotate.log") {
    	check_file(it->fileName(), "qwerty");
      }
      else if (it->fileName().indexOf("test_logging_file_rotate.log") != -1) {
    	check_file(it->fileName(), "0123450123456789");
      }
      else {
    	throw (engine_error() << "bad file name.");
      }
      QFile::remove(it->fileName());
    }
  }
  catch (std::exception const& e) {
    std::cerr << "error: " << e.what() << std::endl;
    return (1);
  }
  catch (...) {
    std::cerr << "error: catch all." << std::endl;
    return (1);
  }
  return (0);
}
