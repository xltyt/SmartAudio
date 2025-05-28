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

# CUDNN Need < 9.0
export ONNX_VERSION=1.16.3

cp -r $CUR_DIR/pkg/ onnx_${ONNX_VERSION}
cd onnx_${ONNX_VERSION}
cd cmake

if [ ! -d $DST/onnx/cpu/lib ]; then
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
  rm -rf build
fi

CUDA_VERSIONS=(12.2)
for CUDA_VERSION in "${CUDA_VERSIONS[@]}"; do
  if [ ! -d $DST/onnx/gpu/${CUDA_VERSION}/lib ]; then
    mkdir -p build_${CUDA_VERSION}
    cd build_${CUDA_VERSION}
    if [ "$PLATFORM" = "arm" ]; then
      echo "Arm"
    else
      cmake \
      -Donnxruntime_USE_CUDA=True \
      -DCUDA_BIN_PATH=/usr/local/cuda-${CUDA_VERSION}/bin \
      -DCUDA_HOME=/usr/local/cuda-${CUDA_VERSION}/ \
      -DCUDA_TOOLKIT_ROOT_DIR=/usr/local/cuda-${CUDA_VERSION}/ \
      -DCMAKE_CUDA_COMPILER=/usr/local/cuda-${CUDA_VERSION}/bin/nvcc \
      -DCMAKE_CUDA_ARCHITECTURES="80;86;89;90" \
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
      -DCMAKE_INSTALL_PREFIX=$DST/onnx/gpu/${CUDA_VERSION} \
      ..
    fi
    make VERBOSE=1 -j2
    make install
    cd ..
    rm -rf build_${CUDA_VERSION}
  fi
done

cd ..
cd ..
rm -rf onnx_${ONNX_VERSION}

# vim: set expandtab ts=4 sw=4 sts=4:
