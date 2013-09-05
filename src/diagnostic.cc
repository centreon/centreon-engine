/*
** Copyright 2013 Merethis
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

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include "com/centreon/engine/configuration/parser.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/diagnostic.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/version.hh"
#include "com/centreon/io/file_stream.hh"
#include "com/centreon/process.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

/**
 *  Default constructor.
 */
diagnostic::diagnostic() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
diagnostic::diagnostic(diagnostic const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
diagnostic::~diagnostic() {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
diagnostic& diagnostic::operator=(diagnostic const& right) {
  (void)right;
  return (*this);
}

/**
 *  Generate a diagnostic file.
 *
 *  @param[in] cfg_file Configuration file.
 *  @param[in] out_file Output file.
 */
void diagnostic::generate(
                   std::string const& cfg_file,
                   std::string const& out_file) {
  // Destination directory.
  std::string tmp_dir;
  {
    char const* tmp_dir_ptr(tmpnam(NULL));
    if (!tmp_dir_ptr)
      throw (engine_error()
             << "Cannot generate diagnostic temporary directory path.");
    tmp_dir = tmp_dir_ptr;
    if (mkdir(tmp_dir.c_str(), S_IRWXU)) {
      char const* msg(strerror(errno));
      throw (engine_error()
             << "Cannot create temporary directory '" << tmp_dir
             << "': " << msg);
    }
  }

  // Files to remove.
  std::vector<std::string> to_remove;

  // Base information about the software.
  logger(logging::log_info_message, logging::basic)
    << "Diagnostic: Centreon Engine " << CENTREON_ENGINE_VERSION_STRING;

  // df.
  logger(logging::log_info_message, logging::basic)
    << "Diagnostic: Getting disk usage";
  {
    std::string df_log_path;
    df_log_path = tmp_dir;
    df_log_path.append("/df.log");
    to_remove.push_back(df_log_path);
    _exec_and_write_to_file("df -P", df_log_path);
  }

  // lsb_release.
  logger(logging::log_info_message, logging::basic)
    << "Diagnostic: Getting LSB information";
  {
    std::string lsb_release_log_path;
    lsb_release_log_path = tmp_dir;
    lsb_release_log_path.append("/lsb_release.log");
    to_remove.push_back(lsb_release_log_path);
    _exec_and_write_to_file("lsb_release -a", lsb_release_log_path);
  }

  // uname.
  logger(logging::log_info_message, logging::basic)
    << "Diagnostic: Getting system name";
  {
    std::string uname_log_path;
    uname_log_path = tmp_dir;
    uname_log_path.append("/uname.log");
    to_remove.push_back(uname_log_path);
    _exec_and_write_to_file("uname -a", uname_log_path);
  }

  // /proc/version
  logger(logging::log_info_message, logging::basic)
    << "Diagnostic: Getting kernel information";
  {
    std::string proc_version_log_path;
    proc_version_log_path = tmp_dir;
    proc_version_log_path.append("/proc_version.log");
    to_remove.push_back(proc_version_log_path);
    _exec_and_write_to_file("cat /proc/version", proc_version_log_path);
  }

  // netstat.
  logger(logging::log_info_message, logging::basic)
    << "Diagnostic: Getting network connections information";
  {
    std::string netstat_log_path;
    netstat_log_path = tmp_dir;
    netstat_log_path.append("/netstat.log");
    to_remove.push_back(netstat_log_path);
    _exec_and_write_to_file(
      "netstat -ap --numeric-hosts",
      netstat_log_path);
  }

  // ps.
  logger(logging::log_info_message, logging::basic)
    << "Diagnostic: Getting processes information";
  {
    std::string ps_log_path;
    ps_log_path = tmp_dir;
    ps_log_path.append("/ps.log");
    to_remove.push_back(ps_log_path);
    _exec_and_write_to_file("ps aux", ps_log_path);
  }

  // rpm.
  logger(logging::log_info_message, logging::basic)
    << "Diagnostic: Getting packages information";
  {
    std::string rpm_log_path;
    rpm_log_path = tmp_dir;
    rpm_log_path.append("/rpm.log");
    to_remove.push_back(rpm_log_path);
    _exec_and_write_to_file("rpm -qa centreon*", rpm_log_path);
  }

  // sestatus.
  logger(logging::log_info_message, logging::basic)
    << "Diagnostic: Getting SELinux status";
  {
    std::string sestatus_log_path;
    sestatus_log_path = tmp_dir;
    sestatus_log_path.append("/selinux.log");
    to_remove.push_back(sestatus_log_path);
    _exec_and_write_to_file("sestatus", sestatus_log_path);
  }

  // Parse configuration file.
  logger(logging::log_info_message, logging::basic)
    << "Diagnostic: Parsing configuration file '" << cfg_file << "'";
  configuration::state conf;
  {
    configuration::parser parsr;
    parsr.parse(cfg_file, conf);
  }

  // Create temporary configuration directory.
  std::string tmp_cfg_dir;
  tmp_cfg_dir = tmp_dir;
  tmp_cfg_dir.append("/cfg/");
  if (mkdir(tmp_cfg_dir.c_str(), S_IRWXU)) {
    char const* msg(strerror(errno));
    throw (engine_error()
           << "Cannot create temporary configuration directory '"
           << tmp_cfg_dir << "': " << msg);
  }

  // Copy base configuration file.
  logger(logging::log_info_message, logging::basic)
    << "Diagnostic: Copying configuration files";
  {
    std::string target_path;
    target_path = tmp_cfg_dir;
    size_t pos(cfg_file.find_last_of('/'));
    if (pos != std::string::npos)
      target_path.append(cfg_file.substr(pos + 1));
    else
      target_path.append(cfg_file);
    to_remove.push_back(target_path);
    std::ostringstream oss;
    oss << "cp '" << cfg_file << "' '" << target_path << "'";
    process p;
    p.exec(oss.str());
    p.wait();
  }

  // Copy other configuration files.
  for (std::list<std::string>::const_iterator
         it(conf.cfg_file().begin()),
         end(conf.cfg_file().end());
       it != end;
       ++it) {
    std::string target_path;
    target_path = tmp_cfg_dir;
    size_t pos(it->find_last_of('/'));
    if (pos != std::string::npos)
      target_path.append(it->substr(pos + 1));
    else
      target_path.append(*it);
    to_remove.push_back(target_path);
    std::ostringstream oss;
    oss << "cp '" << *it << "' '" << target_path << "'";
    process p;
    p.exec(oss.str());
    p.wait();
  }

  // Generate file name if not existing.
  std::string my_out_file;
  if (out_file.empty())
    my_out_file = "centengine-diag.tar.gz";
  else
    my_out_file = out_file;

  // Create tarball.
  logger(logging::log_info_message, logging::basic)
    << "Diagnostic: Creating tarball '" << my_out_file << "'";
  {
    std::ostringstream cmdline;
    cmdline << "tar czf '" << my_out_file << "' '" << tmp_dir << "'";
    process p;
    p.exec(cmdline.str());
    p.wait();
  }

  // Cleanup.
  for (std::vector<std::string>::const_iterator
         it(to_remove.begin()),
         end(to_remove.end());
       it != end;
       ++it)
    ::remove(it->c_str());
  rmdir(tmp_cfg_dir.c_str());
  rmdir(tmp_dir.c_str());

  return ;
}

/**
 *  Execute a command and write its result to a file.
 *
 *  @param[in] cmd      Command file.
 *  @param[in] out_file Output file.
 */
void diagnostic::_exec_and_write_to_file(
                   std::string const& cmd,
                   std::string const& out_file) {
  std::string result;
  {
    process p;
    p.exec(cmd);
    p.wait();
    p.read(result);
  }
  io::file_stream fs;
  fs.open(out_file, "w");
  while (!result.empty()) {
    unsigned long wb(fs.write(result.data(), result.size()));
    result.erase(0, wb);
  }
  return ;
}
