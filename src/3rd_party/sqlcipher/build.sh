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

JSON_C_VERSION=0.12.1-20160607
tar zxf ${CUR_DIR}/pkg/json-c-json-c-${JSON_C_VERSION}.tar.gz
cd json-c-json-c-${JSON_C_VERSION}
sed -i '/#undef malloc/d' config.h.in
sed -i '/#undef realloc/d' config.h.in
sed -i 's/-Werror //g' Makefile.am.inc
sed -i 's/-Werror //g' Makefile.in
libtoolize --force --copy
aclocal
autoconf
automake --add-missing --copy
ac_cv_func_malloc_0_nonnull=yes ac_cv_func_realloc_0_nonnull=yes ./configure --prefix=$DST --enable-shared=no --enable-static=yes ${CROSS_HOST_PARAM}
make -j4 
make install
cd ..
rm -rf json-c-json-c-${JSON_C_VERSION}

export SQLCIPHER_VERSION=4.2.0
tar zxf ${CUR_DIR}/pkg/sqlcipher-${SQLCIPHER_VERSION}.tar.gz
cd sqlcipher-${SQLCIPHER_VERSION}
if [ "$PLATFORM" = "arm" ]; then
  cp -f $CUR_DIR/../config.guess $CUR_DIR/../config.sub .
fi
CFLAGS="-DSQLITE_HAS_CODEC -I$DST/include $PARAM" \
LDFLAGS="-L$DST/lib/ $PARAM_LD -lssl -lcrypto -ldl -lrt -lpthread" \
LIBS="-ldl -lrt -lpthread" \
./configure \
  ${CROSS_HOST_PARAM} \
  --enable-tempstore=yes \
  --prefix=$DST $SHARED_STATIC_SWITCH
make
make install
cd ..
rm -rf sqlcipher-${SQLCIPHER_VERSION}

# vim: set expandtab ts=4 sw=4 sts=4:
