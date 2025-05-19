#!/bin/bash

CUR_FILE=`readlink -f $0`
CUR_DIR=`dirname $CUR_FILE`
set -e
set -x

DST=$CUR_DIR/../../../${PLATFORM}/local
DST_C=$(echo $DST | sed 's/\//\\\//g')
CPU_COUNT=$(cat /proc/cpuinfo | grep "processor" | awk -F": " '{print $2}' | wc -l)

mkdir -p ${CUR_DIR}/../../../build
cd ${CUR_DIR}/../../../build

export PATH=$DST/bin:$PATH
export LD_LIBRARY_PATH=$DST/lib:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=$DST/lib/pkgconfig:$PKG_CONFIG_PATH
. $CUR_DIR/../build_conf.sh

cp -r $CUR_DIR/pkg kaldi
cd kaldi
sed -i '16,22s/^/#/' CMakeLists.txt
mkdir build
cd build
if [ "$PLATFORM" = "arm" ]; then
  cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_LIBDIR=$DST/lib \
    -DCMAKE_INSTALL_PREFIX=$DST/ \
    -DCMAKE_PREFIX_PATH=$DST \
    -DBUILD_SHARED_LIBS=OFF \
    -D_BUILD_PYTHON=OFF \
    -D_BUILD_TESTS=OFF \
    -DCMAKE_C_COMPILER=${CROSS_COMPILE}gcc \
    -DCMAKE_CXX_COMPILER=${CROSS_COMPILE}g++ \
    -DCMAKE_LINKER=${CROSS_COMPILE}ld \
    -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
    -DCMAKE_SYSTEM_NAME=Linux \
    ../
else
  cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_LIBDIR=$DST/lib \
    -DCMAKE_INSTALL_PREFIX=$DST/ \
    -DCMAKE_PREFIX_PATH=$DST \
    -DBUILD_SHARED_LIBS=OFF \
    -D_BUILD_PYTHON=OFF \
    -D_BUILD_TESTS=OFF \
    ../
fi
make -j${CPU_COUNT}
make install
cp -f lib/libkissfft-float.a ${DST}/lib/
cd ..
cd ..
rm -rf kaldi

# vim: set expandtab ts=4 sw=4 sts=4:
