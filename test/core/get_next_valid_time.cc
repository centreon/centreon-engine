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

#include <libgen.h>
#include <cstring>
#include <ctime>
#include <exception>
#include <fstream>
#include <iostream>
#include "com/centreon/engine/configuration/applier/timeperiod.hh"
#include "com/centreon/engine/configuration/timeperiod.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/macros/misc.hh"
#include "com/centreon/engine/objects/timeperiod.hh"
#include "com/centreon/engine/string.hh"
#include "test/unittest.hh"

#ifndef __THROW
#define __THROW
#endif  // !__THROW

// # file.conf format
// preferred_time=%Y-%m-%d %H:%M:%S
// current_time=%Y-%m-%d %H:%M:%S
// ref_time=%Y-%m-%d %H:%M:%S
// weekday=monday 00:00-24:00
// speday=XXX
// exclusion=name....
// timeperiod=testing

using namespace com::centreon::engine;

static time_t _current_time = 0;

struct options {
  options() : preferred_time(0), ref_time(0) {}
  std::vector<timeperiod*> period;
  time_t preferred_time;
  time_t ref_time;
};

static void add_timeperiod(std::string const& name,
                           std::string const& alias,
                           std::vector<std::string> const& range,
                           std::vector<std::string> const& exclude) {
  configuration::timeperiod_ptr obj(new configuration::timeperiod);
  obj->parse("timeperiod_name " + name);
  if (!alias.empty())
    obj->parse("alias " + alias);
  for (std::vector<std::string>::const_iterator it(range.begin()),
       end(range.end());
       it != end; ++it)
    obj->parse(*it);
  for (std::vector<std::string>::const_iterator it(exclude.begin()),
       end(exclude.end());
       it != end; ++it)
    obj->parse(*it);
  configuration::applier::timeperiod app;
  app.add_object(obj);
}

static time_t string_to_time_t(std::string const& data) {
  tm t;
  memset(&t, 0, sizeof(t));
  if (!strptime(data.c_str(), "%Y-%m-%d %H:%M:%S", &t))
    throw(engine_error() << "invalid date format");
  return (mktime(&t));
}

// overload of libc time function.
// always return "Mon Jan 28 13:00:00 CET 2013"
extern "C" time_t time(time_t* t) __THROW {
  if (t)
    *t = _current_time;
  return (_current_time);
}

/**
 *  Parse file configuration.
 *
 *  @param[in]  filename  The configuration file path.
 *  @param[out] opt       Struct to fill.
 */
static void parse_file(char const* filename, options& opt) {
  if (!filename)
    throw(engine_error() << "invalid filename: null pointer");
  std::ifstream stream(filename);
  if (!stream.is_open())
    throw(engine_error() << "invalid filename: can't open " << filename);
  std::vector<std::string> range;
  std::vector<std::string> exclude;
  while (stream.good()) {
    std::string line;
    std::getline(stream, line, '\n');
    string::trim(line);
    if (line.empty() || line[0] == '#')
      continue;
    size_t pos(line.find_first_of('='));
    if (pos == std::string::npos)
      throw(engine_error() << "parsing configuration failed: invalid format");
    std::string key(line.substr(0, pos));
    std::string value(line.substr(pos + 1));
    if (key == "preferred_time")
      opt.preferred_time = string_to_time_t(value);
    else if (key == "current_time")
      _current_time = string_to_time_t(value);
    else if (key == "ref_time")
      opt.ref_time = string_to_time_t(value);
    else if (key == "weekday")
      range.push_back(value);
    else if (key == "speday")
      range.push_back(value);
    else if (key == "exclusion")
      exclude.push_back(value);
    else if (key == "timeperiod") {
      add_timeperiod(value, value, range, exclude);
      timeperiod* p(find_timeperiod(value.c_str()));
      if (!p)
        throw(engine_error() << "invalid timeperiod: can't find " << value);
      for (timeperiodexclusion* e(p->exclusions); e; e = e->next)
        e->timeperiod_ptr = find_timeperiod(e->timeperiod_name);
      exclude.clear();
      range.clear();
      opt.period.push_back(p);
    } else
      throw(engine_error() << "parsing configuration failed: invalid format");
  }
  if (!opt.preferred_time || !_current_time || !opt.ref_time ||
      !opt.period.size())
    throw(engine_error() << "invalid configuration");
}

/**
 *  Check that the get_next_valid_time function works properly.
 *
 *  @return 0 on success.
 */
int main_test(int argc, char** argv) {
  if (argc != 2)
    throw(engine_error() << "usage: " << argv[0] << " file.conf");

  try {
    options opt;
    parse_file(argv[1], opt);

    time_t valid(0);
    get_next_valid_time(opt.preferred_time, &valid, opt.period.back());

    if (valid != opt.ref_time) {
      std::string ref_str(ctime(&opt.ref_time));
      std::string valid_str(ctime(&valid));
      throw(engine_error() << "get next valid time failed: "
                           << basename(argv[1]) << ": ref_time("
                           << string::trim(ref_str) << ") valid_time("
                           << string::trim(valid_str) << ")");
    }
  } catch (...) {
    free_memory(get_global_macros());
    throw;
  }
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}
