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

#include <fstream>
#include <libgen.h>
#include <map>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <string.h>
#include "arg_definition.hh"
#include "builder.hh"
#include "error.hh"

using namespace com::centreon::engine::script;

static char const* copyright =
  "/*\n"
  "** Copyright 2011-2012 Merethis\n"
  "**\n"
  "** This file is part of Centreon Engine.\n"
  "**\n"
  "** Centreon Engine is free software: you can redistribute it and/or\n"
  "** modify it under the terms of the GNU General Public License version 2\n"
  "** as published by the Free Software Foundation.\n"
  "**\n"
  "** Centreon Engine is distributed in the hope that it will be useful,\n"
  "** but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
  "** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU\n"
  "** General Public License for more details.\n"
  "**\n"
  "** You should have received a copy of the GNU General Public License\n"
  "** along with Centreon Engine. If not, see\n"
  "** <http://www.gnu.org/licenses/>.\n"
  "*/\n";

/**
 *  Default constructor.
 *
 *  @param[in] header_src Path of soapStub header.
 *  @param[in] header_dst Path of auto_gen header.
 *  @param[in] source_dst Path of auto_gen code.
 */
builder::builder(
           std::string const& header_src,
           std::string const& header_dst,
           std::string const& source_dst)
  : _header_src(header_src),
    _header_dst(header_dst),
    _source_dst(source_dst) {}

/**
 *  Default copy constructor.
 *
 *  @param[in] right The object to copy.
 */
builder::builder(builder const& right) {
  operator=(right);
}

/**
 * Default destructor.
 */
builder::~builder() throw () {}

/**
 *  Default copy operator.
 *
 *  @param[in] right The object to copy.
 *
 *  @return Return this object.
 */
builder& builder::operator=(builder const& right) {
  if (this != &right) {
    _lst_function = right._lst_function;
    _header_src = right._header_src;
    _header_dst = right._header_dst;
    _source_dst = right._source_dst;
  }
  return (*this);
}

/**
 *  Parse the soapStub.
 */
void builder::parse() {
  std::ifstream file(_header_src.c_str(), std::ios_base::in);
  if (!file.is_open())
    throw (error("open file failed"));

  while (file.good()) {
    std::string data;
    std::getline(file, data, '\n');
    if (!function::is_valid(data))
      continue ;
    function func(data);
    func.build();
    _lst_function.push_back(func);
  }
}

/**
 *  Build auto_gen header and source code.
 */
void builder::build() {
  _build_header();
  _build_source();
}

/**
 *  Get the filename from a file path.
 *
 *  @param[in] path  The file path.
 *
 *  @return The filename.
 */
std::string builder::_basename(std::string const& path) {
  char* _path(strdup(path.c_str()));
  std::string filename(basename(_path));
  free(_path);
  return (filename);
}

/**
 *  Build auto_gen header.
 */
void builder::_build_header() {
  std::ofstream file(
                  _header_dst.c_str(),
                  std::ios_base::out | std::ios_base::trunc);
  if (!file.is_open())
    throw (error("open file failed"));

  file << copyright << "\n"
       << "#ifndef CCE_MOD_WS_CLIENT_AUTO_GEN_HH\n"
       << "# define CCE_MOD_WS_CLIENT_AUTO_GEN_HH\n\n"
       << "# include <map>\n"
       << "# include <string>\n"
       << "# include \"" << _basename(_header_src) << "\"\n\n"
       << "class auto_gen {\n"
       << "public:\n"
       << "  static auto_gen& instance();\n\n"
       << "  void show_help() const;\n"
       << "  void show_help(std::string const& name) const;\n"
       << "  bool execute(std::string const& name, soap* s, char const* end_point, char const* action, std::map<std::string, std::string>& args) const;\n\n"
       << "private:\n"
       << "  auto_gen();\n"
       << "  auto_gen(auto_gen const& right);\n"
       << "  ~auto_gen() throw();\n\n"
       << "  auto_gen& operator=(auto_gen const& right);\n\n"
       << "  std::map<std::string, void (*)()> _help;\n"
       << "  std::map<std::string, bool (*)(soap*, char const*, char const*, std::map<std::string, std::string>&)> _exec;\n"
       << "};\n\n"
       << "#endif // !CCE_MOD_WS_CLIENT_AUTO_GEN_HH\n";
}

/**
 *  Build auto_gen source code.
 */
void builder::_build_source() {
  std::ofstream file(
                  _source_dst.c_str(),
                  std::ios_base::out | std::ios_base::trunc);
  if (!file.is_open())
    throw (error("open file failed"));

  file << copyright << "\n"
       << "#include <cstdlib>\n"
       << "#include <list>\n"
       << "#include <ostream>\n"
       << "#include <string>\n"
       << "#include <vector>\n"
       << "#include \"com/centreon/engine/modules/webservice/error.hh\"\n"
       << "#include \"" << _basename(_header_dst) << "\"\n\n"
       << "using namespace com::centreon::engine::modules::webservice;\n\n";

  file << "static std::string toStdString(std::string const& str) {\n"
       << "  return (str);\n"
       << "}\n\n";

  file << "static double toDouble(std::string const& str) {\n"
       << "  return (strtod(str.c_str(), NULL));\n"
       << "}\n\n";

  file << "static float toFloat(std::string const& str) {\n"
       << "  return (strtof(str.c_str(), NULL));\n"
       << "}\n\n";

  file << "static int toInt(std::string const& str) {\n"
       << "  return (strtol(str.c_str(), NULL, 0));\n"
       << "}\n\n";

  file << "static long long toLongLong(std::string const& str) {\n"
       << "  return (strtoll(str.c_str(), NULL, 0));\n"
       << "}\n\n";

  file << "static unsigned int toUInt(std::string const& str) {\n"
       << "  return (strtoul(str.c_str(), NULL, 0));\n"
       << "}\n\n";

  file << "static unsigned long long toULongLong(std::string const& str) {\n"
       << "  return (strtoull(str.c_str(), NULL, 0));\n"
       << "}\n\n";

  file << "static std::vector<std::string> toStdVector(std::string const& str) {\n"
       << "  std::list<std::string> tmp;\n"
       << "  size_t prev(0);\n"
       << "  size_t current;\n"
       << "  while ((current = str.find(',', prev)) != std::string::npos) {\n"
       << "    tmp.push_back(str.substr(prev, current - prev));\n"
       << "    prev = current + 1;\n"
       << "  }\n"
       << "  tmp.push_back(str.substr(prev));\n"
       << "\n"
       << "  std::vector<std::string> tab;\n"
       << "  for (std::list<std::string>::const_iterator\n"
       << "         it(tmp.begin()),\n"
       << "         end(tmp.end());\n"
       << "      it != end;\n"
       << "      ++it)\n"
       << "    tab.push_back(*it);\n"
       << "  return (tab);\n"
       << "}\n\n";

  file << "template <typename T>\n"
       << "static std::ostream& operator<<(\n"
       << "                       std::ostream& os,\n"
       << "                       std::vector<T> const& cls) {\n"
       << "  os << \"{\";\n"
       << "  for (typename std::vector<T>::const_iterator\n"
       << "         it(cls.begin()),\n"
       << "         end(cls.end());\n"
       << "       it != end;\n"
       << "       ++it) {\n"
       << "    if (it + 1 != end)\n"
       << "      os << *it << \", \";\n"
       << "    else\n"
       << "      os << *it;\n"
       << "  }\n"
       << "  os << \"}\";\n"
       << "\n"
       << "  return (os);\n"
       << "}\n\n";

  std::list<argument> args(arg_definition::instance().get_arguments());
  for (std::list<argument>::const_iterator
         it(args.begin()),
         end(args.end());
       it != end;
       ++it) {
    if (!it->is_primitive())
      file << "static std::ostream& operator<<(std::ostream& os, "
           << it->get_type().c_str() << " const& cls) {\n"
           << _build_ostream_struct("cls.", *it).c_str()
           << "\n"
           << "  return (os);\n"
           << "}\n\n";
  }

  for (std::list<function>::const_iterator
         it(_lst_function.begin()),
         end(_lst_function.end());
       it != end;
       ++it)
    file << "static " << it->get_help_function().c_str() << "\n\n";

  for (std::list<function>::const_iterator
         it(_lst_function.begin()),
         end(_lst_function.end());
       it != end;
       ++it)
    file << "static " << it->get_exec_function().c_str() << "\n\n";

  file << "auto_gen& auto_gen::instance() {\n"
       << "  static auto_gen instance;\n"
       << "  return (instance);\n"
       << "}\n\n"
       << "void auto_gen::show_help() const {\n"
       << "  for (std::map<std::string, void(*)()>::const_iterator it(_help.begin()), end(_help.end());\n"
       << "      it != end;\n"
       << "      ++it) {\n"
       << "    it->second();\n"
       << "  }\n"
       << "}\n\n"
       << "void auto_gen::show_help(std::string const& name) const {\n"
       << "  std::map<std::string, void (*)()>::const_iterator it(_help.find(name));\n"
       << "  if (it == _help.end())\n"
       << "    throw (error(\"function not found\"));\n"
       << "  it->second();\n"
       << "}\n\n"
       << "bool auto_gen::execute(std::string const& name, soap* s, char const* end_point, char const* action, std::map<std::string, std::string>& args) const {\n"
       << "  std::map<std::string, bool (*)(soap*, char const*, char const*, std::map<std::string, std::string>&)>::const_iterator it = _exec.find(name);\n"
       << "  if (it == _exec.end())\n"
       << "    throw (error(\"function not found.\"));\n"
       << "  \n"
       << "  return (it->second(s, end_point, action, args));\n"
       << "}\n\n"
       << "auto_gen::auto_gen() {\n";

  for (std::list<function>::const_iterator
         it(_lst_function.begin()),
         end(_lst_function.end());
       it != end;
       ++it)
    file << "  _help[\"" << it->get_name().c_str()
         << "\"] = &" << it->get_help_name().c_str() << ";\n";
  file << "\n";
  for (std::list<function>::const_iterator
         it(_lst_function.begin()),
         end(_lst_function.end());
       it != end;
       ++it)
    file << "  _exec[\"" << it->get_name().c_str()
         << "\"] = &" << it->get_exec_name().c_str() << ";\n";

  file << "}\n\n"
       << "auto_gen::~auto_gen() throw () {\n"
       << "\n"
       << "}\n\n";
}

/**
 *  Build overload std::ostream operator<< for new type.
 *
 *  @param[in] base The struct name.
 *  @param[in] arg  The struct definition.
 *
 *  @return Return the overload source code.
 */
std::string builder::_build_ostream_struct(
                       std::string const& base,
                       argument const& arg) {
  std::string ret;
  if (arg.is_primitive()) {
    std::ostringstream oss;
    oss << "  os << \"" << arg.get_help() << " = \" << "
        << base << " << std::endl;\n";
    ret = oss.str();
  }
  else {
    std::list<argument> const& args(arg.get_args());
    for (std::list<argument>::const_iterator
           it(args.begin()),
           end(args.end());
         it != end;
         ++it) {
      char const* accessor(it->is_primitive() ? "" : "->");
      std::ostringstream arg;
      arg << base << it->get_name() << accessor;
      ret.append(_build_ostream_struct(arg.str(), *it));
    }
  }
  return (ret);
}
