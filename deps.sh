#!/bin/bash

go run deps.go "$*" > /tmp/deps.dot
dot -Tpng /tmp/deps.dot -o deps.png
lximage-qt deps.png&

