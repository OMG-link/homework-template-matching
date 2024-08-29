#!/bin/bash

CXXFLAGS="-O2"

if [[ "$1" == "-g" ]]; then
    CXXFLAGS="-g -O0 -fsanitize=address,undefined"
fi

set -e
set -x

g++ src/main.cpp -o template-matching -std=c++17 $CXXFLAGS -Wall -Wextra
