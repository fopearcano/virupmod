#!/bin/bash

apt-get update ; apt-get install -y clang-format-15 clang-tidy-15
mkdir -p build ; cd build
cmake ..
make clang-format
make clang-tidy
