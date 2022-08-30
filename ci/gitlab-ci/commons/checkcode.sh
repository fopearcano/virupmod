#!/bin/bash

apt-get update ; apt-get install -y clang-format-14 clang-tidy-14
mkdir -p build ; cd build
cmake ..
make clang-format
make clang-tidy
