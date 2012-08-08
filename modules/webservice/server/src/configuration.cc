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

#include <sstream>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUni.hpp>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/modules/webservice/configuration.hh"

using namespace xercesc;
using namespace com::centreon::engine::modules::webservice;

/**
 *  Default Constructor.
 *
 *  @param[in] filename The configuration filename.
 */
configuration::configuration(std::string const& filename)
  : _accept_timeout(500),
    _filename(filename),
    _port(80),
    _recv_timeout(5),
    _send_timeout(5),
    _ssl_enable(false),
    _thread_count(1) {
  try {
    XMLPlatformUtils::Initialize();
  }
  catch (XMLException const& e) {
    char* err_str(XMLString::transcode(e.getMessage()));
    std::string err(err_str);
    XMLString::release(&err_str);
    throw (engine_error() << "configuration initialize failed: "
           << err);
  }

  _keytab["/webservice"] = NULL;
  _keytab["/webservice/thread_count"] = &configuration::_set_thread_count;
  _keytab["/webservice/host"] = &configuration::_set_host;
  _keytab["/webservice/port"] = &configuration::_set_port;
  _keytab["/webservice/recv_timeout"] = &configuration::_set_recv_timeout;
  _keytab["/webservice/send_timeout"] = &configuration::_set_send_timeout;
  _keytab["/webservice/accept_timeout"] = &configuration::_set_accept_timeout;
  _keytab["/webservice/ssl"] = NULL;
  _keytab["/webservice/ssl/enable"] = &configuration::_set_ssl_enable;
  _keytab["/webservice/ssl/keyfile"] = &configuration::_set_ssl_keyfile;
  _keytab["/webservice/ssl/cacert"] = &configuration::_set_ssl_cacert;
  _keytab["/webservice/ssl/dh"] = &configuration::_set_ssl_dh;
  _keytab["/webservice/ssl/password"] = &configuration::_set_ssl_password;
}

/**
 *  Default Destructor.
 */
configuration::~configuration() throw () {
  try {
    XMLPlatformUtils::Terminate();
  }
  catch (XMLException const& e) {
    (void)e;
  }
}

/**
 *  Get the accept socket timeout.
 *
 *  @return The timeout in micosecond.
 */
int configuration::get_accept_timeout() const throw () {
  return (_accept_timeout);
}

/**
 *  Get the configuration filename.
 *
 *  @return The configuration filename.
 */
std::string const& configuration::get_filename() const throw () {
  return (_filename);
}

/**
 *  Get the server host name.
 *
 *  @return The host name.
 */
std::string const& configuration::get_host() const throw () {
  return (_host);
}

/**
 *  Get the listening port.
 *
 *  @return The port.
 */
int configuration::get_port() const throw () {
  return (_port);
}

/**
 *  Get The receive socket timeout.
 *
 *  @return The timeout in second.
 */
int configuration::get_recv_timeout() const throw () {
  return (_recv_timeout);
}

/**
 *  Get The send socket timeout.
 *
 *  @return The timeout in second.
 */
int configuration::get_send_timeout() const throw () {
  return (_send_timeout);
}

/**
 *  Get the certificate path (for authentication).
 *
 *  @return The certificate path.
 */
std::string const& configuration::get_ssl_cacert() const throw () {
  return (_ssl_cacert);
}

/**
 *  Get the Diffie-Helman path (for authentication).
 *
 *  @return The Diffie-Helman path.
 */
std::string const& configuration::get_ssl_dh() const throw () {
  return (_ssl_dh);
}

/**
 *  Get if the ssl is enable.
 *
 *  @return true is enable, false otherwise.
 */
bool configuration::get_ssl_enable() const throw () {
  return (_ssl_enable);
}

/**
 *  Get the keyfile path (for authentication).
 *
 *  @return The keyfile path.
 */
std::string const& configuration::get_ssl_keyfile() const throw () {
  return (_ssl_keyfile);
}

/**
 *  Get the password (for authentication).
 *
 *  @return The password.
 */
std::string const& configuration::get_ssl_password() const throw () {
  return (_ssl_password);
}

/**
 *  Get the thread count for the thread pool size.
 *
 *  @return The thread count.
 */
unsigned int configuration::get_thread_count() const throw () {
  return (_thread_count);
}

/**
 *  Parse configuration file.
 */
void configuration::parse() {
  try {
    XercesDOMParser parser;

    parser.setValidationScheme(XercesDOMParser::Val_Never);
    parser.setDoNamespaces(false);
    parser.setDoSchema(false);
    parser.setLoadExternalDTD(false);

    parser.parse(_filename.c_str());

    DOMDocument* doc(parser.getDocument());
    if (!doc)
      throw (engine_error() << "parse configuration failed: "
             "document is empty");
    DOMElement* root(doc->getDocumentElement());
    if (!root)
      throw (engine_error() << "parse configuration failed: "
             "root is empty");
    _parse("", root);
  }
  catch (XMLException const& e) {
    char* err_str(XMLString::transcode(e.getMessage()));
    std::string err(err_str);
    XMLString::release(&err_str);
    throw (engine_error() << "parse configuration failed: "
           << err);
  }

}

/**
 *  Set the configuration filename.
 *
 *  @param[in] filename The configuration filename.
 */
void configuration::set_filename(std::string const& filename) {
  _filename = filename;
}

/**
 *  Internal parser.
 *
 *  @param[in] prefix  The string from the root to the value.
 *  @param[in] node    The node to parse.
 */
void configuration::_parse(
                      std::string const& prefix,
                      xercesc::DOMNode const* node) {
  if (!node)
    return;

  if (node->getNodeType() == DOMNode::ELEMENT_NODE) {
    XMLCh const* name(node->getNodeName());
    std::string new_prefix(prefix + "/");
    if (name) {
      char* name_str(XMLString::transcode(name));
      new_prefix += name_str;
      XMLString::release(&name_str);
    }
    for (DOMNode const* child(node->getFirstChild());
         child;
         child = child->getNextSibling())
      _parse(new_prefix, child);
  }
  else if (node->getNodeType() == DOMNode::TEXT_NODE) {
    XMLCh const* value_xml(node->getNodeValue());
    char* value_str(value_xml ? XMLString::transcode(value_xml) : NULL);
    if (value_str) {
      XMLString::trim(value_str);
      std::string value(value_str);
      XMLString::release(&value_str);

      if (!value.empty()) {
        typedef void (configuration::*pfunc)(std::string const&);
        std::map<std::string, pfunc>::const_iterator
          it(_keytab.find(prefix));
        if (it == _keytab.end())
          throw (engine_error() << "parse configuration failed: "
                 "invalid prefix: " << prefix);
        if (it->second)
          (this->*it->second)(value);
      }
    }
  }
}

/**
 *  Set the accept socket timeout.
 *
 *  @param[in] value  The value to set.
 */
void configuration::_set_accept_timeout(std::string const& value) {
  std::istringstream iss(value);
  if (!(iss >> _accept_timeout) || !iss.eof())
    throw (engine_error() << "parse configuration failed: "
           "invalid accept_timeout");
  _accept_timeout *= -1000;
}

/**
 *  Set the server host name.
 *
 *  @param[in] value  The value to set.
 */
void configuration::_set_host(std::string const& value) {
  _host = value;
}

/**
 *  Set the listening port.
 *
 *  @param[in] value  The value to set.
 */
void configuration::_set_port(std::string const& value) {
  std::istringstream iss(value);
  if (!(iss >> _port) || !iss.eof())
    throw (engine_error() << "parse configuration failed: "
           "invalid port");
}

/**
 *  Set the receive socket timeout.
 *
 *  @param[in] value  The value to set.
 */
void configuration::_set_recv_timeout(std::string const& value) {
  std::istringstream iss(value);
  if (!(iss >> _recv_timeout) || !iss.eof())
    throw (engine_error() << "parse configuration failed: "
           "invalid recv_timeout");
}

/**
 *  Set the send socket timeout.
 *
 *  @param[in] value  The value to set.
 */
void configuration::_set_send_timeout(std::string const& value) {
  std::istringstream iss(value);
  if (!(iss >> _send_timeout) || !iss.eof())
    throw (engine_error() << "parse configuration failed: "
           "invalid send_timeout");
}

/**
 *  Set the certificate path (for authentication).
 *
 *  @param[in] value  The value to set.
 */
void configuration::_set_ssl_cacert(std::string const& value) {
  _ssl_cacert = value;
}

/**
 *  Set the Diffie-Helman path (for authentication).
 *
 *  @param[in] value  The value to set.
 */
void configuration::_set_ssl_dh(std::string const& value) {
  _ssl_dh = value;
}

/**
 *  Set if ssl is enable.
 *
 *  @param[in] value  The value to set.
 */
void configuration::_set_ssl_enable(std::string const& value) {
  if (value == "true")
    _ssl_enable = true;
  else if (value == "false")
    _ssl_enable = false;
  else
    throw (engine_error() << "parse configuration failed: "
           "invalid ssl_enable");
}

/**
 *  Set the keyfile path (for authentication).
 *
 *  @param[in] value  The value to set.
 */
void configuration::_set_ssl_keyfile(std::string const& value) {
  _ssl_keyfile = value;
}

/**
 *  Set the password (for authentication).
 *
 *  @param[in] value  The value to set.
 */
void configuration::_set_ssl_password(std::string const& value) {
  _ssl_password = value;
}

/**
 *  Set the thread count.
 *
 *  @param[in] value  The value to set.
 */
void configuration::_set_thread_count(std::string const& value) {
  std::istringstream iss(value);
  if (!(iss >> _thread_count) || !iss.eof() || !_thread_count)
    throw (engine_error() << "parse configuration failed: "
           "invalid thread_count");
}
