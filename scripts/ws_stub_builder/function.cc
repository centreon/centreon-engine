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

#include <QRegExp>
#include <QStringList>
#include <QRegExp>

#include "error.hh"
#include "arg_definition.hh"
#include "function.hh"

using namespace com::centreon::engine::script;

// Regexp pattern to extract name and arguments of soapStub function.
char const* com::centreon::engine::script::function::_pattern =
  "^SOAP_FMAC5 int SOAP_FMAC6 soap_call_centreonengine__(\\w*)\\("
  "struct soap \\*soap, "
  "const char \\*soap_endpoint, "
  "const char \\*soap_action, "
  "(.*)\\);\n$";

/**
 *  Default constructor.
 *
 *  @param[in] data The reference prototype.
 */
function::function(QString const& data)
  : _def(arg_definition::instance()),
    _data(data) {

}

/**
 *  Default copy constructor.
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
function::~function() throw() {

}

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
 *  Check if the reference prototype is valid.
 *
 *  @param[in] data The reference prototype.
 *
 *  @return Return true if data is valid, false otherwise.
 */
bool function::is_valid(QString const& data) throw() {
  static QRegExp check(_pattern);
  return (check.exactMatch(data));
}

/**
 *  Build all (function and prototype).
 */
void function::build() {
  QRegExp reg(_pattern);
  if (reg.indexIn(_data) == -1 || reg.captureCount() != 2) {
    throw (error("build failed `invalid string'."));
  }
  _function = reg.cap(1);
  _new_function = _clean_function_name(_function);
  _build_args_info(reg.cap(2));


  _build_help_prototype();
  _build_exec_prototype();
  _build_help_function();
  _build_exec_function();
}

/**
 *  Get the basic name of function.
 *
 *  @return The basic name of function.
 */
QString const& function::get_name() const throw() {
  return (_new_function);
}

/**
 *  Get the execute function name.
 *
 *  @return The execute function name.
 */
QString function::get_exec_name() const throw() {
  return ("exec_" + _new_function);
}

/**
 *  Get the help function name.
 *
 *  @return The help function name.
 */
QString function::get_help_name() const throw() {
  return ("help_" + _new_function);
}

/**
 *  Get the help function prototype.
 *
 *  @return The help function prototype.
 */
QString const& function::get_help_prototype() const throw() {
  return (_help_prototype);
}

/**
 *  Get the execute function prototype.
 *
 *  @return the execute function prototype.
 */
QString const& function::get_exec_prototype() const throw() {
  return (_exec_prototype);
}

/**
 *  Get the help function code.
 *
 *  @return The help function code.
 */
QString const& function::get_help_function() const throw() {
  return (_help_function);
}

/**
 *  Get the execute function code.
 *
 *  @return The execute function code.
 */
QString const& function::get_exec_function() const throw() {
  return (_exec_function);
}

/**
 *  Build the help function prototype.
 */
void function::_build_help_prototype() {
  QString func("void help_%1()");
  _help_prototype = func.arg(_new_function);
}

/**
 *  Build the execute function prototype.
 */
void function::_build_exec_prototype() {
  QString func("bool exec_%1(soap* s, char const* end_point, char const* action, QHash<QString, QString> const& args)");
  _exec_prototype = func.arg(_new_function);
}

/**
 *  Build the help function code.
 */
void function::_build_help_function() {
  QString func("void help_%1() {\n"
	       "  std::cout << \"%2\" << std::endl;\n"
	       "}");
  QString usage;

  for (QList<arg_info>::const_iterator it = _args_info.begin(), end = _args_info.end();
       it != end;
       ++it) {
    if (it->is_ref == false) {
      usage += _build_help_args(_def.find_argument(it->type));
    }
  }

  _help_function = func.arg(_new_function).arg(_new_function + " " + usage);
}

/**
 *  Build the execute function code.
 */
void function::_build_exec_function() {
  QString func("bool exec_%1(soap* s, char const* end_point, char const* action, QHash<QString, QString> const& args) {\n"
	       "%3\n"
	       "%4\n"
	       "  int ret = soap_call_centreonengine__%5(s, end_point, action%6);\n"
	       "%7\n"
	       "  if (ret != SOAP_OK) {\n"
	       "    soap_print_fault(s, stderr);\n"
	       "    return (false);\n"
	       "  }\n"
	       "\n"
	       "%8\n"
	       "  return (true);\n"
	       "}");

  QString var;
  for (QList<arg_info>::const_iterator it = _args_info.begin(), end = _args_info.end();
       it != end;
       ++it) {
    var += "  " + it->type + " _" + it->name + ";\n";
  }

  QString alloc_var;
  QString init_var;
  QString release_var;
  _list_pos = 0;
  for (QList<arg_info>::const_iterator it = _args_info.begin(), end = _args_info.end();
       it != end;
       ++it) {
    if (it->is_ref == false) {
      argument const& arg = _def.find_argument(it->type);
      QString base = "_" + it->name + (arg.is_primitive() ? "" : ".");
      alloc_var += _build_exec_new(base, arg);
      init_var += _build_exec_struct(base, arg);
      release_var += _build_exec_delete(base, arg);
    }
  }

  if (init_var == "") {
    init_var = "  (void)args;\n";
  }
  else if (alloc_var != "") {
    init_var = alloc_var + "\n" + init_var;
  }

  QString args;
  for (QList<arg_info>::const_iterator it = _args_info.begin(), end = _args_info.end();
       it != end;
       ++it) {
    args += (it->is_pointer ? ", &_" : ", _") + it->name;
  }

  QString display;
  for (QList<arg_info>::const_iterator it = _args_info.begin(), end = _args_info.end();
       it != end;
       ++it) {
    if (it->is_ref == true && _def.exist_argument(it->type)) {
      display += "  std::cout << _" + it->name  + " << std::endl;\n";
    }
  }

  _exec_function = func
    .arg(_new_function)
    .arg(var)
    .arg(init_var)
    .arg(_function)
    .arg(args)
    .arg(release_var)
    .arg(display);
  _exec_function.replace("\n\n\n", "\n\n");
}

/**
 *  Build the arguments info list.
 */
void function::_build_args_info(QString const& args_list) {
  QStringList list = args_list.split(",").replaceInStrings("struct ", "");

  for (QStringList::const_iterator it = list.begin(), end = list.end();
       it != end;
       ++it) {
    QString arg = it->trimmed();

    arg_info info;
    info.is_ref = (arg.indexOf('&') == -1 ? false : true);
    info.is_pointer = (arg.indexOf('*') == -1 ? false : true);

    arg.remove('*');
    arg.remove('&');

    int idx = arg.lastIndexOf(' ');
    if (idx == -1) {
      throw (error("invalid argument."));
    }
    info.type = arg.left(idx).trimmed();
    info.name = arg.right(arg.size() - idx).trimmed();
    _args_info.push_back(info);
  }
}

/**
 *  Build the help arguments for build function prototype.
 *
 *  @param[in] arg The arguments reference.
 *
 *  @return The new arguments.
 */
QString function::_build_help_args(argument const& arg) const {
  if (arg.is_primitive() == true) {
    if (arg.is_optional() == false)
      return (" " + arg.get_help());
    return (" [" + arg.get_help() + "]");
  }

  QString ret;
  QList<argument> const& args = arg.get_args();
  for (QList<argument>::const_iterator it = args.begin(), end = args.end();
       it != end;
       ++it) {
    ret += _build_help_args(*it);
  }
  return (ret);
}

/**
 *  Build the initialization allocation.
 *
 *  @param[in] base The variable name of the struct.
 *  @param[in] arg  The argument to build init struct.
 *
 *  @return The allocation struct.
 */
QString function::_build_exec_new(QString const& base,
				    argument const& arg) {
  if (arg.is_primitive() == true) {
    return ("");
  }

  QString ret;
  QList<argument> const& args = arg.get_args();
  for (QList<argument>::const_iterator it = args.begin(), end = args.end();
       it != end;
       ++it) {

    if (it->is_primitive()) {
      ret += _build_exec_new(base + it->get_name(), *it);
    }
    else {
      ret += "  " + base + it->get_name() + " = new " + it->get_type() + "();\n";
      ret += _build_exec_new(base + it->get_name() + "->", *it);
    }
  }
  return (ret);
}

/**
 *  Build the initialization struct.
 *
 *  @param[in] base The variable name of the struct.
 *  @param[in] arg  The argument to build init struct.
 *
 *  @return The initialization struct.
 */
QString function::_build_exec_struct(QString const& base,
				     argument const& arg) {
  if (arg.is_primitive() == true) {
    if (!arg.is_optional()) {
      return ("  if (args.find(\"" + arg.get_help() + "\") == args.end())\n"
              "    throw (error(\"argument \\\"" + arg.get_help() + "\\\" missing.\"));\n"
              "  " + base + " = " + _get_qstring_methode(arg.get_type())
	      + "(args[\"" + arg.get_help() + "\"]);\n");
    }
    else if (arg.is_array()) {
      return ("  if (args.find(\"" + arg.get_help() + "\") != args.end()) {\n"
              "    " + base + " = " + _get_qstring_methode(arg.get_type())
	      + "(args[\"" + arg.get_help() + "\"]);\n"
              "  }\n");
    }
    else {
      QString varname(base);
      varname.replace(QRegExp("[->\\.]"), "_");
      return ("  " + arg.get_type() + " " + varname + ";\n"
              "  if (args.find(\"" + arg.get_help() + "\") != args.end()) {\n"
              "    " + varname + " = " + _get_qstring_methode(arg.get_type())
	      + "(args[\"" + arg.get_help() + "\"])" + ";\n"
	      + "    " + base + " = &" + varname + ";\n"
              "  }\n");
    }
  }

  QString ret;
  QList<argument> const& args = arg.get_args();
  for (QList<argument>::const_iterator it = args.begin(), end = args.end();
       it != end;
       ++it) {
    char const* accessor = (it->is_primitive() ? "" : "->");
    ret += _build_exec_struct(base + it->get_name() + accessor, *it);
  }
  return (ret);
}

/**
 *  Build the initialization deallocation.
 *
 *  @param[in] base The variable name of the struct.
 *  @param[in] arg  The argument to build init struct.
 *
 *  @return The deallocation struct.
 */
QString function::_build_exec_delete(QString const& base,
				     argument const& arg) {
  if (arg.is_primitive() == true) {
    return ("");
  }

  QString ret;
  QList<argument> const& args = arg.get_args();
  for (QList<argument>::const_iterator it = args.begin(), end = args.end();
       it != end;
       ++it) {

    if (it->is_primitive()) {
      ret += _build_exec_new(base + it->get_name(), *it);
    }
    else {
      ret += "  delete " + base + it->get_name() + ";\n";
      ret += _build_exec_new(base + it->get_name() + "->", *it);
    }
  }
  return (ret);
}

/**
 *  Change a QString to an other type with appropriate QString methode.
 *
 *  @param[in] type The variable type.
 *
 *  @return Return the QString methode name to translate the type.
 */
QString function::_get_qstring_methode(QString type) const {
  if (type == "std::vector<std::string>") {
    return ("toStdVector");
  }

  type.replace("time_t", "long long");
  type.replace("ULONG64", "unsigned long long");
  type.replace("bool", "int");
  type.replace("unsigned", "u").replace("::", " ");
  type = type.toLower();
  type = type.left(1).toUpper() + type.mid(1);

  QString ret("to");
  for (QString::const_iterator it = type.begin(), end = type.end();
       it != end;
       ++it) {
    if (it != type.begin() && *(it - 1) == ' ') {
      ret += it->toUpper();
    }
    else if (*it != ' ') {
      ret += *it;
    }
  }
  return (ret);
}

/**
 *  Cleanup the function name, replace uppercase by an '_' folow by the same
 *  character in lowercase.
 *
 *  @param[in] name The function name.
 *
 *  @return Return the new function name.
 */
QString function::_clean_function_name(QString const& name) {
  QString ret;

  for (QString::const_iterator it = name.begin(), end = name.end();
       it != end;
       ++it) {
    if (it->isUpper()) {
      ret += "_";
      ret += it->toLower();
    }
    else {
      ret += *it;
    }
  }

  return (ret);
}
