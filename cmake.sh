#!/bin/bash

show_help() {
cat << EOF
Usage: ${0##*/} -n=[yes|no] -v

This program build Centreon-engine

    -f|--force    : force rebuild
    -r|--release  : Build on release mode
    -h|--help     : help
EOF
}
BUILD_TYPE="Debug"
for i in "$@"
do
  case $i in
    -f|--force)
      force=1
      shift
      ;;
    -r|--release)
      BUILD_TYPE="Release"
      ;;
    -h|--help)
      show_help
      exit 2
      ;;
    *)
            # unknown option
    ;;
  esac
done

# Am I root?
my_id=$(id -u)

if [ -r /etc/centos-release ] ; then
  maj="centos$(cat /etc/centos-release | awk '{print $4}' | cut -f1 -d'.')"
  v=$(cmake --version)
  if [[ $v =~ "version 3" ]] ; then
    cmake='cmake'
  else
    if rpm -q cmake3 ; then
      cmake='cmake3'
    elif [ $maj = "centos7" ] ; then
      yum -y install epel-release
      yum -y install cmake3
      cmake='cmake3'
    else
      dnf -y install cmake
      cmake='cmake'
    fi
  fi
  if [[ ! -x /usr/bin/python3 ]] ; then
    yum -y install python3
  else
    echo "python3 already installed"
  fi
  if ! rpm -q python3-pip ; then
    yum -y install python3-pip
  else
    echo "pip3 already installed"
  fi

  if ! rpm -q gcc-c++ ; then
    yum -y install gcc-c++
  fi

  pkgs=(
    ninja-build
  )
  for i in "${pkgs[@]}"; do
    if ! rpm -q $i ; then
      if [ $maj = 'centos7' ] ; then
        yum install -y $i
      else
        dnf -y --enablerepo=PowerTools install $i
      fi
    fi
  done
elif [ -r /etc/issue ] ; then
  maj=$(cat /etc/issue | awk '{print $1}')
  version=$(cat /etc/issue | awk '{print $3}')
  if [ $version = "9" ] ; then
    dpkg='dpkg'
  else
    dpkg='dpkg --no-pager'
  fi
  v=$(cmake --version)
  if [[ $v =~ "version 3" ]] ; then
    cmake='cmake'
  elif [ $maj = "Debian" ] ; then
    if $dpkg -l cmake ; then
      echo "Bad version of cmake..."
      exit 1
    else
      if [ $my_id -eq 0 ] ; then
        apt install -y cmake
        cmake='cmake'
      else
        echo -e "cmake is not installed, you could enter, as root:\n\tapt install -y cmake\n\n"
        exit 1
      fi
    fi
  elif [ $maj = "Raspbian" ] ; then
    if $dpkg -l cmake ; then
      echo "Bad version of cmake..."
      exit 1
    else
      if [ $my_id -eq 0 ] ; then
        apt install -y cmake
        cmake='cmake'
      else
        echo -e "cmake is not installed, you could enter, as root:\n\tapt install -y cmake\n\n"
        exit 1
      fi
    fi
  else
    echo "Bad version of cmake..."
    exit 1
  fi
  if [ $maj = "Debian" ] ; then
    pkgs=(
      gcc
      g++
      pkg-config
      ninja-build
      python3
      python3-pip
    )
    for i in "${pkgs[@]}"; do
      if ! $dpkg -l $i | grep "^ii" ; then
        if [ $my_id -eq 0 ] ; then
          apt install -y $i
        else
          echo -e "The package \"$i\" is not installed, you can install it, as root, with the command:\n\tapt install -y $i\n\n"
          exit 1
        fi
      fi
    done
  fi
  if [ $maj = "Raspbian" ] ; then
    pkgs=(
      gcc
      g++
      pkg-config
      ninja-build
      python3
      python3-pip
    )
    for i in "${pkgs[@]}"; do
      if ! $dpkg -l $i | grep "^ii" ; then
        if [ $my_id -eq 0 ] ; then
          apt install -y $i
        else
          echo -e "The package \"$i\" is not installed, you can install it, as root, with the command:\n\tapt install -y $i\n\n"
          exit 1
        fi
      fi
    done
  fi
fi

pip3 install conan --upgrade

if [ $my_id -eq 0 ] ; then
  conan='conan'
else
  conan="$HOME/.local/bin/conan"
fi
if ! $conan remote list | grep ^centreon ; then
  $conan remote add centreon https://api.bintray.com/conan/centreon/centreon
fi
if ! $conan remote list | grep ^centreon ; then
  $conan remote add centreon https://api.bintray.com/conan/centreon/centreon
fi

good=$(gcc --version | awk '/gcc/ && ($3+0)>5.0{print 1}')

if [ ! -d build ] ; then
  mkdir build
else
  echo "'build' directory already there"
fi

if [ "$force" = "1" ] ; then
  rm -rf build
  mkdir build
fi
cd build

if [ $good -eq 1 ] ; then
  $conan install .. --remote centreon -s compiler.libcxx=libstdc++11
else
  $conan install .. --remote centreon -s compiler.libcxx=libstdc++
fi

if [ $maj = "Raspbian" ] ; then
  CXXFLAGS="-Wall -Wextra" $cmake -DWITH_PREFIX=/usr -DWITH_CENTREON_CLIB_LIBRARY_DIR=/usr/lib -DWITH_PREFIX_BIN=/usr/sbin -DWITH_USER=centreon-engine -DWITH_GROUP=centreon-engine -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DWITH_RW_DIR=/var/lib/centreon-engine/rw -DWITH_PREFIX_CONF=/etc/centreon-engine -DWITH_VAR_DIR=/var/log/centreon-engine -DWITH_PREFIX_LIB=/usr/lib64/centreon-engine -DWITH_TESTING=On -DWITH_SIMU=On $* -DWITH_CREATE_FILES=OFF -DWITH_BENCH=On ..
elif [ $maj = "Debian" ] ; then
  CXXFLAGS="-Wall -Wextra" $cmake -DWITH_PREFIX=/usr -DWITH_CENTREON_CLIB_LIBRARY_DIR=/usr/lib64 -DWITH_PREFIX_BIN=/usr/sbin -DWITH_USER=centreon-engine -DWITH_GROUP=centreon-engine -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DWITH_RW_DIR=/var/lib/centreon-engine/rw -DWITH_PREFIX_CONF=/etc/centreon-engine -DWITH_VAR_DIR=/var/log/centreon-engine -DWITH_PREFIX_LIB=/usr/lib64/centreon-engine -DWITH_TESTING=On -DWITH_SIMU=On $* -DWITH_CREATE_FILES=OFF -DWITH_BENCH=On ..
else
  CXXFLAGS="-Wall -Wextra" $cmake -DWITH_PREFIX=/usr -DWITH_CENTREON_CLIB_LIBRARY_DIR=/usr/lib64 -DWITH_PREFIX_BIN=/usr/sbin -DWITH_USER=centreon-engine -DWITH_GROUP=centreon-engine -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DWITH_RW_DIR=/var/lib/centreon-engine/rw -DWITH_PREFIX_CONF=/etc/centreon-engine -DWITH_VAR_DIR=/var/log/centreon-engine -DWITH_PREFIX_LIB=/usr/lib64/centreon-engine -DWITH_TESTING=On -DWITH_SIMU=On $* -DWITH_CREATE_FILES=OFF -DWITH_BENCH=On ..
fi
#CXX=/usr/bin/clang++ CC=/usr/bin/clang cmake -DWITH_PREFIX=/usr -DWITH_PREFIX_BIN=/usr/sbin -DWITH_USER=centreon-engine -DWITH_GROUP=centreon-engine -DCMAKE_BUILD_TYPE=Debug -DWITH_RW_DIR=/var/lib/centreon-engine/rw -DWITH_PREFIX_CONF=/etc/centreon-engine -DWITH_VAR_DIR=/var/log/centreon-engine -DWITH_PREFIX_LIB=/usr/lib64/centreon-engine -DWITH_TESTING=On -DWITH_SIMU=On .

#CXX=/usr/lib64/ccache/g++ CC=/usr/lib64/ccache/gcc cmake -DWITH_PREFIX=/usr -DWITH_PREFIX_BIN=/usr/sbin -DWITH_USER=centreon-engine -DWITH_GROUP=centreon-engine -DCMAKE_BUILD_TYPE=Debug -DWITH_RW_DIR=/var/lib/centreon-engine/rw -DWITH_PREFIX_CONF=/etc/centreon-engine -DWITH_VAR_DIR=/var/log/centreon-engine -DWITH_PREFIX_LIB=/usr/lib64/centreon-engine -DWITH_TESTING=On -DWITH_SIMU=On .

#CXXFLAGS="-O1 -fsanitize=address -fno-omit-frame-pointer" cmake -DWITH_PREFIX=/usr -DWITH_PREFIX_BIN=/usr/sbin -DWITH_USER=centreon-engine -DWITH_GROUP=centreon-engine -DCMAKE_BUILD_TYPE=Debug -DWITH_RW_DIR=/var/lib/centreon-engine/rw -DWITH_PREFIX_CONF=/etc/centreon-engine -DWITH_VAR_DIR=/var/log/centreon-engine -DWITH_PREFIX_LIB=/usr/lib64/centreon-engine -DWITH_TESTING=On -DWITH_SIMU=On .
