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

#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QRegExp>
#include <QString>
#include <QTextStream>
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
builder::builder(QString const& header_src,
                 QString const& header_dst,
                 QString const& source_dst)
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
  QFile file(_header_src);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text) == false) {
    throw (error(file.errorString().toStdString().c_str()));
  }

  while (file.atEnd() == false) {
    QString data = file.readLine();
    if (function::is_valid(data) == false) {
      continue;
    }
    function func(data);
    func.build();
    _lst_function.push_back(func);
  }

  file.close();
}

/**
 *  Build auto_gen header and source code.
 */
void builder::build() {
  _build_header();
  _build_source();
}

/**
 *  Build auto_gen header.
 */
void builder::_build_header() {
  QFile file(_header_dst);
  if (file.open(QIODevice::WriteOnly
                | QIODevice::Text
                | QIODevice::Truncate) == false) {
    throw (error(file.errorString().toStdString().c_str()));
  }
  QTextStream stream(&file);
  stream << copyright << "\n";

  QFileInfo file_info(_header_src);
  stream << "#ifndef CCE_MOD_WS_CLIENT_AUTO_GEN_HH\n"
         << "# define CCE_MOD_WS_CLIENT_AUTO_GEN_HH\n\n"
         << "# include <QHash>\n"
         << "# include <QString>\n"
         << "# include \"" << file_info.fileName() << "\"\n\n"
         << "class auto_gen {\n"
         << "public:\n"
         << "  static auto_gen& instance();\n\n"
         << "  void show_help() const;\n"
         << "  void show_help(QString const& name) const;\n"
         << "  bool execute(QString const& name, soap* s, char const* end_point, char const* action, QHash<QString, QString> const& args) const;\n\n"
         << "private:\n"
         << "  auto_gen();\n"
         << "  auto_gen(auto_gen const& right);\n"
         << "  ~auto_gen() throw();\n\n"
         << "  auto_gen& operator=(auto_gen const& right);\n\n"
         << "  QHash<QString, void (*)()> _help;\n"
         << "  QHash<QString, bool (*)(soap*, char const*, char const*, QHash<QString, QString> const&)> _exec;\n"
         << "};\n\n"
         << "#endif // !CCE_MOD_WS_CLIENT_AUTO_GEN_HH\n";
}

/**
 *  Build auto_gen source code.
 */
void builder::_build_source() {
  QFile file(_source_dst);
  if (file.open(QIODevice::WriteOnly
                | QIODevice::Text
                | QIODevice::Truncate) == false) {
    throw (error(file.errorString().toStdString().c_str()));
  }
  QTextStream stream(&file);
  stream << copyright << "\n";

  QFileInfo file_info(_header_dst);
  stream << "#include <QStringList>\n"
         << "#include <ostream>\n"
         << "#include \"com/centreon/engine/modules/webservice/error.hh\"\n"
         << "#include \"" << file_info.fileName() << "\"\n\n"
         << "using namespace com::centreon::engine::modules::webservice;\n\n";

  stream << "static std::string toStdString(QString const& str) {\n"
         << "  return (str.toStdString());\n"
         << "}\n\n";

  stream << "static double toDouble(QString const& str) {\n"
         << "  return (str.toDouble());\n"
         << "}\n\n";

  stream << "static float toFloat(QString const& str) {\n"
         << "  return (str.toFloat());\n"
         << "}\n\n";

  stream << "static int toInt(QString const& str) {\n"
         << "  return (str.toInt());\n"
         << "}\n\n";

  stream << "static long long toLongLong(QString const& str) {\n"
         << "  return (str.toLongLong());\n"
         << "}\n\n";

  stream << "static unsigned int toUInt(QString const& str) {\n"
         << "  return (str.toUInt());\n"
         << "}\n\n";

  stream << "static unsigned long long toULongLong(QString const& str) {\n"
         << "  return (str.toULongLong());\n"
         << "}\n\n";

  stream << "static std::vector<std::string> toStdVector(QString const& str) {\n"
         << "  QStringList tmp(str.split(','));\n"
         << "  std::vector<std::string> tab;\n"
         << "  for (QStringList::const_iterator it = tmp.begin(), end = tmp.end();\n"
         << "      it != end;\n"
         << "      ++it)\n"
         << "    tab.push_back(it->toStdString());\n"
         << "  return (tab);\n"
         << "}\n\n";

  stream << "template <typename T>\n"
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

  QList<argument> args = arg_definition::instance().get_arguments();
  for (QList<argument>::const_iterator it = args.begin(), end = args.end();
       it != end;
       ++it) {
    if (it->is_primitive() == false) {
      stream << "static std::ostream& operator<<(std::ostream& os, "
             << it->get_type() << " const& cls) {\n"
             << _build_ostream_struct("cls.", *it)
             << "\n"
             << "  return (os);\n"
             << "}\n\n";
    }
  }

  for (QList<function>::const_iterator it = _lst_function.begin(), end = _lst_function.end();
       it != end;
       ++it) {
    stream << "static " << it->get_help_function() << "\n\n";
  }

  for (QList<function>::const_iterator it = _lst_function.begin(), end = _lst_function.end();
       it != end;
       ++it) {
    stream << "static " << it->get_exec_function() << "\n\n";
  }

  stream << "auto_gen& auto_gen::instance() {\n"
         << "  static auto_gen instance;\n"
         << "  return (instance);\n"
         << "}\n\n"
         << "void auto_gen::show_help() const {\n"
         << "  for (QHash<QString, void(*)()>::const_iterator it = _help.begin(), end = _help.end();\n"
         << "      it != end;\n"
         << "      ++it) {\n"
         << "    it.value()();\n"
         << "  }\n"
         << "}\n\n"
         << "void auto_gen::show_help(QString const& name) const {\n"
         << "  QHash<QString, void (*)()>::const_iterator it = _help.find(name);\n"
         << "  if (it == _help.end()) {\n"
         << "    throw (error(\"function not found.\"));\n"
         << "  }\n"
         << "  it.value()();\n"
         << "}\n\n"
         << "bool auto_gen::execute(QString const& name, soap* s, char const* end_point, char const* action, QHash<QString, QString> const& args) const {\n"
         << "  QHash<QString, bool (*)(soap*, char const*, char const*, QHash<QString, QString> const&)>::const_iterator it = _exec.find(name);\n"
         << "  if (it == _exec.end()) {\n"
         << "    throw (error(\"function not found.\"));\n"
         << "  }\n"
         << "  \n"
         << "  return (it.value()(s, end_point, action, args));\n"
         << "}\n\n"
         << "auto_gen::auto_gen() {\n";

  for (QList<function>::const_iterator it = _lst_function.begin(), end = _lst_function.end();
       it != end;
       ++it) {
    stream << "  _help[\"" << it->get_name() << "\"] = &" << it->get_help_name() << ";\n";
  }
  stream << "\n";
  for (QList<function>::const_iterator it = _lst_function.begin(), end = _lst_function.end();
       it != end;
       ++it) {
    stream << "  _exec[\"" << it->get_name() << "\"] = &" << it->get_exec_name() << ";\n";
  }

  stream << "}\n\n"
         << "auto_gen::~auto_gen() throw() {\n"
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
QString builder::_build_ostream_struct(
                   QString const& base,
                   argument const& arg) {
  if (arg.is_primitive() == true) {
    return ("  os << \"" + arg.get_help() + " = \" << " + base + " << std::endl;\n");
  }

  QString ret;
  QList<argument> const& args = arg.get_args();
  for (QList<argument>::const_iterator it = args.begin(), end = args.end();
       it != end;
       ++it) {
    char const* accessor = (it->is_primitive() ? "" : "->");
    ret += _build_ostream_struct(base + it->get_name() + accessor, *it);
  }
  return (ret);
}
