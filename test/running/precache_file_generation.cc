/*
** Copyright 2012-2013 Merethis
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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include "com/centreon/clib.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/io/file_stream.hh"
#include "com/centreon/process.hh"
#include "test/paths.hh"

using namespace com::centreon;

/**
 *  Check that precache files are properly generated.
 *
 *  @return EXIT_SUCCESS on success.
 */
int main() {
  int retval(EXIT_FAILURE);

  // Temporary file names.
  std::string conf_file;
  std::string precache_file;

  try {
    // Initialization.
    clib::load();

    // Generate temporary file names.
    {
      char* ptr;
      ptr = io::file_stream::temp_path();
      if (!ptr)
        throw(engine_error() << "temporary file name generation failure");
      conf_file = ptr;
      ptr = io::file_stream::temp_path();
      if (!ptr)
        throw(engine_error() << "temporary file name generation failure");
      precache_file = ptr;
    }

    // Generate configuration file.
    {
      // Generate data.
      std::string data;
      {
        std::ostringstream oss;
        oss << "log_file=centengine.log\n"
            << "cfg_file=" TEST_DIR "/running/etc/precache/minimal.cfg\n"
            << "precached_object_file=" << precache_file << "\n"
            << "resource_file=" TEST_DIR "/running/etc/precache/resource.cfg\n";
        data = oss.str();
      }

      // Open copied file.
      io::file_stream conf_stream;
      conf_stream.open(conf_file.c_str(), "w");

      // Write data.
      unsigned long remaining(data.size());
      char const* ptr(data.c_str());
      do {
        unsigned long bytes(conf_stream.write(ptr, remaining));
        ptr += bytes;
        remaining -= bytes;
      } while (remaining > 0);
    }

    // Run centengine to generate precache file.
    {
      // Generate command-line.
      std::string cmdline;
      cmdline = CENTENGINE_BINARY;
      cmdline.append(" -v -p '");
      cmdline.append(conf_file);
      cmdline.append("'");

      // Launch process.
      process centengine;
      centengine.exec(cmdline);
      centengine.wait();
    }

    // Compare generated file with reference file.
    {
      // Read reference file.
      std::string ref_content;
      io::file_stream precache_ref;
      precache_ref.open(
          TEST_DIR "/running/etc/precache/objects.precache.expected", "r");
      unsigned long bytes;
      char buffer[1024];
      while ((bytes = precache_ref.read(buffer, sizeof(buffer))) > 0)
        ref_content.append(buffer, bytes);
      precache_ref.close();

      // Read generated file.
      std::string gen_content;
      io::file_stream precache_gen;
      precache_gen.open(precache_file.c_str(), "r");
      while ((bytes = precache_gen.read(buffer, sizeof(buffer))) > 0)
        gen_content.append(buffer, bytes);

      // Remove headers.
      ref_content.erase(0, 359);
      gen_content.erase(0, 359);

      // Compare contents.
      if (ref_content.compare(gen_content))
        throw(engine_error() << "contents differ");

      // Success.
      retval = EXIT_SUCCESS;
    }
  } catch (std::exception const& e) {
    std::cout << e.what() << std::endl;
  }

  // Remove temporary files.
  ::remove(conf_file.c_str());
  ::remove(precache_file.c_str());

  // Cleanup.
  clib::unload();

  return (retval);
}
