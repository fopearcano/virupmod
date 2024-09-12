#!/bin/bash

apt-get update ; apt-get install -y clang-format-18 clang-tidy-18
mkdir -p build ; cd build
cmake ..
make clang-format
make clang-tidy
