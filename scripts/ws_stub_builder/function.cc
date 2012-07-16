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

#include <algorithm>
#include <cctype>
#include <regex.h>
#include <sstream>
#include <string.h>
#include "arg_definition.hh"
#include "error.hh"
#include "function.hh"

using namespace com::centreon::engine::script;

// Regexp pattern to extract name and arguments of soapStub function.
char const* com::centreon::engine::script::function::_pattern =
  "^SOAP_FMAC5 int SOAP_FMAC6 soap_call_centreonengine__(\\w*)\\("
  "struct soap \\*soap, "
  "const char \\*soap_endpoint, "
  "const char \\*soap_action, "
  "(.*)\\);\n$";

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Constructor.
 *
 *  @param[in] data The reference prototype.
 */
function::function(std::string const& data)
  : _data(data),
    _def(arg_definition::instance()) {}

/**
 *  Copy constructor.
 *
 *  @param[in] right The object to copy.
 */
function::function(function const& right)
  : _def(arg_definition::instance()) {
  operator=(right);
}

/**
 *  Default destructor.
 */
function::~function() throw () {}

/**
 *  Default copy operator.
 *
 *  @param[in] right The object to copy.
 *
 *  @return Return this object.
 */
function& function::operator=(function const& right) {
  if (this != &right) {
    _data = right._data;
    _function = right._function;
    _new_function = right._new_function;
    _args_info = right._args_info;
    _help_prototype = right._help_prototype;
    _exec_prototype = right._exec_prototype;
    _help_function = right._help_function;
    _exec_function = right._exec_function;
  }
  return (*this);
}

/**
 *  Build all (function and prototype).
 */
void function::build() {
  regex_t reg;
  if (regcomp(&reg, _pattern, REG_EXTENDED) != 0)
    throw (error("build failed: regcomp failed"));

  static size_t const match(3);
  regmatch_t pmatch[match];
  memset(pmatch, 0, sizeof(pmatch));
  if (regexec(&reg, _data.c_str(), match, pmatch, 0)
      || reg.re_nsub != 2) {
    regfree(&reg);
    throw (error("build failed: regexec failed"));
  }

  std::string function(
                _data,
                pmatch[1].rm_so,
                pmatch[1].rm_eo - pmatch[1].rm_so);
  std::string args(
                _data,
                pmatch[2].rm_so,
                pmatch[2].rm_eo - pmatch[2].rm_so);
  regfree(&reg);


  _function = function;
  _new_function = _clean_function_name(_function);
  _build_args_info(args);
  _build_help_prototype();
  _build_exec_prototype();
  _build_help_function();
  _build_exec_function();
  return ;
}

/**
 *  Get the execute function code.
 *
 *  @return The execute function code.
 */
std::string const& function::get_exec_function() const throw () {
  return (_exec_function);
}

/**
 *  Get the execute function name.
 *
 *  @return The execute function name.
 */
std::string function::get_exec_name() const throw () {
  return (std::string("exec_").append(_new_function));
}

/**
 *  Get the execute function prototype.
 *
 *  @return the execute function prototype.
 */
std::string const& function::get_exec_prototype() const throw () {
  return (_exec_prototype);
}

/**
 *  Get the help function code.
 *
 *  @return The help function code.
 */
std::string const& function::get_help_function() const throw () {
  return (_help_function);
}

/**
 *  Get the help function name.
 *
 *  @return The help function name.
 */
std::string function::get_help_name() const throw () {
  std::string ret("help_");
  ret.append(_new_function);
  return (ret);
}

/**
 *  Get the help function prototype.
 *
 *  @return The help function prototype.
 */
std::string const& function::get_help_prototype() const throw () {
  return (_help_prototype);
}

/**
 *  Get the basic name of function.
 *
 *  @return The basic name of function.
 */
std::string const& function::get_name() const throw () {
  return (_new_function);
}

/**
 *  Check if the reference prototype is valid.
 *
 *  @param[in] data The reference prototype.
 *
 *  @return Return true if data is valid, false otherwise.
 */
bool function::is_valid(std::string const& data) throw () {
  regex_t reg;
  if (regcomp(&reg, _pattern, REG_EXTENDED | REG_NOSUB) != 0)
    return (false);
  bool ret(!regexec(&reg, data.c_str(), 0, NULL, 0));
  regfree(&reg);
  return (ret);
}

/**************************************
*                                     *
*           Private Methods           *
*                                     *
**************************************/

/**
 *  Build the arguments info list.
 */
void function::_build_args_info(std::string const& args_list) {
  // Split arguments.
  std::list<std::string> list;
  size_t prev(0);
  size_t current;
  while ((current = args_list.find(',', prev)) != std::string::npos) {
    list.push_back(args_list.substr(prev, current - prev));
    prev = current + 1;
  }
  list.push_back(args_list.substr(prev));

  for (std::list<std::string>::iterator
         it(list.begin()),
         end(list.end());
       it != end;
       ++it) {
    // Remove struct keyword.
    size_t pos;
    while ((pos = it->find("struct ")) != std::string::npos)
      it->erase(pos, 7);

    // Trim argument.
    while (!it->empty() && isspace((*it)[0]))
      it->erase(0, 1);
    while (!it->empty() && isspace((*it)[it->size() - 1]))
      it->resize(it->size() - 1);

    // Fetch information.
    arg_info info;
    info.is_ref = ((it->find('&') == std::string::npos) ? false : true);
    info.is_pointer = ((it->find('*') == std::string::npos)
                       ? false
                       : true);

    // Remove characters.
    while ((pos = it->find('*')) != std::string::npos)
      it->erase(pos, 1);
    while ((pos = it->find('&')) != std::string::npos)
      it->erase(pos, 1);

    // Find last space.
    size_t idx(it->rfind(' '));
    if (idx == std::string::npos)
      throw (error("invalid argument"));

    // Type.
    info.type = it->substr(0, idx);
    while (!info.type.empty() && isspace(info.type[0]))
      info.type.erase(0, 1);
    while (!info.type.empty()
           && isspace(info.type[info.type.size() - 1]))
      info.type.resize(info.type.size() - 1);

    // Name.
    info.name = it->substr(idx);
    while (!info.name.empty() && isspace(info.name[0]))
      info.name.erase(0, 1);
    while (!info.name.empty()
           && isspace(info.name[info.name.size() - 1]))
      info.name.resize(info.name.size() - 1);

    // Push info.
    _args_info.push_back(info);
  }

  return ;
}

/**
 *  Build the initialization deallocation.
 *
 *  @param[in] base The variable name of the struct.
 *  @param[in] arg  The argument to build init struct.
 *
 *  @return The deallocation struct.
 */
std::string function::_build_exec_delete(
                        std::string const& base,
                        argument const& arg) {
  if (arg.is_primitive())
    return ("");

  std::string ret;
  std::list<argument> const& args(arg.get_args());
  for (std::list<argument>::const_iterator
         it(args.begin()),
         end(args.end());
       it != end;
       ++it) {
    if (it->is_primitive()) {
      std::string arg(base);
      arg.append(it->get_name());
      ret.append(_build_exec_new(arg, *it));
    }
    else {
      ret.append("  delete ");
      ret.append(base);
      ret.append(it->get_name());
      ret.append(";\n");
      std::string arg(base);
      arg.append(it->get_name());
      arg.append("->");
      ret.append(_build_exec_new(arg, *it));
    }
  }
  return (ret);
}

/**
 *  Build the execute function code.
 */
void function::_build_exec_function() {
  std::string var;
  for (std::list<arg_info>::const_iterator
         it(_args_info.begin()),
         end(_args_info.end());
       it != end;
       ++it) {
    std::ostringstream oss;
    oss << "  " << it->type << " _" << it->name << ";\n";
    var.append(oss.str());
  }

  std::string alloc_var;
  std::string init_var;
  std::string release_var;
  _list_pos = 0;
  for (std::list<arg_info>::const_iterator
         it(_args_info.begin()),
         end(_args_info.end());
       it != end;
       ++it) {
    if (!it->is_ref) {
      argument const& arg(_def.find_argument(it->type));
      std::string base;
      {
        std::ostringstream oss;
        oss << "_" << it->name << (arg.is_primitive() ? "" : ".");
        base = oss.str();
      }
      alloc_var.append(_build_exec_new(base, arg));
      init_var.append(_build_exec_struct(base, arg));
      release_var.append(_build_exec_delete(base, arg));
    }
  }

  if (init_var == "")
    init_var = "  (void)args;\n";
  else if (alloc_var != "") {
    std::ostringstream oss;
    oss << alloc_var << "\n" << init_var;
    init_var = oss.str();
  }

  std::string args;
  for (std::list<arg_info>::const_iterator
         it(_args_info.begin()),
         end(_args_info.end());
       it != end;
       ++it) {
    args.append(it->is_pointer ? ", &_" : ", _");
    args.append(it->name);
  }

  std::string display;
  for (std::list<arg_info>::const_iterator
         it(_args_info.begin()),
         end(_args_info.end());
       it != end;
       ++it) {
    if (it->is_ref && _def.exist_argument(it->type)) {
      display.append("  std::cout << _");
      display.append(it->name);
      display.append(" << std::endl;\n");
    }
  }

  std::ostringstream oss;
  oss << "bool exec_" << _new_function << "(soap* s, char const* end_point, char const* action, std::map<std::string, std::string>& args) {\n"
      << var << "\n"
      << init_var << "\n"
      << "  int ret(soap_call_centreonengine__" << _function << "(s, end_point, action" << args << "));\n"
      << release_var << "\n"
      << "  if (ret != SOAP_OK) {\n"
      << "    soap_print_fault(s, stderr);\n"
      << "    return (false);\n"
      << "  }\n"
      << "\n"
      << display << "\n"
      << "  return (true);\n"
      << "}";
  _exec_function = oss.str();
  //_exec_function.replace("\n\n\n", "\n\n");
  return ;
}

/**
 *  Build the initialization allocation.
 *
 *  @param[in] base The variable name of the struct.
 *  @param[in] arg  The argument to build init struct.
 *
 *  @return The allocation struct.
 */
std::string function::_build_exec_new(
                        std::string const& base,
                        argument const& arg) {
  if (arg.is_primitive())
    return ("");

  std::string ret;
  std::list<argument> const& args(arg.get_args());
  for (std::list<argument>::const_iterator
         it(args.begin()),
         end(args.end());
       it != end;
       ++it) {
    if (it->is_primitive())
      ret.append(_build_exec_new(base + it->get_name(), *it));
    else {
      ret.append("  ");
      ret.append(base);
      ret.append(it->get_name());
      ret.append(" = new ");
      ret.append(it->get_type());
      ret.append("();\n");
      ret.append(_build_exec_new(base + it->get_name() + "->", *it));
    }
  }
  return (ret);
}

/**
 *  Build the execute function prototype.
 */
void function::_build_exec_prototype() {
  std::ostringstream oss;
  oss << "bool exec_" << _new_function << "(soap* s, char const* end_point, char const* action, std::map<std::string, std::string>& args)";
  _exec_prototype = oss.str();
  return ;
}

/**
 *  Build the initialization struct.
 *
 *  @param[in] base The variable name of the struct.
 *  @param[in] arg  The argument to build init struct.
 *
 *  @return The initialization struct.
 */
std::string function::_build_exec_struct(
                        std::string const& base,
                        argument const& arg) {
  if (arg.is_primitive()) {
    if (!arg.is_optional()) {
      return ("  if (args.find(\"" + arg.get_help() + "\") == args.end())\n"
              "    throw (error(\"argument \\\"" + arg.get_help() + "\\\" missing.\"));\n"
              "  " + base + " = " + _get_string_method(arg.get_type())
	      + "(args[\"" + arg.get_help() + "\"]);\n");
    }
    else if (arg.is_array()) {
      return ("  if (args.find(\"" + arg.get_help() + "\") != args.end()) {\n"
              "    " + base + " = " + _get_string_method(arg.get_type())
	      + "(args[\"" + arg.get_help() + "\"]);\n"
              "  }\n");
    }
    else {
      std::string varname(base);
      _replace(varname, "-", "_");
      _replace(varname, ">", "_");
      _replace(varname, ".", "_");

      std::ostringstream oss;
      oss << "  " << arg.get_type() << " " << varname << ";\n"
          << "  if (args.find(\"" << arg.get_help() << "\") != args.end()) {\n"
          << "    " << varname << " = " << _get_string_method(arg.get_type())
          << "(args[\"" << arg.get_help() << "\"])" << ";\n"
          << "    " << base << " = &" << varname << ";\n"
          << "  }\n";
      return (oss.str());
    }
  }

  std::string ret;
  std::list<argument> const& args(arg.get_args());
  for (std::list<argument>::const_iterator
         it(args.begin()),
         end(args.end());
       it != end;
       ++it) {
    char const* accessor = (it->is_primitive() ? "" : "->");
    ret.append(
          _build_exec_struct(base + it->get_name() + accessor, *it));
  }
  return (ret);
}

/**
 *  Build the help arguments for build function prototype.
 *
 *  @param[in] arg The arguments reference.
 *
 *  @return The new arguments.
 */
std::string function::_build_help_args(argument const& arg) const {
  if (arg.is_primitive()) {
    if (!arg.is_optional())
      return (" " + arg.get_help());
    return (" [" + arg.get_help() + "]");
  }

  std::string ret;
  std::list<argument> const& args(arg.get_args());
  for (std::list<argument>::const_iterator
         it(args.begin()),
         end(args.end());
       it != end;
       ++it)
    ret.append(_build_help_args(*it));
  return (ret);
}

/**
 *  Build the help function code.
 */
void function::_build_help_function() {
  // Printable usage.
  std::string usage;
  for (std::list<arg_info>::const_iterator
         it(_args_info.begin()),
         end(_args_info.end());
       it != end;
       ++it) {
    if (!it->is_ref)
      usage.append(_build_help_args(_def.find_argument(it->type)));
  }

  // Usage routine.
  std::ostringstream oss;
  oss << "void help_" << _new_function << "() {\n"
      << "  std::cout << \"" << _new_function << " " << usage << "\" << std::endl;\n"
      << "}";
  _help_function = oss.str();

  return ;
}

/**
 *  Build the help function prototype.
 */
void function::_build_help_prototype() {
  std::ostringstream oss;
  oss << "void help_" << _new_function << "()";
  _help_prototype = oss.str();
  return ;
}

/**
 *  Cleanup the function name, replace uppercase by an '_' folow by the same
 *  character in lowercase.
 *
 *  @param[in] name The function name.
 *
 *  @return Return the new function name.
 */
std::string function::_clean_function_name(std::string const& name) {
  std::string ret;
  for (std::string::const_iterator it(name.begin()), end(name.end());
       it != end;
       ++it) {
    if (isupper(*it)) {
      ret += "_";
      ret += tolower(*it);
    }
    else
      ret += *it;
  }

  return (ret);
}
/**
 *  Change a std::string to an other type with appropriate std::string
 *  method.
 *
 *  @param[in] type The variable type.
 *
 *  @return Return the std::string method name to translate the type.
 */
std::string function::_get_string_method(std::string type) const {
  if (type.empty())
    return ("");
  if (type == "std::vector<std::string>")
    return ("toStdVector");
  _replace(type, "time_t", "long long");
  _replace(type, "ULONG64", "unsigned long long");
  _replace(type, "bool", "int");
  _replace(type, "unsigned", "u");
  _replace(type, "::", " ");
  std::transform(
         type.begin(),
         type.end(),
         type.begin(),
         tolower);
  type[0] = toupper(type[0]);

  std::string ret("to");
  for (std::string::const_iterator it(type.begin()), end(type.end());
       it != end;
       ++it) {
    if ((it != type.begin()) && (*(it - 1) == ' '))
      ret += toupper(*it);
    else if (*it != ' ')
      ret += *it;
  }
  return (ret);
}

/**
 *  Find and replace string.
 *
 *  @param[out] str      The string to modify.
 *  @param[in]  old_str  The string to old string.
 *  @param[in]  new_str  The string to new string.
 */
std::string& function::_replace(
                         std::string& str,
                         std::string const& old_str,
                         std::string const& new_str) {
  for (size_t pos(0);
       ((pos = str.find(old_str, pos)) != std::string::npos);
       ++pos)
    str.replace(pos, old_str.size(), new_str);
  return (str);
}
