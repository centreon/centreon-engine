/*
** Copyright 1999-2010 Ethan Galstad
** Copyright 2011-2013 Merethis
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

#ifndef CCE_COMPATIBILITY_COMMENTS_H
#define CCE_COMPATIBILITY_COMMENTS_H

#include "com/centreon/engine/objects/comment.hh"
#include "common.h"
#include "config.h"
#include "objects.h"

/*
** If you are going to be adding a lot of comments in sequence,
** set defer_comment_sorting to 1 before you start and then
** call sort_comments afterwards. Things will go MUCH faster.
*/
extern int defer_comment_sorting;

#endif  // !CCE_COMPATIBILITY_COMMENTS_H
