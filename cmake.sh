mkdir build
cd build
CXXFLAGS="-Wall -Wextra" cmake -DWITH_PREFIX=/usr -DWITH_PREFIX_BIN=/usr/sbin -DWITH_USER=centreon-engine -DWITH_GROUP=centreon-engine -DCMAKE_BUILD_TYPE=Debug -DWITH_RW_DIR=/var/lib/centreon-engine/rw -DWITH_PREFIX_CONF=/etc/centreon-engine -DWITH_VAR_DIR=/var/log/centreon-engine -DWITH_PREFIX_LIB=/usr/lib64/centreon-engine -DWITH_TESTING=On -DWITH_SIMU=On $* -DWITH_CREATE_FILES=OFF ..

#CXX=/usr/bin/clang++ CC=/usr/bin/clang cmake -DWITH_PREFIX=/usr -DWITH_PREFIX_BIN=/usr/sbin -DWITH_USER=centreon-engine -DWITH_GROUP=centreon-engine -DCMAKE_BUILD_TYPE=Debug -DWITH_RW_DIR=/var/lib/centreon-engine/rw -DWITH_PREFIX_CONF=/etc/centreon-engine -DWITH_VAR_DIR=/var/log/centreon-engine -DWITH_PREFIX_LIB=/usr/lib64/centreon-engine -DWITH_TESTING=On -DWITH_SIMU=On .

#CXX=/usr/lib64/ccache/g++ CC=/usr/lib64/ccache/gcc cmake -DWITH_PREFIX=/usr -DWITH_PREFIX_BIN=/usr/sbin -DWITH_USER=centreon-engine -DWITH_GROUP=centreon-engine -DCMAKE_BUILD_TYPE=Debug -DWITH_RW_DIR=/var/lib/centreon-engine/rw -DWITH_PREFIX_CONF=/etc/centreon-engine -DWITH_VAR_DIR=/var/log/centreon-engine -DWITH_PREFIX_LIB=/usr/lib64/centreon-engine -DWITH_TESTING=On -DWITH_SIMU=On .

#CXXFLAGS="-O1 -fsanitize=address -fno-omit-frame-pointer" cmake -DWITH_PREFIX=/usr -DWITH_PREFIX_BIN=/usr/sbin -DWITH_USER=centreon-engine -DWITH_GROUP=centreon-engine -DCMAKE_BUILD_TYPE=Debug -DWITH_RW_DIR=/var/lib/centreon-engine/rw -DWITH_PREFIX_CONF=/etc/centreon-engine -DWITH_VAR_DIR=/var/log/centreon-engine -DWITH_PREFIX_LIB=/usr/lib64/centreon-engine -DWITH_TESTING=On -DWITH_SIMU=On .
