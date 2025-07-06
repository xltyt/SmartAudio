#!/bin/bash

CUR_FILE=`readlink -f $0`
CUR_DIR=`dirname $CUR_FILE`
set -e
set -x

DST=$CUR_DIR/../../../${PLATFORM}/local
DST_C=$(echo $DST | sed 's/\//\\\//g')
#CPU_COUNT=$(cat /proc/cpuinfo | grep "processor" | awk -F": " '{print $2}' | wc -l)
CPU_COUNT=4

mkdir -p ${CUR_DIR}/../../../build
cd ${CUR_DIR}/../../../build

export PATH=$DST/bin:$PATH
export LD_LIBRARY_PATH=$DST/lib:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=$DST/lib/pkgconfig:$PKG_CONFIG_PATH
. $CUR_DIR/../build_conf.sh

tar zxf ${CUR_DIR}/pkg/sqlcipher-3.4.2.tar.gz
cd sqlcipher-3.4.2
if [ "$PLATFORM" = "arm" ]; then
  cp -f $CUR_DIR/../config.guess $CUR_DIR/../config.sub .
fi
CFLAGS="-DSQLITE_HAS_CODEC -I$DST/include/ $PARAM" \
LDFLAGS="-L$DST/lib/ $PARAM_LD -lssl -lcrypto -ldl -lrt -lpthread" \
LIBS="-ldl -lrt -lpthread" \
./configure \
  ${CROSS_HOST_PARAM} \
  --enable-tempstore=yes \
  --prefix=$DST $SHARED_STATIC_SWITCH
make
make install
cd ..
rm -rf sqlcipher-3.4.2

# vim: set expandtab ts=4 sw=4 sts=4:
