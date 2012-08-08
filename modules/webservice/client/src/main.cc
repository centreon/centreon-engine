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

#include <limits.h>
#if defined(__GNU_LIBRARY__) || defined(__GLIBC__) // Version 5 and 6, respectively.
#  include <getopt.h>
#  define HAVE_GETOPT_LONG
#endif // glibc

#include <cstdlib>
#include <exception>
#include <iostream>
#include <map>
#include <signal.h>
#include <string>
#include "soapH.h"
#include "centreonengine.nsmap" // gSOAP namespaces.
#include "auto_gen.hh"
#include "com/centreon/engine/modules/webservice/webservice.hh"

using namespace com::centreon::engine::modules::webservice;

static char const* OPTIONS = "a:c:e:f:hk:lp:s";

/**
 *  This function show the application's usage and then quit the
 *  program.
 *
 *  @param[in] appname The application name.
 */
static void usage(char const* appname) {
  std::cout
    << "usage: " << appname
#ifdef WITH_OPENSSL
    << " [-a action] [-e end_point] [-h] [-l] [-s [-c cacert] [-k keyfile] [-p password]] [-f] function [arg ...]\n"
#else
    << " [-a action] [-e end_point] [-h] [-l] [-f] function [arg ...]\n"
#endif // !WITH_OPENSSL
    << " -a, --action    Override the default SOAP action.\n"
    << " -e, --end_point Override the default SOAP service location.\n"
    << " -f, --function  The function name to call.\n"
    << " -h, --help      This help.\n"
    << " -l, --list      List all prototype."
#ifdef WITH_OPENSSL
    << "\n"
    << " -c, --cacert    Optional cacert file to authenticate trusted certificates.\n"
    << " -d, --dh        DH file name or DH key lenght in bits.\n"
    << " -k, --keyfile   Required when server must authenticate to clients.\n"
    << " -p, --password  Password to read the key file.\n"
    << " -s, --ssl       Enable ssl."
#endif // !WITH_OPENSSL
    << std::endl;
  exit(EXIT_FAILURE);
  return ;
}

/**
 *  This function show all function prototypes.
 */
static void show_prototype() {
  auto_gen::instance().show_help();
  exit(EXIT_SUCCESS);
  return ;
}

/**
 *  Parse the command line options.
 *
 *  @param[out] args Options list that will be filled.
 *  @param[in]  argc Number of arguments.
 *  @param[in]  argv Arguments value.
 *
 *  @return The function arguments.
 */
static std::map<std::string, std::string> parse_option(
                                            std::map<char, std::string>& opt,
                                            int argc,
                                            char** argv) {
#ifdef HAVE_GETOPT_LONG
  static struct option const long_opt[] = {
    { "action",    required_argument, NULL, 'a' },
    { "end_point", required_argument, NULL, 'e' },
    { "function",  required_argument, NULL, 'f' },
    { "list",      no_argument,       NULL, 'l' },
#  ifdef WITH_OPENSSL
    { "keyfile",   required_argument, NULL, 'k' },
    { "cacert",    required_argument, NULL, 'c' },
    { "password",  required_argument, NULL, 'p' },
    { "ssl",       no_argument,       NULL, 's' },
#  endif // !WITH_OPENSSL
    { NULL,        0,                 NULL, 0 }
  };
#endif // !HAVE_GETOPT_LONG

  // By default, connect to localhost on port 4242.
  opt['e'] = "127.0.0.1:4242";
  // Without SSL.
  opt['s'] = "false";

  // Browse arguments.
  char c;
#ifdef HAVE_GETOPT_LONG
  while ((c = getopt_long(argc, argv, OPTIONS, long_opt, NULL)) != -1) {
#else
  while ((c = getopt(argc, argv, OPTIONS)) != -1) {
#endif /* !HAVE_GETOPT_LONG */
    switch (c) {
    case 'a':
    case 'e':
    case 'f':
#ifdef WITH_OPENSSL
    case 'c':
    case 'k':
    case 'p':
#endif // !WITH_OPENSSL
      opt[c] = optarg;
      break;

    case 'l':
      show_prototype();
      break;

#ifdef WITH_OPENSSL
    case 's':
      opt[c] = "true";
      break;
#endif // !WITH_OPENSSL

    default:
      usage(argv[0]);
    }
  }

  // Illogical arguments.
  if (optind == argc
      || (opt['s'] == "false"
          && (opt['c'] != ""
              || opt['d'] != ""
              || opt['k'] != ""
              || opt['p'] != "")))
    usage(argv[0]);

  // Function can be set by -f flag or be the first argument.
  if (opt['f'] == "")
    opt['f'] = argv[optind++];

  // Process remaining arguments.
  std::map<std::string, std::string> args;
  for (int i(optind); i < argc; ++i) {
    // Copy of argument.
    std::string tmp(argv[i]);

    // Arguments can be provided by 1) key=val or 2) key val.
    size_t pos(tmp.find('='));
    // 1)
    if (pos != std::string::npos) {
      std::string key(tmp.substr(0, pos));
      std::string val(tmp.substr(pos + 1));
      args[key] = val;
    }
    // 2)
    else if (i + 1 < argc)
      args[tmp] = argv[++i];
    // Error.
    else
      usage(argv[0]);
  }
  return (args);
}

int main(int argc, char** argv) {
  int ret = EXIT_SUCCESS;

  try {
    if (argc >= 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))) {
      if (argc == 2) {
        std::cout << "usage: " << argv[0] << std::endl;
        auto_gen::instance().show_help();
      }
      else {
        std::cout << "usage: " << argv[0] << " ";
        for (int i = 2; i < argc; ++i)
          auto_gen::instance().show_help(argv[i]);
      }
    }
    else {
      std::map<char, std::string> opt;
      std::map<std::string, std::string>
        args(parse_option(opt, argc, argv));

      webservice ws(opt['s'] == "true", opt['k'], opt['p'], opt['c']);
      ws.set_end_point(opt['e']);
      ws.set_action(opt['a']);

      ret = (ws.execute(opt['f'], args) == true ? EXIT_SUCCESS : EXIT_FAILURE);
    }
  }
  catch (std::exception const& e) {
    std::cerr << "error: " << e.what() << std::endl;
    return (EXIT_FAILURE);
  }
  catch (...) {
    std::cerr << "error: catch all." << std::endl;
    return (EXIT_FAILURE);
  }
  return (ret);
}
