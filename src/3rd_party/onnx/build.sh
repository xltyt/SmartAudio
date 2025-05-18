#!/bin/bash

set -x
set -e

CUR_FILE=`readlink -f $0`
CUR_DIR=`dirname $CUR_FILE`
set -e
set -x

DST=$CUR_DIR/../../../${PLATFORM}/local
DST_C=$(echo $DST | sed 's/\//\\\//g')
CPU_COUNT=$(cat /proc/cpuinfo | grep "processor" | awk -F": " '{print $2}' | wc -l)
#CPU_COUNT=12

mkdir -p ${CUR_DIR}/../../../build
cd ${CUR_DIR}/../../../build

export PATH=$DST/bin:$PATH
export LD_LIBRARY_PATH=$DST/lib:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=$DST/lib/pkgconfig:$PKG_CONFIG_PATH
. $CUR_DIR/../build_conf.sh

cp -r $CUR_DIR/pkg/ onnx
cd onnx
cd cmake
mkdir build
cd build
if [ "$PLATFORM" = "arm" ]; then
  cmake \
  -Donnxruntime_USE_CUDA=False \
  -Donnxruntime_USE_OPENVINO=False \
  -Donnxruntime_DISABLE_ABSEIL=True \
  -Donnxruntime_RUN_ONNX_TESTS=False \
  -Donnxruntime_BUILD_UNIT_TESTS=False \
  -Donnxruntime_ENABLE_PYTHON=False \
  -Donnxruntime_USE_PREINSTALLED_PROTOBUF=OFF \
  -Donnxruntime_USE_PREINSTALLED_EIGEN=ON \
  -Deigen_SOURCE_PATH=$DST \
  -Donnxruntime_BUILD_SHARED_LIB=ON \
  -DCMAKE_C_FLAGS="-fPIC -lpthread -I$DST/include/eigen3" \
  -DCMAKE_CXX_FLAGS="-fPIC -lpthread -I$DST/include/eigen3" \
  -DCMAKE_CPP_FLAGS="-fPIC -lpthread -I$DST/include/eigen3" \
  -DCMAKE_EXE_LINKER_FLAGS="-static-libstdc++ -static-libgcc" \
  -DCMAKE_MODULE_LINKER_FLAGS="-static-libstdc++ -static-libgcc" \
  -DCMAKE_SHARED_LINKER_FLAGS="-static-libstdc++ -static-libgcc" \
  -DCMAKE_INSTALL_PREFIX=$DST/onnx/cpu \
  -DCMAKE_C_COMPILER=${CROSS_COMPILE}gcc \
  -DCMAKE_CXX_COMPILER=${CROSS_COMPILE}g++ \
  -DCMAKE_LINKER=${CROSS_COMPILE}ld \
  -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
  -DCMAKE_SYSTEM_NAME=Linux \
  ..
else
  cmake \
  -Donnxruntime_USE_CUDA=False \
  -Donnxruntime_USE_OPENVINO=False \
  -Donnxruntime_DISABLE_ABSEIL=True \
  -Donnxruntime_RUN_ONNX_TESTS=False \
  -Donnxruntime_BUILD_UNIT_TESTS=False \
  -Donnxruntime_ENABLE_PYTHON=False \
  -Donnxruntime_USE_PREINSTALLED_PROTOBUF=OFF \
  -Donnxruntime_USE_PREINSTALLED_EIGEN=ON \
  -Deigen_SOURCE_PATH=$DST \
  -Donnxruntime_BUILD_SHARED_LIB=ON \
  -DCMAKE_C_FLAGS="-fPIC -lpthread -I$DST/include/eigen3" \
  -DCMAKE_CXX_FLAGS="-fPIC -lpthread -I$DST/include/eigen3" \
  -DCMAKE_CPP_FLAGS="-fPIC -lpthread -I$DST/include/eigen3" \
  -DCMAKE_EXE_LINKER_FLAGS="-static-libstdc++ -static-libgcc" \
  -DCMAKE_MODULE_LINKER_FLAGS="-static-libstdc++ -static-libgcc" \
  -DCMAKE_SHARED_LINKER_FLAGS="-static-libstdc++ -static-libgcc" \
  -DCMAKE_INSTALL_PREFIX=$DST/onnx/cpu \
  ..
fi
make VERBOSE=1 -j${CPU_COUNT}
make install
cd ..
cd ..
cd ..
rm -rf onnx

# vim: set expandtab ts=4 sw=4 sts=4:
