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

#include "configuration/applier/base.hh"

using namespace com::centreon::engine::configuration::applier;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Default constructor.
 */
base::base() {}

/**
 *  Copy constructor.
 *
 *  @param[in] b Object to copy.
 */
base::base(base const& b) {
  (void)b;
}

/**
 *  Destructor.
 */
base::~base() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] b Object to copy.
 *
 *  @return This object.
 */
base& base::operator=(base const& b) {
  (void)b;
  return (*this);
}
