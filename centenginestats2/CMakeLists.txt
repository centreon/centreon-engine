##
## Copyright 2020 Centreon
##
## This file is part of Centreon Engine.
##
## Centreon Engine is free software: you can redistribute it and/or
## modify it under the terms of the GNU General Public License version 2
## as published by the Free Software Foundation.
##
## Centreon Engine is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Centreon Engine. If not, see
## <http://www.gnu.org/licenses/>.
##

set(stats_src
    centenginestats_client.cc
    main.cc

    centenginestats_client.hh
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_executable(centenginestats2 ${stats_src})
target_link_libraries(centenginestats2 cerpc ${grpc_LIBS} ${OpenSSL_LIBS} ${c-ares_LIBS} ${ZLIB_LIBS} dl)
