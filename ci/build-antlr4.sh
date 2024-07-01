#!/bin/sh
set -ex

mkdir /tmp/antlr
cd /tmp/antlr

wget https://www.antlr.org/download/antlr-4.13.1-complete.jar
wget https://www.antlr.org/download/antlr4-cpp-runtime-4.13.1-source.zip

mkdir -p ./antlr4-cpp/build
unzip antlr4-cpp-runtime-4.13.1-source -d antlr4-cpp

cd ./antlr4-cpp/build
cmake .. -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DCMAKE_CXX_STANDARD=17 \
  -DANTLR4_INSTALL=ON \
  -DINSTALL_GTEST=OFF
make -j$(nproc)
make install -j$(nproc)
