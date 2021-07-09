#!/bin/bash

go run deps.go "$*" > /tmp/deps.dot
dot -Tpng /tmp/deps.dot -o deps.png
if [ -x /usr/bin/eog ] ; then
  eog deps.png&
elif [ -x /usr/bin/lximage-qt ] ; then
  lximage-qt deps.png&
elif [ -x /usr/bin/lximage ] ; then
  lximage deps.png&
fi
