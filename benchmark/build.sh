
RESET='\033[0m'
RED='\033[0;31m'
LGREEN='\033[0;32m'

if [[ $1 == "-f" ]] ; then
  force="rm"
elif [[ $1 == "-n" ]] ; then
  force="ignore"
fi

root_dir=$PWD
if [[ $(basename $root_dir) != "benchmark" ]] ; then
  echo "This script must be executed from the benchmark directory"
  exit 1
fi

if [[ -d log ]] ; then
  rm -rf log
fi
mkdir log

if [[ -d lib ]] ; then
  rm -rf lib
fi
mkdir -p lib/rw

if [[ -d "build" ]] ; then
  a=""
  if [[ $force == "rm" ]] ; then
    a="y"
  elif [[ $force == "ignore" ]] ; then
    a="n"
  fi
  while [[ $a != "y" && $a != "n" ]] ; do
    echo "Do you want to clean the build directory?"
    read a
  done
  if [[ $a == "y" ]] ; then
    rm -rf build
    mkdir build
  fi
else
  mkdir build
fi

cd build
conan remote add centreon https://api.bintray.com/conan/centreon/centreon
conan install --remote centreon ../..

cmake -GNinja -DWITH_SIMU=On -DWITH_RW_DIR=$root_dir/lib -DWITH_VAR_DIR=$root_dir/log ../..

echo -e "\n${LGREEN}Building Engine${RESET}"
ninja

cd $root_dir
echo -e "\n${LGREEN}Building Config${RESET}"
python3 ./build_conf.py conf.json

echo -e "\n${LGREEN}Launching Engine${RESET}"
#SIMUMOD=log/simumod.log build/centengine centreon-engine/centengine.cfg
build/centengine centreon-engine/centengine.cfg
