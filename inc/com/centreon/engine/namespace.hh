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

#ifndef CCE_NAMESPACE_HH
#define CCE_NAMESPACE_HH

#ifdef CCE_BEGIN
#undef CCE_BEGIN
#endif  // CCE_BEGIN
#define CCE_BEGIN()    \
  namespace com {      \
  namespace centreon { \
  namespace engine {

#ifdef CCE_END
#undef CCE_END
#endif  // CCE_END
#define CCE_END() \
  }               \
  }               \
  }

#endif  // !CCE_NAMESPACE_HH
